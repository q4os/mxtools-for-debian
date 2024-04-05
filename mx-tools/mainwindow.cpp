/**********************************************************************
 * Copyright (C) 2014 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of MX Tools.
 *
 * MX Tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MX Tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MX Tools.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScreen>

#include "about.h"
#include "flatbutton.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setConnections();
    setWindowFlags(Qt::Window); // For the close, min and max buttons
    // Detect if tools are displayed in the menu (check for only one since all are set at the same time)
    if (system("grep -q \"NoDisplay=true\" /home/$USER/.local/share/applications/mx-user.desktop >/dev/null 2>&1")
        == 0) {
        ui->checkHide->setChecked(true);
    }

    const QString search_folder = "/usr/share/applications";
    live_list = listDesktopFiles("MX-Live", search_folder);
    maintenance_list = listDesktopFiles("MX-Maintenance", search_folder);
    setup_list = listDesktopFiles("MX-Setup", search_folder);
    software_list = listDesktopFiles("MX-Software", search_folder);
    utilities_list = listDesktopFiles("MX-Utilities", search_folder);

    QVector<QStringList *> lists {&live_list, &maintenance_list, &setup_list, &software_list, &utilities_list};

    const QString partitionType = getCmdOut("df -T / |tail -n1 |awk '{print $2}'");

    // Remove mx-remastercc and live-kernel-updater from list if not running Live
    bool live = (partitionType == "aufs" || partitionType == "overlay");
    if (!live) {
        const QStringList live_list_copy = live_list;
        for (const QString &item : live_list_copy) {
            if (item.contains("mx-remastercc.desktop") || item.contains("live-kernel-updater.desktop")) {
                live_list.removeOne(item);
            }
        }
    }

    QStringList termsToRemove;
    termsToRemove << (live ? "MX-OnlyInstalled" : "MX-OnlyLive");
    // Since we are loading only MX apps we control this works OK, however we need to keep in mind that some
    // app .desktop files have something like "OnlyShownIn=Blah;Xfce;KDE;Blah" so this login would fail in that case
    QString desktop = qgetenv("XDG_CURRENT_DESKTOP");
    if (desktop != "XFCE") {
        termsToRemove << "OnlyShowIn=XFCE";
    }
    if (desktop != "Fluxbox") {
        termsToRemove << "OnlyShowIn=FLUXBOX";
    }
    if (desktop != "KDE") {
        termsToRemove << "OnlyShowIn=KDE";
    }
    for (auto &list : lists) {
        removeEnvExclusive(list, termsToRemove);
    }

    category_map.insert("MX-Live", live_list);
    category_map.insert("MX-Maintenance", maintenance_list);
    category_map.insert("MX-Setup", setup_list);
    category_map.insert("MX-Software", software_list);
    category_map.insert("MX-Utilities", utilities_list);

    readInfo(category_map);
    addButtons(info_map);
    ui->textSearch->setFocus();
    QSize size = this->size();
    restoreGeometry(settings.value("geometry").toByteArray());
    if (isMaximized()) { // if started maximized give option to resize to normal window size
        resize(size);
        QRect screenGeometry = QApplication::primaryScreen()->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
    icon_size = settings.value("icon_size", icon_size).toInt();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setConnections()
{
    connect(ui->checkHide, &QCheckBox::clicked, this, &MainWindow::checkHide_clicked);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushCancel, &QPushButton::clicked, this, &MainWindow::close);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->textSearch, &QLineEdit::textChanged, this, &MainWindow::textSearch_textChanged);
}

QString MainWindow::getCmdOut(const QString &cmd)
{
    QProcess proc;
    proc.start("/bin/bash", {"-c", cmd});
    proc.waitForFinished(-1);
    return proc.readAllStandardOutput().trimmed();
}

// List .desktop files that contain a specific string
QStringList MainWindow::listDesktopFiles(const QString &search_string, const QString &location)
{
    QString cmd = QStringLiteral("grep -Elr %1 %2 | sort").arg(search_string, location);
    QString out = getCmdOut(cmd);
    if (out.isEmpty()) {
        return {};
    }
    return out.split('\n');
}

// Load info (name, comment, exec, icon_name, category, terminal) to the info_map
void MainWindow::readInfo(const QMultiMap<QString, QStringList> &category_map)
{
    const QString lang = QLocale().name().split('_').first();
    const QString lang_region = QLocale().name();

    QMapIterator<QString, QStringList> it(category_map);
    while (it.hasNext()) {
        const QString category = it.next().key();
        const QStringList list = category_map.value(category);

        QMultiMap<QString, QStringList> categoryInfoMap;
        for (const QString &file_name : qAsConst(list)) {
            QFile file(file_name);
            if (!file.open(QFile::Text | QFile::ReadOnly)) {
                continue;
            }
            const QString text = file.readAll();
            file.close();

            QString name;
            QString comment;
            if (lang != "en") {
                name = getTranslation(text, "Name", lang_region, lang);
                comment = getTranslation(text, "Comment", lang_region, lang);
            }

            name = name.isEmpty() ? getValueFromText(text, "Name").remove(QRegularExpression("^MX ")).replace('&', "&&")
                                  : name;
            comment = comment.isEmpty() ? getValueFromText(text, "Comment") : comment;

            QString exec = getValueFromText(text, "Exec");
            fixExecItem(&exec);

            const QString icon_name = getValueFromText(text, "Icon");
            const QString terminal_switch = getValueFromText(text, "Terminal");

            categoryInfoMap.insert(file_name, {name, comment, icon_name, exec, category, terminal_switch});
        }
        info_map.insert(category, categoryInfoMap);
    }
}

QString MainWindow::getTranslation(const QString &text, const QString &key, const QString &lang_region,
                                   const QString &lang)
{
    QRegularExpression re('^' + key + "\\[" + lang_region + "\\]=(.*)$");
    re.setPatternOptions(QRegularExpression::MultilineOption);
    QString translation = re.match(text).captured(1).trimmed();
    if (!translation.isEmpty()) {
        return translation;
    }
    re.setPattern('^' + key + "\\[" + lang + "\\]=(.*)$");
    return re.match(text).captured(1).trimmed();
}

QString MainWindow::getValueFromText(const QString &text, const QString &key)
{
    QRegularExpression re('^' + key + "=(.*)$");
    re.setPatternOptions(QRegularExpression::MultilineOption);
    return re.match(text).captured(1).trimmed();
}

// Read the info_map and add the buttons to the UI
void MainWindow::addButtons(const QMultiMap<QString, QMultiMap<QString, QStringList>> &info_map)
{
    int col = 0;
    int row = 0;
    int max = 200;

    max_elements = 0;
    QMapIterator<QString, QMultiMap<QString, QStringList>> it(info_map);
    QString category;
    while (it.hasNext()) {
        category = it.next().key();
        if (info_map.value(category).keys().count() > max_elements) {
            max_elements = info_map.value(category).keys().count();
        }
    }

    QString name;
    QString comment;
    QString exec;
    QString icon_name;
    QString terminal_switch;

    // Get max button size
    QMapIterator<QString, QStringList> itsize(info_map.value(category));
    int max_button_width = 20; // set a min != 0 to avoid div/0 in case of error
    while (itsize.hasNext()) {
        QString file_name = itsize.next().key();
        QStringList file_info = info_map.value(category).value(file_name);
        name = file_info.at(Info::Name);
        max_button_width = qMax(name.size() * QApplication::font().pointSize() + icon_size, max_button_width);
    }
    max = width() / max_button_width;
    it.toFront();
    while (it.hasNext()) {
        category = it.next().key();
        if (!info_map.value(category).isEmpty()) {
            // Add empty row and delimiter except for the first row
            if (row != 0) {
                ++row;
                auto *line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                ui->gridLayout_btn->addWidget(line, row, 0, 1, -1);
            }
            auto *label = new QLabel();
            QFont font;
            font.setBold(true);
            font.setUnderline(true);
            label->setFont(font);
            QString label_txt = category;
            label_txt.remove(QRegularExpression("^MX-"));
            label->setText(label_txt);
            ++row;
            ui->gridLayout_btn->addWidget(label, row, 0);
            ++row;
            col = 0;
            QMapIterator<QString, QStringList> it(info_map.value(category));
            while (it.hasNext()) {
                QString file_name = it.next().key();
                if (col >= col_count) {
                    col_count = col + 1;
                }
                QStringList file_info = info_map.value(category).value(file_name);
                name = file_info.at(Info::Name);
                comment = file_info.at(Info::Comment);
                icon_name = file_info.at(Info::IconName);
                exec = file_info.at(Info::Exec);
                terminal_switch = file_info.at(Info::Terminal);
                btn = new FlatButton(name);
                btn->setToolTip(comment);
                btn->setAutoDefault(false);
                btn->setIcon(findIcon(icon_name));
                btn->setIconSize(icon_size, icon_size);
                ui->gridLayout_btn->addWidget(btn, row, col);
                // ui->gridLayout_btn->setRowStretch(row, 0);
                ++col;

                if (col >= max) {
                    col = 0;
                    ++row;
                }
                if (terminal_switch == "true") {
                    btn->setObjectName("x-terminal-emulator -e " + exec);
                } else {
                    btn->setObjectName(exec);
                }
                QObject::connect(btn, &FlatButton::clicked, this, &MainWindow::btn_clicked);
            }
        }
    }
    ui->gridLayout_btn->setRowStretch(row + 2, 1);
}

QIcon MainWindow::findIcon(QString icon_name)
{
    if (icon_name.isEmpty()) {
        return {};
    }
    if (QFileInfo::exists('/' + icon_name)) {
        return QIcon(icon_name);
    }

    QString search_term = icon_name;
    const QRegularExpression pattern("\\.(png|svg|xpm)$");
    if (!pattern.match(icon_name).hasMatch()) {
        search_term = icon_name + ".*";
    }
    icon_name.remove(pattern);

    // Return the icon from the theme if it exists
    if (QIcon::hasThemeIcon(icon_name)) {
        return QIcon::fromTheme(icon_name);
    }

    // Try to find in most obvious places
    QStringList search_paths {QDir::homePath() + "/.local/share/icons/", "/usr/share/pixmaps/",
                              "/usr/local/share/icons/", "/usr/share/icons/hicolor/48x48/apps/"};
    for (const QString &path : search_paths) {
        if (!QFileInfo::exists(path)) {
            search_paths.removeOne(path);
            continue;
        }
        for (const QString ext : {".png", ".svg", ".xpm"}) {
            QString file = path + icon_name + ext;
            if (QFileInfo::exists(file)) {
                return QIcon(file);
            }
        }
    }

    // Search recursive
    search_paths.append("/usr/share/icons/hicolor/48x48/");
    search_paths.append("/usr/share/icons/hicolor/");
    search_paths.append("/usr/share/icons/");
    QString out
        = getCmdOut("find " + search_paths.join(' ') + " -iname \"" + search_term + "\" -print -quit 2>/dev/null");
    return (!out.isEmpty()) ? QIcon(out) : QIcon();
}

void MainWindow::btn_clicked()
{
    hide();
    system(sender()->objectName().toUtf8());
    show();
}

void MainWindow::closeEvent(QCloseEvent * /*unused*/)
{
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (event->oldSize().width() == event->size().width()) {
        return;
    }
    int new_count = width() / 200;
    if (width() / 200 != col_count) {
        if (new_count > max_elements && col_count == max_elements) {
            return;
        }
        col_count = 0;
        if (ui->textSearch->text().isEmpty()) {
            QLayoutItem *child = nullptr;
            while ((child = ui->gridLayout_btn->takeAt(0)) != nullptr) {
                delete child->widget();
                delete child;
            }
            addButtons(info_map);
        } else {
            textSearch_textChanged(ui->textSearch->text());
        }
    }
}

// Hide icons in menu checkbox
void MainWindow::checkHide_clicked(bool checked)
{
    for (const QStringList &list : qAsConst(category_map)) {
        for (const QString &file_name : qAsConst(list)) {
            hideShowIcon(file_name, checked);
        }
    }
    system("sh -c 'pgrep xfce4-panel >/dev/null && xfce4-panel --restart'");
}

// Hide or show icon for .desktop file
void MainWindow::hideShowIcon(const QString &file_name, bool hide)
{
    QFileInfo file(file_name);
    QString file_name_home = QDir::homePath() + "/.local/share/applications/" + file.fileName();
    if (!hide) {
        QFile::remove(file_name_home);
    } else {
        QFile::copy(file_name, file_name_home);
        QString cmd = "sed -i -r -e '/^(NoDisplay|Hidden)=/d' ";
        cmd += "-e '/Exec/aNoDisplay=true' ";
        cmd += file_name_home;
        system(cmd.toUtf8());
    }
}

void MainWindow::pushAbout_clicked()
{
    hide();
    displayAboutMsgBox(
        tr("About MX Tools"),
        "<p align=\"center\"><b><h2>" + tr("MX Tools") + "</h2></b></p><p align=\"center\">" + tr("Version: ") + VERSION
            + "</p><p align=\"center\"><h3>" + tr("Configuration Tools for MX Linux")
            + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p>"
              "<p align=\"center\">"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-tools/license.html", tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::pushHelp_clicked()
{
    if (QFile::exists("/usr/bin/mx-manual")) {
        QProcess::startDetached("mx-manual", {});
    } else { // for MX19?
        QProcess::startDetached("xdg-open", {"file:///usr/local/share/doc/mxum.html#toc-Subsection-3.2"});
    }
}

void MainWindow::textSearch_textChanged(const QString &arg1)
{
    // Remove all items from the layout
    QLayoutItem *child = nullptr;
    while ((child = ui->gridLayout_btn->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    QMultiMap<QString, QMultiMap<QString, QStringList>> new_map;
    QMultiMap<QString, QStringList> map;

    // Create a new_map with items that match the search argument
    QMapIterator<QString, QMultiMap<QString, QStringList>> it(info_map);
    while (it.hasNext()) {
        QString category = it.next().key();
        QMultiMap<QString, QStringList> file_info = info_map.value(category);
        for (const QString &file_name : category_map.value(category)) {
            QString name = file_info.value(file_name).at(0);
            QString comment = file_info.value(file_name).at(1);
            if (name.contains(arg1, Qt::CaseInsensitive) || comment.contains(arg1, Qt::CaseInsensitive)
                || category.contains(arg1, Qt::CaseInsensitive)) {
                map.insert(file_name, info_map.value(category).value(file_name));
            }
        }
        if (!map.isEmpty()) {
            new_map.insert(category, map);
            map.clear();
        }
    }
    if (!new_map.isEmpty()) {
        arg1.isEmpty() ? addButtons(info_map) : addButtons(new_map);
    }
}

// Strip %f, %F, %U, etc. if exec expects a file name since it's called without an argument from this launcher.
void MainWindow::fixExecItem(QString *item)
{
    item->remove(QRegularExpression(R"( %[a-zA-Z])"));
}

// When running live remove programs meant only for installed environments and the other way round
// Remove XfceOnly and FluxboxOnly when not running in that environment
void MainWindow::removeEnvExclusive(QStringList *list, const QStringList &termsToRemove)
{
    for (auto it = list->begin(); it != list->end();) {
        QFile file(*it);
        if (file.open(QFile::Text | QFile::ReadOnly)) {
            QString text = file.readAll();
            file.close();
            if (std::none_of(termsToRemove.cbegin(), termsToRemove.cend(),
                             [&text](const QString &term) { return text.contains(term); })) {
                ++it;
            } else {
                it = list->erase(it);
            }
        } else {
            qWarning() << "Could not open file:" << *it;
            ++it;
        }
    }
}
