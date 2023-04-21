/**********************************************************************
 *  MainWindow.cpp
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

#include <QColorDialog>
#include <QDebug>
#include <QFileDialog>
#include <QRegularExpression>
#include <QTextEdit>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent, const QString &file)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
{
    debug = (QProcessEnvironment::systemEnvironment().value(QStringLiteral("DEBUG")).length() > 0);

    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << QCoreApplication::applicationVersion();
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons

    connect(QApplication::instance(), &QApplication::aboutToQuit, this, &MainWindow::cleanup);

    file_name = file;
    this->setWindowTitle(tr("MX Conky"));

    // set regexp pattern
    lua_format = QStringLiteral("^\\s*(--|conky.config\\s*=\\s*{|conky.text\\s*=\\s*{)");
    lua_config = QStringLiteral("^\\s*(conky.config\\s*=\\s*{");
    old_format = QStringLiteral("^\\s*#|^TEXT$");
    lua_comment_line = QStringLiteral("^\\s*--");
    lua_comment_start = QStringLiteral("^\\s*--\\[\\[");
    lua_comment_end = QStringLiteral("^\\s*\\]\\]");
    old_comment_line = QStringLiteral("^\\s*#");
    capture_lua_color = QStringLiteral("^(?<before>(.*\\]\\])?\\s*(?<color_item>default_color|color\\d)(?:\\s*=\\s*["
                                       "\\\"\\']))(?:#?)(?<color_value>[[:alnum:]]+)(?<after>(?:[\\\"\\']).*)");
    capture_old_color = QStringLiteral(
        "^(?<before>\\s*(?<color_item>default_color|color\\d)(?:\\s+))(?:#?)(?<color_value>[[:alnum:]]+)(?<after>.*)");

    refresh();
    restoreGeometry(settings.value(QStringLiteral("geometery")).toByteArray());
}

MainWindow::~MainWindow() { delete ui; }

// detect conky format old or lua
void MainWindow::detectConkyFormat()
{
    QRegularExpression lua_format_regexp(lua_format);
    QRegularExpression old_format_regexp(old_format);

    const QStringList list = file_content.split(QStringLiteral("\n"));
    if (debug)
        qDebug() << "Detecting conky format: " + file_name;

    conky_format_detected = false;

    for (const QString &row : list) {
        QRegularExpressionMatch lua_format_match = lua_format_regexp.match(row);
        if (lua_format_match.hasMatch()) {
            is_lua_format = true;
            conky_format_detected = true;
            if (debug)
                qDebug() << "Conky format detected 'lua-format' :" + file_name;
            break;
        }
        QRegularExpressionMatch old_format_match = old_format_regexp.match(row);
        if (old_format_match.hasMatch()) {
            is_lua_format = false;
            conky_format_detected = true;
            if (debug)
                qDebug() << "Conky format detected 'old-format' :" + file_name;
            break;
        }
    }
}

// find defined colors in the config file
void MainWindow::parseContent()
{

    QRegularExpression regexp_old_comment_line(old_comment_line);
    QRegularExpression regexp_lua_comment_line(lua_comment_line);
    QRegularExpression regexp_lua_color(capture_lua_color);
    QRegularExpression regexp_old_color(capture_old_color);
    QRegularExpressionMatch match_color;
    QRegularExpression regexp_color;
    QRegularExpression regexp_comment_line;
    bool own_window_hints_found = false;

    QString comment_sep;
    //    QString block_comment_start = "--[[";
    //    QString block_comment_end = "]]";

    if (is_lua_format) {
        regexp_color = regexp_lua_color;
        regexp_comment_line = regexp_lua_comment_line;
        comment_sep = QStringLiteral("--");
    } else {
        regexp_color = regexp_old_color;
        regexp_comment_line = regexp_old_comment_line;
        comment_sep = QStringLiteral("#");
    }

    const QStringList list = file_content.split(QStringLiteral("\n"));

    if (debug)
        qDebug() << "Parsing content: " + file_name;

    bool lua_block_comment = false;
    //  bool lua_config = false;
    for (const QString &row : list) {
        // lua comment block
        QString trow = row.trimmed();
        if (is_lua_format) {
            if (lua_block_comment) {
                if (trow.endsWith(block_comment_end)) {
                    lua_block_comment = false;
                    if (debug)
                        qDebug() << "Lua block comment end 'ENDS WITH LINE' found";
                    continue;
                }
                if (trow.contains(block_comment_end)) {
                    lua_block_comment = false;
                    QStringList ltrow = trow.split(block_comment_end);
                    ltrow.removeFirst();
                    trow = ltrow.join(block_comment_end);
                    trow = trow.trimmed();
                    if (debug)
                        qDebug() << "Lua block comment end CONTAINS line found: after ]]: " << trow;
                } else {
                    continue;
                }
            }
            if (!lua_block_comment) {
                if (trow.startsWith(block_comment_start)) {
                    if (debug)
                        qDebug() << "Lua block comment 'STARTS WITH LINE' found";
                    lua_block_comment = true;
                    continue;
                }
                if (trow.contains(block_comment_start)) {
                    lua_block_comment = true;
                    QStringList ltrow = trow.split(block_comment_start);
                    trow = ltrow[0];
                    trow = trow.trimmed();
                    if (debug)
                        qDebug() << "Lua block comment start CONTAINS line found: before start --[[: " << trow;
                }
            }
        }
        // comment line
        if (trow.startsWith(comment_sep))
            continue;

        if (trow.contains(comment_sep))
            trow = trow.split(comment_sep)[0];

        // color line
        match_color = regexp_color.match(trow);
        if (match_color.hasMatch()) {
            QString color_item = match_color.captured(QStringLiteral("color_item"));
            QString color_value = match_color.captured(QStringLiteral("color_value"));

            if (color_item == QLatin1String("default_color")) {
                setColor(ui->widgetDefaultColor, strToColor(color_value));
            } else if (color_item == QLatin1String("color0")) {
                setColor(ui->widgetColor0, strToColor(color_value));
            } else if (color_item == QLatin1String("color1")) {
                setColor(ui->widgetColor1, strToColor(color_value));
            } else if (color_item == QLatin1String("color2")) {
                setColor(ui->widgetColor2, strToColor(color_value));
            } else if (color_item == QLatin1String("color3")) {
                setColor(ui->widgetColor3, strToColor(color_value));
            } else if (color_item == QLatin1String("color4")) {
                setColor(ui->widgetColor4, strToColor(color_value));
            } else if (color_item == QLatin1String("color5")) {
                ui->labelColor5->setText(ui->labelColor0->text().replace('0', '5'));
                setColor(ui->widgetColor5, strToColor(color_value));
            } else if (color_item == QLatin1String("color6")) {
                ui->labelColor6->setText(ui->labelColor0->text().replace('0', '6'));
                setColor(ui->widgetColor6, strToColor(color_value));
            } else if (color_item == QLatin1String("color7")) {
                ui->labelColor7->setText(ui->labelColor0->text().replace('0', '7'));
                setColor(ui->widgetColor7, strToColor(color_value));
            } else if (color_item == QLatin1String("color8")) {
                ui->labelColor8->setText(ui->labelColor0->text().replace('0', '8'));
                setColor(ui->widgetColor8, strToColor(color_value));
            } else if (color_item == QLatin1String("color9")) {
                ui->labelColor9->setText(ui->labelColor0->text().replace('0', '9'));
                setColor(ui->widgetColor9, strToColor(color_value));
            }
            continue;
        }

        // Desktop config
        if (trow.startsWith(QLatin1String("own_window_hints"))) {
            own_window_hints_found = true;
            if (debug)
                qDebug() << "own_window_hints line found: " << trow;
            if (trow.contains(QLatin1String("sticky")))
                ui->radioAllDesktops->setChecked(true);
            else
                ui->radioDesktop1->setChecked(true);
            continue;
        }

        // Day/Month format
        if (trow.contains(QLatin1String("%A")))
            ui->radioDayLong->setChecked(true);
        else if (row.contains(QLatin1String("%a")))
            ui->radioDayShort->setChecked(true);
        if (row.contains(QLatin1String("%B")))
            ui->radioMonthLong->setChecked(true);
        else if (row.contains(QLatin1String("%b")))
            ui->radioMonthShort->setChecked(true);
    }
    if (!own_window_hints_found)
        ui->radioDesktop1->setChecked(true);
}

bool MainWindow::checkConkyRunning()
{
    int ret = system("sleep 0.3; pgrep -u $(id -nu) -x conky >/dev/null 2>&1");
    if (debug)
        qDebug() << system("echo pgrep -u $(id -nu) -x conky : ") << ret;
    if (ret == 0) {
        ui->pushToggleOn->setText(QStringLiteral("Stop"));
        ui->pushToggleOn->setIcon(QIcon::fromTheme(QStringLiteral("stop")));
        return true;
    } else {
        ui->pushToggleOn->setText(QStringLiteral("Run"));
        ui->pushToggleOn->setIcon(QIcon::fromTheme(QStringLiteral("start")));
        return false;
    }
}

// read config file
bool MainWindow::readFile(const QString &file_name)
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

QColor MainWindow::strToColor(const QString &colorstr)
{
    QColor color(colorstr);
    if (!color.isValid()) // if color is invalid assume RGB values and add a # in front of the string
        color.setNamedColor("#" + colorstr);
    return color;
}

void MainWindow::refresh()
{
    modified = false;

    // hide all color frames by default, display only the ones in the config file
    const QList<QWidget *> frames({ui->frameDefault, ui->frame0, ui->frame1, ui->frame2, ui->frame3, ui->frame4,
                                   ui->frame5, ui->frame6, ui->frame7, ui->frame8, ui->frame9});
    for (QWidget *w : frames)
        w->hide();
    // draw borders around color widgets
    const QList<QWidget *> widgets({ui->widgetDefaultColor, ui->widgetColor0, ui->widgetColor1, ui->widgetColor2,
                                    ui->widgetColor3, ui->widgetColor4, ui->widgetColor5, ui->widgetColor6,
                                    ui->widgetColor7, ui->widgetColor8, ui->widgetColor9});
    for (QWidget *w : widgets)
        w->setStyleSheet(QStringLiteral("border: 1px solid black"));

    QString conky_name = QFileInfo(file_name).fileName();
    ui->pushChange->setText(conky_name);

    checkConkyRunning();

    QFile(file_name + ".bak").remove();
    QFile::copy(file_name, file_name + ".bak");

    if (readFile(file_name)) {
        detectConkyFormat();
        parseContent();
    }
    this->adjustSize();
}

void MainWindow::saveBackup()
{
    if (modified) {
        int ans = QMessageBox::question(this, QStringLiteral("Backup config file"),
                                        QStringLiteral("Do you want to preserve the original file?"));
        if (ans == QMessageBox::Yes) {
            QString time_stamp = cmd.getCmdOut(QStringLiteral("date +%y%m%d_%H%m%S"));
            QFileInfo fi(file_name);
            QString new_name = fi.canonicalPath() + "/" + fi.baseName() + "_" + time_stamp;
            if (fi.completeSuffix().length() > 0)
                new_name += "." + fi.completeSuffix();
            QFile::copy(file_name + ".bak", new_name);
            QMessageBox::information(this, QStringLiteral("Backed up config file"),
                                     "The original configuration was backed up to " + new_name);
        }
    }
    QFile(file_name + ".bak").remove();
}

// write color change back to the file
void MainWindow::writeColor(QWidget *widget, const QColor &color)
{
    QRegularExpression regexp_old_comment_line(old_comment_line);
    QRegularExpression regexp_lua_comment_line(lua_comment_line);
    QRegularExpression regexp_lua_color(capture_lua_color);
    QRegularExpression regexp_old_color(capture_old_color);

    QRegularExpressionMatch match_color;
    QRegularExpression regexp_color;
    QRegularExpression regexp_comment_line;

    QString comment_sep;
    //    QString block_comment_start = "--[[";
    //    QString block_comment_end = "]]";

    if (is_lua_format) {
        regexp_color = regexp_lua_color;
        regexp_comment_line = regexp_lua_comment_line;
        comment_sep = QStringLiteral("--");
    } else {
        regexp_color = regexp_old_color;
        regexp_comment_line = regexp_old_comment_line;
        comment_sep = QStringLiteral("#");
    }

    QString color_name;

    QString item_name;
    if (widget->objectName() == QLatin1String("widgetDefaultColor"))
        item_name = QStringLiteral("default_color");
    else if (widget->objectName() == QLatin1String("widgetColor0"))
        item_name = QStringLiteral("color0");
    else if (widget->objectName() == QLatin1String("widgetColor1"))
        item_name = QStringLiteral("color1");
    else if (widget->objectName() == QLatin1String("widgetColor2"))
        item_name = QStringLiteral("color2");
    else if (widget->objectName() == QLatin1String("widgetColor3"))
        item_name = QStringLiteral("color3");
    else if (widget->objectName() == QLatin1String("widgetColor4"))
        item_name = QStringLiteral("color4");
    else if (widget->objectName() == QLatin1String("widgetColor5"))
        item_name = QStringLiteral("color5");
    else if (widget->objectName() == QLatin1String("widgetColor6"))
        item_name = QStringLiteral("color6");
    else if (widget->objectName() == QLatin1String("widgetColor7"))
        item_name = QStringLiteral("color7");
    else if (widget->objectName() == QLatin1String("widgetColor8"))
        item_name = QStringLiteral("color8");
    else if (widget->objectName() == QLatin1String("widgetColor9"))
        item_name = QStringLiteral("color9");

    const QStringList list = file_content.split(QStringLiteral("\n"));
    QStringList new_list;
    new_list.reserve(list.size());
    bool lua_block_comment = false;
    for (const QString &row : list) {
        // lua comment block
        QString trow = row.trimmed();
        if (is_lua_format) {
            if (lua_block_comment) {
                if (trow.endsWith(block_comment_end)) {
                    lua_block_comment = false;
                    if (debug)
                        qDebug() << "Lua block comment end 'ENDS WITH LINE' found";
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_end)) {
                    lua_block_comment = false;
                    QStringList ltrow = trow.split(block_comment_end);
                    ltrow.removeFirst();
                    trow = ltrow.join(block_comment_end);
                    trow = trow.trimmed();
                    if (debug)
                        qDebug() << "Lua block comment end CONTAINS line found: after ]]: " << trow;
                } else {
                    new_list << row;
                    continue;
                }
            }
            if (!lua_block_comment) {
                if (trow.startsWith(block_comment_start)) {
                    if (debug)
                        qDebug() << "Lua block comment 'STARTS WITH LINE' found";
                    lua_block_comment = true;
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_start)) {
                    lua_block_comment = true;
                    QStringList ltrow = trow.split(block_comment_start);
                    trow = ltrow[0];
                    trow = trow.trimmed();
                    if (debug)
                        qDebug() << "Lua block comment start CONTAINS line found: before start --[[: " << trow;
                }
            }
        }
        // comment line
        if (trow.startsWith(comment_sep)) {
            new_list << row;
            continue;
        }

        match_color = regexp_color.match(row);
        if (match_color.hasMatch() && match_color.captured(QStringLiteral("color_item")) == item_name) {
            color_name = color.name();
            if (!is_lua_format)
                color_name = color.name().remove('#');
            new_list << match_color.captured(QStringLiteral("before")) + color_name
                            + match_color.captured(QStringLiteral("after"));
        } else {
            new_list << row;
            continue;
        }
    }
    file_content = new_list.join(QStringLiteral("\n")).append("\n");
    writeFile(file_name, file_content);
}

void MainWindow::writeFile(const QString &file_name, const QString &content)
{
    QFile file(file_name);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << content;
        file.close();
        modified = true;
    } else {
        qDebug() << "Error opening file " + file_name + " for output";
    }
}

void MainWindow::cleanup()
{
    saveBackup();

    //    if (!cmd->terminate())
    //        cmd->kill();
}

void MainWindow::pickColor(QWidget *widget)
{
    QColor color = QColorDialog::getColor(widget->palette().color(QWidget::backgroundRole()));
    if (color.isValid()) {
        setColor(widget, color);
        writeColor(widget, color);
    }
}

void MainWindow::setColor(QWidget *widget, const QColor &color)
{
    widget->parentWidget()->show();
    if (color.isValid()) {
        QPalette pal = palette();
        pal.setColor(QPalette::Window, color);
        widget->setAutoFillBackground(true);
        widget->setPalette(pal);
    }
}

void MainWindow::cmdStart() { setCursor(QCursor(Qt::BusyCursor)); }

void MainWindow::cmdDone()
{
    //   ui->progressBar->setValue(ui->progressBar->maximum());
    setCursor(QCursor(Qt::ArrowCursor));
    // cmd->disconnect();
}

void MainWindow::setConnections()
{
    //    connect(cmd, &Cmd::started, this, &MainWindow::cmdStart);
    //    connect(cmd, &Cmd::finished, this, &MainWindow::cmdDone);
}

void MainWindow::on_pushAbout_clicked()
{
    this->hide();
    const QString url = QStringLiteral("file:///usr/share/doc/mx-conky/license.html");
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
    btnCancel->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));

    msgBox.exec();

    if (msgBox.clickedButton() == btnLicense) {
        if (system("command -v mx-viewer") == 0)
            system("mx-viewer " + url.toUtf8() + "&");
        else
            system("xdg-open " + url.toUtf8());
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
        btnClose->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
        connect(btnClose, &QPushButton::clicked, changelog, &QDialog::close);

        auto *layout = new QVBoxLayout;
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog->setLayout(layout);
        changelog->exec();
    }
    this->show();
}

void MainWindow::on_pushHelp_clicked()
{
    const QString url = QStringLiteral("/usr/share/doc/mx-conky/mx-conky.html");
    QString cmd;
    if (system("command -v mx-viewer") == 0)
        cmd = QString("mx-viewer " + url + " " + tr("MX Conky Help") + "&");
    else
        cmd = QString("xdg-open " + url);
    system(cmd.toUtf8());
}

void MainWindow::on_pushDefaultColor_clicked() { pickColor(ui->widgetDefaultColor); }

void MainWindow::on_pushColor0_clicked() { pickColor(ui->widgetColor0); }

void MainWindow::on_pushColor1_clicked() { pickColor(ui->widgetColor1); }

void MainWindow::on_pushColor2_clicked() { pickColor(ui->widgetColor2); }

void MainWindow::on_pushColor3_clicked() { pickColor(ui->widgetColor3); }

void MainWindow::on_pushColor4_clicked() { pickColor(ui->widgetColor4); }

void MainWindow::on_pushColor5_clicked() { pickColor(ui->widgetColor5); }
void MainWindow::on_pushColor6_clicked() { pickColor(ui->widgetColor6); }
void MainWindow::on_pushColor7_clicked() { pickColor(ui->widgetColor7); }
void MainWindow::on_pushColor8_clicked() { pickColor(ui->widgetColor8); }
void MainWindow::on_pushColor9_clicked() { pickColor(ui->widgetColor9); }

void MainWindow::on_pushToggleOn_clicked()
{
    QString cmd;
    if (checkConkyRunning())
        cmd = QString("pkill -u $(id -nu) -x conky");
    else
        cmd = QString("cd \"$(dirname '%1')\"; conky -c '%1' &").arg(file_name);
    system(cmd.toUtf8());
    checkConkyRunning();
}

void MainWindow::on_pushRestore_clicked()
{
    if (QFile(file_name + ".bak").exists())
        QFile(file_name).remove();
    if (QFile::copy(file_name + ".bak", file_name))
        refresh();
}

void MainWindow::on_pushEdit_clicked()
{
    this->hide();
    QString editor;
    Cmd cmd;

    QString run = QStringLiteral("set -o pipefail; ");
    run += QLatin1String("XDHD=${XDG_DATA_HOME:-$HOME/.local/share}:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}; ");
    run += QLatin1String("eval grep -sh -m1 ^Exec {${XDHD//://applications/,}/applications/}");
    run += QLatin1String("$(xdg-mime query default text/plain)  2>/dev/null ");
    run += QLatin1String("| head -1 | sed 's/^Exec=//' | tr -d '\"' | tr -s ' ' | sed 's/@@//g; s/%f//I; s/%u//I' ");
    bool quiet = true;
    if (debug)
        qDebug() << "run-cmd: " << run;
    if (debug)
        quiet = false;
    bool error = cmd.run(run, editor, quiet);
    if (debug)
        qDebug() << "run:'" + editor + " '" + file_name.toUtf8() + "'";
    if (editor.startsWith(QLatin1String("kate -s")) || editor.startsWith(QLatin1String("kate --start")))
        editor = QStringLiteral("kate");
    if (system(editor.toUtf8() + " '" + file_name.toUtf8() + "'") != 0) {
        if (error || (system("which " + editor.toUtf8() + " 1>/dev/null") != 0)) {
            if (debug)
                qDebug() << "no default text editor defined" << editor;
            // try featherpad explicitly
            if (system("which featherpad 1>/dev/null") == 0) {
                if (debug)
                    qDebug() << "try featherpad text editor ";
                system("featherpad '" + file_name.toUtf8() + "'");
            }
            refresh();
            this->show();
            return;
        }
    }
    refresh();
    this->show();
}

void MainWindow::on_pushChange_clicked()
{
    saveBackup();
    QString selected = QFileDialog::getOpenFileName(nullptr, QObject::tr("Select Conky Manager config file"),
                                                    QFileInfo(file_name).path());
    if (!selected.isEmpty())
        file_name = selected;
    refresh();
}

void MainWindow::on_radioDesktop1_clicked()
{
    QString comment_sep;
    QRegularExpression regexp_lua_owh(capture_lua_owh);
    QRegularExpression regexp_old_owh(capture_old_owh);
    QRegularExpression regexp_owh;
    QRegularExpressionMatch match_owh;

    bool lua_block_comment = false;

    if (is_lua_format) {
        regexp_owh = regexp_lua_owh;
        comment_sep = QStringLiteral("--");
    } else {
        regexp_owh = regexp_old_owh;
        comment_sep = QStringLiteral("#");
    }

    const QStringList list = file_content.split(QStringLiteral("\n"));
    QStringList new_list;
    new_list.reserve(list.size());
    for (const QString &row : list) {
        // lua comment block
        QString trow = row.trimmed();
        if (is_lua_format) {
            if (lua_block_comment) {
                if (trow.endsWith(block_comment_end)) {
                    lua_block_comment = false;
                    if (debug)
                        qDebug() << "Lua block comment end 'ENDS WITH LINE' found";
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_end)) {
                    lua_block_comment = false;
                    QStringList ltrow = trow.split(block_comment_end);
                    ltrow.removeFirst();
                    trow = ltrow.join(block_comment_end);
                    trow = trow.trimmed();
                    if (debug)
                        qDebug() << "Lua block comment end CONTAINS line found: after ]]: " << trow;
                } else {
                    new_list << row;
                    continue;
                }
            }
            if (!lua_block_comment) {
                if (trow.startsWith(block_comment_start)) {
                    if (debug)
                        qDebug() << "Lua block comment 'STARTS WITH LINE' found";
                    lua_block_comment = true;
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_start)) {
                    lua_block_comment = true;
                    QStringList ltrow = trow.split(block_comment_start);
                    trow = ltrow[0];
                    trow = trow.trimmed();
                    if (debug)
                        qDebug() << "Lua block comment start CONTAINS line found: before start --[[: " << trow;
                }
            }
        }
        // comment line
        if (trow.startsWith(comment_sep)) {
            new_list << row;
            continue;
        }

        if (!trow.startsWith(QLatin1String("own_window_hints"))) {
            new_list << row;
            continue;
        } else {

            if (debug)
                qDebug() << "on_radioDesktops1_clicked: own_window_hints found row : " << row;
            if (debug)
                qDebug() << "on_radioDesktops1_clicked: own_window_hints found trow: " << trow;
            match_owh = regexp_owh.match(row);
            if (match_owh.hasMatch()
                && match_owh.captured(QStringLiteral("item")) == QLatin1String("own_window_hints")) {
                QString owh_value = match_owh.captured(QStringLiteral("value"));
                owh_value.replace(QLatin1String(",sticky"), QLatin1String(""));
                owh_value.replace(QLatin1String("sticky"), QLatin1String(""));
                QString new_row = match_owh.captured(QStringLiteral("before")) + owh_value
                                  + match_owh.captured(QStringLiteral("after"));
                if (debug)
                    qDebug() << "Removed sticky: " << new_row;
                new_list << new_row;
            } else {
                if (debug)
                    qDebug() << "ERROR : " << row;
                if (is_lua_format) {
                    if (debug)
                        qDebug() << "regexp_owh : " << regexp_lua_owh;
                    if (debug)
                        qDebug() << "regexp_owh : " << regexp_owh;
                } else {
                    if (debug)
                        qDebug() << "regexp_owh : " << regexp_old_owh;
                }
                new_list << row;
            }
        }
    }

    file_content = new_list.join(QStringLiteral("\n")).append("\n");
    writeFile(file_name, file_content);
}

void MainWindow::on_radioAllDesktops_clicked()
{
    QString comment_sep;
    QRegularExpression regexp_lua_owh(capture_lua_owh);
    QRegularExpression regexp_old_owh(capture_old_owh);
    QRegularExpression regexp_owh;
    QRegularExpressionMatch match_owh;
    QString conky_config_lua_end = QStringLiteral("}");
    QString conky_config_old_end = QStringLiteral("TEXT");
    QString conky_config_end;
    QString conky_sticky;
    bool lua_block_comment {false};
    bool conky_config {false};

    if (is_lua_format) {
        regexp_owh = regexp_lua_owh;
        comment_sep = QStringLiteral("--");
        conky_config = false;
        conky_config_end = conky_config_lua_end;
        conky_sticky = QStringLiteral("\town_window_hints = 'sticky',");
    } else {
        regexp_owh = regexp_old_owh;
        comment_sep = QStringLiteral("#");
        conky_config = true;
        conky_config_end = conky_config_old_end;
        conky_sticky = QStringLiteral("own_window_hints sticky");
    }

    bool found = false;
    const QStringList list = file_content.split(QStringLiteral("\n"));
    QStringList new_list;
    QString trow;
    for (const QString &row : list) {
        trow = row.trimmed();
        if (is_lua_format) {
            if (lua_block_comment) {
                if (trow.endsWith(block_comment_end)) {
                    lua_block_comment = false;
                    if (debug)
                        qDebug() << "Lua block comment end 'ENDS WITH LINE' found";
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_end)) {
                    lua_block_comment = false;
                    QStringList ltrow = trow.split(block_comment_end);
                    ltrow.removeFirst();
                    trow = ltrow.join(block_comment_end);
                    trow = trow.trimmed();
                    if (debug)
                        qDebug() << "Lua block comment end CONTAINS line found: after ]]: " << trow;
                } else {
                    new_list << row;
                    continue;
                }
            }
            if (!lua_block_comment) {
                if (trow.startsWith(block_comment_start)) {
                    if (debug)
                        qDebug() << "Lua block comment 'STARTS WITH LINE' found";
                    lua_block_comment = true;
                    new_list << row;
                    continue;
                }
                if (trow.contains(block_comment_start)) {
                    lua_block_comment = true;
                    QStringList ltrow = trow.split(block_comment_start);
                    trow = ltrow[0];
                    trow = trow.trimmed();
                    if (debug)
                        qDebug() << "Lua block comment start CONTAINS line found: before start --[[: " << trow;
                }
            }
        }
        // comment line
        if (trow.startsWith(comment_sep)) {
            new_list << row;
            continue;
        }
        if (is_lua_format && trow.startsWith(QLatin1String("conky.config")))
            conky_config = true;

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

        if (!trow.startsWith(QLatin1String("own_window_hints "))) {
            new_list << row;
            continue;
        } else {
            if (debug)
                qDebug() << "on_radioAllDesktops_clicked: own_window_hints found row : " << row;
            if (debug)
                qDebug() << "on_radioAllDesktops_clicked: own_window_hints found trow: " << trow;
            match_owh = regexp_owh.match(row);
            if (match_owh.hasMatch()
                && match_owh.captured(QStringLiteral("item")) == QLatin1String("own_window_hints")) {
                QString owh_value = match_owh.captured(QStringLiteral("value"));
                if (owh_value.length() == 0)
                    owh_value = QStringLiteral("sticky");
                else
                    owh_value.append(",sticky");
                QString new_row = match_owh.captured(QStringLiteral("before")) + owh_value
                                  + match_owh.captured(QStringLiteral("after"));
                if (debug)
                    qDebug() << "Append sticky: " << new_row;

                new_list << match_owh.captured(QStringLiteral("before")) + owh_value
                                + match_owh.captured(QStringLiteral("after"));
                found = true;
            } else {
                if (debug)
                    qDebug() << "ERROR : " << row;
                if (is_lua_format) {
                    if (debug)
                        qDebug() << "regexp_owh : " << regexp_lua_owh;
                } else {
                    if (debug)
                        qDebug() << "regexp_owh : " << regexp_old_owh;
                }
                found = false;
                new_list << row;
            }
        }
    }

    file_content = new_list.join(QStringLiteral("\n")).append("\n");
    writeFile(file_name, file_content);
}

void MainWindow::on_radioDayLong_clicked()
{
    file_content.replace(QLatin1String("%a"), QLatin1String("%A"));
    writeFile(file_name, file_content);
}

void MainWindow::on_radioDayShort_clicked()
{
    file_content.replace(QLatin1String("%A"), QLatin1String("%a"));
    writeFile(file_name, file_content);
}

void MainWindow::on_radioMonthLong_clicked()
{
    file_content.replace(QLatin1String("%b"), QLatin1String("%B"));
    writeFile(file_name, file_content);
}

void MainWindow::on_radioMonthShort_clicked()
{
    file_content.replace(QLatin1String("%B"), QLatin1String("%b"));
    writeFile(file_name, file_content);
}

void MainWindow::on_pushCM_clicked()
{
    this->hide();
    system("command -v conky-manager && conky-manager || command -v conky-manager2  && conky-manager2");
    this->show();
}

void MainWindow::closeEvent(QCloseEvent * /*event*/) { settings.setValue(QStringLiteral("geometery"), saveGeometry()); }
