#pragma once

#include <QAbstractListModel>
#include <QUrl>
#include <QVector>

struct WelcomeAction
{
    QString identifier;
    QString title;
    QString description;
    QUrl iconSource;
    bool enabled = true;
};

class ActionModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        IdentifierRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        IconSourceRole,
        EnabledRole
    };
    Q_ENUM(Role)

    explicit ActionModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void setActions(QVector<WelcomeAction> actions);

private:
    QVector<WelcomeAction> m_actions;
};
