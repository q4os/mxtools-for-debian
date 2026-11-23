/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023-2026 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include "webview.h"
#include "mainwindow.h"
#include "historystore.h"

#include <algorithm>

#include <QApplication>
#include <QAuthenticator>
#include <QBuffer>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDataStream>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QPointer>
#include <QTimer>
#include <QWebEngineCertificateError>
#include <QWebEngineContextMenuRequest>
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QWebEnginePermission>
#endif
#include <QWebEngineProfile>
#include <QWebEngineScript>

#include <qt6keychain/keychain.h>

namespace
{
constexpr char keychainService[] {"mx-viewer"};
} // namespace

// Static member definitions
bool WebView::s_ctrlHeld = false;
bool WebView::s_middleClick = false;
bool WebView::s_consumed = false;
qint64 WebView::s_clickTime = 0;

WebPage::WebPage(QWebEngineProfile *profile, WebView *parent)
    : QWebEnginePage(profile, parent),
      m_webView(parent)
{
    connect(this, &QWebEnginePage::fullScreenRequested, this, [this](QWebEngineFullScreenRequest request) {
        auto *mw = qobject_cast<MainWindow *>(m_webView->window());
        if (!mw) {
            request.reject();
            return;
        }
        mw->handleFullScreenRequest(std::move(request), m_webView);
    });
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    connect(this, &QWebEnginePage::permissionRequested, this, &WebPage::handlePermissionRequest);
#else
    connect(this, &QWebEnginePage::featurePermissionRequested, this, &WebPage::handleFeaturePermissionRequest);
#endif
    connect(this, &QWebEnginePage::certificateError, this, &WebPage::handleCertificateError);
    connect(this, &QWebEnginePage::authenticationRequired, this,
            [this](const QUrl &requestUrl, QAuthenticator *auth) {
                const QString origin = requestUrl.adjusted(QUrl::RemoveUserInfo | QUrl::RemovePath | QUrl::RemoveQuery
                                                           | QUrl::RemoveFragment)
                                           .toString();
                askCredentials(tr("%1 requires a user name and password.").arg(requestUrl.host()),
                               QStringLiteral("site %1 %2").arg(origin, auth->realm()), auth);
            });
    connect(this, &QWebEnginePage::proxyAuthenticationRequired, this,
            [this](const QUrl &, QAuthenticator *auth, const QString &proxyHost) {
                // The signal gives no port, so proxies on one host with the same realm share an entry.
                askCredentials(tr("The proxy %1 requires a user name and password.").arg(proxyHost),
                               QStringLiteral("proxy %1 %2").arg(proxyHost, auth->realm()), auth);
            });
}

// The authenticator must be filled in before the signal returns, so this dialog has to be modal.
// Passwords are saved only in the system keyring (gnome-keyring, KWallet, KeePassXC), never in a file.
void WebPage::askCredentials(const QString &message, const QString &keychainKey, QAuthenticator *auth)
{
    // Heap-allocated and guarded: if the tab is closed during exec(), the view deletes the dialog
    // and the authenticator must not be touched.
    QPointer<QDialog> dialog = new QDialog(m_webView);
    dialog->setWindowTitle(tr("Authentication required"));
    auto *layout = new QFormLayout(dialog);
    auto *label = new QLabel(message, dialog);
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    layout->addRow(label);
    if (!auth->realm().isEmpty()) {
        auto *realm = new QLabel(tr("Realm: %1").arg(auth->realm()), dialog);
        realm->setTextFormat(Qt::PlainText);
        realm->setWordWrap(true);
        layout->addRow(realm);
    }
    auto *user = new QLineEdit(auth->user(), dialog);
    auto *password = new QLineEdit(dialog);
    password->setEchoMode(QLineEdit::Password);
    layout->addRow(tr("User name:"), user);
    layout->addRow(tr("Password:"), password);
    // Nothing is saved from a private window.
    const bool privateWindow = profile()->isOffTheRecord();
#ifdef HAVE_KEYCHAIN_IS_AVAILABLE
    const bool canRemember = !privateWindow && QKeychain::isAvailable();
#else
    // Older qtkeychain cannot tell beforehand; a missing keyring just makes the jobs fail.
    const bool canRemember = !privateWindow;
#endif
    QCheckBox *remember {nullptr};
    if (!privateWindow) {
        remember = new QCheckBox(tr("Remember password"), dialog);
        // Enabled once the keyring has answered, so unticking always knows whether there is an entry to delete.
        remember->setEnabled(false);
        if (!canRemember) {
            remember->setToolTip(tr("Install a password manager such as gnome-keyring or KeePassXC to save passwords."));
        }
        layout->addRow(remember);
    }
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    layout->addRow(buttons);
    // A saved password only fills in the dialog: sending it without asking would loop on a wrong one.
    // The lambda captures locals by reference; this is safe only because the dialog, its context object,
    // is deleted before this function returns.
    bool wasSaved = false;
    QString savedUser;
    QString savedPassword;
    if (canRemember) {
        auto *read = new QKeychain::ReadPasswordJob(keychainService);
        read->setKey(keychainKey);
        connect(read, &QKeychain::Job::finished, dialog,
                [read, user, password, remember, &wasSaved, &savedUser, &savedPassword] {
                    if (read->error() == QKeychain::EntryNotFound) {
                        remember->setEnabled(true);
                        return;
                    }
                    // Not even a lookup worked (no keyring running, unlock refused), so saving would fail too.
                    if (read->error() != QKeychain::NoError) {
                        remember->setToolTip(tr("Passwords cannot be saved: %1").arg(read->errorString()));
                        return;
                    }
                    remember->setEnabled(true);
                    QDataStream stream(read->binaryData());
                    stream.setVersion(QDataStream::Qt_6_0);
                    stream >> savedUser >> savedPassword;
                    if (stream.status() != QDataStream::Ok) {
                        savedUser.clear();
                        savedPassword.clear();
                        return;
                    }
                    wasSaved = true;
                    // Ticked even when not filled in, so OK does not delete an entry the user never unticked.
                    remember->setChecked(true);
                    // Never overwrite what the user typed while the keyring was answering.
                    if (!user->isModified() && !password->isModified()) {
                        user->setText(savedUser);
                        password->setText(savedPassword);
                    }
                });
        read->start();
    }
    const int result = dialog->exec();
    if (!dialog) {
        return;
    }
    if (result == QDialog::Accepted) {
        auth->setUser(user->text());
        auth->setPassword(password->text());
        const bool keep = remember && remember->isChecked();
        const bool changed = user->text() != savedUser || password->text() != savedPassword;
        // Typing before the keyring answered leaves the saved password unfilled; an empty one is not a new password.
        if (keep && (!wasSaved || (changed && !password->text().isEmpty()))) {
            QByteArray data;
            QDataStream stream(&data, QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_6_0);
            stream << user->text() << password->text();
            auto *write = new QKeychain::WritePasswordJob(keychainService);
            write->setKey(keychainKey);
            write->setBinaryData(data);
            connect(write, &QKeychain::Job::finished, write, [write] {
                if (write->error() != QKeychain::NoError) {
                    qWarning() << "Could not save the password:" << write->errorString();
                }
            });
            write->start();
        } else if (!keep && wasSaved) {
            auto *remove = new QKeychain::DeletePasswordJob(keychainService);
            remove->setKey(keychainKey);
            connect(remove, &QKeychain::Job::finished, remove, [remove] {
                if (remove->error() != QKeychain::NoError) {
                    qWarning() << "Could not delete the saved password:" << remove->errorString();
                }
            });
            remove->start();
        }
    } else {
        // A null authenticator cancels the request.
        *auth = QAuthenticator();
    }
    delete dialog;
}

void WebPage::handleCertificateError(QWebEngineCertificateError error)
{
    // Only offer an override for the page itself; broken subresources are just blocked.
    // Chromium remembers an accepted certificate for the rest of the session.
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    const bool mainFrame = error.isMainFrame();
#else
    // Older Qt does not say which frame failed; an error for the host being loaded is taken for the page.
    const bool mainFrame = error.url().host() == requestedUrl().host();
#endif
    if (!error.isOverridable() || !mainFrame) {
        error.rejectCertificate();
        return;
    }
    error.defer();
    auto *box = new QMessageBox(QMessageBox::Warning, tr("Certificate error"),
                                tr("The connection to %1 is not secure.").arg(error.url().host()),
                                QMessageBox::NoButton, m_webView);
    box->setTextFormat(Qt::PlainText);
    box->setInformativeText(error.description() + "\n\n"
                            + tr("Someone could be trying to impersonate the site or intercept your data."));
    auto *back = box->addButton(tr("Go back"), QMessageBox::RejectRole);
    auto *proceed = box->addButton(tr("Proceed anyway (unsafe)"), QMessageBox::DestructiveRole);
    box->setDefaultButton(back);
    box->setEscapeButton(back);
    box->setAttribute(Qt::WA_DeleteOnClose);
    connect(box, &QMessageBox::finished, this, [box, proceed, error]() mutable {
        if (box->clickedButton() == proceed) {
            error.acceptCertificate();
        } else {
            error.rejectCertificate();
        }
    });
    box->open();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
QString WebPage::permissionDescription(QWebEnginePermission::PermissionType type)
{
    using Type = QWebEnginePermission::PermissionType;
    switch (type) {
    case Type::MediaAudioCapture:
        return tr("use your microphone");
    case Type::MediaVideoCapture:
        return tr("use your camera");
    case Type::MediaAudioVideoCapture:
        return tr("use your camera and microphone");
    case Type::DesktopVideoCapture:
        return tr("share your screen");
    case Type::DesktopAudioVideoCapture:
        return tr("share your screen and audio");
    case Type::MouseLock:
        return tr("lock your mouse pointer");
    case Type::Geolocation:
        return tr("know your location");
    case Type::ClipboardReadWrite:
        return tr("read and write your clipboard");
    case Type::LocalFontsAccess:
        return tr("access the fonts installed on your computer");
    case Type::Notifications:
    case Type::Unsupported:
        break;
    }
    return {};
}

void WebPage::handlePermissionRequest(QWebEnginePermission permission)
{
    // No notification presenter is installed, so granting would have no visible effect;
    // decline quietly instead of prompting.
    const QString what = permissionDescription(permission.permissionType());
    if (what.isEmpty()) {
        permission.deny();
        return;
    }
    // The profile stores the answer on disk for persistent permission types, so a site is asked only once.
    askPermission(permission.origin(), what, [permission](bool allowed) {
        if (allowed) {
            permission.grant();
        } else {
            permission.deny();
        }
    });
}
#else
QString WebPage::permissionDescription(QWebEnginePage::Feature feature)
{
    switch (feature) {
    case MediaAudioCapture:
        return tr("use your microphone");
    case MediaVideoCapture:
        return tr("use your camera");
    case MediaAudioVideoCapture:
        return tr("use your camera and microphone");
    case DesktopVideoCapture:
        return tr("share your screen");
    case DesktopAudioVideoCapture:
        return tr("share your screen and audio");
    case MouseLock:
        return tr("lock your mouse pointer");
    case Geolocation:
        return tr("know your location");
    default:
        break;
    }
    return {};
}

void WebPage::handleFeaturePermissionRequest(const QUrl &origin, QWebEnginePage::Feature feature)
{
    // Notifications and unknown features are declined quietly, as with the newer permission API.
    const QString what = permissionDescription(feature);
    if (what.isEmpty()) {
        setFeaturePermission(origin, feature, PermissionDeniedByUser);
        return;
    }
    QPointer<WebPage> self = this;
    askPermission(origin, what, [self, origin, feature](bool allowed) {
        if (self) {
            self->setFeaturePermission(origin, feature, allowed ? PermissionGrantedByUser : PermissionDeniedByUser);
        }
    });
}
#endif

void WebPage::askPermission(const QUrl &origin, const QString &what, const std::function<void(bool)> &answer)
{
    const QString host = origin.host().isEmpty() ? origin.toDisplayString() : origin.host();
    auto *box = new QMessageBox(QMessageBox::Question, tr("Permission request"), tr("%1 wants to %2.").arg(host, what),
                                QMessageBox::NoButton, m_webView);
    box->setTextFormat(Qt::PlainText);
    box->addButton(tr("Allow"), QMessageBox::AcceptRole);
    auto *block = box->addButton(tr("Block"), QMessageBox::RejectRole);
    box->setDefaultButton(block);
    box->setAttribute(Qt::WA_DeleteOnClose);
    connect(box, &QMessageBox::finished, this, [box, block, answer] {
        answer(box->clickedButton() && box->clickedButton() != block);
    });
    box->open();
}

void WebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber,
                                       const QString &sourceID)
{
    Q_UNUSED(level);
    Q_UNUSED(message);
    Q_UNUSED(lineNumber);
    Q_UNUSED(sourceID);
}

bool WebPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(isMainFrame);
    // Internal actions may only come from the internal page itself or from the address bar,
    // never from a remote page navigating to an mx-history:// or mx-settings:// URL.
    const bool internalScheme = url.scheme() == "mx-history" || url.scheme() == "mx-settings";
    if (internalScheme && type != NavigationTypeTyped && this->url().scheme() != url.scheme()) {
        return false;
    }
    if (url.scheme() == "mx-history") {
        auto *mw = qobject_cast<MainWindow *>(m_webView->window());
        if (!mw) {
            mw = qobject_cast<MainWindow *>(QApplication::activeWindow());
        }
        if (mw && mw->handleHistoryRequest(url)) {
            return false;
        }
    }
    if (url.scheme() == "mx-settings") {
        auto *mw = qobject_cast<MainWindow *>(m_webView->window());
        if (!mw) {
            mw = qobject_cast<MainWindow *>(QApplication::activeWindow());
        }
        if (mw && mw->handleSettingsRequest(url)) {
            return false;
        }
    }
    // Handle Ctrl+click / middle-click on regular links
    if (type == NavigationTypeLinkClicked && WebView::consumeIfNewTabRequest()) {
        auto *mw = qobject_cast<MainWindow *>(m_webView->window());
        if (!mw) {
            mw = qobject_cast<MainWindow *>(QApplication::activeWindow());
        }
        if (mw) {
            mw->openLinkInNewTab(url, m_webView);
        }
        return false;
    }
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

WebView::WebView(QWebEngineProfile *profile, QWidget *parent)
    : QWebEngineView(profile, parent),
      profile(profile)
{
    setPage(new WebPage(profile, this));
    connect(this, &WebView::loadFinished, this, &WebView::handleLoadFinished);
    connect(this, &WebView::iconChanged, this, &WebView::handleIconChanged);
    connect(this, &WebView::renderProcessTerminated, this, &WebView::handleRenderProcessTerminated);
    // window.print() from the page
    connect(this, &WebView::printRequested, this, [this] {
        if (auto *mw = qobject_cast<MainWindow *>(window())) {
            mw->printPage(this);
        }
    });
}

void WebView::handleRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status)
{
    if (status == QWebEnginePage::NormalTerminationStatus) {
        return;
    }
    const QUrl crashedUrl = url();
    // Load the replacement page outside the signal handler, once the dead renderer is torn down.
    QTimer::singleShot(0, this, [this, crashedUrl] {
        const QString link = QString::fromUtf8(crashedUrl.toEncoded()).toHtmlEscaped();
        const QString html
            = QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>%1</title></head>"
                             "<body style=\"font-family: sans-serif; text-align: center; margin-top: 15%;\">"
                             "<h2>%1</h2><p>%2</p><p><a href=\"%3\">%4</a></p></body></html>")
                  .arg(tr("This tab crashed").toHtmlEscaped(),
                       tr("The page stopped unexpectedly.").toHtmlEscaped(), link, tr("Reload").toHtmlEscaped());
        readerMode = false;
        // Keep the original URL as base so the address bar still shows it.
        setHtml(html, crashedUrl);
    });
}

void WebView::showReaderPage(const QString &html)
{
    readerMode = true;
    setHtml(html, url());
}

bool WebView::isGeneratedPage() const
{
    // setHtml() loads a data: URL while url() keeps the base URL, and this holds after back, forward
    // and reload too. Internal mx-* pages are generated the same way but regenerate themselves.
    const QString scheme = url().scheme();
    return history()->currentItem().url().scheme() == "data" && scheme != "data" && scheme != "mx-history"
           && scheme != "mx-settings" && scheme != "mx-newtab";
}

void WebView::reloadPage()
{
    if (isGeneratedPage()) {
        setUrl(url());
    } else {
        reload();
    }
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    auto *menu = createStandardContextMenu();
    // Spelling suggestions for a misspelled word in an editable field go first.
    const auto *request = lastContextMenuRequest();
    if (request && !request->misspelledWord().isEmpty()) {
        QAction *first = menu->actions().value(0);
        const QStringList suggestions = request->spellCheckerSuggestions();
        for (const QString &suggestion : suggestions) {
            auto *action = new QAction(suggestion, menu);
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
            connect(action, &QAction::triggered, this, [this, suggestion] { page()->replaceMisspelledWord(suggestion); });
            menu->insertAction(first, action);
        }
        if (suggestions.isEmpty()) {
            auto *none = new QAction(tr("No spelling suggestions"), menu);
            none->setEnabled(false);
            menu->insertAction(first, none);
        }
        menu->insertSeparator(first);
    }
    // "Open link in new window" also ends up as a tab here, so it would just duplicate "new tab".
    menu->removeAction(pageAction(QWebEnginePage::OpenLinkInNewWindow));
    const QString selection = selectedText().simplified();
    if (!selection.isEmpty()) {
        const QString shown = selection.size() > 30 ? selection.left(30) + QChar(0x2026) : selection;
        auto *search = new QAction(tr("Search the web for \"%1\"").arg(shown), menu);
        connect(search, &QAction::triggered, this, [this, selection] {
            if (auto *mw = qobject_cast<MainWindow *>(window())) {
                mw->searchInNewTab(selection);
            }
        });
        const auto actions = menu->actions();
        const auto copy = std::find(actions.cbegin(), actions.cend(), pageAction(QWebEnginePage::Copy));
        QAction *before = (copy != actions.cend() && std::next(copy) != actions.cend()) ? *std::next(copy) : nullptr;
        menu->insertAction(before, search);
    }
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(event->globalPos());
}

void WebView::installEventFilterOnFocusProxy()
{
    QWidget *proxy = focusProxy();
    if (proxy && proxy != m_currentProxy) {
        proxy->installEventFilter(this);
        m_currentProxy = proxy;
    }
}

bool WebView::event(QEvent *event)
{
    // focusProxy() is created lazily, install event filter when it becomes available
    if (event->type() == QEvent::ChildAdded) {
        QTimer::singleShot(0, this, &WebView::installEventFilterOnFocusProxy);
    }
    return QWebEngineView::event(event);
}

bool WebView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == focusProxy() && event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent *>(event);
        s_ctrlHeld = (me->modifiers() & Qt::ControlModifier);
        s_middleClick = (me->button() == Qt::MiddleButton);
        s_consumed = false;  // New click, reset consumed state
        s_clickTime = QDateTime::currentMSecsSinceEpoch();
    } else if (obj == focusProxy() && event->type() == QEvent::KeyPress) {
        // A link followed from the keyboard is not the earlier click (the modifier being held for a
        // click is not a key that follows a link).
        const int key = static_cast<QKeyEvent *>(event)->key();
        if (key != Qt::Key_Control && key != Qt::Key_Shift && key != Qt::Key_Alt && key != Qt::Key_Meta) {
            clearClickState();
        }
    }
    return QWebEngineView::eventFilter(obj, event);
}

// A click that did not follow a link (a paste, a word selection) must not divert a navigation that
// comes much later, so the click only counts for a moment.
bool WebView::lastClickWasNewTabRequest()
{
    return (s_ctrlHeld || s_middleClick) && QDateTime::currentMSecsSinceEpoch() - s_clickTime < clickLifetimeMs;
}

bool WebView::consumeIfNewTabRequest()
{
    if (lastClickWasNewTabRequest()) {
        s_consumed = true;
        s_ctrlHeld = false;
        s_middleClick = false;
        return true;
    }
    return false;
}

void WebView::clearClickState()
{
    s_ctrlHeld = false;
    s_middleClick = false;
    s_consumed = false;
}

bool WebView::wasClickConsumed()
{
    return s_consumed;
}

WebView *WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    auto *newView = new WebView(profile);
    // Check if acceptNavigationRequest already handled this click
    const bool background = !wasClickConsumed() && lastClickWasNewTabRequest();
    // The link then loads in the new view's page, which must not take the click as its own and open
    // yet another tab, leaving this one empty.
    clearClickState();
    // A page in a tab the user is not looking at never brings its popups to the front.
    const auto *stack = qobject_cast<QStackedWidget *>(parentWidget());
    const bool openerShown = !stack || stack->currentWidget() == this;
    if (type == QWebEnginePage::WebBrowserTab) {
        emit newWebView(newView, !background && openerShown);
    } else if (type == QWebEnginePage::WebBrowserBackgroundTab) {
        emit newWebView(newView, false);
    } else {
        // WebBrowserWindow / WebDialog: open in a tab of this window so the view is owned, shown and
        // shares this window's profile (a separate MainWindow would own a different profile object).
        emit newWebView(newView, openerShown);
    }
    return newView;
}

void WebView::handleLoadFinished(bool ok)
{
    // Generated pages (crash notice, reader view) are not visits; any other load leaves reader view.
    if (isGeneratedPage()) {
        // Back or forward can bring a reader page back, so ask the page which one it is.
        page()->runJavaScript(QStringLiteral("document.documentElement.hasAttribute('data-mx-reader')"),
                              QWebEngineScript::ApplicationWorld, [view = QPointer<WebView>(this)](const QVariant &result) {
                                  if (view && result.isValid() && view->isGeneratedPage()) {
                                      view->readerMode = result.toBool();
                                  }
                              });
        return;
    }
    readerMode = false;
    if (!ok || page()->profile()->isOffTheRecord()) {
        return;
    }
    const QUrl loadedUrl = url();
    if (!loadedUrl.isValid() || loadedUrl.toString() == "about:blank" || loadedUrl.scheme() == "mx-history"
        || loadedUrl.scheme() == "mx-settings" || loadedUrl.scheme() == "mx-newtab") {
        return;
    }
    QTimer::singleShot(750, this, [this, loadedUrl] {
        if (page()->isLoading()) {
            return;
        }
        if (url() != loadedUrl) {
            return;
        }
        if (!HistoryStore::addVisit(loadedUrl, title())) {
            return;
        }
        lastHistoryUrl = loadedUrl;
        handleIconChanged();
    });
}

void WebView::handleIconChanged()
{
    if (icon().isNull() || lastHistoryUrl.isEmpty() || url() != lastHistoryUrl) {
        return;
    }
    QPixmap iconPixmap = icon().pixmap(QSize(22, 22));
    QByteArray iconByteArray;
    QBuffer buffer(&iconByteArray);
    if (buffer.open(QIODevice::WriteOnly)) {
        iconPixmap.save(&buffer, "PNG");
    }
    HistoryStore::setIcon(lastHistoryUrl, iconByteArray);
}
