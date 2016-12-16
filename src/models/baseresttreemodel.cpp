#include <QStringList>
#include "baseresttreemodel.h"

BaseRestTreeModel::BaseRestTreeModel(QObject *parent)
	: BaseRestItemModel(parent)
{
	rootItem = new RestTreeItem();

/*	RestTreeItem *grandChildItem = new RestTreeItem();
	RestTreeItem *childItem = new RestTreeItem();
	rootItem->addChildItem(childItem);
	childItem->addChildItem(grandChildItem);*/
}

BaseRestTreeModel::~BaseRestTreeModel()
{
	delete rootItem;
}

Qt::ItemFlags BaseRestTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return QAbstractItemModel::flags(index);
}

QModelIndex BaseRestTreeModel::index(int row, int column, const QModelIndex &parent)
		const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	RestTreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<RestTreeItem*>(parent.internalPointer());

	RestTreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex BaseRestTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	RestTreeItem *childItem = static_cast<RestTreeItem*>(index.internalPointer());
	RestTreeItem *parentItem = childItem->parentItem();

	if (parentItem == rootItem)
		return QModelIndex();

	if (parentItem == NULL) {
		qErrnoWarning("parentItem == NULL");
		return QModelIndex();
	}

	return createIndex(parentItem->row(), 0, parentItem);
}

int BaseRestTreeModel::rowCount(const QModelIndex &parent) const
{
	RestTreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<RestTreeItem*>(parent.internalPointer());

	return parentItem->childItems().count();
}

bool BaseRestTreeModel::doInsertItems(QVariantList values) {	
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
	QModelIndex idx = this->index(rowCount(), 0);
	beginInsertRows(idx, insertFrom, insertCount);

	rootItem->addRecursiveData(values, this->idField(), this->childrenField());
	//this->addRecursiveData(rootItem, values, this->idField(), this->childrenField(), idx);

	return true;
}


void BaseRestTreeModel::addRecursiveData(RestTreeItem *curItem, const QVariantList &values, QString idFieldName, QString childrenFieldName, QModelIndex idx) {
	QListIterator<QVariant> i(values);
	while ( i.hasNext() ) {
		QVariantMap   fields = i.next().toMap();
		RestTreeItem *item   = new RestTreeItem(fields, idFieldName, curItem);
		qDebug() << "id == " << fields[idFieldName];
		QVariant childrenField = fields[childrenFieldName];
		if (childrenField.isValid()) {
			QModelIndex curIdx = this->index(0, 0, idx);
			QVariantList children = childrenField.toList();
			this->addRecursiveData(item, children, idFieldName, childrenFieldName, curIdx);
		}

		curItem->addChildItem(item);
	}
}

void BaseRestTreeModel::modelEndInsertRows()
{
	this->endInsertRows();
}

/*RestTreeItem *BaseRestTreeModel::findTreeItemByIndex(QModelIndex idx) const
{
	QList<QModelIndex> path = getTreeItemPath(idx);

	RestTreeItem *curNode = this->rootItem;
	while (path.count() > 0) {
		QModelIndex curIdx = path.takeFirst();
		curNode = curNode->child(curIdx.row());
	}

	return curNode;
}*/

QVariant BaseRestTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	//RestTreeItem *treeItem = this->findTreeItemByIndex(index);
	RestTreeItem *treeItem = static_cast<RestTreeItem*>(index.internalPointer());
	return treeItem->value(this->getRoleName(role));
}

int BaseRestTreeModel::count() const
{
	qFatal("not implemented, yet");
	return 0;
}

RestItem *BaseRestTreeModel::firstRestItem() {
	if (this->rootItem->childCount() == 0) {
		qFatal("rootItem->childCount() == 0");
	}
	return this->rootItem->child(0)->restItem();
}


void BaseRestTreeModel::reset()
{
	this->beginResetModel();
	qDeleteAll(this->rootItem->childItems());
	this->endResetModel();
}


const RestItem BaseRestTreeModel::findItemById(QString id)
{
	Q_UNUSED(id)
	qFatal("not implemented, yet");
	return RestItem(QVariantMap(), "");
}


void BaseRestTreeModel::updateItem(QVariantMap value)
{
	Q_UNUSED(value)
	qFatal("not implemented, yet");
}

QList<QModelIndex> BaseRestTreeModel::getTreeItemPath(QModelIndex idx) const
{
	QList<QModelIndex> path;

	do {
		path.push_back(idx);
		idx = idx.parent();
	} while (idx.isValid());

	return path;
}



void BaseRestTreeModel::setChildrenField(QString childrenField)
{
	if (m_childrenField == childrenField)
		return;

	m_childrenField = childrenField;
	emit childrenFieldChanged(childrenField);
}

QString BaseRestTreeModel::childrenField() const
{
	return this->m_childrenField;
}


const QList<RestTreeItem *> BaseRestTreeModel::tree() const
{
	return this->rootItem->childItems();
}

const QList<QObject *> BaseRestTreeModel::treeAsQObjects() const
{
	return this->rootItem->childItemsAsQObject();
}
