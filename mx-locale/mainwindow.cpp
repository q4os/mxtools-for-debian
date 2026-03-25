/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2024 MX Authors
 *
 * Authors: Dolphin Oracle
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

#include "mainwindow.h"
#include "about.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDirIterator>
#include <QFileDialog>
#include <QProgressDialog>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QTextStream>
#include <QUrl>

#include "choosedialog.h"
#include "common.h"
#include "cmd.h"
#include <unistd.h>

namespace {
bool isSafeLocaleToken(const QString &value)
{
    static const QRegularExpression regex(R"(^[A-Za-z0-9_.@-]+$)");
    return regex.match(value).hasMatch();
}

bool isSafeLocaleGenLine(const QString &value)
{
    static const QRegularExpression regex(R"(^[A-Za-z0-9_.@-]+(?:\s+[A-Za-z0-9_.@-]+)?$)");
    return regex.match(value).hasMatch();
}

void showCommandError(QWidget *parent, Cmd &cmd, const QString &fallback = QObject::tr("Operation failed."))
{
    const QString details = cmd.readAllOutput();
    QMessageBox::critical(parent, QObject::tr("Error"), details.isEmpty() ? fallback : details);
}
} // namespace

MainWindow::MainWindow(const QCommandLineParser &args, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // For the close, min and max buttons
    setup();
    configureTabs(args);
    configureCategories(args);
    adjustSize();
}

void MainWindow::configureTabs(const QCommandLineParser &args)
{
    if (args.isSet("only-lang")) {
        ui->tabWidget->setTabVisible(Tab::Subvariables, false);
        ui->tabWidget->setTabVisible(Tab::Management, false);
    }
}

void MainWindow::configureCategories(const QCommandLineParser &args)
{
    const bool fullCategories = args.isSet("full-categories");
    ui->label_Ctype->setHidden(!fullCategories);
    ui->pushButtonCType->setHidden(!fullCategories);
    ui->label_Ident->setHidden(!fullCategories);
    ui->pushButtonIdentification->setHidden(!fullCategories);
}

MainWindow::~MainWindow()
{
    if (localeGenChanged) {
        localeGen();
    }
    delete ui;
}

// Setup various items first time program runs
void MainWindow::setup()
{
    this->setWindowTitle(tr("MX Locale"));
    ui->tabWidget->setCurrentIndex(0);
    ui->buttonLang->setText(getCurrentLang());
    setSubvariables();
    setButtons();
    setConnections();
#ifdef MX_LOCALE_ARCH
    ui->pushRemoveManuals->setVisible(false);
#endif
    ui->pushResetSubvar->setVisible(anyDifferentSubvars());
}

// Check if there are subvariables different than LANG
bool MainWindow::anyDifferentSubvars() const
{
    // Skip the first button: ButtonID::LANG
    const auto buttons = buttonGroup->buttons();
    return std::any_of(std::next(buttons.begin()), buttons.end(),
                       [this](const auto *button) { return button->text() != ui->buttonLang->text(); });
}

void MainWindow::disableGUI(bool disable)
{
    ui->tabWidget->setDisabled(disable);
    ui->comboFilter->setDisabled(disable);
    ui->textSearch->setDisabled(disable);
    ui->pushDisableLocales->setDisabled(disable);
    ui->pushResetLocales->setDisabled(disable);
    ui->buttonCancel->setDisabled(disable);
}

void MainWindow::onGroupButton(int buttonId)
{
    ChooseDialog dialog;
    dialog.setModal(true);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const QString selection = dialog.selection();
    if (selection.isEmpty()) {
        return;
    }
    if (!isSafeLocaleToken(selection)) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid locale value."));
        return;
    }
    auto *selectedButton = buttonGroup->button(buttonId);
    static const QHash<int, QString> hashVarName {
        {ButtonID::Lang, "LANG"},
        {ButtonID::Address, "LC_ADDRESS"},
        {ButtonID::Collate, "LC_COLLATE"},
        {ButtonID::CType, "LC_CTYPE"},
        {ButtonID::Identification, "LC_IDENTIFICATION"},
        {ButtonID::Measurement, "LC_MEASUREMENT"},
        {ButtonID::Messages, "LC_MESSAGES"},
        {ButtonID::Monetary, "LC_MONETARY"},
        {ButtonID::Name, "LC_NAME"},
        {ButtonID::Numeric, "LC_NUMERIC"},
        {ButtonID::Paper, "LC_PAPER"},
        {ButtonID::Telephone, "LC_TELEPHONE"},
        {ButtonID::Time, "LC_TIME"},
    };
    Cmd cmd;
    if (!cmd.runAsRoot("set-locale", {hashVarName.value(buttonId), selection})) {
        showCommandError(this, cmd);
        return;
    }
    selectedButton->setText(selection);
    if (buttonId == ButtonID::Lang) {
        setSubvariables();
    }
    ui->pushResetSubvar->setVisible(anyDifferentSubvars());
}

void MainWindow::resetSubvariables()
{
    const QString langValue = buttonGroup->button(ButtonID::Lang)->text();
    if (!isSafeLocaleToken(langValue)) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid locale value."));
        return;
    }
    Cmd cmd;
    if (!cmd.runAsRoot("reset-subvariables", {langValue})) {
        showCommandError(this, cmd);
        return;
    }
    setSubvariables();
    ui->pushResetSubvar->setVisible(anyDifferentSubvars());
}

void MainWindow::aboutClicked()
{
    this->hide();
    displayAboutMsgBox(tr("About %1").arg(this->windowTitle()),
                       "<p align=\"center\"><b><h2>" + this->windowTitle() + "</h2></b></p><p align=\"center\">"
                           + tr("Version: ") + VERSION + "</p><p align=\"center\"><h3>"
                           + tr("Program for changing language and locale categories")
                           + "</h3></p><p align=\"center\"><a "
                             "href=\"http://mxlinux.org\">http://mxlinux.org</a><br "
                             "/></p><p align=\"center\">"
                           + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
                       QDir(Paths::mxLocaleDoc).filePath("license.html"), tr("%1 License").arg(this->windowTitle()));
    this->show();
}

void MainWindow::helpClicked()
{
    const QString helpPath = QDir(Paths::mxLocaleDoc).filePath("help/mx-locale.html");
    displayHelpDoc(helpPath, tr("%1 Help").arg(this->windowTitle()));
}

QString MainWindow::getCurrentLang() const
{
    QSettings defaultlocale(Paths::defaultLocale, QSettings::NativeFormat);
    return defaultlocale.value("LANG", "C").toString().replace(".utf8", ".UTF-8");
}

QString MainWindow::getCurrentSessionLang() const
{
    QString sessionLang = qEnvironmentVariable("LANG").trimmed().replace(".utf8", ".UTF-8");
    if (!isSafeLocaleToken(sessionLang)) {
        qWarning() << "Ignoring unsafe session LANG value";
        return {};
    }
    qDebug() << "Session lang" << sessionLang;
    return sessionLang;
}

void MainWindow::disableAllButCurrent()
{
    const QString currentLang = ui->buttonLang->text();
    const QString sessionLang = getCurrentSessionLang();
    if (!isSafeLocaleToken(currentLang)) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid locale value."));
        return;
    }

    QStringList helperArguments {currentLang};
    if (!sessionLang.isEmpty()) {
        helperArguments.append(sessionLang);
    }

    Cmd cmd;
    if (!cmd.runAsRoot("filter-locale-gen", helperArguments)) {
        showCommandError(this, cmd);
        return;
    }
    displayLocalesGen();
    localeGenChanged = true;
}

void MainWindow::setSubvariables()
{
    QSettings defaultlocale(Paths::defaultLocale, QSettings::NativeFormat);

    QString lang = ui->buttonLang->text();

    QString ctype = defaultlocale.value("LC_CTYPE", lang).toString();
    QString numeric = defaultlocale.value("LC_NUMERIC", lang).toString();
    QString time = defaultlocale.value("LC_TIME", lang).toString();
    QString collate = defaultlocale.value("LC_COLLATE", lang).toString();
    QString monetary = defaultlocale.value("LC_MONETARY", lang).toString();
    QString messages = defaultlocale.value("LC_MESSAGES", lang).toString();
    QString paper = defaultlocale.value("LC_PAPER", lang).toString();
    QString name = defaultlocale.value("LC_NAME", lang).toString();
    QString address = defaultlocale.value("LC_ADDRESS", lang).toString();
    QString telephone = defaultlocale.value("LC_TELEPHONE", lang).toString();
    QString measurement = defaultlocale.value("LC_MEASUREMENT", lang).toString();
    QString identification = defaultlocale.value("LC_IDENTIFICATION", lang).toString();

    ui->pushButtonCType->setText(ctype);
    ui->pushButtonNumeric->setText(numeric);
    ui->pushButtonTime->setText(time);
    ui->pushButtonCollate->setText(collate);
    ui->pushButtonMonetary->setText(monetary);
    ui->pushButtonMessages->setText(messages);
    ui->pushButtonPaper->setText(paper);
    ui->pushButtonName->setText(name);
    ui->pushButtonAddress->setText(address);
    ui->pushButtonTelephone->setText(telephone);
    ui->pushButtonMeasurement->setText(measurement);
    ui->pushButtonIdentification->setText(identification);
}

void MainWindow::setButtons()
{
    buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->pushButtonAddress, ButtonID::Address);
    buttonGroup->addButton(ui->pushButtonCollate, ButtonID::Collate);
    buttonGroup->addButton(ui->pushButtonCType, ButtonID::CType);
    buttonGroup->addButton(ui->pushButtonIdentification, ButtonID::Identification);
    buttonGroup->addButton(ui->pushButtonMeasurement, ButtonID::Measurement);
    buttonGroup->addButton(ui->pushButtonMessages, ButtonID::Messages);
    buttonGroup->addButton(ui->pushButtonMonetary, ButtonID::Monetary);
    buttonGroup->addButton(ui->pushButtonName, ButtonID::Name);
    buttonGroup->addButton(ui->pushButtonNumeric, ButtonID::Numeric);
    buttonGroup->addButton(ui->pushButtonPaper, ButtonID::Paper);
    buttonGroup->addButton(ui->pushButtonTelephone, ButtonID::Telephone);
    buttonGroup->addButton(ui->pushButtonTime, ButtonID::Time);
    buttonGroup->addButton(ui->buttonLang, ButtonID::Lang);
}

void MainWindow::setConnections()
{
    auto connectClicked = [this](auto *button, auto handler) { connect(button, &QPushButton::clicked, this, handler); };

    connect(buttonGroup, &QButtonGroup::idClicked, this, &MainWindow::onGroupButton);
    connectClicked(ui->buttonAbout, &MainWindow::aboutClicked);
    connectClicked(ui->buttonHelp, &MainWindow::helpClicked);
    connectClicked(ui->pushDisableLocales, &MainWindow::disableAllButCurrent);
    connectClicked(ui->pushResetLocales, &MainWindow::resetLocaleGen);
    connectClicked(ui->pushResetSubvar, &MainWindow::resetSubvariables);
    connectClicked(ui->pushRemoveManuals, &MainWindow::removeManuals);

    connect(ui->comboFilter, &QComboBox::currentTextChanged, this, &MainWindow::onFilterChanged);
    connect(ui->listWidget, &QListWidget::itemChanged, this, &MainWindow::listItemChanged);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabWidgetCurrentChanged);
    connect(ui->textSearch, &QLineEdit::textChanged, this, &MainWindow::textSearch_textChanged);
}

void MainWindow::tabWidgetCurrentChanged()
{
    auto *currentWidget = ui->tabWidget->currentWidget();
    if (currentWidget == ui->tabManagement) {
        ui->labelCurrentLocale->setText(
            tr("Locale in use: <b>%1</b>", "shows the current system locale, in bold").arg(getCurrentLang()));
        ui->listWidget->count() == 0 ? displayLocalesGen() : onFilterChanged(ui->comboFilter->currentText());
    }
    if (localeGenChanged) {
        localeGen();
    }
}

void MainWindow::textSearch_textChanged()
{
    onFilterChanged(ui->comboFilter->currentText());
}

void MainWindow::onFilterChanged(const QString &text)
{
    QString searchText = ui->textSearch->text();
    bool filterAll = text == tr("All", "all as in everything");
    bool searchEmpty = searchText.isEmpty();

    auto shouldHideItem = [&](QListWidgetItem *item) {
        if (!item) {
            return false;
        }

        if (!filterAll) {
            bool isDisabled = item->checkState() == Qt::Checked && text == tr("Disabled");
            bool isEnabled = item->checkState() == Qt::Unchecked && text == tr("Enabled");
            if (isDisabled || isEnabled) {
                return true;
            }
        }

        if (!searchEmpty && !item->text().contains(searchText, Qt::CaseInsensitive)) {
            return true;
        }

        return false;
    };

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        auto *item = ui->listWidget->item(i);
        item->setHidden(shouldHideItem(item));
    }
}

void MainWindow::listItemChanged(QListWidgetItem *item)
{
    // Check for disabling of running locale
    QString localeCode = item->text().section(' ', 0, 0);
    if (item->checkState() == Qt::Unchecked
        && (localeCode == getCurrentLang() || localeCode == getCurrentSessionLang())) {
        const QSignalBlocker blocker(ui->listWidget);
        item->setCheckState(Qt::Checked);
        QMessageBox::warning(this, tr("Error"),
                             tr("Can't disable locale in use", "message that the chosen locale cannot be "
                                                               "disabled because it is in active usage"));
        onFilterChanged(ui->comboFilter->currentText());
        return;
    }
    ui->listWidget->disconnect();
    QString text = item->text().section(QRegularExpression(R"(\s*\t)"), 0, 0);
    if (!isSafeLocaleGenLine(text) || !isSafeLocaleToken(localeCode)) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid locale value."));
        connect(ui->listWidget, &QListWidget::itemChanged, this, &MainWindow::listItemChanged);
        onFilterChanged(ui->comboFilter->currentText());
        return;
    }
    Cmd cmd;
    bool success = false;
    if (item->checkState() == Qt::Checked) {
        success = cmd.runAsRoot("enable-locale", {text});
        if (success) {
            localeGenChanged = true;
            ++countEnabled;
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    } else {
        success = cmd.runAsRoot("disable-locale", {text});
        if (success) {
            localeGenChanged = true;
            setSubvariables();
            --countEnabled;
        } else {
            item->setCheckState(Qt::Checked);
        }
    }
    if (!success) {
        showCommandError(this, cmd);
        connect(ui->listWidget, &QListWidget::itemChanged, this, &MainWindow::listItemChanged);
        onFilterChanged(ui->comboFilter->currentText());
        return;
    }
    ui->labelCountLocale->setText(
        tr("Locales enabled: %1", "label for a numerical count of enabled and available locales").arg(countEnabled));
    connect(ui->listWidget, &QListWidget::itemChanged, this, &MainWindow::listItemChanged);
    if (ui->comboFilter->currentText() != tr("All")) {
        onFilterChanged(ui->comboFilter->currentText());
    }
}

void MainWindow::displayLocalesGen()
{
    countEnabled = 0;
    ui->listWidget->clear();
    QStringList supportedFiles = {Paths::i18nSupported, Paths::i18nSupportedLocal};
    QStringList enabledLocales;
    if (!readEnabledLocales(Paths::localeGen, enabledLocales)) {
        return;
    }

    hashLocale.clear();
    processLocaleFiles(getLocaleFiles({Paths::i18nLocales, Paths::i18nLocalesLocal}));

    for (const QString &filePath : supportedFiles) {
        if (!QFile::exists(filePath)) {
            continue;
        }
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open %1").arg(file.fileName()));
            return;
        }
        readLocaleFile(file, enabledLocales);
        file.close();
    }
    updateLocaleListUI();
}

bool MainWindow::readEnabledLocales(const QString &filePath, QStringList &enabledLocales)
{
    enabledLocales.clear();
    QFile localeFile(filePath);

    if (!localeFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open %1").arg(localeFile.fileName()));
        return false;
    }

    QTextStream in(&localeFile);
    QRegularExpression localeRegex("^[a-z]{2,3}([^_]|_[A-Z]{2})");

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (localeRegex.match(line).hasMatch()) {
            enabledLocales.append(line);
        }
    }
    localeFile.close();
    return true;
}

QStringList MainWindow::getLocaleFiles(const QStringList &directories) const
{
    QStringList localeFiles;
    for (const QString &dirPath : directories) {
        QDirIterator it(dirPath, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            localeFiles.append(it.next());
        }
    }
    return localeFiles;
}

void MainWindow::processLocaleFiles(const QStringList &localeFiles)
{
    QRegularExpression titleRegex(R"(^title[[:space:]]+["](?<title>[^"]+))", QRegularExpression::MultilineOption);

    for (const QString &fileName : localeFiles) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        auto it = titleRegex.globalMatch(content);
        while (it.hasNext()) {
            auto titleMatch = it.next();
            QString title = titleMatch.captured("title");
            title = replaceUnicodeSequences(title);
            hashLocale.insert(QFileInfo(fileName).fileName(), title);
        }
    }
}

QString MainWindow::replaceUnicodeSequences(const QString &title) const
{
    QRegularExpression unicodeRegex(R"(<U(?<code>[[:xdigit:]]{4})>)");
    QString processedTitle = title;
    auto i = unicodeRegex.globalMatch(title);
    while (i.hasNext()) {
        auto match = i.next();
        processedTitle.replace(match.captured(0), QString(QChar(match.captured("code").toUShort(nullptr, 16))));
    }
    return processedTitle;
}

void MainWindow::updateLocaleListUI()
{
    disableGUI(true);
    ui->listWidget->sortItems();
    ui->labelCountLocale->setText(tr("Locales enabled: %1").arg(countEnabled));
    disableGUI(false);
}

void MainWindow::localeGen()
{
    ui->tabWidget->setDisabled(true);

    const int totalSteps = countEnabled * 2 + 2; // no_items * 2 (for locale + locale... done) + 2 for header and footer
    QProgressDialog progressDialog(tr("Updating locales, please wait"), nullptr, 0, totalSteps);
    progressDialog.setWindowModality(Qt::WindowModal);

    Cmd cmd;
    connect(&cmd, &Cmd::outputAvailable, &progressDialog, [&progressDialog] {
        progressDialog.setValue(progressDialog.value() + 1);
        QCoreApplication::processEvents();
    });

    progressDialog.show();
    if (cmd.runAsRoot("run-locale-gen")) {
        localeGenChanged = false;
    } else {
        showCommandError(this, cmd, tr("Could not update locales."));
    }
    ui->tabWidget->setEnabled(true);
}

void MainWindow::readLocaleFile(QFile &file, const QStringList &enabledLocale)
{
    QTextStream in(&file);
    QRegularExpression localeRegex("^[a-z]{2,3}([^_]|_[A-Z]{2})");
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (localeRegex.match(line).hasMatch()) {
            auto item = new QListWidgetItem;
            item->setCheckState(enabledLocale.contains(line) ? Qt::Checked : Qt::Unchecked);
            if (item->checkState() == Qt::Checked) {
                ++countEnabled;
            }
            line = line.leftJustified(20, ' ');
            item->setText(line + '\t' + hashLocale.value(line.section(QRegularExpression(R"(\s|\.)"), 0, 0)));
            ui->listWidget->addItem(item);
        }
    }
}

void MainWindow::removeManuals()
{
    QString lang = ui->buttonLang->text().section('.', 0, 0);

    // Fix for pt_BR, others use base language
    if (lang == "pt_BR") {
        lang = "pt-br";
    } else {
        lang = lang.section("_", 0, 0);
    }

    if (lang.isEmpty()) {
        return;
    }

    QString exclusionPattern
        = QString("mx-(docs|faq)-(en|common%1)").arg(lang == "en" || lang == "C" ? "" : QString("|%1").arg(lang));

    Cmd queryCmd;
    const QRegularExpression exclusionRegex(exclusionPattern);
    QStringList packageList
        = queryCmd.getOut("dpkg-query", {"-W", "--showformat=${Package}\n", "--", "mx-docs-*", "mx-faq-*"}, true)
              .split('\n', Qt::SkipEmptyParts);
    if (!queryCmd.succeeded()) {
        showCommandError(this, queryCmd, tr("Could not query installed manuals."));
        return;
    }
    for (qsizetype index = packageList.size() - 1; index >= 0; --index) {
        if (exclusionRegex.match(packageList.at(index)).hasMatch()) {
            packageList.removeAt(index);
        }
    }

    if (packageList.isEmpty()) {
        QMessageBox::information(this, tr("Remove Manuals"), tr("No manuals to remove."));
        return;
    }

    ui->tabWidget->setDisabled(true);
    Cmd authCmd;
    if (!authCmd.runAsRoot("noop", {}, true)) {
        ui->tabWidget->setEnabled(true);
        showCommandError(this, authCmd, tr("Authentication failed."));
        return;
    }

    Cmd cmd;
    QProgressDialog prog(tr("Removing packages, please wait"), nullptr, 0, packageList.count());
    connect(&cmd, &Cmd::outputAvailable, this, [&prog](const QString &) {
        prog.setValue(prog.value() + 1);
    });
    prog.show();
    if (!cmd.runAsRoot("purge-packages", packageList, true)) {
        showCommandError(this, cmd, tr("Could not remove packages."));
    }
    ui->tabWidget->setEnabled(true);
}

void MainWindow::resetLocaleGen()
{
    Cmd cmd;
    if (!cmd.runAsRoot("reset-locale-gen")) {
        showCommandError(this, cmd);
        return;
    }
    displayLocalesGen();
    localeGenChanged = true;
}
