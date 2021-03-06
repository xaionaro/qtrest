#ifndef BASERESTLISTMODEL_H
#define BASERESTLISTMODEL_H

#include <QObject>
#include <QModelIndex>
#include "baserestitemmodel.h"
#include "pagination.h"

class BaseRestListModel : public BaseRestItemModel
{
		Q_OBJECT

	protected:
		virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
		virtual QModelIndex parent(const QModelIndex &child) const;
		virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
		virtual bool doInsertItems(QVariantList values);
		void updateHeadersData(QNetworkReply *reply);

		//Items management
		RestItem createItem(QVariantMap value);
		void updateItem(QVariantMap value);
		void append(const RestItem &item);
		void reset();
		const RestItem &at(int row) const;
		//void modelEndInsertRows();

	public:

		//Specify pagination
		Q_PROPERTY(Pagination *pagination READ pagination)

		BaseRestListModel(QObject *parent = 0);
		QVariant data(const QModelIndex &index, int role) const;
		int count() const;
		RestItem *firstRestItem();
		Pagination *pagination();

		Q_INVOKABLE QVariantMap get(const QModelIndex &index ) const;
		Q_INVOKABLE void setFieldValue(QString fieldName, QVariant newValue);

	public slots:
		bool canFetchMore(const QModelIndex &parent) const;
		virtual void fetchMore(const QModelIndex &parent);
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		const RestItem *findItemById(QString id);

	private:
		QList<RestItem> m_items;
		Pagination m_pagination;
};

#endif // BASERESTLISTMODEL_H
