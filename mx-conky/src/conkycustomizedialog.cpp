/**********************************************************************
 *  ConkyCustomizeDialog.cpp
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

#include "conkycustomizedialog.h"
#include <QColorDialog>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QLineEdit>
#include <QMouseEvent>
#include <QSpacerItem>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>

ConkyCustomizeDialog::ConkyCustomizeDialog(const QString &filePath, QWidget *parent)
    : QDialog(parent),
      file_name(filePath),
      capture_lua_color(
          R"(^(?<before>(.*\]\])?\s*(?<color_item>default_color|color\d)(?:\s*=\s*[\"\']))(?:#?)(?<color_value>[[:alnum:]]+)(?<after>(?:[\"\']).*))"),
      capture_old_color(
          R"(^(?<before>\s*(?<color_item>default_color|color\d)(?:\s+))(?:#?)(?<color_value>[[:alnum:]]+)(?<after>.*))"),
      lua_comment_end(R"(^\s*\]\])"),
      lua_comment_line(R"(^\s*--)"),
      lua_comment_start(R"(^\s*--\[\[)"),
      lua_config(R"(^\s*(conky.config\s*=\s*{))"),
      lua_format(R"(^\s*(--|conky.config\s*=\s*{|conky.text\s*=\s*{))"),
      old_comment_line(R"(^\s*#)"),
      old_format(R"(^\s*#|^TEXT$)")
{
    debug = !QProcessEnvironment::systemEnvironment().value("DEBUG").isEmpty();

    setWindowTitle(tr("Customize Conky - %1").arg(QFileInfo(filePath).fileName()));
    setWindowFlags(Qt::Window);
    setupUI();
    setConnections();
    refresh();

    adjustSize();
    setMinimumWidth(600); // Ensure reasonable minimum width for tab content
}

ConkyCustomizeDialog::~ConkyCustomizeDialog()
{
    saveBackup();
}

void ConkyCustomizeDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

    // Create Launch group box (common to all tabs)
    groupBoxRun = new QGroupBox(tr("Launch"));
    groupBoxRun->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto *runLayout = new QGridLayout(groupBoxRun);

    pushToggleOn = new QPushButton(tr("Start"));
    pushToggleOn->setIcon(QIcon::fromTheme("media-playback-start"));
    pushRestore = new QPushButton(tr("Undo"));
    pushRestore->setIcon(QIcon::fromTheme("edit-undo"));

    runLayout->addWidget(pushToggleOn, 0, 0);
    runLayout->addWidget(pushRestore, 0, 1);

    // Create tab widget
    tabWidget = new QTabWidget;
    tabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // Create scroll area for colors
    scrollAreaColors = new QScrollArea;
    scrollAreaColors->setFrameShape(QFrame::Box);
    scrollAreaColors->setFrameShadow(QFrame::Sunken);
    scrollAreaColors->setWidgetResizable(true);
    scrollAreaColors->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    scrollAreaColors->setMaximumHeight(400); // Prevent excessive height

    scrollAreaWidgetContents = new QWidget;
    scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    auto *gridLayout = new QGridLayout(scrollAreaWidgetContents);

    // Colors Group
    groupBoxColors = new QGroupBox(tr("Colors"));
    auto *colorsLayout = new QGridLayout(groupBoxColors);

    // Create color frames and widgets
    QList<QWidget **> colorWidgets
        = {&widgetDefaultColor, &widgetColor0, &widgetColor1, &widgetColor2, &widgetColor3, &widgetColor4,
           &widgetColor5,       &widgetColor6, &widgetColor7, &widgetColor8, &widgetColor9};

    QList<QToolButton **> colorButtons
        = {&pushDefaultColor, &pushColor0, &pushColor1, &pushColor2, &pushColor3, &pushColor4,
           &pushColor5,       &pushColor6, &pushColor7, &pushColor8, &pushColor9};

    QList<QFrame **> colorFrames
        = {&frameDefault, &frame0, &frame1, &frame2, &frame3, &frame4, &frame5, &frame6, &frame7, &frame8, &frame9};

    QStringList colorLabels = {tr("Default"), tr("Color0"), tr("Color1"), tr("Color2"), tr("Color3"), tr("Color4"),
                               tr("Color5"),  tr("Color6"), tr("Color7"), tr("Color8"), tr("Color9")};

    for (int i = 0; i < colorWidgets.size(); ++i) {
        auto *frame = new QFrame;
        frame->setFrameShape(QFrame::NoFrame);
        frame->hide(); // Initially hidden
        *colorFrames[i] = frame;

        auto *frameLayout = new QHBoxLayout(frame);
        frameLayout->setContentsMargins(0, 0, 0, 0);

        frameLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

        auto *label = new QLabel(colorLabels[i]);
        label->setMinimumWidth(110);
        label->setAlignment(Qt::AlignCenter);
        frameLayout->addWidget(label);

        frameLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Minimum));

        auto *colorWidget = new QWidget;
        colorWidget->setMinimumSize(100, 20);
        colorWidget->setStyleSheet("border: 1px solid black;");
        colorWidget->setCursor(Qt::PointingHandCursor);

        // Set object name for findChild to work
        if (i == 0) {
            colorWidget->setObjectName("widgetDefaultColor");
        } else {
            colorWidget->setObjectName(QString("widgetColor%1").arg(i - 1));
        }

        *colorWidgets[i] = colorWidget;

        // Install event filter to make color widget clickable
        colorWidget->installEventFilter(this);

        frameLayout->addWidget(colorWidget);

        frameLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Minimum));

        auto *button = new QToolButton;
        button->setText("...");
        if (i == 0) {
            button->setObjectName("pushDefaultColor");
        } else {
            button->setObjectName(QString("pushColor%1").arg(i - 1));
        }
        *colorButtons[i] = button;
        frameLayout->addWidget(button);

        frameLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

        colorsLayout->addWidget(frame, i, 0);
    }

    // Add groups to main layout
    gridLayout->addWidget(groupBoxColors, 0, 0);

    scrollAreaColors->setWidget(scrollAreaWidgetContents);

    // Create and add advanced tabs in new order
    createLocationTab();
    createSizeTab();
    tabWidget->addTab(scrollAreaColors, tr("Colors"));
    createTransparencyTab();
    createTimeTab();
    createNetworkTab();

    // Bottom buttons
    auto *buttonLayout = new QHBoxLayout;
    auto *closeButton = new QPushButton(tr("Close"));
    closeButton->setIcon(QIcon::fromTheme("window-close"));
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    // Add Launch group at top, then tabs, then buttons
    mainLayout->addWidget(groupBoxRun);
    mainLayout->addWidget(tabWidget, 1); // Give tab widget stretch factor of 1
    mainLayout->addLayout(buttonLayout);
}

void ConkyCustomizeDialog::setConnections()
{
    connect(pushToggleOn, &QPushButton::clicked, this, &ConkyCustomizeDialog::pushToggleOn_clicked);
    connect(pushRestore, &QPushButton::clicked, this, &ConkyCustomizeDialog::pushRestore_clicked);
    connect(radioAllDesktops, &QRadioButton::clicked, this, &ConkyCustomizeDialog::radioAllDesktops_clicked);
    connect(radioDayLong, &QRadioButton::clicked, this, &ConkyCustomizeDialog::radioDayLong_clicked);
    connect(radioDayShort, &QRadioButton::clicked, this, &ConkyCustomizeDialog::radioDayShort_clicked);
    connect(radioDesktop1, &QRadioButton::clicked, this, &ConkyCustomizeDialog::radioDesktop1_clicked);
    connect(radioMonthLong, &QRadioButton::clicked, this, &ConkyCustomizeDialog::radioMonthLong_clicked);
    connect(radioMonthShort, &QRadioButton::clicked, this, &ConkyCustomizeDialog::radioMonthShort_clicked);
    connect(pushDefaultColor, &QToolButton::clicked, this, &ConkyCustomizeDialog::pushDefaultColor_clicked);

    for (int i = 0; i < 10; ++i) {
        auto *colorButton = groupBoxColors->findChild<QToolButton *>(QString("pushColor%1").arg(i));
        if (colorButton) {
            connect(colorButton, &QToolButton::clicked, this, [this, i]() { pushColorButton_clicked(i); });
        }
    }

    // New tab connections
    if (cmbAlignment) {
        connect(cmbAlignment, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &ConkyCustomizeDialog::onAlignmentChanged);
    }

    if (spinGapX) {
        connect(spinGapX, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConkyCustomizeDialog::onGapChanged);
    }

    if (spinGapY) {
        connect(spinGapY, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConkyCustomizeDialog::onGapChanged);
    }

    if (spinMinWidth) {
        connect(spinMinWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConkyCustomizeDialog::onSizeChanged);
    }

    if (spinMinHeight) {
        connect(spinMinHeight, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConkyCustomizeDialog::onSizeChanged);
    }

    if (spinHeightPadding) {
        connect(spinHeightPadding, QOverload<int>::of(&QSpinBox::valueChanged), this,
                &ConkyCustomizeDialog::onSizeChanged);
    }

    if (cmbTransparencyType) {
        connect(cmbTransparencyType, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &ConkyCustomizeDialog::onTransparencyChanged);
    }

    if (spinOpacity) {
        connect(spinOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this,
                &ConkyCustomizeDialog::onTransparencyChanged);
    }

    if (btnBackgroundColor) {
        connect(btnBackgroundColor, &QPushButton::clicked, this, &ConkyCustomizeDialog::onBackgroundColorClicked);
    }

    if (cmbTimeFormat) {
        connect(cmbTimeFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &ConkyCustomizeDialog::onTimeFormatChanged);
    }

    if (txtNetworkDevice) {
        connect(txtNetworkDevice, &QLineEdit::textChanged, this, &ConkyCustomizeDialog::onNetworkDeviceChanged);
    }

    if (btnWifi) {
        connect(btnWifi, &QPushButton::clicked, this, &ConkyCustomizeDialog::onWifiClicked);
    }

    if (btnLan) {
        connect(btnLan, &QPushButton::clicked, this, &ConkyCustomizeDialog::onLanClicked);
    }
}

bool ConkyCustomizeDialog::readFile(const QString &file_name)
{
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file: " << file.fileName();
        return false;
    }
    file_content = file.readAll().trimmed();
    file.close();
    return true;
}

void ConkyCustomizeDialog::refresh()
{
    modified = false;

    // Hide all color frames by default, only show those specified in the config file
    const QList<QFrame *> frames
        = {frameDefault, frame0, frame1, frame2, frame3, frame4, frame5, frame6, frame7, frame8, frame9};
    for (QFrame *frame : frames) {
        frame->hide();
    }

    checkConkyRunning();

    // Backup the current configuration file
    QString bakFileName = file_name + ".bak";
    QFile::remove(bakFileName);

    // Try normal copy first, then elevation if needed
    if (!QFile::copy(file_name, bakFileName)) {
        QFileInfo fileInfo(file_name);
        QFileInfo dirInfo(fileInfo.absolutePath());

        // Check if we need elevation for backup directory
        if (!dirInfo.isWritable()) {
            if (!copyFileWithElevation(file_name, bakFileName)) {
                qDebug() << "Failed to create backup file with elevation:" << bakFileName;
            }
        }
    }

    // Read the configuration file and parse its content
    if (readFile(file_name)) {
        detectConkyFormat();
        parseContent();
    }
}

bool ConkyCustomizeDialog::checkConkyRunning()
{
    // Skip if cmd is already running to avoid conflicts
    if (cmd.state() != QProcess::NotRunning) {
        return pushToggleOn->text() == tr("Stop"); // Return current state
    }

    QString output = cmd.getCmdOut(QString("pgrep -u %1 -x conky").arg(qgetenv("USER")), true);
    bool isRunning = !output.trimmed().isEmpty();
    pushToggleOn->setText(isRunning ? tr("Stop") : tr("Start"));
    pushToggleOn->setIcon(QIcon::fromTheme(isRunning ? "media-playback-stop" : "media-playback-start"));
    return isRunning;
}

void ConkyCustomizeDialog::detectConkyFormat()
{
    QRegularExpression lua_format_regexp(lua_format);
    QRegularExpression old_format_regexp(old_format);
    const QStringList list = file_content.split('\n');

    if (debug) {
        qDebug() << "Detecting conky format: " << file_name;
    }

    conky_format_detected = false;

    for (const QString &row : list) {
        if (lua_format_regexp.match(row).hasMatch()) {
            is_lua_format = true;
            conky_format_detected = true;
            if (debug) {
                qDebug() << "Conky format detected: 'lua-format' in " << file_name;
            }
            return;
        }
        if (old_format_regexp.match(row).hasMatch()) {
            is_lua_format = false;
            conky_format_detected = true;
            if (debug) {
                qDebug() << "Conky format detected: 'old-format' in " << file_name;
            }
            return;
        }
    }
}

void ConkyCustomizeDialog::parseContent()
{
    QRegularExpression regexp_color
        = is_lua_format ? QRegularExpression(capture_lua_color) : QRegularExpression(capture_old_color);
    QString comment_sep = is_lua_format ? "--" : "#";
    bool own_window_hints_found = false;
    const QStringList list = file_content.split('\n', Qt::SkipEmptyParts);

    if (debug) {
        qDebug() << "Parsing content: " + file_name;
    }

    bool lua_block_comment = false;
    for (const QString &row : list) {
        QString trow = row.trimmed();
        if (is_lua_format) {
            if (lua_block_comment) {
                if (trow.endsWith(block_comment_end)) {
                    lua_block_comment = false;
                    if (debug) {
                        qDebug() << "Lua block comment end 'ENDS WITH LINE' found";
                    }
                    continue;
                }
                if (trow.contains(block_comment_end)) {
                    lua_block_comment = false;
                    QStringList ltrow = trow.split(block_comment_end);
                    ltrow.removeFirst();
                    trow = ltrow.join(block_comment_end);
                    trow = trow.trimmed();
                    if (debug) {
                        qDebug() << "Lua block comment end CONTAINS line found: after ]]: " << trow;
                    }
                } else {
                    continue;
                }
            }
            if (!lua_block_comment) {
                if (trow.startsWith(block_comment_start)) {
                    if (debug) {
                        qDebug() << "Lua block comment 'STARTS WITH LINE' found";
                    }
                    lua_block_comment = true;
                    continue;
                }
                if (trow.contains(block_comment_start)) {
                    lua_block_comment = true;
                    QStringList ltrow = trow.split(block_comment_start);
                    trow = ltrow[0];
                    trow = trow.trimmed();
                    if (debug) {
                        qDebug() << "Lua block comment start CONTAINS line found: before start --[[: " << trow;
                    }
                }
            }
        }

        // Comment line
        if (trow.startsWith(comment_sep)) {
            continue;
        }

        // Remove inline comments
        if (trow.contains(comment_sep)) {
            trow = trow.split(comment_sep).first().trimmed();
        }

        // Match color lines
        auto match_color = regexp_color.match(trow);
        if (match_color.hasMatch()) {
            const QString color_item = match_color.captured("color_item");
            const QString color_value = match_color.captured("color_value");
            QWidget *colorWidget = nullptr;

            if (color_item == "default_color") {
                colorWidget = widgetDefaultColor;
            } else if (color_item.startsWith("color")) {
                const int index = color_item.mid(5).toInt();
                colorWidget = groupBoxColors->findChild<QWidget *>(QString("widgetColor%1").arg(index));
            }
            if (colorWidget) {
                setColor(colorWidget, strToColor(color_value));
            }
            continue;
        }

        // Handle desktop configuration
        if (trow.startsWith("own_window_hints")) {
            own_window_hints_found = true;
            if (debug) {
                qDebug() << "own_window_hints line found:" << trow;
            }
            radioAllDesktops->setChecked(trow.contains("sticky"));
            radioDesktop1->setChecked(!trow.contains("sticky"));
            continue;
        }

        // Parse alignment
        if (trow.startsWith("alignment") || trow.contains("alignment =")) {
            QRegularExpression alignRe(is_lua_format ? R"(alignment\s*=\s*["']([^"']+)["'])"
                                                     : R"(alignment\s+([\w_]+))");
            auto match = alignRe.match(trow);
            if (match.hasMatch()) {
                QString alignment = match.captured(1);
                for (int i = 0; i < cmbAlignment->count(); ++i) {
                    if (cmbAlignment->itemData(i).toString() == alignment) {
                        cmbAlignment->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }

        // Parse gap_x
        if (trow.startsWith("gap_x") || trow.contains("gap_x =")) {
            QRegularExpression gapXRe(is_lua_format ? R"(gap_x\s*=\s*([\d-]+))" : R"(gap_x\s+([\d-]+))");
            auto match = gapXRe.match(trow);
            if (match.hasMatch()) {
                spinGapX->setValue(match.captured(1).toInt());
            }
        }

        // Parse gap_y
        if (trow.startsWith("gap_y") || trow.contains("gap_y =")) {
            QRegularExpression gapYRe(is_lua_format ? R"(gap_y\s*=\s*([\d-]+))" : R"(gap_y\s+([\d-]+))");
            auto match = gapYRe.match(trow);
            if (match.hasMatch()) {
                spinGapY->setValue(match.captured(1).toInt());
            }
        }

        // Parse minimum_width
        if (trow.startsWith("minimum_width") || trow.contains("minimum_width =")) {
            QRegularExpression minWidthRe(is_lua_format ? R"(minimum_width\s*=\s*([\d]+))"
                                                        : R"(minimum_width\s+([\d]+))");
            auto match = minWidthRe.match(trow);
            if (match.hasMatch()) {
                spinMinWidth->setValue(match.captured(1).toInt());
            }
        }

        // Parse minimum_height
        if (trow.startsWith("minimum_height") || trow.contains("minimum_height =")) {
            QRegularExpression minHeightRe(is_lua_format ? R"(minimum_height\s*=\s*([\d]+))"
                                                         : R"(minimum_height\s+([\d]+))");
            auto match = minHeightRe.match(trow);
            if (match.hasMatch()) {
                spinMinHeight->setValue(match.captured(1).toInt());
            }
        }

        // Set day/month format
        if (trow.contains("%A")) {
            radioDayLong->setChecked(true);
        } else if (trow.contains("%a")) {
            radioDayShort->setChecked(true);
        }

        if (trow.contains("%B")) {
            radioMonthLong->setChecked(true);
        } else if (trow.contains("%b")) {
            radioMonthShort->setChecked(true);
        }

        // Parse network interface
        if (trow.contains("${if_up") && txtNetworkDevice) {
            QRegularExpression netRe(R"(\$\{if_up\s+([\w\d]+)\})");
            auto match = netRe.match(trow);
            if (match.hasMatch()) {
                txtNetworkDevice->setText(match.captured(1));
            }
        }
    }

    if (!own_window_hints_found) {
        radioDesktop1->setChecked(true);
    }

    // Parse transparency settings
    if (cmbTransparencyType && spinOpacity) {
        // Create helper function to check boolean values in main conky config (not fluxbox overrides)
        auto checkBooleanValue = [&](const QString &key) -> bool {
            // Split content into main config and fluxbox sections
            QStringList sections = file_content.split("-- fluxbox adjustment");
            QString mainConfig = sections.isEmpty() ? file_content : sections[0];

            QString truePattern = is_lua_format ? QString("%1\\s*=\\s*(true|yes|1)\\s*,?").arg(key)
                                                : QString("%1\\s+(true|yes|1)").arg(key);
            return mainConfig.contains(QRegularExpression(truePattern, QRegularExpression::CaseInsensitiveOption));
        };

        bool ownWindow = checkBooleanValue("own_window");
        bool ownWindowTransparent = checkBooleanValue("own_window_transparent");
        bool ownWindowArgb = checkBooleanValue("own_window_argb_visual");

        // Determine transparency type based on combinations
        if (debug) {
            qDebug() << "Transparency detection: ownWindow=" << ownWindow
                     << "ownWindowTransparent=" << ownWindowTransparent << "ownWindowArgb=" << ownWindowArgb;
        }

        // Check for opaque first: both own_window and own_window_transparent are false
        if (!ownWindow && !ownWindowTransparent) {
            cmbTransparencyType->setCurrentIndex(cmbTransparencyType->findData("opaque"));
            if (debug) {
                qDebug() << "Detected: opaque";
            }
        } else if (ownWindow && ownWindowTransparent && !ownWindowArgb) {
            cmbTransparencyType->setCurrentIndex(cmbTransparencyType->findData("trans"));
            if (debug) {
                qDebug() << "Detected: trans";
            }
        } else if (ownWindow && !ownWindowTransparent && ownWindowArgb) {
            cmbTransparencyType->setCurrentIndex(cmbTransparencyType->findData("semi"));
            if (debug) {
                qDebug() << "Detected: semi";
            }
        } else if (!ownWindow && ownWindowTransparent && !ownWindowArgb) {
            cmbTransparencyType->setCurrentIndex(cmbTransparencyType->findData("pseudo"));
            if (debug) {
                qDebug() << "Detected: pseudo";
            }
        } else {
            // Fallback case
            cmbTransparencyType->setCurrentIndex(cmbTransparencyType->findData("opaque"));
            if (debug) {
                qDebug() << "Detected: fallback to opaque";
            }
        }

        // Parse opacity value from own_window_argb_value using format-specific pattern
        QString argbValuePattern
            = is_lua_format ? "own_window_argb_value\\s*=\\s*(\\d+)\\s*,?" : "own_window_argb_value\\s+(\\d+)";
        QRegularExpression argbValueRegex(argbValuePattern, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = argbValueRegex.match(file_content);
        if (match.hasMatch()) {
            int argbValue = match.captured(1).toInt();
            int percentage = static_cast<int>((argbValue / 255.0) * 100);
            spinOpacity->setValue(percentage);
        } else {
            spinOpacity->setValue(100); // Default to fully opaque
        }

        // Parse background color from own_window_colour using format-specific pattern
        if (widgetBackgroundColor) {
            QString colorPattern = is_lua_format ? "own_window_colour\\s*=\\s*['\"]?([a-fA-F0-9]{6})['\"]?\\s*,?"
                                                 : "own_window_colour\\s+([a-fA-F0-9]{6})";
            QRegularExpression colorRegex(colorPattern, QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch colorMatch = colorRegex.match(file_content);
            if (colorMatch.hasMatch()) {
                QString colorHex = "#" + colorMatch.captured(1);
                QColor bgColor(colorHex);
                if (bgColor.isValid()) {
                    widgetBackgroundColor->setStyleSheet(
                        QString("border: 1px solid black; background-color: %1;").arg(colorHex));
                }
            }
        }
    }

    // Parse time format (detect 12-hour vs 24-hour format)
    if (cmbTimeFormat) {
        if (file_content.contains("%I") || file_content.contains("%l") || file_content.contains("%p")) {
            cmbTimeFormat->setCurrentIndex(cmbTimeFormat->findData("12"));
        } else if (file_content.contains("%H") || file_content.contains("%k")) {
            cmbTimeFormat->setCurrentIndex(cmbTimeFormat->findData("24"));
        } else {
            // Default to 24-hour if no time format detected
            cmbTimeFormat->setCurrentIndex(cmbTimeFormat->findData("24"));
        }
    }

    // Parse height padding (count empty lines at end of TEXT section)
    if (spinHeightPadding) {
        int paddingLines = countHeightPadding();
        spinHeightPadding->setValue(paddingLines);
    }

    // Save initial values for change tracking
    saveInitialValues();
}

QColor ConkyCustomizeDialog::strToColor(const QString &colorstr)
{
    QColor color(colorstr);
    if (!color.isValid()) {
        color = QColor::fromString('#' + colorstr);
    }
    return color;
}

void ConkyCustomizeDialog::setColor(QWidget *widget, const QColor &color)
{
    if (widget && color.isValid()) {
        widget->parentWidget()->show();
        QPalette pal = widget->palette();
        pal.setColor(QPalette::Window, color);
        widget->setAutoFillBackground(true);
        widget->setPalette(pal);
    }
}

void ConkyCustomizeDialog::pickColor(QWidget *widget)
{
    QColor initialColor = widget->palette().color(QWidget::backgroundRole());
    QColor selectedColor = QColorDialog::getColor(initialColor, this, tr("Select Color"));

    if (selectedColor.isValid()) {
        setColor(widget, selectedColor);
        writeColor(widget, selectedColor);
    }
}

void ConkyCustomizeDialog::writeColor(QWidget *widget, const QColor &color)
{
    QRegularExpression regexp_color
        = is_lua_format ? QRegularExpression(capture_lua_color) : QRegularExpression(capture_old_color);
    QString comment_sep = is_lua_format ? "--" : "#";

    QString item_name
        = (widget->objectName() == "widgetDefaultColor")
              ? "default_color"
              : (widget->objectName().startsWith("widgetColor") ? QString("color%1").arg(widget->objectName().mid(11))
                                                                : QString());

    const QStringList list = file_content.split('\n');
    QStringList new_list;
    new_list.reserve(list.size());
    bool lua_block_comment = false;

    for (const QString &row : list) {
        QString trow = row.trimmed();

        // Handle Lua block comments
        if (is_lua_format) {
            if (lua_block_comment) {
                if (trow.endsWith(block_comment_end)) {
                    lua_block_comment = false;
                    if (debug) {
                        qDebug() << "Lua block comment end 'ENDS WITH LINE' found";
                    }
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_end)) {
                    lua_block_comment = false;
                    QStringList ltrow = trow.split(block_comment_end);
                    ltrow.removeFirst();
                    trow = ltrow.join(block_comment_end);
                    trow = trow.trimmed();
                    if (debug) {
                        qDebug() << "Lua block comment end CONTAINS line found: after ]]: " << trow;
                    }
                } else {
                    new_list << row;
                    continue;
                }
            }
            if (!lua_block_comment) {
                if (trow.startsWith(block_comment_start)) {
                    if (debug) {
                        qDebug() << "Lua block comment 'STARTS WITH LINE' found";
                    }
                    lua_block_comment = true;
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_start)) {
                    lua_block_comment = true;
                    QStringList ltrow = trow.split(block_comment_start);
                    trow = ltrow[0];
                    trow = trow.trimmed();
                    if (debug) {
                        qDebug() << "Lua block comment start CONTAINS line found: before start --[[: " << trow;
                    }
                }
            }
        }
        // Comment line
        if (trow.startsWith(comment_sep)) {
            new_list << row;
            continue;
        }

        // Match and update color lines
        auto match_color = regexp_color.match(row);
        if (match_color.hasMatch() && match_color.captured("color_item") == item_name) {
            QString color_name = is_lua_format ? color.name() : color.name().remove('#');
            new_list << match_color.captured("before") + color_name + match_color.captured("after");
        } else {
            new_list << row;
        }
    }

    file_content = new_list.join('\n') + '\n';
    writeFile(QFile(file_name), file_content);
}

void ConkyCustomizeDialog::writeFile(const QFile &file, const QString &content)
{
    writeFile(file.fileName(), content);
}

void ConkyCustomizeDialog::writeFile(const QString &fileName, const QString &content)
{
    QFileInfo fileInfo(fileName);

    // Check if we have write permission
    if (!fileInfo.isWritable() && fileInfo.exists()) {
        // Try to write with elevation
        if (!writeFileWithElevation(fileName, content)) {
            QMessageBox::warning(this, tr("Permission Denied"),
                                 tr("Cannot write to file: %1\nInsufficient permissions.").arg(fileName));
            return;
        }
    } else {
        // Try normal write first
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            // If normal write fails, try elevation
            if (!writeFileWithElevation(fileName, content)) {
                qDebug() << "Error opening file " + fileName + " for output";
                QMessageBox::warning(this, tr("Write Error"), tr("Cannot write to file: %1").arg(fileName));
                return;
            }
        } else {
            QTextStream out(&file);
            out << content;
            file.close();
        }
    }
    modified = true;
}

bool ConkyCustomizeDialog::writeFileWithElevation(const QString &fileName, const QString &content)
{
    // Create a temporary file with the content
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tempFileName = tempDir + "/mx-conky-temp-" + QString::number(QCoreApplication::applicationPid()) + ".tmp";

    QFile tempFile(tempFileName);
    if (!tempFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot create temporary file:" << tempFileName;
        return false;
    }

    QTextStream out(&tempFile);
    out << content;
    tempFile.close();

    // Use pkexec if available, otherwise fall back to gksu
    QString elevationTool = QFile::exists("/usr/bin/pkexec") ? "pkexec" : (QFile::exists("/usr/bin/gksu") ? "gksu" : "sudo");
    QString command = QString("%1 cp '%2' '%3'").arg(elevationTool, tempFileName, fileName);

    QProcess process;
    process.start("sh", QStringList() << "-c" << command);
    process.waitForFinished();

    bool success = (process.exitCode() == 0);

    // Clean up temporary file
    tempFile.remove();

    if (!success) {
        qDebug() << "Failed to write file with elevation:" << fileName;
    }

    return success;
}

bool ConkyCustomizeDialog::copyFileWithElevation(const QString &sourceFile, const QString &destFile)
{
    // Use pkexec if available, otherwise fall back to gksu
    QString elevationTool = QFile::exists("/usr/bin/pkexec") ? "pkexec" : (QFile::exists("/usr/bin/gksu") ? "gksu" : "sudo");
    QString command = QString("%1 cp '%2' '%3'").arg(elevationTool, sourceFile, destFile);

    QProcess process;
    process.start("sh", QStringList() << "-c" << command);
    process.waitForFinished();

    bool success = (process.exitCode() == 0);

    if (!success) {
        qDebug() << "Failed to copy file with elevation:" << sourceFile << "to" << destFile;
    }

    return success;
}

void ConkyCustomizeDialog::saveBackup()
{
    if (!modified) {
        return;
    }

    int ans = QMessageBox::question(this, tr("Backup Config File"), tr("Do you want to preserve the original file?"));
    if (ans == QMessageBox::Yes) {
        QString time_stamp = QDateTime::currentDateTime().toString("yyMMdd_HHmmss");
        QFileInfo fi(file_name);
        QString new_name = fi.canonicalPath() + '/' + fi.baseName() + '_' + time_stamp;

        if (!fi.completeSuffix().isEmpty()) {
            new_name += '.' + fi.completeSuffix();
        }

        // Try normal copy first, then elevation if needed
        bool copySuccess = QFile::copy(file_name + ".bak", new_name);
        if (!copySuccess) {
            QFileInfo newFileInfo(new_name);
            QFileInfo newDirInfo(newFileInfo.absolutePath());

            // Check if we need elevation for destination directory
            if (!newDirInfo.isWritable()) {
                copySuccess = copyFileWithElevation(file_name + ".bak", new_name);
            }
        }

        if (copySuccess) {
            QMessageBox::information(this, tr("Backed Up Config File"),
                                     tr("The original configuration was backed up to %1").arg(new_name));
        } else {
            QMessageBox::warning(this, tr("Backup Failed"), tr("Failed to create a backup file."));
        }
    }

    QFile::remove(file_name + ".bak");
}

void ConkyCustomizeDialog::pushToggleOn_clicked()
{
    if (checkConkyRunning()) {
        // Stop conky processes - get PIDs and kill them individually
        qDebug() << "ConkyCustomizeDialog: Stopping conky processes";
        QString pidOutput = cmd.getCmdOut(QString("pgrep -u %1 -x conky").arg(qgetenv("USER")), true);

        if (!pidOutput.trimmed().isEmpty()) {
            // Split PIDs by newlines and kill each one individually
            QStringList pids = pidOutput.trimmed().split('\n', Qt::SkipEmptyParts);
            for (const QString &pid : pids) {
                QString pidTrimmed = pid.trimmed();
                if (!pidTrimmed.isEmpty()) {
                    qDebug() << "ConkyCustomizeDialog: Killing PID:" << pidTrimmed;
                    cmd.run(QString("kill %1").arg(pidTrimmed), false);
                }
            }
            qDebug() << "ConkyCustomizeDialog: Killed" << pids.size() << "conky processes";
        }

        // Add a small delay to ensure process is fully stopped before checking
        QTimer::singleShot(500, this, [this]() { checkConkyRunning(); });
    } else {
        // Start conky using startDetached
        qDebug() << "ConkyCustomizeDialog: Starting conky:" << file_name;
        QFileInfo fileInfo(file_name);

        QString program = "conky";
        QStringList arguments;
        arguments << "-c" << file_name;

        bool started = QProcess::startDetached(program, arguments, fileInfo.absolutePath());

        if (started) {
            qDebug() << "ConkyCustomizeDialog: Conky started successfully";
        } else {
            qDebug() << "ConkyCustomizeDialog: Failed to start conky";
        }

        // Add a small delay to ensure process is fully started before checking
        QTimer::singleShot(500, this, [this]() { checkConkyRunning(); });
    }
}

void ConkyCustomizeDialog::pushRestore_clicked()
{
    QString backupFileName = file_name + ".bak";
    if (QFile::exists(backupFileName)) {
        QFile::remove(file_name);

        // Try normal copy first, then elevation if needed
        bool restoreSuccess = QFile::copy(backupFileName, file_name);
        if (!restoreSuccess) {
            QFileInfo fileInfo(file_name);
            QFileInfo dirInfo(fileInfo.absolutePath());

            // Check if we need elevation for destination directory
            if (!dirInfo.isWritable()) {
                restoreSuccess = copyFileWithElevation(backupFileName, file_name);
            }
        }

        if (restoreSuccess) {
            refresh();
        } else {
            qWarning() << "Failed to restore from backup:" << backupFileName;
            QMessageBox::warning(this, tr("Restore Failed"), tr("Failed to restore from backup file."));
        }
    } else {
        QMessageBox::warning(this, tr("Restore Failed"), tr("Backup file does not exist."));
    }
}

void ConkyCustomizeDialog::pushDefaultColor_clicked()
{
    pushColorButton_clicked(-1); // Default color
}

void ConkyCustomizeDialog::pushColorButton_clicked(int colorIndex)
{
    QWidget *colorWidget = (colorIndex >= 0 && colorIndex < 10)
                               ? groupBoxColors->findChild<QWidget *>(QString("widgetColor%1").arg(colorIndex))
                               : widgetDefaultColor;

    if (colorWidget) {
        pickColor(colorWidget);
    }
}

void ConkyCustomizeDialog::radioDesktop1_clicked()
{
    QRegularExpression regexp_lua_owh(capture_lua_owh);
    QRegularExpression regexp_old_owh(capture_old_owh);
    QRegularExpression regexp_owh = is_lua_format ? regexp_lua_owh : regexp_old_owh;
    QString comment_sep = is_lua_format ? "--" : "#";
    QRegularExpressionMatch match_owh;

    bool lua_block_comment = false;

    const QStringList list = file_content.split('\n');
    QStringList new_list;
    new_list.reserve(list.size());
    for (const QString &row : list) {
        // Lua comment block
        QString trow = row.trimmed();
        if (is_lua_format) {
            if (lua_block_comment) {
                if (trow.endsWith(block_comment_end)) {
                    lua_block_comment = false;
                    if (debug) {
                        qDebug() << "Lua block comment end 'ENDS WITH LINE' found";
                    }
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_end)) {
                    lua_block_comment = false;
                    QStringList ltrow = trow.split(block_comment_end);
                    ltrow.removeFirst();
                    trow = ltrow.join(block_comment_end);
                    trow = trow.trimmed();
                    if (debug) {
                        qDebug() << "Lua block comment end CONTAINS line found: after ]]: " << trow;
                    }
                } else {
                    new_list << row;
                    continue;
                }
            }
            if (!lua_block_comment) {
                if (trow.startsWith(block_comment_start)) {
                    if (debug) {
                        qDebug() << "Lua block comment 'STARTS WITH LINE' found";
                    }
                    lua_block_comment = true;
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_start)) {
                    lua_block_comment = true;
                    QStringList ltrow = trow.split(block_comment_start);
                    trow = ltrow[0];
                    trow = trow.trimmed();
                    if (debug) {
                        qDebug() << "Lua block comment start CONTAINS line found: before start --[[: " << trow;
                    }
                }
            }
        }
        // Comment line
        if (trow.startsWith(comment_sep)) {
            new_list << row;
            continue;
        }

        if (!trow.startsWith("own_window_hints")) {
            new_list << row;
            continue;
        } else {

            if (debug) {
                qDebug() << "on_radioDesktops1_clicked: own_window_hints found row : " << row;
                qDebug() << "on_radioDesktops1_clicked: own_window_hints found trow: " << trow;
            }
            match_owh = regexp_owh.match(row);
            if (match_owh.hasMatch()) {
                QString owh_value = match_owh.captured(2);
                owh_value.replace(",sticky", "");
                owh_value.replace("sticky", "");
                QString new_row = match_owh.captured(1) + owh_value + match_owh.captured(3);
                if (debug) {
                    qDebug() << "Removed sticky: " << new_row;
                }
                new_list << new_row;
            } else {
                if (debug) {
                    qDebug() << "ERROR : " << row;
                }
                if (is_lua_format) {
                    if (debug) {
                        qDebug() << "regexp_owh : " << regexp_lua_owh;
                        qDebug() << "regexp_owh : " << regexp_owh;
                    }
                } else {
                    if (debug) {
                        qDebug() << "regexp_owh : " << regexp_old_owh;
                    }
                }
                new_list << row;
            }
        }
    }

    file_content = new_list.join('\n').append('\n');
    writeFile(file_name, file_content);
}

void ConkyCustomizeDialog::radioAllDesktops_clicked()
{
    QString comment_sep;
    QRegularExpression regexp_lua_owh(capture_lua_owh);
    QRegularExpression regexp_old_owh(capture_old_owh);
    QRegularExpression regexp_owh;
    QRegularExpressionMatch match_owh;
    QString conky_config_lua_end = "}";
    QString conky_config_old_end = "TEXT";
    QString conky_config_end;
    QString conky_sticky;
    bool lua_block_comment {false};
    bool conky_config {false};

    if (is_lua_format) {
        regexp_owh = regexp_lua_owh;
        comment_sep = "--";
        conky_config = false;
        conky_config_end = conky_config_lua_end;
        conky_sticky = "\town_window_hints = 'sticky',";
    } else {
        regexp_owh = regexp_old_owh;
        comment_sep = "#";
        conky_config = true;
        conky_config_end = conky_config_old_end;
        conky_sticky = "own_window_hints sticky";
    }

    bool found = false;
    const QStringList list = file_content.split('\n');
    QStringList new_list;
    QString trow;
    for (const QString &row : list) {
        trow = row.trimmed();
        if (is_lua_format) {
            if (lua_block_comment) {
                if (trow.endsWith(block_comment_end)) {
                    lua_block_comment = false;
                    if (debug) {
                        qDebug() << "Lua block comment end 'ENDS WITH LINE' found";
                    }
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_end)) {
                    lua_block_comment = false;
                    QStringList ltrow = trow.split(block_comment_end);
                    ltrow.removeFirst();
                    trow = ltrow.join(block_comment_end);
                    trow = trow.trimmed();
                    if (debug) {
                        qDebug() << "Lua block comment end CONTAINS line found: after ]]: " << trow;
                    }
                } else {
                    new_list << row;
                    continue;
                }
            }
            if (!lua_block_comment) {
                if (trow.startsWith(block_comment_start)) {
                    if (debug) {
                        qDebug() << "Lua block comment 'STARTS WITH LINE' found";
                    }
                    lua_block_comment = true;
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_start)) {
                    lua_block_comment = true;
                    QStringList ltrow = trow.split(block_comment_start);
                    trow = ltrow[0];
                    trow = trow.trimmed();
                    if (debug) {
                        qDebug() << "Lua block comment start CONTAINS line found: before start --[[: " << trow;
                    }
                }
            }
        }
        // Comment line
        if (trow.startsWith(comment_sep)) {
            new_list << row;
            continue;
        }
        if (is_lua_format && trow.startsWith("conky.config")) {
            conky_config = true;
        }

        if (!found && conky_config && trow.startsWith(conky_config_end)) {
            conky_config = false;
            new_list << conky_sticky;
            new_list << row;
            continue;
        }
        if (!conky_config) {
            new_list << row;
            continue;
        }

        if (!trow.startsWith("own_window_hints ")) {
            new_list << row;
            continue;
        } else {
            if (debug) {
                qDebug() << "on_radioAllDesktops_clicked: own_window_hints found row : " << row;
                qDebug() << "on_radioAllDesktops_clicked: own_window_hints found trow: " << trow;
            }
            match_owh = regexp_owh.match(row);
            if (match_owh.hasMatch()) {
                QString owh_value = match_owh.captured(2);
                if (owh_value.length() == 0) {
                    owh_value = "sticky";
                } else {
                    owh_value.append(",sticky");
                }
                QString new_row = match_owh.captured(1) + owh_value + match_owh.captured(3);
                if (debug) {
                    qDebug() << "Append sticky: " << new_row;
                }

                new_list << match_owh.captured(1) + owh_value + match_owh.captured(3);
                found = true;
            } else {
                if (debug) {
                    qDebug() << "ERROR : " << row;
                }
                if (is_lua_format) {
                    if (debug) {
                        qDebug() << "regexp_owh : " << regexp_lua_owh;
                    }
                } else {
                    if (debug) {
                        qDebug() << "regexp_owh : " << regexp_old_owh;
                    }
                }
                found = false;
                new_list << row;
            }
        }
    }

    file_content = new_list.join('\n').append('\n');
    writeFile(file_name, file_content);
}

void ConkyCustomizeDialog::radioDayLong_clicked()
{
    file_content.replace("%a", "%A");
    writeFile(file_name, file_content);
}

void ConkyCustomizeDialog::radioDayShort_clicked()
{
    file_content.replace("%A", "%a");
    writeFile(file_name, file_content);
}

void ConkyCustomizeDialog::radioMonthLong_clicked()
{
    file_content.replace("%b", "%B");
    writeFile(file_name, file_content);
}

void ConkyCustomizeDialog::radioMonthShort_clicked()
{
    file_content.replace("%B", "%b");
    writeFile(file_name, file_content);
}

void ConkyCustomizeDialog::closeEvent(QCloseEvent *event)
{
    // Apply only the specific changes that were made
    if (changedFlags.alignment) {
        applyAlignmentChanges();
    }
    if (changedFlags.gaps) {
        applyGapChanges();
    }
    if (changedFlags.size) {
        applySizeChanges();
    }
    if (changedFlags.transparency) {
        applyTransparencyChanges();
    }
    if (changedFlags.timeFormat) {
        applyTimeFormatChanges();
    }
    if (changedFlags.networkDevice) {
        applyNetworkDeviceChanges();
    }

    QDialog::closeEvent(event);
}

bool ConkyCustomizeDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // Check if this is a color widget
            QWidget *widget = qobject_cast<QWidget *>(obj);
            if (widget && widget->objectName().startsWith("widget")) {
                if (widget->objectName() == "widgetDefaultColor") {
                    pushColorButton_clicked(-1); // Default color
                } else if (widget->objectName().startsWith("widgetColor")) {
                    int colorIndex = widget->objectName().mid(11).toInt(); // Remove "widgetColor" prefix
                    pushColorButton_clicked(colorIndex);
                } else if (widget->objectName() == "widgetBackgroundColor") {
                    onBackgroundColorClicked(); // Background color for transparency
                }
                return true; // Event handled
            }
        }
    }
    return QDialog::eventFilter(obj, event); // Pass to base class
}

void ConkyCustomizeDialog::createLocationTab()
{
    auto *locationWidget = new QWidget;
    auto *locationLayout = new QVBoxLayout(locationWidget);
    locationLayout->setContentsMargins(12, 12, 12, 12);
    locationLayout->setSpacing(12);

    // Position Settings Group
    auto *positionGroup = new QGroupBox(tr("Position"));
    auto *positionLayout = new QGridLayout(positionGroup);
    positionLayout->setSpacing(6);

    int row = 0;

    // Alignment
    auto *lblAlignment = new QLabel(tr("Alignment"));
    lblAlignment->setAlignment(Qt::AlignLeft);
    positionLayout->addWidget(lblAlignment, row, 0);

    cmbAlignment = new QComboBox;
    cmbAlignment->addItem(tr("Top Left"), "top_left");
    cmbAlignment->addItem(tr("Top Right"), "top_right");
    cmbAlignment->addItem(tr("Top Middle"), "top_middle");
    cmbAlignment->addItem(tr("Bottom Left"), "bottom_left");
    cmbAlignment->addItem(tr("Bottom Right"), "bottom_right");
    cmbAlignment->addItem(tr("Bottom Middle"), "bottom_middle");
    cmbAlignment->addItem(tr("Middle Left"), "middle_left");
    cmbAlignment->addItem(tr("Middle Right"), "middle_right");
    cmbAlignment->addItem(tr("Middle Middle"), "middle_middle");
    positionLayout->addWidget(cmbAlignment, row, 1);

    // Horizontal Gap
    ++row;
    auto *lblGapX = new QLabel(tr("Horizontal Gap"));
    lblGapX->setToolTip(tr("[GAP_X] Horizontal distance from window border (in pixels)"));
    lblGapX->setAlignment(Qt::AlignLeft);
    positionLayout->addWidget(lblGapX, row, 0);

    spinGapX = new QSpinBox;
    spinGapX->setRange(-10000, 10000);
    spinGapX->setSingleStep(10);
    spinGapX->setValue(0);
    positionLayout->addWidget(spinGapX, row, 1);

    // Vertical Gap
    ++row;
    auto *lblGapY = new QLabel(tr("Vertical Gap"));
    lblGapY->setToolTip(tr("[GAP_Y] Vertical distance from window border (in pixels)"));
    lblGapY->setAlignment(Qt::AlignLeft);
    positionLayout->addWidget(lblGapY, row, 0);

    spinGapY = new QSpinBox;
    spinGapY->setRange(-10000, 10000);
    spinGapY->setSingleStep(10);
    spinGapY->setValue(0);
    spinGapY->setMinimumWidth(120);
    positionLayout->addWidget(spinGapY, row, 1);

    // Desktop Group - moved from Basic tab
    groupBoxDesktop = new QGroupBox(tr("Desktop"));
    auto *desktopLayout = new QHBoxLayout(groupBoxDesktop);

    radioDesktop1 = new QRadioButton(tr("Desktop 1"));
    radioAllDesktops = new QRadioButton(tr("All Desktops"));

    desktopLayout->addStretch();
    desktopLayout->addWidget(radioDesktop1);
    desktopLayout->addWidget(radioAllDesktops);
    desktopLayout->addStretch();

    locationLayout->addWidget(groupBoxDesktop);
    locationLayout->addWidget(positionGroup);

    locationLayout->addStretch();

    tabWidget->addTab(locationWidget, tr("Location"));
}

void ConkyCustomizeDialog::createSizeTab()
{
    auto *sizeWidget = new QWidget;
    auto *sizeLayout = new QGridLayout(sizeWidget);
    sizeLayout->setContentsMargins(12, 12, 12, 12);
    sizeLayout->setSpacing(6);

    int row = 0;

    QString tooltip = tr(
        "Width should be larger than the size of window contents,\notherwise this setting will not have any effect");

    // Minimum Width
    auto *lblMinWidth = new QLabel(tr("Minimum Width"));
    lblMinWidth->setAlignment(Qt::AlignLeft);
    lblMinWidth->setToolTip(tooltip);
    sizeLayout->addWidget(lblMinWidth, row, 0);

    spinMinWidth = new QSpinBox;
    spinMinWidth->setRange(0, 9999);
    spinMinWidth->setSingleStep(10);
    spinMinWidth->setValue(100);
    spinMinWidth->setMinimumWidth(120);
    spinMinWidth->setToolTip(tooltip);
    sizeLayout->addWidget(spinMinWidth, row, 1);

    // Minimum Height
    ++row;
    tooltip = tr(
        "Height should be larger than the size of window contents,\notherwise this setting will not have any effect");
    auto *lblMinHeight = new QLabel(tr("Minimum Height"));
    lblMinHeight->setAlignment(Qt::AlignLeft);
    lblMinHeight->setToolTip(tooltip);
    sizeLayout->addWidget(lblMinHeight, row, 0);

    spinMinHeight = new QSpinBox;
    spinMinHeight->setRange(0, 9999);
    spinMinHeight->setSingleStep(10);
    spinMinHeight->setValue(100);
    spinMinHeight->setToolTip(tooltip);
    sizeLayout->addWidget(spinMinHeight, row, 1);

    // Height Padding
    ++row;
    tooltip = tr("Increases the window height by adding empty lines at the end of the Conky config file");
    auto *lblHeightPadding = new QLabel(tr("Height Padding"));
    lblHeightPadding->setAlignment(Qt::AlignLeft);
    lblHeightPadding->setToolTip(tooltip);
    sizeLayout->addWidget(lblHeightPadding, row, 0);

    spinHeightPadding = new QSpinBox;
    spinHeightPadding->setRange(0, 100);
    spinHeightPadding->setSingleStep(1);
    spinHeightPadding->setValue(0);
    spinHeightPadding->setToolTip(tooltip);
    sizeLayout->addWidget(spinHeightPadding, row, 1);

    // Add empty row with stretch to push content to top
    sizeLayout->setRowStretch(++row, 1);

    tabWidget->addTab(sizeWidget, tr("Size"));
}

void ConkyCustomizeDialog::createTransparencyTab()
{
    auto *transparencyWidget = new QWidget;
    auto *transparencyLayout = new QGridLayout(transparencyWidget);
    transparencyLayout->setContentsMargins(12, 12, 12, 12);
    transparencyLayout->setSpacing(6);

    int row = 0;

    // Transparency Type
    auto *lblTransparencyType = new QLabel(tr("Transparency Type"));
    lblTransparencyType->setAlignment(Qt::AlignLeft);
    transparencyLayout->addWidget(lblTransparencyType, row, 0);

    cmbTransparencyType = new QComboBox;
    cmbTransparencyType->addItem(tr("Opaque"), "opaque");
    cmbTransparencyType->addItem(tr("Transparent"), "trans");
    cmbTransparencyType->addItem(tr("Pseudo-Transparent"), "pseudo");
    cmbTransparencyType->addItem(tr("Semi-Transparent"), "semi");
    transparencyLayout->addWidget(cmbTransparencyType, row, 1);

    // Opacity
    ++row;
    auto *lblOpacity = new QLabel(tr("Opacity (%)"));
    QString opacityTooltip = tr("Window Opacity\n\n0 = Fully Transparent, 100 = Fully Opaque");
    lblOpacity->setToolTip(opacityTooltip);
    lblOpacity->setAlignment(Qt::AlignLeft);
    transparencyLayout->addWidget(lblOpacity, row, 0);

    spinOpacity = new QSpinBox;
    spinOpacity->setRange(0, 100);
    spinOpacity->setSingleStep(10);
    spinOpacity->setValue(100);
    spinOpacity->setToolTip(opacityTooltip);
    transparencyLayout->addWidget(spinOpacity, row, 1);

    // Background Color
    ++row;
    auto *lblBackgroundColor = new QLabel(tr("Background Color"));
    lblBackgroundColor->setAlignment(Qt::AlignLeft);
    transparencyLayout->addWidget(lblBackgroundColor, row, 0);

    btnBackgroundColor = new QPushButton;
    btnBackgroundColor->setText(tr("Choose Color"));
    btnBackgroundColor->setIcon(QIcon::fromTheme("color-picker"));
    btnBackgroundColor->setMinimumWidth(120);
    transparencyLayout->addWidget(btnBackgroundColor, row, 1);

    // Color preview widget
    widgetBackgroundColor = new QWidget;
    widgetBackgroundColor->setMinimumSize(30, 30);
    widgetBackgroundColor->setStyleSheet("border: 1px solid black; background-color: #000000;");
    widgetBackgroundColor->setCursor(Qt::PointingHandCursor);
    widgetBackgroundColor->setObjectName("widgetBackgroundColor");

    // Install event filter to make background color widget clickable
    widgetBackgroundColor->installEventFilter(this);

    transparencyLayout->addWidget(widgetBackgroundColor, row, 2);

    // Add informational notes as separate widgets spanning full width
    ++row;
    QString note1 = "○ "
                    + tr("Setting Type to \"Transparent\" will make the whole window transparent (including any "
                         "images). Use \"Pseudo-Transparent\" if you want the images to be opaque.");
    auto *lblNote1 = new QLabel(note1);
    lblNote1->setWordWrap(true);
    lblNote1->setAlignment(Qt::AlignLeft);
    lblNote1->setContentsMargins(6, 6, 6, 6);
    transparencyLayout->addWidget(lblNote1, row, 0, 1, 3);

    ++row;
    QString note2 = "○ "
                    + tr("Setting Type to \"Pseudo-Transparent\" will make the window transparent but the window will "
                         "have a shadow. The shadow can be disabled by configuring your window manager.");
    auto *lblNote2 = new QLabel(note2);
    lblNote2->setWordWrap(true);
    lblNote2->setAlignment(Qt::AlignLeft);
    lblNote2->setContentsMargins(6, 6, 6, 6);
    transparencyLayout->addWidget(lblNote2, row, 0, 1, 3);

    // Add empty row with stretch to push content to top
    transparencyLayout->setRowStretch(++row, 1);

    tabWidget->addTab(transparencyWidget, tr("Transparency"));
}

void ConkyCustomizeDialog::createTimeTab()
{
    auto *timeWidget = new QWidget;
    auto *timeLayout = new QVBoxLayout(timeWidget);
    timeLayout->setContentsMargins(12, 12, 12, 12);
    timeLayout->setSpacing(12);

    // Date Format Group
    groupBoxFormat = new QGroupBox(tr("Date Format"));
    auto *formatLayout = new QGridLayout(groupBoxFormat);

    auto *dayLabel = new QLabel(tr("Day"));
    dayLabel->setAlignment(Qt::AlignCenter);
    radioDayLong = new QRadioButton(tr("Long"));
    radioDayShort = new QRadioButton(tr("Short"));
    radioDayShort->setToolTip(tr("Abbreviated name, e.g. Tu"));

    // Create button group for Day format (exclusive)
    auto *dayButtonGroup = new QButtonGroup(this);
    dayButtonGroup->addButton(radioDayLong);
    dayButtonGroup->addButton(radioDayShort);

    auto *monthLabel = new QLabel(tr("Month"));
    monthLabel->setAlignment(Qt::AlignCenter);
    radioMonthLong = new QRadioButton(tr("Long"));
    radioMonthShort = new QRadioButton(tr("Short"));
    radioMonthShort->setToolTip(tr("Abbreviated name, e.g. Oct"));

    // Create button group for Month format (exclusive)
    auto *monthButtonGroup = new QButtonGroup(this);
    monthButtonGroup->addButton(radioMonthLong);
    monthButtonGroup->addButton(radioMonthShort);

    formatLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding), 0, 0);
    formatLayout->addWidget(dayLabel, 0, 1);
    formatLayout->addWidget(radioDayLong, 0, 2);
    formatLayout->addWidget(radioDayShort, 0, 3);
    formatLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding), 0, 4);
    formatLayout->addWidget(monthLabel, 1, 1);
    formatLayout->addWidget(radioMonthLong, 1, 2);
    formatLayout->addWidget(radioMonthShort, 1, 3);

    timeLayout->addWidget(groupBoxFormat);

    // Time Format Group
    auto *timeFormatGroup = new QGroupBox(tr("Time Format"));
    auto *timeFormatLayout = new QGridLayout(timeFormatGroup);
    timeFormatLayout->setSpacing(6);

    int row = 0;

    // Time Format
    auto *lblTimeFormat = new QLabel(tr("Format"));
    lblTimeFormat->setAlignment(Qt::AlignLeft);
    timeFormatLayout->addWidget(lblTimeFormat, row, 0);

    cmbTimeFormat = new QComboBox;
    cmbTimeFormat->addItem(tr("12 Hour"), "12");
    cmbTimeFormat->addItem(tr("24 Hour"), "24");
    timeFormatLayout->addWidget(cmbTimeFormat, row, 1);

    // Note about time format
    ++row;
    lblTimeNotFound = new QLabel;
    lblTimeNotFound->setAlignment(Qt::AlignLeft);
    lblTimeNotFound->setWordWrap(true);
    lblTimeNotFound->setContentsMargins(6, 6, 6, 6);
    timeFormatLayout->addWidget(lblTimeNotFound, row, 0, 1, 2);

    timeLayout->addWidget(timeFormatGroup);

    timeLayout->addStretch();

    tabWidget->addTab(timeWidget, tr("Date && Time"));
}

void ConkyCustomizeDialog::createNetworkTab()
{
    auto *networkWidget = new QWidget;
    auto *networkLayout = new QGridLayout(networkWidget);
    networkLayout->setContentsMargins(12, 12, 12, 12);
    networkLayout->setSpacing(6);

    int row = 0;

    // Network Interface
    auto *lblNetworkDevice = new QLabel(tr("Interface"));
    lblNetworkDevice->setAlignment(Qt::AlignLeft);
    networkLayout->addWidget(lblNetworkDevice, row, 0);

    txtNetworkDevice = new QLineEdit;
    networkLayout->addWidget(txtNetworkDevice, row, 1);

    // WiFi and LAN buttons
    btnWifi = new QPushButton(tr("WiFi"));
    btnWifi->setIcon(QIcon::fromTheme("network-wireless"));
    btnWifi->setMinimumWidth(50);
    btnWifi->setToolTip(tr("WiFi Network") + " (wlan0)");
    networkLayout->addWidget(btnWifi, row, 2);

    btnLan = new QPushButton(tr("LAN"));
    btnLan->setIcon(QIcon::fromTheme("network-wired"));
    btnLan->setMinimumWidth(50);
    btnLan->setToolTip(tr("Wired LAN Network") + " (eth0)");
    networkLayout->addWidget(btnLan, row, 3);

    // Note about network interface
    ++row;
    lblNetworkNotFound = new QLabel;
    lblNetworkNotFound->setAlignment(Qt::AlignLeft);
    lblNetworkNotFound->setWordWrap(true);
    lblNetworkNotFound->setContentsMargins(6, 6, 6, 6);
    networkLayout->addWidget(lblNetworkNotFound, row, 0, 1, 4);

    // Add empty row with stretch to push content to top
    networkLayout->setRowStretch(++row, 1);

    tabWidget->addTab(networkWidget, tr("Network"));
}

// Implementation for new tabs functionality
void ConkyCustomizeDialog::onAlignmentChanged()
{
    if (!cmbAlignment) {
        return;
    }

    changedFlags.alignment = true;
}

void ConkyCustomizeDialog::applyAlignmentChanges()
{
    if (!cmbAlignment) {
        return;
    }

    QString alignment = cmbAlignment->currentData().toString();
    writeConfigValue("alignment", alignment, true); // true for quoted value
}

void ConkyCustomizeDialog::onGapChanged()
{
    changedFlags.gaps = true;
}

void ConkyCustomizeDialog::applyGapChanges()
{
    if (spinGapX) {
        writeConfigValue("gap_x", QString::number(spinGapX->value()));
    }
    if (spinGapY) {
        writeConfigValue("gap_y", QString::number(spinGapY->value()));
    }
}

void ConkyCustomizeDialog::onSizeChanged()
{
    changedFlags.size = true;
}

void ConkyCustomizeDialog::applySizeChanges()
{
    if (spinMinWidth) {
        writeConfigValue("minimum_width", QString::number(spinMinWidth->value()));
    }
    if (spinMinHeight) {
        writeConfigValue("minimum_height", QString::number(spinMinHeight->value()));
    }
    if (spinHeightPadding) {
        updateHeightPadding(spinHeightPadding->value());
    }
}

void ConkyCustomizeDialog::onTransparencyChanged()
{
    if (!cmbTransparencyType || !spinOpacity) {
        return;
    }

    // Auto-set opacity to 100% when Opaque is selected
    QString transparencyType = cmbTransparencyType->currentData().toString();
    if (transparencyType == "opaque") {
        spinOpacity->setValue(100);
    }

    changedFlags.transparency = true;
}

void ConkyCustomizeDialog::applyTransparencyChanges()
{
    if (!cmbTransparencyType || !spinOpacity) {
        return;
    }

    QString transparencyType = cmbTransparencyType->currentData().toString();
    int opacity = spinOpacity->value();

    // Set own_window based on transparency type
    if (transparencyType == "opaque") {
        writeConfigValue("own_window", "false");
        writeConfigValue("own_window_transparent", "false");
        writeConfigValue("own_window_argb_visual", "false");
        // Remove argb_value for opaque
        // writeConfigValue("own_window_argb_value", ""); // Don't set for opaque
    } else if (transparencyType == "trans") {
        writeConfigValue("own_window", "true");
        writeConfigValue("own_window_transparent", "true");
        writeConfigValue("own_window_argb_visual", "false");
        // For transparent, we still want to set opacity
        double opacityValue = opacity / 100.0;
        writeConfigValue("own_window_argb_value", QString::number(static_cast<int>(opacityValue * 255)));
    } else if (transparencyType == "pseudo") {
        // Pseudo-transparent: no own window, uses desktop background
        writeConfigValue("own_window", "false");
        writeConfigValue("own_window_transparent", "true");
        writeConfigValue("own_window_argb_visual", "false");
        // Set opacity for pseudo-transparency
        double opacityValue = opacity / 100.0;
        writeConfigValue("own_window_argb_value", QString::number(static_cast<int>(opacityValue * 255)));
    } else if (transparencyType == "semi") {
        writeConfigValue("own_window", "true");
        writeConfigValue("own_window_transparent", "false");
        writeConfigValue("own_window_argb_visual", "true");
        // Set opacity for semi-transparent
        double opacityValue = opacity / 100.0;
        writeConfigValue("own_window_argb_value", QString::number(static_cast<int>(opacityValue * 255)));
    }
}

void ConkyCustomizeDialog::onTimeFormatChanged()
{
    if (!cmbTimeFormat) {
        return;
    }

    changedFlags.timeFormat = true;
}

void ConkyCustomizeDialog::applyTimeFormatChanges()
{
    if (!cmbTimeFormat) {
        return;
    }

    QString format = cmbTimeFormat->currentData().toString();
    if (format == "12") {
        // Replace 24-hour format with 12-hour format
        file_content.replace("%H", "%I");
        file_content.replace("%k", "%l");
        // Add AM/PM indicator if not present
        if (!file_content.contains("%p")) {
            file_content.replace("%M", "%M %p");
        }
    } else if (format == "24") {
        // Replace 12-hour format with 24-hour format
        file_content.replace("%I", "%H");
        file_content.replace("%l", "%k");
        // Remove AM/PM indicator
        file_content.replace(QRegularExpression("\\s*%p"), "");
    }
    writeFile(file_name, file_content);
}

void ConkyCustomizeDialog::onNetworkDeviceChanged()
{
    if (!txtNetworkDevice) {
        return;
    }

    changedFlags.networkDevice = true;
}

void ConkyCustomizeDialog::applyNetworkDeviceChanges()
{
    if (!txtNetworkDevice) {
        return;
    }

    QString newDevice = txtNetworkDevice->text().trimmed();
    if (newDevice.isEmpty()) {
        return;
    }

    // Comprehensive network interface replacement based on conky-manager2 patterns

    // Pattern 1: Basic network variables (totaldown, totalup, upspeed, etc.)
    QString pattern1
        = R"(\$\{(totaldown|totalup|upspeed|upspeedf|downspeed|downspeedf|wireless_ap|wireless_bitrate|wireless_essid|wireless_link_qual|wireless_link_qual_max|wireless_link_qual_perc|wireless_mode|if_up|addr|TOTALDOWN|TOTALUP|UPSPEED|UPSPEEDF|DOWNSPEED|DOWNSPEEDF|WIRELESS_AP|WIRELESS_BITRATE|WIRELESS_ESSID|WIRELESS_LINK_QUAL|WIRELESS_LINK_QUAL_MAX|WIRELESS_LINK_QUAL_PERC|WIRELESS_MODE|IF_UP|ADDR)[ \t]*([A-Za-z0-9]+)[ \t]*\})";

    // Pattern 2: Network graph variables (upspeedgraph, downspeedgraph)
    QString pattern2
        = R"(\$\{(upspeedgraph|downspeedgraph|UPSPEEDGRAPH|DOWNSPEEDGRAPH)[ \t]*([A-Za-z0-9]+)[ \t]*[^}]*\})";

    // Pattern 3: Wireless bar variables
    QString pattern3
        = R"(\$\{(wireless_link_bar|WIRELESS_LINK_BAR)[ \t]*[0-9]+[ \t]*,[0-9]+[ \t]*([A-Za-z0-9]+)[ \t]*\})";

    const QStringList lines = file_content.split('\n');
    QStringList newLines;
    newLines.reserve(lines.count());

    QRegularExpression regex1(pattern1);
    QRegularExpression regex2(pattern2);
    QRegularExpression regex3(pattern3);

    for (const QString &line : lines) {
        QString newLine = line;

        // Apply Pattern 1: Basic network variables
        QRegularExpressionMatchIterator it1 = regex1.globalMatch(newLine);
        while (it1.hasNext()) {
            QRegularExpressionMatch match = it1.next();
            QString fullMatch = match.captured(0);
            QString networkVar = match.captured(1);
            QString oldDevice = match.captured(2);
            QString replacement = QString("${%1 %2}").arg(networkVar, newDevice);
            newLine.replace(fullMatch, replacement);
        }

        // Apply Pattern 2: Network graph variables
        QRegularExpressionMatchIterator it2 = regex2.globalMatch(newLine);
        while (it2.hasNext()) {
            QRegularExpressionMatch match = it2.next();
            QString fullMatch = match.captured(0);
            QString networkVar = match.captured(1);
            QString oldDevice = match.captured(2);
            // Preserve everything after the device name
            QString afterDevice = fullMatch;
            int deviceEnd = afterDevice.indexOf(oldDevice) + oldDevice.length();
            QString suffix = afterDevice.mid(deviceEnd);
            suffix = suffix.left(suffix.lastIndexOf('}'));
            QString replacement = QString("${%1 %2%3}").arg(networkVar, newDevice, suffix);
            newLine.replace(fullMatch, replacement);
        }

        // Apply Pattern 3: Wireless bar variables
        QRegularExpressionMatchIterator it3 = regex3.globalMatch(newLine);
        while (it3.hasNext()) {
            QRegularExpressionMatch match = it3.next();
            QString fullMatch = match.captured(0);
            QString networkVar = match.captured(1);
            QString oldDevice = match.captured(2);
            // Extract the width,height parameters
            QString beforeDevice = fullMatch;
            int deviceStart = beforeDevice.indexOf(oldDevice);
            QString prefix = beforeDevice.left(deviceStart);
            QString replacement = QString("%1%2}").arg(prefix, newDevice);
            newLine.replace(fullMatch, replacement);
        }

        newLines << newLine;
    }

    file_content = newLines.join('\n');
    writeFile(file_name, file_content);
}

void ConkyCustomizeDialog::onWifiClicked()
{
    if (txtNetworkDevice) {
        txtNetworkDevice->setText("wlan0");
        onNetworkDeviceChanged();
    }
}

void ConkyCustomizeDialog::onLanClicked()
{
    if (txtNetworkDevice) {
        txtNetworkDevice->setText("eth0");
        onNetworkDeviceChanged();
    }
}

void ConkyCustomizeDialog::onBackgroundColorClicked()
{
    if (!btnBackgroundColor || !widgetBackgroundColor) {
        return;
    }

    QColor currentColor = widgetBackgroundColor->palette().color(QWidget::backgroundRole());
    QColor selectedColor = QColorDialog::getColor(currentColor, this, tr("Select Background Color"));

    if (selectedColor.isValid()) {
        widgetBackgroundColor->setStyleSheet(
            QString("border: 1px solid black; background-color: %1;").arg(selectedColor.name()));
        writeConfigValue("own_window_colour", selectedColor.name().remove('#'), true);
    }
}

void ConkyCustomizeDialog::writeConfigValue(const QString &key, const QString &value, bool quoted)
{
    QString newValueStr = quoted ? (is_lua_format ? QString("'%1'").arg(value) : value) : value;
    QString newLine = is_lua_format ? QString("    %1 = %2,").arg(key, newValueStr) : QString("%1 %2").arg(key, value);

    const QStringList lines = file_content.split('\n');
    QStringList newLines;
    newLines.reserve(lines.count());
    bool found = false;
    bool inConfig = !is_lua_format; // Old format is always in config

    for (const QString &line : lines) {
        QString trow = line.trimmed();

        // Track if we're in the config section for Lua format
        if (is_lua_format) {
            if (trow.startsWith("conky.config")) {
                inConfig = true;
                newLines << line;
                continue;
            }
            if (inConfig && trow == "}") {
                if (!found) {
                    // Add the new line before the closing brace
                    newLines << newLine;
                    found = true;
                }
                inConfig = false;
                newLines << line;
                continue;
            }
        }

        // Check if this line contains the key we want to update
        if (inConfig && (trow.startsWith(key) || trow.contains(key + " ="))) {
            QRegularExpression keyRe(is_lua_format ? QString(R"(%1\s*=\s*[^,]*,?)").arg(key)
                                                   : QString(R"(%1\s+.*)").arg(key));

            if (keyRe.match(trow).hasMatch()) {
                newLines << QString(line).replace(keyRe, newLine.trimmed());
                found = true;
                continue;
            }
        }

        newLines << line;
    }

    // If the key wasn't found, add it
    if (!found) {
        if (is_lua_format) {
            // Insert before the closing brace of conky.config
            for (int i = newLines.size() - 1; i >= 0; --i) {
                if (newLines[i].trimmed() == "}") {
                    newLines.insert(i, newLine);
                    break;
                }
            }
        } else {
            // Add before TEXT section in old format
            for (int i = 0; i < newLines.size(); ++i) {
                if (newLines[i].trimmed() == "TEXT") {
                    newLines.insert(i, newLine);
                    break;
                }
            }
        }
    }

    file_content = newLines.join('\n');
    writeFile(file_name, file_content);
}
int ConkyCustomizeDialog::countHeightPadding()
{
    const QStringList lines = file_content.split('\n');

    if (is_lua_format) {
        // Find the ]] marker
        int closingBracketIndex = -1;
        for (int i = lines.size() - 1; i >= 0; i--) {
            if (lines[i].trimmed() == "]]") {
                closingBracketIndex = i;
                break;
            }
        }

        if (closingBracketIndex == -1) {
            return 0; // No ]] found
        }

        // Find the last non-empty line before ]]
        int lastContentIndex = closingBracketIndex - 1;
        while (lastContentIndex >= 0 && lines[lastContentIndex].trimmed().isEmpty()) {
            lastContentIndex--;
        }

        // Count empty lines between last content and ]]
        int paddingCount = closingBracketIndex - lastContentIndex - 1;
        return qMax(0, paddingCount);

    } else {
        // Old format - count empty lines at the end
        int count = 0;
        for (int k = lines.size() - 1; k >= 0; k--) {
            QString line = lines[k].trimmed();
            if (line.isEmpty()) {
                count++;
            } else {
                break;
            }
        }
        return count;
    }
}

void ConkyCustomizeDialog::updateHeightPadding(int paddingLines)
{
    const QStringList lines = file_content.split('\n');
    QString newText = "";

    if (is_lua_format) {
        // Find the ]] marker
        int closingBracketIndex = -1;
        for (int i = lines.size() - 1; i >= 0; i--) {
            if (lines[i].trimmed() == "]]") {
                closingBracketIndex = i;
                break;
            }
        }

        if (closingBracketIndex == -1) {
            return; // No ]] found, can't process
        }

        // Find the last non-empty line before ]]
        int lastContentIndex = closingBracketIndex - 1;
        while (lastContentIndex >= 0 && lines[lastContentIndex].trimmed().isEmpty()) {
            lastContentIndex--;
        }

        // Copy everything up to and including the last content line
        for (int i = 0; i <= lastContentIndex; i++) {
            newText += lines[i] + "\n";
        }

        // Add the requested number of padding lines
        for (int i = 0; i < paddingLines; i++) {
            newText += "\n";
        }

        // Add the ]] marker
        newText += "]]\n";

        // Add any lines that come after ]] (if any)
        for (int i = closingBracketIndex + 1; i < lines.size(); i++) {
            if (i == lines.size() - 1 && lines[i].isEmpty()) {
                // Skip the last empty line if it exists
                continue;
            }
            newText += lines[i] + "\n";
        }

        // Remove the trailing newline
        if (newText.endsWith("\n")) {
            newText = newText.left(newText.length() - 1);
        }

    } else {
        // Old format - add padding lines at the end
        // Find the last non-empty line
        int lastContentIndex = lines.size() - 1;
        while (lastContentIndex >= 0 && lines[lastContentIndex].trimmed().isEmpty()) {
            lastContentIndex--;
        }

        // Copy everything up to and including the last content line
        for (int i = 0; i <= lastContentIndex; i++) {
            newText += lines[i] + "\n";
        }

        // Add the requested number of padding lines
        for (int i = 0; i < paddingLines; i++) {
            newText += "\n";
        }

        // Remove the trailing newline
        if (newText.endsWith("\n")) {
            newText = newText.left(newText.length() - 1);
        }
    }

    file_content = newText;
    writeFile(file_name, file_content);
}

void ConkyCustomizeDialog::saveInitialValues()
{
    // Save current UI values as initial state
    initialValues.alignment = cmbAlignment ? cmbAlignment->currentData().toString() : "";
    initialValues.gapX = spinGapX ? static_cast<int>(spinGapX->value()) : 0;
    initialValues.gapY = spinGapY ? static_cast<int>(spinGapY->value()) : 0;
    initialValues.minWidth = spinMinWidth ? static_cast<int>(spinMinWidth->value()) : 0;
    initialValues.minHeight = spinMinHeight ? static_cast<int>(spinMinHeight->value()) : 0;
    initialValues.heightPadding = spinHeightPadding ? static_cast<int>(spinHeightPadding->value()) : 0;
    initialValues.transparencyType = cmbTransparencyType ? cmbTransparencyType->currentData().toString() : "";
    initialValues.opacity = spinOpacity ? static_cast<int>(spinOpacity->value()) : 100;
    initialValues.backgroundColor = widgetBackgroundColor ? widgetBackgroundColor->styleSheet() : "";
    initialValues.timeFormat = cmbTimeFormat ? cmbTimeFormat->currentData().toString() : "";
    initialValues.networkDevice = txtNetworkDevice ? txtNetworkDevice->text() : "";

    // Reset all change flags
    changedFlags = ChangedFlags {};
}

bool ConkyCustomizeDialog::checkForChanges()
{
    // Compare current values with initial values
    bool changed = false;

    if (cmbAlignment && cmbAlignment->currentData().toString() != initialValues.alignment) {
        changed = true;
    }
    if (spinGapX && static_cast<int>(spinGapX->value()) != initialValues.gapX) {
        changed = true;
    }
    if (spinGapY && static_cast<int>(spinGapY->value()) != initialValues.gapY) {
        changed = true;
    }
    if (spinMinWidth && static_cast<int>(spinMinWidth->value()) != initialValues.minWidth) {
        changed = true;
    }
    if (spinMinHeight && static_cast<int>(spinMinHeight->value()) != initialValues.minHeight) {
        changed = true;
    }
    if (spinHeightPadding && static_cast<int>(spinHeightPadding->value()) != initialValues.heightPadding) {
        changed = true;
    }
    if (cmbTransparencyType && cmbTransparencyType->currentData().toString() != initialValues.transparencyType) {
        changed = true;
    }
    if (spinOpacity && static_cast<int>(spinOpacity->value()) != initialValues.opacity) {
        changed = true;
    }
    if (widgetBackgroundColor && widgetBackgroundColor->styleSheet() != initialValues.backgroundColor) {
        changed = true;
    }
    if (cmbTimeFormat && cmbTimeFormat->currentData().toString() != initialValues.timeFormat) {
        changed = true;
    }
    if (txtNetworkDevice && txtNetworkDevice->text() != initialValues.networkDevice) {
        changed = true;
    }

    return changed;
}
