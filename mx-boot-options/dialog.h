#ifndef DIALOG_H
#define DIALOG_H

#include <QComboBox>
#include <QDialog>

class CustomDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CustomDialog(const QStringList &items, QDialog *parent = nullptr);

    QComboBox *comboBox();

signals:

public slots:

private:
    QComboBox *box;
};

#endif // DIALOG_H
