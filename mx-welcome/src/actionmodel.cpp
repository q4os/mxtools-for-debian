#include "actionmodel.h"

ActionModel::ActionModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ActionModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_actions.size());
}

QVariant ActionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_actions.size()) {
        return {};
    }

    const auto &action = m_actions.at(index.row());
    switch (role) {
    case IdentifierRole:
        return action.identifier;
    case TitleRole:
        return action.title;
    case DescriptionRole:
        return action.description;
    case IconSourceRole:
        return action.iconSource;
    case EnabledRole:
        return action.enabled;
    default:
        return {};
    }
}

QHash<int, QByteArray> ActionModel::roleNames() const
{
    return {{IdentifierRole, "identifier"},
            {TitleRole, "title"},
            {DescriptionRole, "description"},
            {IconSourceRole, "iconSource"},
            {EnabledRole, "actionEnabled"}};
}

void ActionModel::setActions(QVector<WelcomeAction> actions)
{
    beginResetModel();
    m_actions = std::move(actions);
    endResetModel();
}
