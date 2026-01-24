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
#include <QTextStream>
#include <QUrl>

#include "choosedialog.h"
#include "common.h"
#include "cmd.h"
#include <unistd.h>

namespace {
QString buildLocaleUpdateCommand(const QString &key, const QString &value)
{
#ifdef MX_LOCALE_ARCH
    return QString("localectl set-locale %1=%2").arg(key, value);
#else
    return QString("update-locale %1='%2'").arg(key, value);
#endif
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
    auto *selectedButton = buttonGroup->button(buttonId);
    selectedButton->setText(selection);
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
    if (buttonId == ButtonID::Lang) {
        setSubvariables();
    }
    const QString updateLocaleCommand = buildLocaleUpdateCommand(hashVarName.value(buttonId), selectedButton->text());
    Cmd().runAsRoot(updateLocaleCommand);
    ui->pushResetSubvar->setVisible(anyDifferentSubvars());
}

void MainWindow::resetSubvariables()
{
    const QString langValue = buttonGroup->button(ButtonID::Lang)->text();
    Cmd cmd;
#ifdef MX_LOCALE_ARCH
    cmd.runAsRoot("localectl set-locale LANG=" + langValue);
#else
    cmd.runAsRoot("rm " + Paths::defaultLocale);
    //debian moved locale configuration in trixie, not etc/default/locale is a symlink
    //to /etc/locale.conf :rollseyes:
    if (QFile("/etc/locale.conf").exists()){
        //also remove this file
        cmd.runAsRoot("rm " + Paths::defaultLocale);
        cmd.runAsRoot("touch /etc/locale.conf");
        cmd.runAsRoot("ln -srf /etc/locale.conf " + Paths::defaultLocale);
    }
    cmd.runAsRoot("update-locale LANG=" + langValue);
#endif
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
    const QString url = QUrl::fromLocalFile(helpPath).toString();
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

QString MainWindow::getCurrentLang() const
{
    QSettings defaultlocale(Paths::defaultLocale, QSettings::NativeFormat);
    return defaultlocale.value("LANG", "C").toString().replace(".utf8", ".UTF-8");
}

QString MainWindow::getCurrentSessionLang() const
{
    QString sessionLang = qgetenv("LANG").replace(".utf8", ".UTF-8");
    qDebug() << "Session lang" << sessionLang;
    return sessionLang;
}

void MainWindow::disableAllButCurrent()
{
    Cmd().runAsRoot("sed -i \"/^" + ui->buttonLang->text() + "\\|" + getCurrentSessionLang()
                    + "\\|^#/! s/#*/# /\" " + Paths::localeGen);
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
        QMessageBox::warning(this, tr("Error"),
                             tr("Can't disable locale in use", "message that the chosen locale cannot be "
                                                               "disabled because it is in active usage"));
        onFilterChanged(ui->comboFilter->currentText());
        return;
    }
    ui->listWidget->disconnect();
    localeGenChanged = true;
    QString text = item->text().section(QRegularExpression(R"(\s*\t)"), 0, 0);
    if (item->checkState() == Qt::Checked) {
        bool exists = Cmd().run("grep -qF '" + text + "' " + Paths::localeGen);
        if (!exists) {
            Cmd().runAsRoot("echo " + text + " >>" + Paths::localeGen);
        } else {
            Cmd().runAsRoot(QString("sed -i -e 's/^[[:space:]]*//; 0,/%1/{//s/.*/%1/};' -e "
                                    "'/#.*%1/d' " + Paths::localeGen)
                                .arg(text));
        }
        ++countEnabled;
    } else {
        Cmd().runAsRoot(
            QString("sed -i 's/^[[:space:]]*//; /^#.*%1/d; s/^%1/%2/;' " + Paths::localeGen).arg(text, "# " + text));
        QString delStr = text.section(' ', 0, 0);
        //sed will delete break a symlink when editing a file.
        Cmd().runAsRoot("sed -i --follow-symlinks '/" + delStr + "/d' " + Paths::defaultLocale);
        if (delStr.contains("@")) {
            Cmd().runAsRoot("sed -i --follow-symlinks '/" + delStr.section('@', 0, 0) + ".UTF-8@" + delStr.section('@', 1)
                            + "/d' " + Paths::defaultLocale);
        }
        setSubvariables();
        --countEnabled;
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
    QStringList enabledLocales = readEnabledLocales(Paths::localeGen);
    if (enabledLocales.isEmpty()) {
        return;
    }

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

QStringList MainWindow::readEnabledLocales(const QString &filePath)
{
    QStringList enabledLocales;
    QFile localeFile(filePath);

    if (!localeFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open %1").arg(localeFile.fileName()));
        return enabledLocales;
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
    return enabledLocales;
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
    QRegularExpression titleRegex(R"(^title[[:space:]]+["](?<title>[^"]+))");

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
    cmd.runAsRoot("locale-gen");
    localeGenChanged = false;
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
    QString listCmd
        = QString("dpkg-query -W --showformat=\"\\${Package}\\n\" -- 'mx-docs-*' 'mx-faq-*' | grep -vE '%1'")
              .arg(exclusionPattern);

    Cmd cmd;
    QStringList packageList = cmd.getOut(listCmd, true).split('\n', Qt::SkipEmptyParts);

    if (packageList.isEmpty()) {
        QMessageBox::information(this, tr("Remove Manuals"), tr("No manuals to remove."));
        return;
    }

    QString purgeCmd = QString("apt-get purge -y %1").arg(packageList.join(' '));

    ui->tabWidget->setDisabled(true);
    QProgressDialog prog(tr("Removing packages, please wait"), nullptr, 0, packageList.count());
    connect(&cmd, &Cmd::outputAvailable, this, [&prog](const QString &) { prog.setValue(prog.value() + 1); });
    prog.show();
    cmd.runAsRoot(purgeCmd, true);
    ui->tabWidget->setEnabled(true);
}

void MainWindow::resetLocaleGen()
{
    Cmd().runAsRoot("cp " + QDir(Paths::mxLocaleLib).filePath("locale.gen") + " /etc/");
    displayLocalesGen();
    localeGenChanged = true;
}
