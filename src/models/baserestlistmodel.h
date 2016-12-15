#ifndef BASERESTLISTMODEL_H
#define BASERESTLISTMODEL_H

#include <QObject>
#include <QModelIndex>
#include "baserestitemmodel.h"

class BaseRestListModel : public BaseRestItemModel
{
		Q_OBJECT

	protected:
		virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
		virtual QModelIndex parent(const QModelIndex &child) const;
		virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
		virtual bool doInsertItems(QVariantList values);

	public:
		BaseRestListModel(QObject *parent = 0);
};

#endif // BASERESTLISTMODEL_H
