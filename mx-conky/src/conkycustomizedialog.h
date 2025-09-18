/**********************************************************************
 *  ConkyCustomizeDialog.h
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
#pragma once

#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "cmd.h"

class ConkyCustomizeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConkyCustomizeDialog(const QString &filePath, QWidget *parent = nullptr);
    ~ConkyCustomizeDialog() override;

private slots:
    void closeEvent(QCloseEvent *event) override;
    void pushColorButton_clicked(int colorIndex);
    void pushDefaultColor_clicked();
    void pushRestore_clicked();
    void pushToggleOn_clicked();
    void radioAllDesktops_clicked();
    void radioDayLong_clicked();
    void radioDayShort_clicked();
    void radioDesktop1_clicked();
    void radioMonthLong_clicked();
    void radioMonthShort_clicked();

    // New tab slots
    void onAlignmentChanged();
    void onGapChanged();
    void onSizeChanged();
    void onTransparencyChanged();
    void onTimeFormatChanged();
    void onNetworkDeviceChanged();
    void onWifiClicked();
    void onLanClicked();
    void onBackgroundColorClicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Cmd cmd;
    QSettings settings;
    QString file_content;
    QString file_name;
    bool modified = false;

    // Change tracking
    struct ChangedFlags {
        bool alignment = false;
        bool gaps = false;
        bool size = false;
        bool transparency = false;
        bool timeFormat = false;
        bool networkDevice = false;
    } changedFlags;

    struct InitialValues {
        QString alignment;
        int gapX, gapY;
        int minWidth, minHeight, heightPadding;
        QString transparencyType;
        int opacity;
        QString backgroundColor;
        QString timeFormat;
        QString networkDevice;
    } initialValues;

    bool is_lua_format = false;
    bool conky_format_detected = false;
    bool debug = false;

    // UI Components
    QTabWidget *tabWidget;
    QScrollArea *scrollAreaColors;
    QWidget *scrollAreaWidgetContents;
    QGroupBox *groupBoxRun;
    QGroupBox *groupBoxDrag;
    QGroupBox *groupBoxDesktop;
    QGroupBox *groupBoxFormat;
    QGroupBox *groupBoxColors;

    // Location tab
    QComboBox *cmbAlignment;
    QSpinBox *spinGapX;
    QSpinBox *spinGapY;

    // Size tab
    QSpinBox *spinMinWidth;
    QSpinBox *spinMinHeight;
    QSpinBox *spinHeightPadding;

    // Transparency tab
    QComboBox *cmbTransparencyType;
    QSpinBox *spinOpacity;
    QPushButton *btnBackgroundColor;
    QWidget *widgetBackgroundColor;

    // Time tab
    QComboBox *cmbTimeFormat;
    QLabel *lblTimeNotFound;

    // Network tab
    QLineEdit *txtNetworkDevice;
    QPushButton *btnWifi;
    QPushButton *btnLan;
    QLabel *lblNetworkNotFound;

    QPushButton *pushToggleOn;
    QPushButton *pushRestore;
    QRadioButton *radioDesktop1;
    QRadioButton *radioAllDesktops;
    QRadioButton *radioDayLong;
    QRadioButton *radioDayShort;
    QRadioButton *radioMonthLong;
    QRadioButton *radioMonthShort;

    QWidget *widgetDefaultColor;
    QWidget *widgetColor0;
    QWidget *widgetColor1;
    QWidget *widgetColor2;
    QWidget *widgetColor3;
    QWidget *widgetColor4;
    QWidget *widgetColor5;
    QWidget *widgetColor6;
    QWidget *widgetColor7;
    QWidget *widgetColor8;
    QWidget *widgetColor9;

    QToolButton *pushDefaultColor;
    QToolButton *pushColor0;
    QToolButton *pushColor1;
    QToolButton *pushColor2;
    QToolButton *pushColor3;
    QToolButton *pushColor4;
    QToolButton *pushColor5;
    QToolButton *pushColor6;
    QToolButton *pushColor7;
    QToolButton *pushColor8;
    QToolButton *pushColor9;

    QFrame *frameDefault;
    QFrame *frame0;
    QFrame *frame1;
    QFrame *frame2;
    QFrame *frame3;
    QFrame *frame4;
    QFrame *frame5;
    QFrame *frame6;
    QFrame *frame7;
    QFrame *frame8;
    QFrame *frame9;

    // Regexp patterns
    QString capture_lua_color;
    QString capture_old_color;
    QString lua_comment_end;
    QString lua_comment_line;
    QString lua_comment_start;
    QString lua_config;
    QString lua_format;
    QString old_comment_line;
    QString old_format;

    QString capture_lua_owh = QStringLiteral(R"(^(\s*own_window_hints\s*=\s*')([^']*)('.*))");
    QString capture_old_owh = QStringLiteral(R"(^(\s*own_window_hints\s+)([^\s]*)(.*))");

    QString block_comment_start = QStringLiteral("--[[");
    QString block_comment_end = QStringLiteral("]]");

    QRegularExpression regexp_lua_color;

    [[nodiscard]] bool readFile(const QString &file_name);
    [[nodiscard]] static QColor strToColor(const QString &colorstr);
    bool checkConkyRunning();
    void detectConkyFormat();
    void parseContent();
    void pickColor(QWidget *widget);
    void refresh();
    void saveBackup();
    void setColor(QWidget *widget, const QColor &color);
    void setConnections();
    void setupUI();
    void writeColor(QWidget *widget, const QColor &color);
    void writeFile(const QFile &file, const QString &content);
    void writeFile(const QString &fileName, const QString &content);
    bool writeFileWithElevation(const QString &fileName, const QString &content);
    bool copyFileWithElevation(const QString &sourceFile, const QString &destFile);
    void writeConfigValue(const QString &key, const QString &value, bool quoted = false);
    int countHeightPadding();
    void updateHeightPadding(int paddingLines);

    // Change tracking methods
    void saveInitialValues();
    bool checkForChanges();

    // Apply methods that actually write changes
    void applyAlignmentChanges();
    void applyGapChanges();
    void applySizeChanges();
    void applyTransparencyChanges();
    void applyTimeFormatChanges();
    void applyNetworkDeviceChanges();

    // New tab creation methods
    void createLocationTab();
    void createNetworkTab();
    void createSizeTab();
    void createTimeTab();
    void createTransparencyTab();
};
