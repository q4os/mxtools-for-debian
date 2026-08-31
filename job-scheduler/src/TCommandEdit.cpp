/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "TCommandEdit.h"

#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtGui>
#include <ranges>

#include "Clib.h"
#include "CronTime.h"
#include "Crontab.h"
#include "TimeDialog.h"
#include "constants.h"
#include <chrono>

using namespace std::chrono_literals;

namespace {
// Bound how long we'll wait on the "command -v" child process before giving
// up and failing open (no warning), rather than risking the UI hanging.
constexpr int kCommandCheckTimeoutMs = 2000;
}

TCommandEdit::TCommandEdit(QWidget *parent)
    : QWidget(parent)
{

    QPushButton *timeButton = nullptr;
    QGroupBox *exeBox = nullptr;
    QHBoxLayout *h = nullptr;

    auto *mainLayout = new QVBoxLayout;
    {
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addWidget(new QLabel(tr("User:"), this));
            h->addWidget((userCombo = new QComboBox(this)));
            h->addWidget((userLabel = new QLabel(QLatin1String(""), this)));
            h->addStretch();
        }
        mainLayout->addSpacing(5);
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addWidget(new QLabel(tr("Time:"), this));
            h->addWidget((timeEdit = new QLineEdit(this)));
            h->addWidget((timeButton = new QPushButton(
                              QIcon::fromTheme(QStringLiteral("edit-symbolic"), QIcon(":/images/edit_small.png")),
                              tr("Time String E&ditor"), this)));
            timeButton->setMinimumSize(
                QSize(JobScheduler::COMMAND_TIME_BUTTON_MIN_WIDTH, timeButton->maximumHeight()));
        }
        mainLayout->addSpacing(5);
        mainLayout->addWidget(new QLabel(tr("Command:"), this));
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addWidget((commandEdit = new QLineEdit(this)));
        }
        mainLayout->addWidget((commandWarningLabel = new QLabel(QLatin1String(""), this)));
        mainLayout->addSpacing(5);
        mainLayout->addWidget(new QLabel(tr("Comment:"), this));
        mainLayout->addWidget((commentEdit = new QTextEdit(this)));
        mainLayout->addSpacing(5);
        mainLayout->addWidget((exeBox = new QGroupBox(tr("Job Schedule:"), this)));
        {
            exeBox->setLayout((h = new QHBoxLayout));
            {
                h->addWidget((exeLabel = new QLabel(QStringLiteral("\n\n\n\n\n\n\n"), this)));
            }
        }
    }
    setLayout(mainLayout);

    exeLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    userLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    userCombo->addItems(Clib::allUsers());
    userCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    userLabel->hide();

    QPalette warnPal = commandWarningLabel->palette();
    warnPal.setColor(QPalette::WindowText, QColor(189, 55, 44));
    commandWarningLabel->setPalette(warnPal);
    commandWarningLabel->hide();

    commentEdit->setAutoFormatting(QTextEdit::AutoNone);

    viewChanging = true;

    connect(commandEdit, &QLineEdit::textEdited, this, &TCommandEdit::commandEdited);
    connect(timeEdit, &QLineEdit::textEdited, this, &TCommandEdit::timeEdited);
    connect(commentEdit, &QTextEdit::textChanged, this, &TCommandEdit::commentEdited);
    connect(userCombo, qOverload<int>(&QComboBox::activated), this, &TCommandEdit::userChanged);
    connect(&timer, &QTimer::timeout, this, &TCommandEdit::resetExeTime);
    connect(timeButton, &QPushButton::clicked, this, &TCommandEdit::doTimeDialog);
}

void TCommandEdit::changeCurrent(Crontab *cron, TCommand *cmnd)
{
    viewChanging = true;
    tCommand = cmnd;
    if (cmnd == nullptr) {
        setEnabled(false);
        timer.stop();
    } else {
        setEnabled(true);
        timer.start(1min);
        timeEdit->setText(tCommand->getTime());
        timeEdit->setCursorPosition(0);
        if (cron->getCronOwner() == QLatin1String("/etc/crontab")) {
            userCombo->setCurrentIndex(userCombo->findText(tCommand->getUser()));
            userCombo->show();
            userLabel->hide();
        } else {
            userLabel->setText("  " + cron->getCronOwner() + "  ");
            userCombo->hide();
            userLabel->show();
        }
        commandEdit->setText(tCommand->getCommand());
        commandEdit->setCursorPosition(0);
        commentEdit->setPlainText(tCommand->getComment());
        setExecuteList(tCommand->getTime());
        updateCommandWarning(tCommand->getCommand());
    }
    viewChanging = false;
}

bool TCommandEdit::isCommandAvailable(const QString &exe)
{
    if (!QStandardPaths::findExecutable(exe).isEmpty()) {
        return true;
    }

    // Not a standalone executable - cron runs commands via `sh -c`, so it may
    // still be a shell builtin (cd, echo, ".", ":", ...) or function, which
    // findExecutable() can't see. Ask the shell itself via "command -v",
    // passed as an argument (not interpolated) to avoid any shell injection.
    QProcess proc;
    proc.start(QStringLiteral("sh"), {QStringLiteral("-c"), QStringLiteral("command -v \"$1\" >/dev/null 2>&1"),
                                      QStringLiteral("sh"), exe});
    if (!proc.waitForStarted(kCommandCheckTimeoutMs)) {
        return true; // fail open: can't check, so don't warn
    }
    if (!proc.waitForFinished(kCommandCheckTimeoutMs)) {
        proc.kill();
        proc.waitForFinished(kCommandCheckTimeoutMs);
        return true; // fail open
    }
    return proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0;
}

void TCommandEdit::updateCommandWarning(const QString &command)
{
    // Heuristic only: takes the first whitespace-separated token as the
    // command. Shell constructs like pipes and env-var prefixes ("FOO=bar
    // cmd") aren't parsed, so this can still flag commands that are actually
    // fine - it's a hint, not validation.
    const QString exe = command.section(QRegularExpression(QStringLiteral("\\s+")), 0, 0);
    if (exe.isEmpty() || isCommandAvailable(exe)) {
        commandWarningLabel->hide();
        return;
    }
    commandWarningLabel->setText(tr("Warning: \"%1\" was not found").arg(exe));
    commandWarningLabel->show();
}

void TCommandEdit::setExecuteList(const QString &time)
{

    CronTime cronTime(time);
    if (!cronTime.isValid()) {
        exeLabel->setText("\n\n   " + tr("Time Format Error") + "\n\n\n");
        return;
    }
    QDate today = QDate::currentDate();
    QDate tomorrow = today.addDays(1);
    QDateTime cur(QDateTime::currentDateTime());
    QDateTime dt = cur;
    QString str;
    for (int i : std::views::iota(0, 7)) {
        if (!str.isEmpty()) {
            str += '\n';
        }
        dt = cronTime.getNextTime(dt);
        if (!dt.isValid()) {
            exeLabel->setText("\n\n   " + tr("No matching schedule") + "\n\n\n");
            return;
        }
        qint64 sec = cur.secsTo(dt);
        str += QStringLiteral("%1 - %2:%3 later")
                   .arg(dt.toString(QStringLiteral("yyyy-MM-dd(ddd) hh:mm")))
                   .arg(sec / (60 * 60))
                   .arg((sec / 60) % 60, 2, 10, QChar('0'));
        if (dt.date() == today) {
            str += QStringLiteral(" - %1").arg(tr("Today"));
        } else if (dt.date() == tomorrow) {
            str += QStringLiteral(" - %1").arg(tr("Tomorrow"));
        }
    }
    exeLabel->setText(str);
}

void TCommandEdit::commandEdited(const QString &str)
{
    tCommand->setCommand(str);
    updateCommandWarning(str);
    emit dataChanged();
}

void TCommandEdit::timeEdited(const QString &str)
{
    tCommand->setTime(str);
    emit dataChanged();
    setExecuteList(str);
}

void TCommandEdit::commentEdited()
{
    if (!viewChanging) {
        tCommand->setComment(commentEdit->toPlainText());
        emit dataChanged();
    }
}

void TCommandEdit::userChanged(int index)
{

    tCommand->setUser(userCombo->itemText(index));
    emit dataChanged();
}

void TCommandEdit::resetExeTime()
{
    if (timeEdit->text().isEmpty()) {
        return;
    }
    setExecuteList(timeEdit->text());
}

void TCommandEdit::doTimeDialog()
{
    TimeDialog dialog(timeEdit->text(), this);
    int ret = dialog.exec();
    if (ret == QDialog::Accepted) {
        QString s = dialog.time();
        if (timeEdit->text() != s) {
            timeEdit->setText(s);
            setExecuteList(s);
            tCommand->setTime(s);
            emit dataChanged();
        }
    }
}
