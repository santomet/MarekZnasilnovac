#include "requestsmodel.h"

RequestsModel::RequestsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant RequestsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(role)
    if(orientation == Qt::Horizontal)
        return "Requests";
    else
        return QVariant();
}


int RequestsModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return mRequests.count();
}

QVariant RequestsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int i = index.row();
    if ((i < 0) || (i >= mRequests.count()))
    {
        qDebug() << "Count isn't right!";
        return QVariant();
    }

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:

        return mRequests.at(i);
        break;

    }

    return QVariant();
    return QVariant();
}

bool RequestsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        int i = index.row();
        if ((i < 0) || (i >= mRequests.count()))
            return false;

        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:

            mRequests[i] = value.toString();
            break;
        }

        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags RequestsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable|Qt::ItemIsEnabled|Qt::ItemIsSelectable; // FIXME: Implement me!
}

bool RequestsModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    for(int i = 0; i<count; ++i)
    {
        mRequests.insert(row + i, "");
    }
    endInsertRows();
    return true;
}

bool RequestsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(mRequests.count() < row + count)
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for(int i = 0; i<count; ++i)
    {
        mRequests.removeAt(row);
    }
    endRemoveRows();
    return true;
}

QStringList RequestsModel::getRequests()
{
    return mRequests;
}
