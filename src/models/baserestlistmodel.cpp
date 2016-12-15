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

bool BaseRestListModel::doInsertItems(QVariantList values) {

	//prepare vars
	int insertFrom  = rowCount();
	int insertCount = rowCount()+values.count()-1;

	//check if we need to full reload
	if (this->loadingStatus() == LoadingStatus::FullReloadProcessing) {
		reset();
		insertFrom  = rowCount();
		insertCount = values.count()-1;
	}

	//check for assertion or empty data
	if (insertCount < insertFrom) { insertCount = insertFrom; }

	if (insertCount == 0) {
		return false;
	}

	//append rows to model
	beginInsertRows(this->index(rowCount(), 0), insertFrom, insertCount);

	QListIterator<QVariant> i(values);
	while ( i.hasNext() ) {
		RestItem item = createItem(i.next().toMap());
		append(item);
	}

	return true;
}
