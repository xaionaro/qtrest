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
	//rootItem->addRecursiveData(values, this->idField(), this->childrenField());
	this->addRecursiveData(rootItem, values, this->idField(), this->childrenField(), this->index(rowCount(), 0));

	return true;
}


void BaseRestTreeModel::addRecursiveData(RestTreeItem *curItem, const QVariantList &values, QString idFieldName, QString childrenFieldName, QModelIndex idx) {
	QListIterator<QVariant> i(values);
	while ( i.hasNext() ) {
		QVariantMap   fields = i.next().toMap();
		RestTreeItem *item   = new RestTreeItem(fields, idFieldName, curItem);
		//qDebug() << "id == " << fields[idFieldName];
		QVariant childrenField = fields[childrenFieldName];
		if (childrenField.isValid()) {
			QModelIndex curIdx = this->index(0, 0, idx);
			QVariantList children = childrenField.toList();
			this->addRecursiveData(item, children, idFieldName, childrenFieldName, curIdx);
		}

		curItem->addChildItem(item);
		emit item->contentChanged();
		emit this->itemChanged(item);
		//qDebug() << "itemChanged()";
	}
}

void BaseRestTreeModel::rebuildCache(RestTreeItem *parent) {
	if (parent == NULL) {
		this->itemById.clear();
		parent = this->rootItem;
	}

	int childCount = parent->childCount();
	for(int i = 0; i != childCount; i++) {
		RestTreeItem *child = parent->child(i);
		this->itemById[child->id()] = child;

		this->rebuildCache(child);
	}

	return;
}

void BaseRestTreeModel::modelEndInsertRows()
{
	this->endInsertRows();

	this->rebuildCache();

	emit this->treeChanged();
	//qDebug() << this->itemById;
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

QVariantMap BaseRestTreeModel::get( const QModelIndex &index ) const {
	if (!index.isValid()) {
		qDebug("!index.isValid()");
		return QVariantMap();
	}

	//RestTreeItem *treeItem = this->findTreeItemByIndex(index);
	return static_cast<RestTreeItem*>(index.internalPointer())->object();
}

QVariant BaseRestTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) {
		qDebug("!index.isValid()");
		return QVariant();
	}

	return static_cast<RestTreeItem*>(index.internalPointer())->value(this->getRoleName(role));
}

int BaseRestTreeModel::count() const
{
	return this->rootItem->count();
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
	this->rootItem->reset();
	this->endResetModel();
}

RestTreeItem *BaseRestTreeModel::findTreeItemById(QString id)
{
	return this->itemById[id];
}

const RestItem *BaseRestTreeModel::findItemById(QString id)
{
	return this->findTreeItemById(id);
}

const QModelIndex BaseRestTreeModel::createIndexByItem(RestTreeItem *item) const {
	Q_ASSERT(item != NULL);
	RestTreeItem *parent = item->parentItem();

	if (parent == NULL) {
		//RqDebug() << "BaseRestTreeModel::createIndexByItem(): parent == NULL";
		return QModelIndex();
	}

	int itemCandidateCount = parent->childCount();
	for(int i = 0; i != itemCandidateCount; i++) {
		RestTreeItem *itemCandidate = parent->child(i);
		if (itemCandidate == item) {
			return this->createIndex(i, 0, item);
		}
	}

	qDebug() << "BaseRestTreeModel::createIndexByItem(): not found";
	return QModelIndex();
}

const QModelIndex BaseRestTreeModel::findIndexById(QString id)
{
	RestTreeItem *item = this->findTreeItemById(id);
	if (item == NULL) {
		qDebug() << "BaseRestTreeModel::findIndexById(" << id << "): not found";
		return QModelIndex();
	}
	QModelIndex index = this->createIndexByItem(item);
	//qDebug() << "BaseRestTreeModel::findIndexById(" << id << "): " << index;
	return index;
}

void BaseRestTreeModel::updateItem(QVariantMap value)
{
	Q_UNUSED(value)
	qFatal("not implemented, yet #1");
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

QVariantList BaseRestTreeModel::getChildrenIndexes(QModelIndex index) const
{
	if (!index.isValid()) {
		qDebug() << "getChildrenIndexes(): !index.isValid(): " << index;
		return QVariantList();
	}


	QVariantList indexes;
	RestTreeItem *parent = static_cast<RestTreeItem*>(index.internalPointer());

	int childCount = parent->childCount();
	for(int i = 0; i != childCount; i++) {
		RestTreeItem *child = parent->child(i);
		indexes.push_back(createIndex(i, 0, child));
	}

	return indexes;

}

QVariantList BaseRestTreeModel::getIndexesByFieldValue_recursive( QString fieldName, QVariant fieldValue, int searchFlags, QModelIndex parentIndex ) const
{
	return this->getIndexesByFieldValue_recursive(fieldName, fieldValue, QFlags<SearchFlags>(searchFlags), parentIndex);
}

QVariantList BaseRestTreeModel::getIndexesByFieldValue_recursive( QString fieldName, QVariant fieldValue, QFlags<SearchFlags> searchFlags, QModelIndex parentIndex ) const
{
	QVariantList resultIndexes;

	RestTreeItem *parent;
	if (parentIndex.isValid()) {
		parent = static_cast<RestTreeItem*>(parentIndex.internalPointer());
	} else {
		parent = this->rootItem;
	}

	if (searchFlags.testFlag(CaseInsensitive)) {
		fieldValue = fieldValue.toString().toLower();
	}

	int childCount = parent->childCount();
	for(int i = 0; i != childCount; i++) {
		RestTreeItem *child = parent->child(i);
		QModelIndex childIndex = this->createIndex(i, 0, child);

		QVariant value = child->value(fieldName);


		if (searchFlags.testFlag(CaseInsensitive)) {
			value = value.toString().toLower();
		}

		bool matches;
		if (searchFlags.testFlag(MatchIfSubstring)) {
			matches = value.toString().contains(fieldValue.toString());
		} else {
			matches = value == fieldValue;
		}

		if (matches) {
			resultIndexes.push_back(childIndex);
		}

		QVariantList matchedGrandChildrenIndexes = this->getIndexesByFieldValue_recursive(fieldName, fieldValue, searchFlags, childIndex);
		resultIndexes.append(matchedGrandChildrenIndexes);
	}

	return resultIndexes;
}

QModelIndex BaseRestTreeModel::getRootIndex() const
{
	return this->createIndex(0, 0, this->rootItem);
}

QModelIndex BaseRestTreeModel::getParentIndex(QModelIndex childIndex) const
{
	RestTreeItem *child = static_cast<RestTreeItem*>(childIndex.internalPointer());

	RestTreeItem *parent = child->parentItem();
	if (parent == NULL) {
		return QModelIndex();
	}

	return this->createIndexByItem(parent);
}

void BaseRestTreeModel::hideAll() {
	this->rootItem->hideRecursively();
}
void BaseRestTreeModel::showAll() {
	this->rootItem->showRecursively();
}
void BaseRestTreeModel::hideRecursivelyIndex(QModelIndex index) {
	RestTreeItem *item = static_cast<RestTreeItem*>(index.internalPointer());
	item->hideRecursively();
}
void BaseRestTreeModel::showRecursivelyIndex(QModelIndex index) {
	RestTreeItem *item = static_cast<RestTreeItem*>(index.internalPointer());
	item->showRecursively();
}
void BaseRestTreeModel::hideIndex(QModelIndex index) {
	RestTreeItem *item = static_cast<RestTreeItem*>(index.internalPointer());
	item->hide();
}
void BaseRestTreeModel::showIndex(QModelIndex index) {
	RestTreeItem *item = static_cast<RestTreeItem*>(index.internalPointer());
	item->show();
}

bool BaseRestTreeModel::isHiddenIndex(QModelIndex index) const
{
	RestTreeItem *item = static_cast<RestTreeItem*>(index.internalPointer());

	if (!index.isValid()) {
		return true;
	}

	if (item == NULL) {
		qDebug() << "BaseRestTreeModel::isHiddenIndex(): item == NULL (index == " << index << ")";
		return true;
	}
	if (!item->isValid()) {
		qDebug() << "BaseRestTreeModel::isHiddenIndex(): !item->isValid() (index == " << index << ")";
		return true;
	}

	bool isHidden = item->isHidden();

	//qDebug() << "BaseRestTreeModel::isHiddenIndex(" << index << "): " << isHidden;
	return isHidden;
}

bool BaseRestTreeModel::isClickableIndex(QModelIndex index) const
{
	RestTreeItem *item = static_cast<RestTreeItem*>(index.internalPointer());

	if (item == NULL) {
		qDebug() << "BaseRestTreeModel::isClickableIndex(): item == NULL (index == " << index << ")";
		return true;
	}
	if (!item->isValid()) {
		qDebug() << "BaseRestTreeModel::isClickableIndex(): !item->isValid() (index == " << index << ")";
		return true;
	}

	bool isClickable = item->isClickable();

	//qDebug() << "BaseRestTreeModel::isClickableIndex(" << index << "): " << isClickable;
	return isClickable;
}

bool BaseRestTreeModel::callRecursive(bool(*foreachFunc)(RestTreeItem *, void *), void *arg ) {
	return this->rootItem->callRecursive(foreachFunc, arg);
}
