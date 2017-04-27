#include "baserestlistmodel.h"

BaseRestListModel::BaseRestListModel(QObject *parent) : BaseRestItemModel(parent)
{

}

void BaseRestListModel::fetchMore(const QModelIndex &parent)
{
	Q_UNUSED(parent)

	switch (loadingStatus()) {
		case LoadingStatus::RequestToReload:
			m_pagination.setCurrentPage(0);
			m_pagination.setOffset(0);
			m_pagination.setCursorValue(0);
			setLoadingStatus(LoadingStatus::FullReloadProcessing);
			break;
		case LoadingStatus::Idle:
			setLoadingStatus(LoadingStatus::LoadMoreProcessing);
			break;
		default:
			return;
			break;
	}

	switch(m_pagination.policy()) {
		case Pagination::PageNumber: {
			int nextPage = m_pagination.currentPage()+1;
			m_pagination.setCurrentPage(nextPage);
			break;
		}
		case Pagination::LimitOffset: {
			int offset = m_pagination.offset()+m_pagination.limit();
			m_pagination.setOffset(offset);
			break;
		}
		case Pagination::Cursor: {
			QString cursor = 0;
			if (!m_items.isEmpty()) {
				cursor = m_items.last().id();
			}
			m_pagination.setCursorValue(cursor);
			break;
		}
		default:
			break;
	}

	QNetworkReply *reply = fetchMoreImpl(parent);
	connect(reply, SIGNAL(finished()), this, SLOT(fetchMoreFinished()));
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

	QListIterator<QVariant> i(values);
	while ( i.hasNext() ) {
		QVariantMap fields = i.next().toMap();
		//qDebug() << "fields: " << fields;
		append(createItem(fields));
	}

	return true;
}

void BaseRestListModel::setFieldValue(QString fieldName, QVariant newValue) {
	QMutableListIterator<RestItem> iter( this->m_items );
	while ( iter.hasNext() ) {
			RestItem &item = iter.next();
			item.setFieldValue(fieldName, newValue);
	}

	return;
}
bool BaseRestListModel::canFetchMore(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	switch(m_pagination.policy()) {
	case Pagination::PageNumber:
		if (m_pagination.currentPage() < m_pagination.pageCount()) {
			return true;
		} else {
			return false;
		}
		break;
	case Pagination::LimitOffset:
	case Pagination::Cursor:
		if (rowCount() < m_pagination.totalCount()) {
			return true;
		} else {
			return false;
		}
		break;
	case Pagination::None:
		return false;
		break;
	case  Pagination::Infinity:
		return true;
		break;
	default:
		return false;
	}
}

const RestItem *BaseRestListModel::findItemById(QString id)
{
	QListIterator<RestItem> i(m_items);
	while (i.hasNext()) {
		const RestItem *item = &i.next();
		if (item->id() == id) {
			return item;
		}
	}

	qFatal("BaseRestListModel::findItemById(): Cannot find item");
	return NULL;
}

void BaseRestListModel::updateItem(QVariantMap value)
{
	QString id = fetchDetailLastId();
	const RestItem *itemPtr = findItemById(id);

	if (itemPtr == NULL) {
		qWarning() << QString("No item with id %1").arg(id);
		return;
	}

	RestItem item = *itemPtr;

	int row = m_items.indexOf(item);
	item.update(value);
	m_items.replace(row, item);
	emit dataChanged(index(row),index(row));
}

const RestItem &BaseRestListModel::at(int row) const
{
	const RestItem &result = this->m_items.at(row);
	//qDebug() << row << ": " << result.object() << "; ";
	return result;
}

QVariantMap BaseRestListModel::get( const QModelIndex &index ) const {
	if (index.row() < 0 || index.row() >= m_items.count()) {
		qDebug() << "Row not found" << index.row();
		return QVariantMap();
	}

	return this->at(index.row()).object();
}

QVariant BaseRestListModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_items.count()) {
		qDebug() << "Row not found" << index.row();
		return QVariant();
	}

	return this->at(index.row()).value(this->getRoleName(role));
}

int BaseRestListModel::count() const
{
	return m_items.count();
}

void BaseRestListModel::reset()
{
	beginResetModel();
	m_items.clear();
	endResetModel();
}


void BaseRestListModel::append(const RestItem &item)
{
	m_items.append(item);
}


int BaseRestListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_items.count();
}


Pagination *BaseRestListModel::pagination()
{
	return &m_pagination;
}


void BaseRestListModel::updateHeadersData(QNetworkReply *reply)
{
	//update headers data
	QByteArray currentPage;
	currentPage.append(m_pagination.currentPageHeader());
	QByteArray totalCount;
	totalCount.append(m_pagination.totalCountHeader());
	QByteArray pageCount;
	pageCount.append(m_pagination.pageCountHeader());

	m_pagination.setCurrentPage(reply->rawHeader(currentPage).toInt());
	m_pagination.setTotalCount(reply->rawHeader(totalCount).toInt());
	m_pagination.setPageCount(reply->rawHeader(pageCount).toInt());
	reply->deleteLater();

	//todo other headers (limit offset and cursor)
}

RestItem BaseRestListModel::createItem(QVariantMap value)
{
	return RestItem(preProcessItem(value), idField());
}

RestItem *BaseRestListModel::firstRestItem()
{
	if (this->m_items.count() == 0) {
		qFatal("m_items.count() == 0");
	}
	return &this->m_items[0];
}
