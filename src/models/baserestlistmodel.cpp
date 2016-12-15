#include "baserestlistmodel.h"

BaseRestListModel::BaseRestListModel(QObject *parent) : BaseRestItemModel(parent)
{

}

QModelIndex BaseRestListModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column) : QModelIndex();
}

QModelIndex BaseRestListModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int BaseRestListModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

