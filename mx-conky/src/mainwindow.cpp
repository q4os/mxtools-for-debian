/**********************************************************************
 *  MainWindow.cpp
 **********************************************************************
 * Copyright (C) 2017-2025 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-conky.
 *
 * mx-conky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-conky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-conky.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeySequence>
#include <QMessageBox>
#include <QMovie>
#include <QProcess>
#include <QRegularExpression>
#include <QShortcut>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTimer>
#include <chrono>
#include <unistd.h>

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      m_conkyManager(nullptr),
      m_loadingMovie(nullptr),
      m_copyDialogShownThisSession(false)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << QCoreApplication::applicationVersion();

    setWindowFlags(Qt::Window);
    setWindowTitle(tr("MX Conky"));
    setupUI();

    restoreGeometry(settings.value("geometry").toByteArray());

    // Show window immediately with loading state
    show();

    // Start loading conkies asynchronously
    QTimer::singleShot(100ms, this, [this]() {
        m_conkyManager = new ConkyManager(this);

        // Now create the widgets that depend on ConkyManager
        m_conkyListWidget = new ConkyListWidget(m_conkyManager);
        m_previewWidget = new ConkyPreviewWidget;

        m_splitter->addWidget(m_conkyListWidget);
        m_splitter->addWidget(m_previewWidget);

        setConnections();

        // Manually trigger the transition since the initial scan already happened
        QTimer::singleShot(50ms, this, &MainWindow::onConkyItemsLoaded);
    });
}

void MainWindow::onConkyItemsLoaded()
{
    // Switch from loading to main content
    if (m_loadingMovie) {
        m_loadingMovie->stop();
    }
    m_stackedWidget->setCurrentWidget(m_mainWidget);

    // Refresh filter options based on current search paths
    populateFilterComboBox();
}

MainWindow::~MainWindow()
{
    if (m_conkyManager) {
        m_conkyManager->saveSettings();
    }
    if (m_loadingMovie) {
        m_loadingMovie->stop();
    }
}

void MainWindow::setupUI()
{
    resize(1000, 700);

    auto *mainLayout = new QVBoxLayout(this);

    // Create stacked widget to switch between loading and main content
    m_stackedWidget = new QStackedWidget;

    // Setup loading widget
    setupLoadingWidget();

    // Setup main widget
    setupMainWidget();

    m_stackedWidget->addWidget(m_loadingWidget);
    m_stackedWidget->addWidget(m_mainWidget);
    m_stackedWidget->setCurrentWidget(m_loadingWidget);

    mainLayout->addWidget(m_stackedWidget);
}

void MainWindow::setupLoadingWidget()
{
    m_loadingWidget = new QWidget;
    auto *loadingLayout = new QVBoxLayout(m_loadingWidget);
    loadingLayout->setAlignment(Qt::AlignCenter);

    // Loading animation
    m_loadingLabel = new QLabel;
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setMinimumSize(64, 64);

    // Try to use a spinner animation - fallback to text if no animation available
    QString iconPath = ":/icons/loading.gif";
    if (QFile::exists(iconPath)) {
        m_loadingMovie = new QMovie(iconPath);
        m_loadingLabel->setMovie(m_loadingMovie);
        m_loadingMovie->start();
    } else {
        // Fallback: create a simple animated text
        m_loadingLabel->setText("⏳");
        m_loadingLabel->setStyleSheet("font-size: 48px;");
    }

    auto *textLabel = new QLabel(tr("Loading Conky configurations..."));
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet("font-size: 16px; margin: 20px;");

    loadingLayout->addStretch();
    loadingLayout->addWidget(m_loadingLabel);
    loadingLayout->addWidget(textLabel);
    loadingLayout->addStretch();
}

void MainWindow::setupMainWidget()
{
    m_mainWidget = new QWidget;

    auto *mainContentLayout = new QVBoxLayout(m_mainWidget);

    // Combined toolbar and filter/search layout
    auto *topLayout = new QHBoxLayout;

    m_previewsButton = new QPushButton(tr("Previews"));
    m_previewsButton->setIcon(QIcon::fromTheme("image-x-generic-symbolic"));
    m_previewsButton->setToolTip(tr("Generate preview images for conkies"));

    m_settingsButton = new QPushButton(tr("Settings"));
    m_settingsButton->setIcon(QIcon::fromTheme("preferences-system-symbolic"));
    m_settingsButton->setToolTip(tr("Configure conky search paths"));

    m_refreshButton = new QPushButton(tr("Refresh"));
    m_refreshButton->setIcon(QIcon::fromTheme("view-refresh"));
    m_refreshButton->setToolTip(tr("Refresh conky list"));

    m_startAllButton = new QPushButton(tr("Start All"));
    m_startAllButton->setIcon(QIcon::fromTheme("media-playback-start"));
    m_startAllButton->setToolTip(tr("Start all enabled conkies"));

    m_stopAllButton = new QPushButton(tr("Stop All"));
    m_stopAllButton->setIcon(QIcon::fromTheme("media-playback-stop"));
    m_stopAllButton->setToolTip(tr("Stop all running conkies"));

    m_filterComboBox = new QComboBox;
    populateFilterComboBox();
    m_filterComboBox->setCurrentText(tr("All"));
    m_filterComboBox->setToolTip(tr("Filter conkies by running status or location"));

    m_searchLineEdit = new QLineEdit;
    m_searchLineEdit->setPlaceholderText(tr("Search conky by name..."));
    m_searchLineEdit->setClearButtonEnabled(true);
    m_searchLineEdit->setToolTip(tr("Search conkies by name (Ctrl+F)"));

    topLayout->addWidget(m_filterComboBox);
    topLayout->addSpacing(10);
    topLayout->addWidget(m_searchLineEdit);
    topLayout->addStretch(); // Add stretch between search and buttons
    topLayout->addWidget(m_refreshButton);
    topLayout->addWidget(m_startAllButton);
    topLayout->addWidget(m_stopAllButton);
    topLayout->addWidget(m_previewsButton);
    topLayout->addWidget(m_settingsButton);

    m_splitter = new QSplitter(Qt::Horizontal);

    // Note: ConkyListWidget and PreviewWidget will be created when ConkyManager is ready
    m_conkyListWidget = nullptr;
    m_previewWidget = nullptr;

    // Widgets will be added to splitter when they're created

    // Set default splitter geometry to equal panels when no saved state
    QByteArray splitterState = settings.value("splitter").toByteArray();
    if (splitterState.isEmpty()) {
        // Set equal split (50/50)
        QList<int> sizes;
        sizes << 500 << 500; // Equal sizes
        m_splitter->setSizes(sizes);
    } else {
        m_splitter->restoreState(splitterState);
    }

    // Create button bar
    auto *bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(5);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    m_aboutButton = new QPushButton(tr("About..."));
    m_aboutButton->setIcon(QIcon::fromTheme("help-about"));
    m_aboutButton->setToolTip(tr("About this application"));
    m_aboutButton->setShortcut(QKeySequence("Alt+B"));
    m_aboutButton->setAutoDefault(true);
    m_aboutButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    m_helpButton = new QPushButton(tr("Help"));
    m_helpButton->setIcon(QIcon::fromTheme("help-contents"));
    m_helpButton->setToolTip(tr("Display help"));
    m_helpButton->setShortcut(QKeySequence("Alt+H"));
    m_helpButton->setAutoDefault(true);
    m_helpButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // Add logo in center
    auto *logoLabel = new QLabel;
    logoLabel->setMaximumSize(32, 32);
    logoLabel->setPixmap(QPixmap(":/icons/logo.svg"));
    logoLabel->setScaledContents(true);

    m_closeButton = new QPushButton(tr("Close"));
    m_closeButton->setIcon(QIcon::fromTheme("window-close"));
    m_closeButton->setToolTip(tr("Quit application"));
    m_closeButton->setShortcut(QKeySequence("Alt+N"));
    m_closeButton->setAutoDefault(true);
    m_closeButton->setDefault(true);
    m_closeButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // Layout: About | Help | [stretch] | Logo | [stretch] | (spacer) | Close

    bottomLayout->addWidget(m_aboutButton);
    bottomLayout->addWidget(m_helpButton);
    bottomLayout->addStretch();
    bottomLayout->addWidget(logoLabel);
    bottomLayout->addStretch();

    // Add a spacer to the right of the logo, same width as the close button
    QSpacerItem *buttonSpacer
        = new QSpacerItem(m_closeButton->sizeHint().width(), 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
    bottomLayout->addItem(buttonSpacer);

    bottomLayout->addWidget(m_closeButton);

    mainContentLayout->addLayout(topLayout);
    mainContentLayout->addWidget(m_splitter, 1);
    mainContentLayout->addLayout(bottomLayout);
}

void MainWindow::setConnections()
{
    if (!m_conkyManager) {
        return;
    }

    connect(m_previewsButton, &QPushButton::clicked, this, &MainWindow::onPreviewsClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_startAllButton, &QPushButton::clicked, this, &MainWindow::onStartAllClicked);
    connect(m_stopAllButton, &QPushButton::clicked, this, &MainWindow::onStopAllClicked);

    connect(m_aboutButton, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(m_helpButton, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);

    connect(m_conkyListWidget, &ConkyListWidget::itemSelectionChanged, this, &MainWindow::onItemSelectionChanged);
    connect(m_conkyListWidget, &ConkyListWidget::editRequested, this, &MainWindow::onEditRequested);
    connect(m_conkyListWidget, &ConkyListWidget::customizeRequested, this, &MainWindow::onCustomizeRequested);
    connect(m_conkyListWidget, &ConkyListWidget::deleteRequested, this, &MainWindow::onDeleteRequested);

    connect(m_previewWidget, &ConkyPreviewWidget::previewImageLoaded, this, &MainWindow::onPreviewImageLoaded);

    // Connect filter and search
    connect(m_filterComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
            &MainWindow::onFilterChanged);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);

    // Add Ctrl+F shortcut to focus search field
    auto *searchShortcut = new QShortcut(QKeySequence::Find, this);
    connect(searchShortcut, &QShortcut::activated, this, &MainWindow::focusSearchField);

    // Connect to know when conkies are loaded
    connect(m_conkyManager, &ConkyManager::conkyItemsChanged, this, &MainWindow::onConkyItemsLoaded);
}

void MainWindow::onPreviewsClicked()
{
    if (!m_conkyManager || !m_conkyListWidget) {
        return;
    }

    // Get currently selected item if any
    ConkyItem *selectedItem = m_conkyListWidget->selectedConkyItem();

    PreviewDialog dialog(m_conkyManager, selectedItem, this);

    // Connect signals to handle conky list refresh and selection
    connect(&dialog, &PreviewDialog::conkyListNeedsRefresh, this, [this]() {
        if (m_conkyListWidget) {
            m_conkyListWidget->refreshList();
        }
    });

    connect(&dialog, &PreviewDialog::conkyItemNeedsSelection, this, [this](const QString &filePath) {
        if (m_conkyListWidget) {
            m_conkyListWidget->selectConkyItem(filePath);
        }
    });

    dialog.exec();

    // Refresh preview widget in case new images were generated
    ConkyItem *currentSelected = m_conkyListWidget->selectedConkyItem();
    if (m_previewWidget && currentSelected) {
        m_previewWidget->setConkyItem(currentSelected);
    }
}

void MainWindow::onSettingsClicked()
{
    if (!m_conkyManager) {
        return;
    }
    SettingsDialog dialog(m_conkyManager, this);
    dialog.exec();
}

void MainWindow::onRefreshClicked()
{
    if (!m_conkyManager || !m_conkyListWidget) {
        return;
    }
    m_conkyManager->scanForConkies();
    m_conkyListWidget->refreshList();
}

void MainWindow::onStartAllClicked()
{
    if (!m_conkyManager) {
        return;
    }
    m_conkyManager->startAutostart();
}

void MainWindow::onStopAllClicked()
{
    if (!m_conkyManager) {
        return;
    }
    m_conkyManager->stopAllRunning();
}

void MainWindow::onItemSelectionChanged(ConkyItem *item)
{
    if (!m_previewWidget) {
        return;
    }
    m_previewWidget->setConkyItem(item);
}

void MainWindow::onEditRequested(ConkyItem *item)
{
    if (!item) {
        return;
    }

    QString filePath = item->filePath();
    QFileInfo fileInfo(filePath);

    QString userConkyPath = QDir::homePath() + "/.conky";
    QString systemThemesPath = "/usr/share/mx-conky-data/themes";

    // Check if file needs to be copied (only from system themes) or edited in place
    if (!item->directory().startsWith(userConkyPath)) {
        // File is outside ~/.conky
        if (item->directory().startsWith(systemThemesPath)) {
            // Copy from system themes folder to ~/.conky
            QString sourceFolderPath = fileInfo.absolutePath();
            QString defaultName = QFileInfo(sourceFolderPath).fileName();

            bool ok;
            QString dialogMessage = m_copyDialogShownThisSession
                ? tr("Enter a name for the copy:")
                : tr("In order for you to edit and save a conky, it must first be copied to "
                     "~/.conky where you have permission.\nEnter a name for the copy.");

            QString newName = QInputDialog::getText(this, tr("Copy Conky"), dialogMessage,
                                                    QLineEdit::Normal, defaultName, &ok);

            if (!ok || newName.isEmpty()) {
                return; // User cancelled or entered empty name
            }

            m_copyDialogShownThisSession = true;

            // Check if destination directory already exists
            QString destPath = userConkyPath + "/" + newName;

            if (QFileInfo::exists(destPath)) {
                int result
                    = QMessageBox::question(this, tr("Directory Exists"),
                                            tr("A conky with the name '%1' already exists in your personal folder.\n"
                                               "Do you want to overwrite it?")
                                                .arg(newName),
                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

                if (result != QMessageBox::Yes) {
                    return; // User chose not to overwrite
                }
            }

            QString copiedPath = m_conkyManager->copyFolderToUserConkyWithName(sourceFolderPath, newName);

            if (!copiedPath.isEmpty()) {
                // Update the file path to point to the copied version
                QString fileName = fileInfo.fileName();
                filePath = copiedPath + "/" + fileName;

                // Switch filter to "All" to show the copied version
                if (m_filterComboBox) {
                    m_filterComboBox->setCurrentText(tr("All"));
                }

                // Add the new copy to the conky list (much faster than full rescan)
                m_conkyManager->addConkyItemsFromDirectory(copiedPath);

                // Select the newly copied conky in the interface
                if (m_conkyListWidget) {
                    m_conkyListWidget->selectConkyItem(filePath);
                }

                QMessageBox::information(
                    this, tr("Conky Copied"),
                    tr("Conky has been copied to your personal folder for editing:\n%1").arg(copiedPath));
            } else {
                QMessageBox::critical(this, tr("Copy Failed"), tr("Failed to copy conky to your personal folder."));
                return;
            }
        } else {
            // File in other location - check if writable, otherwise offer elevation
            if (!fileInfo.isWritable()) {
                // Check if the detected editor can handle elevation automatically
                QString editor;
                QString default_editor = Cmd().getCmdOut("xdg-mime query default text/plain");
                QString desktop_file = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, default_editor,
                                                              QStandardPaths::LocateFile);

                QFile file(desktop_file);
                if (file.open(QIODevice::ReadOnly)) {
                    while (!file.atEnd()) {
                        QString line = file.readLine();
                        if (line.contains(QRegularExpression("^Exec="))) {
                            editor = line.remove(QRegularExpression("^Exec=|%u|%U|%f|%F|%c|%C|-b")).trimmed();
                            break;
                        }
                    }
                    file.close();
                }
                if (editor.isEmpty()) {
                    editor = "nano";
                }

                const bool isEditorThatElevates
                    = QRegularExpression(R"((kate|kwrite|featherpad|code|codium)$)").match(editor).hasMatch();

                // Only show elevation prompt if editor cannot handle it automatically
                if (!isEditorThatElevates) {
                    int result = QMessageBox::question(
                        this, tr("Edit Conky"),
                        tr("This conky file is read-only and requires administrator privileges to edit.\n"
                           "Do you want to edit it with elevated privileges?"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

                    if (result != QMessageBox::Yes) {
                        return;
                    }
                }
                // filePath remains the same - editConkyFile will handle elevation
            }
            // For writable files in other locations, edit directly
        }
    }

    editConkyFile(filePath);
}

void MainWindow::onDeleteRequested(ConkyItem *item)
{
    if (!item) {
        return;
    }

    QString filePath = item->filePath();
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    int result = QMessageBox::question(
        this, tr("Delete Conky"),
        tr("Are you sure you want to delete the conky file:\n%1\n\nThis action cannot be undone.").arg(fileName),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    // Stop the conky if it's running
    if (item->isRunning()) {
        m_conkyManager->stopConky(item);
    }

    bool needsElevation = false;

    if (fileInfo.exists() && !fileInfo.isWritable()) {
        needsElevation = true;
    }

    bool success = false;
    QString elevate = QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu";

    if (needsElevation) {
        QString command = elevate + " rm '" + filePath + "'";
        success = QProcess::execute("sh", QStringList() << "-c" << command) == 0;
    } else {
        success = QFile::remove(filePath);
    }

    if (success) {
        // Remove from manager - this will emit conkyItemsChanged signal
        m_conkyManager->removeConkyItem(item);

        QMessageBox::information(this, tr("Delete Successful"), tr("Conky file deleted successfully."));
    } else {
        QMessageBox::critical(this, tr("Delete Failed"), tr("Failed to delete conky file:\n%1").arg(filePath));
    }
}

void MainWindow::onCustomizeRequested(ConkyItem *item)
{
    if (!item) {
        return;
    }

    QString filePath = item->filePath();
    QFileInfo fileInfo(filePath);

    QString userConkyPath = QDir::homePath() + "/.conky";
    QString systemThemesPath = "/usr/share/mx-conky-data/themes";

    // Check if file needs to be copied (only from system themes) or customized in place
    if (!item->directory().startsWith(userConkyPath)) {
        // File is outside ~/.conky
        if (item->directory().startsWith(systemThemesPath)) {
            // Copy from system themes folder to ~/.conky
            QString sourceFolderPath = fileInfo.absolutePath();
            QString defaultName = QFileInfo(sourceFolderPath).fileName();

            bool ok;
            QString dialogMessage = m_copyDialogShownThisSession
                ? tr("Enter a name for the copy:")
                : tr("In order for you to edit and save a conky, it must first be copied to "
                     "~/.conky where you have permission.\nEnter a name for the copy.");

            QString newName = QInputDialog::getText(this, tr("Copy Conky"), dialogMessage,
                                                    QLineEdit::Normal, defaultName, &ok);

            if (ok && !newName.isEmpty()) {
                m_copyDialogShownThisSession = true;
            }

            if (!ok || newName.isEmpty()) {
                return; // User cancelled or entered empty name
            }

            // Check if destination directory already exists
            QString destPath = userConkyPath + "/" + newName;

            if (QFileInfo::exists(destPath)) {
                int result
                    = QMessageBox::question(this, tr("Directory Exists"),
                                            tr("A conky with the name '%1' already exists in your personal folder.\n"
                                               "Do you want to overwrite it?")
                                                .arg(newName),
                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

                if (result != QMessageBox::Yes) {
                    return; // User chose not to overwrite
                }
            }

            QString copiedPath = m_conkyManager->copyFolderToUserConkyWithName(sourceFolderPath, newName);

            if (!copiedPath.isEmpty()) {
                // Update the file path to point to the copied version
                QString fileName = fileInfo.fileName();
                filePath = copiedPath + "/" + fileName;

                // Switch filter to "All" to show the copied version
                if (m_filterComboBox) {
                    m_filterComboBox->setCurrentText(tr("All"));
                }

                // Add the new copy to the conky list (much faster than full rescan)
                m_conkyManager->addConkyItemsFromDirectory(copiedPath);

                // Select the newly copied conky in the interface
                if (m_conkyListWidget) {
                    m_conkyListWidget->selectConkyItem(filePath);
                }

                QMessageBox::information(
                    this, tr("Conky Copied"),
                    tr("Conky has been copied to your personal folder for customization:\n%1").arg(copiedPath));
            } else {
                QMessageBox::critical(this, tr("Copy Failed"), tr("Failed to copy conky to your personal folder."));
                return;
            }
        } else {
            // File in other location - check if writable, otherwise offer elevation
            if (!fileInfo.isWritable()) {
                int result = QMessageBox::question(
                    this, tr("Customize Conky"),
                    tr("This conky file is read-only and requires administrator privileges to customize.\n"
                       "Do you want to customize it with elevated privileges?"),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

                if (result != QMessageBox::Yes) {
                    return;
                }

                // Test if elevation works by attempting a quick touch test
                QString elevationTool
                    = QFile::exists("/usr/bin/pkexec") ? "pkexec" : (QFile::exists("/usr/bin/gksu") ? "gksu" : "sudo");
                QString testCommand = QString("%1 touch '%2'").arg(elevationTool, filePath);

                int exitCode = QProcess::execute("sh", QStringList() << "-c" << testCommand);
                if (exitCode != 0) {
                    // User cancelled elevation or it failed
                    return;
                }
            }
            // For writable files in other locations, customize directly
        }
    }

    ConkyCustomizeDialog dialog(filePath, this);
    dialog.exec();
}

void MainWindow::onPreviewImageLoaded(const QSize imageSize)
{
    if (imageSize.isEmpty()) {
        return;
    }

    // Use a small delay to ensure preview widget layout is complete before adjusting splitter
    QTimer::singleShot(10, this, [this, imageSize]() {
        // Calculate desired preview pane width (image width, max 50% of window)
        int maxPreviewWidth = width() / 2;
        int desiredWidth = qMin(imageSize.width() + 20, maxPreviewWidth); // Add 20px padding

        QList<int> sizes = m_splitter->sizes();
        if (sizes.size() == 2) {
            int totalWidth = sizes[0] + sizes[1];
            int newPreviewWidth = desiredWidth;
            int newListWidth = totalWidth - newPreviewWidth;

            // Ensure minimum widths
            if (newListWidth < 300) {
                newListWidth = 300;
                newPreviewWidth = totalWidth - newListWidth;
            }

            if (newPreviewWidth > 100) { // Only resize if preview width is reasonable
                sizes[0] = newListWidth;
                sizes[1] = newPreviewWidth;
                m_splitter->setSizes(sizes);
            }
        }
    });
}

void MainWindow::editConkyFile(const QString &filePath)
{
    hide();

    // Check if we have write permission to the file
    QFileInfo fileInfo(filePath);
    bool needsElevation = false;

    if (fileInfo.exists() && !fileInfo.isWritable()) {
        needsElevation = true;
    } else if (!fileInfo.exists()) {
        // Check if we can write to the directory
        QFileInfo dirInfo(fileInfo.absolutePath());
        if (!dirInfo.isWritable()) {
            needsElevation = true;
        }
    }

    bool debug = !QProcessEnvironment::systemEnvironment().value("DEBUG").isEmpty();

    QString editor;
    QString default_editor = Cmd().getCmdOut("xdg-mime query default text/plain");
    QString desktop_file
        = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, default_editor, QStandardPaths::LocateFile);

    QFile file(desktop_file);
    if (file.open(QIODevice::ReadOnly)) {
        while (!file.atEnd()) {
            QString line = file.readLine();
            if (line.contains(QRegularExpression("^Exec="))) {
                editor = line.remove(QRegularExpression("^Exec=|%u|%U|%f|%F|%c|%C|-b")).trimmed();
                break;
            }
        }
        file.close();
    }

    if (editor.isEmpty()) {
        editor = "nano";
    }

    if (debug) {
        qDebug() << "Detected editor:" << editor;
    }

    const bool isRoot = getuid() == 0;
    const bool isEditorThatElevates
        = QRegularExpression(R"((kate|kwrite|featherpad|code|codium)$)").match(editor).hasMatch();
    const bool isCliEditor = QRegularExpression(R"(nano|vi|vim|nvim|micro|emacs)").match(editor).hasMatch();

    QString elevate = QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu";
    QString command;

    if (needsElevation) {
        if (isEditorThatElevates && !isRoot) {
            command = editor + " \"" + filePath + "\"";
        } else if (isRoot && isEditorThatElevates) {
            command = elevate + " --user $(logname) " + editor + " \"" + filePath + "\"";
        } else if (isCliEditor) {
            command = "x-terminal-emulator -e " + elevate + " " + editor + " \"" + filePath + "\"";
        } else {
            command = elevate + " env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY " + editor + " \"" + filePath + "\"";
        }
    } else {
        if (isEditorThatElevates && !isRoot) {
            command = editor + " \"" + filePath + "\"";
        } else if (isRoot && isEditorThatElevates) {
            command = elevate + " --user $(logname) " + editor + " \"" + filePath + "\"";
        } else if (isCliEditor) {
            command = "x-terminal-emulator -e " + editor + " \"" + filePath + "\"";
        } else {
            command = editor + " \"" + filePath + "\"";
        }
    }

    if (debug) {
        qDebug() << "Final command:" << command;
    }

    bool started = QProcess::startDetached("sh", QStringList() << "-c" << command);

    if (!started) {
        if (debug) {
            qDebug() << "MainWindow: Failed to start editor with command:" << command;
        }

        // Fallback to simple featherpad launch
        QString fallbackCommand = needsElevation ? elevate + " featherpad " + filePath : "featherpad " + filePath;
        started = QProcess::startDetached("sh", QStringList() << "-c" << fallbackCommand);

        if (!started) {
            QMessageBox::warning(this, tr("Editor Error"), tr("Cannot start editor for file: %1").arg(filePath));
        }
    }
    show();
}

void MainWindow::pushAbout_clicked()
{
    hide();
    const QString url = "file:///usr/share/doc/mx-conky/license.html";
    QMessageBox msgBox(QMessageBox::NoIcon, tr("About MX Conky"),
                       "<p align=\"center\"><b><h2>" + tr("MX Conky") + "</h2></b></p><p align=\"center\">"
                           + tr("Version: ") + QCoreApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
                           + tr("GUI program for configuring Conky in MX Linux")
                           + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br "
                             "/></p><p align=\"center\">"
                           + tr("Copyright (c) MX Linux") + "<br /><br /></p>");
    auto *btnLicense = msgBox.addButton(tr("License"), QMessageBox::HelpRole);
    auto *btnChangelog = msgBox.addButton(tr("Changelog"), QMessageBox::HelpRole);
    auto *btnCancel = msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
    btnCancel->setIcon(QIcon::fromTheme("window-close"));

    msgBox.exec();

    if (msgBox.clickedButton() == btnLicense) {
        const QString executablePath = QStandardPaths::findExecutable("mx-viewer");
        const QString cmd_str = executablePath.isEmpty() ? "xdg-open" : "mx-viewer";
        QProcess::startDetached(cmd_str, {url});
    } else if (msgBox.clickedButton() == btnChangelog) {
        auto *changelog = new QDialog(this);
        changelog->setWindowTitle(tr("Changelog"));
        const auto height {500};
        const auto width {600};
        changelog->resize(width, height);

        auto *text = new QTextEdit;
        text->setReadOnly(true);
        Cmd cmd;
        text->setText(cmd.getCmdOut("zless /usr/share/doc/"
                                    + QFileInfo(QCoreApplication::applicationFilePath()).fileName() + "/changelog.gz"));

        auto *btnClose = new QPushButton(tr("&Close"));
        btnClose->setIcon(QIcon::fromTheme("window-close"));
        connect(btnClose, &QPushButton::clicked, changelog, &QDialog::close);

        auto *layout = new QVBoxLayout;
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog->setLayout(layout);
        changelog->exec();
    }
    show();
}

void MainWindow::pushHelp_clicked()
{
    QString url = "/usr/share/doc/mx-conky/mx-conky.html";
    qDebug() << "MainWindow: Opening help URL:" << url;

    // Check if mx-viewer exists using synchronous approach
    QProcess checkProcess;
    qDebug() << "MainWindow::pushHelp_clicked: Creating which QProcess object";
    checkProcess.setProgram("which");
    checkProcess.setArguments(QStringList() << "mx-viewer");
    checkProcess.start();

    bool started = false;
    if (checkProcess.waitForFinished(3000)) {
        qDebug() << "MainWindow: which command finished with exit code:" << checkProcess.exitCode();

        if (checkProcess.exitCode() == 0) {
            qDebug() << "MainWindow: Using mx-viewer for help";
            started = QProcess::startDetached("mx-viewer", QStringList() << url << tr("MX Conky Help"));
        } else {
            qDebug() << "MainWindow: Using xdg-open for help";
            started = QProcess::startDetached("xdg-open", QStringList() << url);
        }

        // Ensure process is fully finished and cleaned up
        checkProcess.kill();
        checkProcess.waitForFinished(1000);
    } else {
        qDebug() << "MainWindow: which command timed out, using xdg-open as fallback";
        checkProcess.kill();
        checkProcess.waitForFinished(1000);
        started = QProcess::startDetached("xdg-open", QStringList() << url);
    }
    qDebug() << "MainWindow::pushHelp_clicked: Destroying which QProcess object";

    if (started) {
        qDebug() << "MainWindow: Help viewer started successfully";
    } else {
        qDebug() << "MainWindow: Failed to start help viewer";
    }
}

void MainWindow::pushCM_clicked()
{
    QString command = "command -v conky-manager && conky-manager || command -v conky-manager2 && conky-manager2";
    QProcess::startDetached("bash", {"-c", command});
}

void MainWindow::onFilterChanged()
{
    if (!m_conkyListWidget) {
        return;
    }
    QString filter = m_filterComboBox->currentText();
    
    // Convert translated text to internal key
    QString filterKey;
    if (filter == tr("All")) {
        filterKey = "All";
    } else if (filter == tr("Running")) {
        filterKey = "Running";
    } else if (filter == tr("Stopped")) {
        filterKey = "Stopped";
    } else {
        filterKey = filter; // For folder-based filters
    }
    
    m_conkyListWidget->setStatusFilter(filterKey);
}

void MainWindow::onSearchTextChanged()
{
    if (!m_conkyListWidget) {
        return;
    }
    QString searchText = m_searchLineEdit->text();
    m_conkyListWidget->setSearchText(searchText);
}

void MainWindow::focusSearchField()
{
    if (m_searchLineEdit) {
        m_searchLineEdit->setFocus();
        m_searchLineEdit->selectAll();
    }
}

void MainWindow::populateFilterComboBox()
{
    if (!m_filterComboBox || !m_conkyManager) {
        return;
    }

    // Clear existing items and add status-based filters
    m_filterComboBox->clear();
    m_filterComboBox->addItem(tr("All"));
    m_filterComboBox->addItem(tr("Running"));
    m_filterComboBox->addItem(tr("Stopped"));

    // Add folder-based filters from search paths
    QStringList searchPaths = m_conkyManager->searchPaths();
    for (const QString &path : searchPaths) {
        QFileInfo pathInfo(path);
        QString folderName = pathInfo.fileName();
        if (folderName.isEmpty()) {
            folderName = pathInfo.absolutePath().split('/').last();
        }

        // Include parent directory for better clarity (e.g., "mx-conky-data/themes", "~/.conky")
        QFileInfo parentInfo(pathInfo.absolutePath());
        QString parentFolderName = parentInfo.fileName();
        QString displayName;

        if (path.startsWith(QDir::homePath())) {
            // Replace home path with ~ for readability
            displayName = QString("~") + path.mid(QDir::homePath().length());
        } else if (!parentFolderName.isEmpty() && parentFolderName != folderName) {
            displayName = QString("%1/%2").arg(parentFolderName, folderName);
        } else {
            displayName = folderName;
        }

        m_filterComboBox->addItem(displayName);
    }
}

void MainWindow::closeEvent(QCloseEvent * /*event*/)
{
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitter", m_splitter->saveState());
}
