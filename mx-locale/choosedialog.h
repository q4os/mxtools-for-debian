#pragma once

#include <QDialog>
#include <QHash>

#include "cmd.h"

namespace Ui
{
class ChooseDialog;
}

class ChooseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseDialog(QWidget *parent = nullptr);
    ~ChooseDialog();

    QString selection() const;

private slots:
    void textSearch_textChanged();

private:
    Ui::ChooseDialog *ui;
    QHash<QString, QString> localeLib;

    void setup();
    void buildLocaleList();
};
