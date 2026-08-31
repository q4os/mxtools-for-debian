/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "VariableEdit.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>
#include <QtGui>

#include <memory>

#include "Clib.h"
#include "Crontab.h"
#include "VariableModel.h"
#include "VariableView.h"

VariableEdit::VariableEdit(QWidget *parent)
    : QWidget(parent)
{

    QFrame *sepFrame = nullptr;
    QPushButton *newButton = nullptr;
    QPushButton *deleteButton = nullptr;
    QGroupBox *varGroup = nullptr;
    QGridLayout *g = nullptr;
    QHBoxLayout *h = nullptr;
    QHBoxLayout *h2 = nullptr;
    QVBoxLayout *v = nullptr;

    variableModel = new VariableModel(this);

    auto *mainLayout = new QVBoxLayout;
    {
        mainLayout->addWidget((commentEdit = new QTextEdit()));

        mainLayout->addWidget((varGroup = new QGroupBox(tr("Variables"))));
        varGroup->setLayout((g = new QGridLayout));
        {
            g->addLayout((h = new QHBoxLayout), 0, 0, 1, 2);
            {
                h->addWidget(new QLabel(tr("Mail:")));
                h->addWidget((mailOffRadio = new QRadioButton(tr("Don't send"))));
                h->addWidget((mailOnRadio = new QRadioButton(tr("Send"))));
                h->addWidget(mailToRadio = new QRadioButton(tr("To")));
                h->addWidget((userCombo = new QComboBox()));
            }
            g->addWidget((sepFrame = new QFrame()), 1, 0, 1, 2);
            g->addLayout((v = new QVBoxLayout), 2, 0, 1, 1);
            {
                v->addWidget(variableView = new VariableView(variableModel));
                v->addLayout((h2 = new QHBoxLayout));
                {
                    h2->addStretch();
                    h2->addWidget(
                        (newButton = new QPushButton(
                             QIcon::fromTheme(QStringLiteral("filenew"), QIcon(":/images/filenew.png")), tr("&New"))));
                    h2->addWidget((deleteButton = new QPushButton(QIcon::fromTheme(QStringLiteral("edit-delete"),
                                                                                   QIcon(":/images/editdelete.png")),
                                                                  tr("&Delete"))));
                }
            }

            g->addLayout((v = new QVBoxLayout), 2, 1, 1, 1);
            {
                v->addLayout((h = new QHBoxLayout));
                {
                    h->addWidget(new QLabel(tr("Name:")));
                    h->addWidget((nameEdit = new QLineEdit()));
                }
                v->addLayout((h = new QHBoxLayout));
                {
                    h->addWidget(new QLabel(tr("Value:")));
                    h->addWidget((valueEdit = new QLineEdit()));
                }
                v->addWidget(new QLabel(tr("Comment:")));
                v->addWidget((varCommentEdit = new QTextEdit()));
            }
        }
    }
    setLayout(mainLayout);

    userCombo->addItems(Clib::allUsers());

    sepFrame->setFrameShape(QFrame::HLine);
    sepFrame->setFrameShadow(QFrame::Sunken);

    connect(variableView, &VariableView::changeVar, this, &VariableEdit::varViewSelected);
    connect(commentEdit, &QTextEdit::textChanged, this, &VariableEdit::commentChanged);
    connect(nameEdit, &QLineEdit::textEdited, this, &VariableEdit::varEdited);
    connect(valueEdit, &QLineEdit::textEdited, this, &VariableEdit::valEdited);
    connect(varCommentEdit, &QTextEdit::textChanged, this, &VariableEdit::varCommentChanged);
    connect(deleteButton, &QPushButton::clicked, this, &VariableEdit::deleteClicked);
    connect(newButton, &QPushButton::clicked, this, &VariableEdit::newClicked);
    connect(mailOnRadio, &QRadioButton::toggled, this, &VariableEdit::mailOnClicked);
    connect(mailOffRadio, &QRadioButton::toggled, this, &VariableEdit::mailOffClicked);
    connect(mailToRadio, &QRadioButton::toggled, this, &VariableEdit::mailToClicked);
    connect(userCombo, qOverload<int>(&QComboBox::activated), this, &VariableEdit::userActivated);

    viewChanging = true;
    varViewChanging = true;
}

void VariableEdit::changeCurrent(Crontab *cron, TCommand * /*unused*/)
{
    if (cron == nullptr) {
        setEnabled(false);
        return;
    }

    setEnabled(true);
    viewChanging = true;

    crontab = cron;
    commentEdit->setPlainText(cron->getComment());
    variableModel->resetData(&cron->getVariables());
    variableView->resetView();
    setMailCombo(cron->getVariables());

    viewChanging = false;
}

void VariableEdit::varViewSelected(Variable *var)
{
    if (var == nullptr) {
        nameEdit->setEnabled(false);
        valueEdit->setEnabled(false);
        varCommentEdit->setEnabled(false);
        return;
    }
    nameEdit->setEnabled(true);
    valueEdit->setEnabled(true);
    varCommentEdit->setEnabled(true);
    variable = var;
    varViewChanging = true;
    nameEdit->setText(var->getName());
    valueEdit->setText(var->getValue());
    nameEdit->setCursorPosition(0);
    valueEdit->setCursorPosition(0);
    varCommentEdit->setPlainText(var->getComment());
    varViewChanging = false;
}

void VariableEdit::commentChanged()
{
    if (!viewChanging) {
        crontab->setComment(commentEdit->toPlainText());
        emit dataChanged();
    }
}
void VariableEdit::varEdited(const QString &str)
{
    variable->setName(str);
    variableView->varDataChanged();
    emit dataChanged();
}
void VariableEdit::valEdited(const QString &str)
{
    variable->setValue(str);
    variableView->varDataChanged();
    emit dataChanged();
}
void VariableEdit::varCommentChanged()
{
    if (!varViewChanging) {
        variable->setComment(varCommentEdit->toPlainText());
        emit dataChanged();
    }
}
void VariableEdit::deleteClicked()
{
    varViewChanging = true;
    variableView->removeVariable();
    setMailCombo(variableModel->getVariables());
    emit dataChanged();
    varViewChanging = false;
}

void VariableEdit::newClicked()
{
    variableView->insertVariable();
    emit dataChanged();
}

void VariableEdit::mailOnClicked(bool state)
{
    if (state) {
        setMailVar(0);
    }
}

void VariableEdit::mailOffClicked(bool state)
{
    if (state) {
        setMailVar(1);
    }
}
void VariableEdit::mailToClicked(bool state)
{
    if (state) {
        setMailVar(2);
    }
}
void VariableEdit::userActivated(int /*unused*/)
{
    setMailVar(-1);
}

void VariableEdit::setMailVar(int flag)
{
    // 0 = On, 1 = Off, 2 = To, -1 = User activated
    int curFlag = 0;
    Variable *v = nullptr;
    int row = -1;
    const auto &vars = crontab->getVariables();
    for (size_t i = 0; i < vars.size(); ++i) {
        if (vars[i]->getName() == QLatin1String("MAILTO")) {
            curFlag = (vars[i]->getValue() == QLatin1String("\"\"")) ? 1 : 2;
            v = vars[i].get();
            row = static_cast<int>(i);
            break;
        }
    }
    if (flag != -1) {
        if (curFlag == flag) {
            return;
        }
    } else {
        if (curFlag != 2) {
            return;
        }
        flag = 2;
    }
    // Route inserts/removals and edits through VariableModel so its
    // beginInsertRows()/beginRemoveRows()/dataChanged() notifications fire,
    // since the model aliases this same Crontab::getVariables() vector.
    if (flag == 0) {
        if (curFlag == 1 || curFlag == 2) {
            variableModel->removeVariable(row);
        }
    } else if (flag == 1) {
        if (curFlag == 0) {
            variableModel->insertVariable(static_cast<int>(crontab->getVariables().size()),
                                           std::make_unique<Variable>(QStringLiteral("MAILTO"),
                                                                       QStringLiteral("\"\""),
                                                                       QStringLiteral("Don't send mail to anyone")));
        } else if (curFlag == 2) {
            v->setValue(QStringLiteral("\"\""));
            v->setComment(QStringLiteral("Don't send mail to anyone"));
            variableModel->varDataChanged(variableModel->index(row, 0, QModelIndex()));
        }
    } else if (flag == 2) {
        QString u = userCombo->currentText();
        if (curFlag == 0) {
            QString c = "Send mail to \"" + u + "\"";
            variableModel->insertVariable(static_cast<int>(crontab->getVariables().size()),
                                           std::make_unique<Variable>(QStringLiteral("MAILTO"), u, c));
        } else if (curFlag == 1) {
            v->setValue(u);
            v->setComment("Send mail to \"" + u + "\"");
            variableModel->varDataChanged(variableModel->index(row, 0, QModelIndex()));
        } else if (curFlag == 2) {
            if (v->getValue() != u) {
                v->setValue(u);
                v->setComment("Send mail to \"" + u + "\"");
                variableModel->varDataChanged(variableModel->index(row, 0, QModelIndex()));
            } else {
                return;
            }
        }
    }
    variableView->resetView();
    emit dataChanged();
}

void VariableEdit::setMailCombo(const std::vector<std::unique_ptr<Variable>> &var)
{
    bool mvar = false;
    for (const auto &v : var) {
        if (v->getName() == QLatin1String("MAILTO")) {
            mvar = true;
            if (v->getValue() == QLatin1String("\"\"")) {
                mailOffRadio->setChecked(true);
                userCombo->setCurrentIndex(userCombo->findText(crontab->getCronOwner()));
            } else {
                mailToRadio->setChecked(true);
                userCombo->setCurrentIndex(userCombo->findText(v->getValue()));
            }
            break;
        }
    }
    if (!mvar) {
        mailOnRadio->setChecked(true);
        userCombo->setCurrentIndex(userCombo->findText(crontab->getCronOwner()));
    }
}
