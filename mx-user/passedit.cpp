/***************************************************************************
 * PassEdit class - QLineEdit operating as a pair for editing passwords.
 *
 *   Copyright (C) 2021 by AK-47
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * This file is part of the gazelle-installer.
 **************************************************************************/

#include "passedit.h"
#include <QApplication>
#include <QContextMenuEvent>
#include <QDebug>
#include <QFile>
#include <QMenu>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <random>
#include <zxcvbn.h>

// Password generator parameters applicable accross every PassEdit instance.
static const int GEN_WORD_MAX = 6;       // Maximum number of characters per word.
static const int GEN_NUMBER_MAX = 999;   // Numbers will go from 0 to GEN_NUMBER_MAX without duplicates.
static const int GEN_WORD_NUM_RATIO = 3; // Ratio N:1 of words to numbers (if less than GEN_NUMBER_MAX).

PassEdit::PassEdit(QLineEdit *master, QLineEdit *slave, int min, QObject *parent) noexcept
    : QObject(parent),
      master(master),
      slave(slave),
      min(min)
{
    disconnect(master);
    disconnect(slave);
    master->setClearButtonEnabled(true);
    slave->setClearButtonEnabled(true);
    master->installEventFilter(this);
    master->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(master, &QLineEdit::textChanged, this, &PassEdit::masterTextChanged);
    connect(slave, &QLineEdit::textChanged, this, &PassEdit::slaveTextChanged);
    if (min == 0) {
        lastValid = true; // Control starts with no text
    }

    generate(); // Pre-load the generator
    if (!gentext.isEmpty()) {
        connect(master, &QWidget::customContextMenuRequested, this, &PassEdit::masterContextMenu);
    }

    actionEye = master->addAction(QIcon(":/eye-show"), QLineEdit::TrailingPosition);
    actionEye->setCheckable(true);
    connect(actionEye, &QAction::toggled, this, &PassEdit::eyeToggled);
    eyeToggled(false); // Initialize the eye.
    actionGauge = slave->addAction(QIcon(":/gauge/0"), QLineEdit::TrailingPosition);

    masterTextChanged(master->text());
}

void PassEdit::generate() noexcept
{
    static QStringList words;
    static int pos;
    if (words.isEmpty()) {
        QFile file("/usr/share/dict/words");
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            while (!file.atEnd()) {
                const QByteArray &word = file.readLine().trimmed();
                if (word.size() <= GEN_WORD_MAX) {
                    words.append(word);
                }
            }
            file.close();
        }
        if (words.isEmpty()) {
            return;
        }
        for (int i = std::min(GEN_NUMBER_MAX, (words.count() / GEN_WORD_NUM_RATIO) - 1); i >= 0; --i) {
            words.append(QString::number(i));
        }
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        pos = words.count();
    }
    gentext.clear();
    double entropy = 0;
    const int genmax = master->maxLength();
    do {
        if (pos >= words.count()) {
            std::shuffle(words.begin(), words.end(), std::mt19937(std::random_device()()));
            pos = 0;
        }
        const QString &word = words.at(pos);
        const int origlen = gentext.length();
        if (origlen != 0) {
            gentext.append(' ');
        }
        gentext.append(word);
        if (gentext.length() > genmax) {
            gentext.truncate(origlen);
            break;
        }
        entropy = ZxcvbnMatch(gentext.toUtf8().constData(), nullptr, nullptr);
        ++pos;
    } while (gentext.length() <= min || entropy <= VeryStrong);
}

void PassEdit::masterContextMenu(QPoint pos) noexcept
{
    QMenu *menu = master->createStandardContextMenu();
    menu->addSeparator();
    QAction *actGenPass = menu->addAction(gentext);
    connect(actGenPass, &QAction::triggered, this, [this]() {
        master->setText(gentext);
        generate();
    });
    menu->popup(master->mapToGlobal(pos));
}

bool PassEdit::eventFilter(QObject *watched, QEvent *event) noexcept
{
    const QEvent::Type etype = event->type();
    if (etype == QEvent::EnabledChange || etype == QEvent::Hide) {
        auto *w = qobject_cast<QLineEdit *>(watched);
        if ((actionEye != nullptr) && !(w->isVisible() && w->isEnabled())) {
            actionEye->setChecked(false);
        }
    }
    return false;
}

void PassEdit::masterTextChanged(const QString &text) noexcept
{
    slave->clear();
    master->setPalette(QPalette());
    slave->setPalette(QPalette());
    const bool valid = (text.isEmpty() && min == 0);

    const double entropy = ZxcvbnMatch(text.toUtf8().constData(), nullptr, nullptr);
    const std::array<double, 5> thresholds {Negligible, VeryWeak, Weak, Strong, VeryStrong};
    int score = 0;
    for (const double threshold : thresholds) {
        if (entropy < threshold || entropy == 0) {
            break;
        } else {
            ++score;
        }
    }

    actionGauge->setIcon(QIcon(":/gauge/" + QString::number(score)));
    const std::array<QString, 6> ratings
        = {QT_TR_NOOP("Negligible"), QT_TR_NOOP("Very weak"), QT_TR_NOOP("Weak"),
           QT_TR_NOOP("Moderate"),   QT_TR_NOOP("Strong"),    QT_TR_NOOP("Very strong")};
    actionGauge->setToolTip(tr("Password strength: %1").arg(ratings.at(score)));

    // The validation could change if the box is empty and no minimum is set.
    if (valid != lastValid) {
        lastValid = valid;
        emit validationChanged(valid);
    }
}

void PassEdit::slaveTextChanged(const QString &text) noexcept
{
    QPalette pal = master->palette();
    bool valid = true;
    if (text == master->text()) {
        QColor col(255, 255, 0, 40);
        if (text.length() >= min) {
            col.setRgb(0, 255, 0, 40);
        } else {
            valid = false;
        }
        pal.setColor(QPalette::Base, col);
    } else {
        pal.setColor(QPalette::Base, QColor(255, 0, 0, 70));
        valid = false;
    }
    master->setPalette(pal);
    slave->setPalette(pal);
    if (valid != lastValid) {
        lastValid = valid;
        emit validationChanged(valid);
    }
}

void PassEdit::eyeToggled(bool checked) noexcept
{
    actionEye->setIcon(QIcon(checked ? ":/eye-hide" : ":/eye-show"));
    actionEye->setToolTip(checked ? tr("Hide the password") : tr("Show the password"));
    master->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}
