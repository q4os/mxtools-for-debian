/**********************************************************************
 *  MainWindow.h
 **********************************************************************
 * Copyright (C) 2017 MX Authors
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
#pragma once

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMovie>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "cmd.h"
#include "conkycustomizedialog.h"
#include "conkylistwidget.h"
#include "conkymanager.h"
#include "previewdialog.h"
#include "settingsdialog.h"

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void pushHelp_clicked();
    void closeEvent(QCloseEvent *event) override;
    void onConkyItemsLoaded();
    void onCustomizeRequested(ConkyItem *item);
    void onEditRequested(ConkyItem *item);
    void onDeleteRequested(ConkyItem *item);
    void onItemSelectionChanged(ConkyItem *item);
    void onPreviewImageLoaded(const QSize imageSize);
    void onPreviewsClicked();
    void onRefreshClicked();
    void onSettingsClicked();
    void onStartAllClicked();
    void onStopAllClicked();
    void pushAbout_clicked();
    void pushCM_clicked();

    // Filter and search slots
    void onFilterChanged();
    void onSearchTextChanged();
    void focusSearchField();

private:
    QSettings settings;

    ConkyListWidget *m_conkyListWidget;
    ConkyManager *m_conkyManager;
    ConkyPreviewWidget *m_previewWidget;
    QSplitter *m_splitter;

    QPushButton *m_aboutButton;
    QPushButton *m_closeButton;
    QPushButton *m_helpButton;
    QPushButton *m_previewsButton;
    QPushButton *m_refreshButton;
    QPushButton *m_settingsButton;
    QPushButton *m_startAllButton;
    QPushButton *m_stopAllButton;

    // Filter and search widgets
    QComboBox *m_filterComboBox;
    QLineEdit *m_searchLineEdit;

    // Loading state widgets
    QLabel *m_loadingLabel;
    QMovie *m_loadingMovie;
    QStackedWidget *m_stackedWidget;
    QWidget *m_loadingWidget;
    QWidget *m_mainWidget;

    // Session tracking
    bool m_copyDialogShownThisSession;

    void editConkyFile(const QString &filePath);
    void populateFilterComboBox();
    void setConnections();
    void setupLoadingWidget();
    void setupMainWidget();
    void setupUI();
};
