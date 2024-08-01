#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>

class ManageRemotes : public QDialog
{
    Q_OBJECT
public:
    explicit ManageRemotes(QWidget *parent = nullptr, const QString &user = "--system ");
    [[nodiscard]] bool isChanged() const
    {
        return changed;
    }
    void listFlatpakRemotes() const;
    [[nodiscard]] QString getInstallRef() const
    {
        return install_ref;
    }
    [[nodiscard]] QString getUser() const
    {
        return user;
    }

signals:

public slots:
    void removeItem();
    void addItem();
    void setInstall();
    void userSelected(int index);

private:
    bool changed;
    QComboBox *comboRemote;
    QComboBox *comboUser;
    QLineEdit *editAddRemote;
    QLineEdit *editInstallFlatpakref;
    QString user;
    QString install_ref;
};
