#include <QtQml>
#include "baserestitemmodel.h"

BaseRestItemModel::BaseRestItemModel(QObject *parent) : QAbstractItemModel(parent),
    m_roleNamesIndex(0), m_detailRoleNamesGenerated(false), m_sort("-id"), m_loadingStatus(LoadingStatus::Idle), m_apiInstance(nullptr)
{

}

void BaseRestItemModel::declareQML()
{
    qRegisterMetaType<DetailsModel*>("DetailsModel*");
    qmlRegisterType<Pagination>("com.github.qtrest.pagination", 1, 0, "Pagination");
}

void BaseRestItemModel::reload()
{
    setLoadingStatus(LoadingStatus::RequestToReload);
    this->fetchMore(QModelIndex());
}

bool BaseRestItemModel::canFetchMore(const QModelIndex &parent) const
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

void BaseRestItemModel::fetchMore(const QModelIndex &parent)
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

void BaseRestItemModel::fetchMoreFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (apiInstance()->checkReplyIsError(reply) || !reply->isFinished()) {
        return;
    }

    if (this->loadingStatus() == LoadingStatus::Idle) {
        return;
    }

    updateHeadersData(reply);

    QVariantList values = getVariantList(reply->readAll());

	if ( !this->doInsertItems(values) ) {
		setLoadingStatus(LoadingStatus::Error);
		emit countChanged();
		qDebug() << "Nothing to insert! Please check your parser!" << count() << loadingStatus();
		return;
	}

    //get all role names
    generateRoleNames();

    endInsertRows();

    setLoadingStatus(LoadingStatus::Idle);

    emit countChanged();
}

void BaseRestItemModel::fetchDetail(QString id)
{
    m_fetchDetailLastId = id;
    RestItem item = findItemById(id);
    if (item.isUpdated()) {
        return;
    }

    switch (loadingStatus()) {
    case LoadingStatus::Idle:
        setLoadingStatus(LoadingStatus::LoadDetailsProcessing);
        break;
    default:
        return;
        break;
    }

    m_detailsModel.invalidateModel();

    QNetworkReply *reply = fetchDetailImpl(id);
    connect(reply, SIGNAL(finished()), this, SLOT(fetchDetailFinished()));
}

void BaseRestItemModel::fetchDetailFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (apiInstance()->checkReplyIsError(reply) || !reply->isFinished()) {
        return;
    }

    if (this->loadingStatus() == LoadingStatus::Idle) {
        return;
    }

    QVariantMap item = preProcessItem(getVariantMap(reply->readAll()));

    updateItem(item);

    generateDetailsRoleNames(item);

    detailsModel()->setSourceModel(this);

    setLoadingStatus(LoadingStatus::IdleDetails);
}

void BaseRestItemModel::setLoadingStatus(BaseRestItemModel::LoadingStatus loadingStatus)
{
    if (m_loadingStatus == loadingStatus) {
        return;
    }

    m_loadingStatus = loadingStatus;
    emit loadingStatusChanged(loadingStatus);
}

void BaseRestItemModel::setAccept(QString accept)
{
    apiInstance()->setAccept(accept);
}

void BaseRestItemModel::setLoadingErrorString(QString loadingErrorString)
{
    if (m_loadingErrorString == loadingErrorString)
        return;

    m_loadingErrorString = loadingErrorString;
    emit loadingErrorStringChanged(loadingErrorString);
}

void BaseRestItemModel::setLoadingErrorCode(QNetworkReply::NetworkError loadingErrorCode)
{
    if (m_loadingErrorCode == loadingErrorCode)
        return;

    m_loadingErrorCode = loadingErrorCode;
    emit loadingErrorCodeChanged(loadingErrorCode);
}

void BaseRestItemModel::replyError(QNetworkReply *reply, QNetworkReply::NetworkError error, QString errorString)
{
    Q_UNUSED(reply)
    setLoadingErrorCode(error);
    setLoadingErrorString(errorString);
    setLoadingStatus(LoadingStatus::Error);
}

RestItem BaseRestItemModel::createItem(QVariantMap value)
{
    return RestItem(preProcessItem(value),idField());
}

RestItem BaseRestItemModel::findItemById(QString id)
{
    QListIterator<RestItem> i(m_items);
    while (i.hasNext()) {
        RestItem item = i.next();
        if (item.id() == id) {
            return item;
        }
    }

    qFatal("BaseRestItemModel::findItemById(): Cannot find item");
    return RestItem(QVariantMap(), "");
}

void BaseRestItemModel::updateItem(QVariantMap value)
{
    RestItem item = findItemById(fetchDetailLastId());
    int row = m_items.indexOf(item);
    item.update(value);
    m_items.replace(row, item);
    emit dataChanged(index(row),index(row));
}

QVariant BaseRestItemModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_items.count()) {
        qDebug() << "Row not found" << index.row();
        return QVariant();
    }

    RestItem item = m_items.at(index.row());
    return item.value(m_roleNames[role]);
}

QStringList BaseRestItemModel::sort() const
{
    return m_sort;
}

BaseRestItemModel::LoadingStatus BaseRestItemModel::loadingStatus() const
{
    return m_loadingStatus;
}

QVariantMap BaseRestItemModel::filters() const
{
    return m_filters;
}

QString BaseRestItemModel::loadingErrorString() const
{
    return m_loadingErrorString;
}

QNetworkReply::NetworkError BaseRestItemModel::loadingErrorCode() const
{
    return m_loadingErrorCode;
}

QStringList BaseRestItemModel::fields() const
{
    return m_fields;
}

QString BaseRestItemModel::idField() const
{
    return m_idField;
}

int BaseRestItemModel::idFieldRole() const
{
    QByteArray obj;
    obj.append(idField());
    return m_roleNames.key(obj);
}

QString BaseRestItemModel::fetchDetailLastId() const
{
    return m_fetchDetailLastId;
}

DetailsModel *BaseRestItemModel::detailsModel()
{
    return &m_detailsModel;
}

Pagination *BaseRestItemModel::pagination()
{
    return &m_pagination;
}

QByteArray BaseRestItemModel::accept()
{
    return apiInstance()->accept();
}

int BaseRestItemModel::count() const
{
    return m_items.count();
}

QHash<int, QByteArray> BaseRestItemModel::roleNames() const
{
    return m_roleNames;
}

QHash<int, QByteArray> BaseRestItemModel::detailsRoleNames() const
{
    return m_roleNames;
}

void BaseRestItemModel::updateHeadersData(QNetworkReply *reply)
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

void BaseRestItemModel::reset()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

void BaseRestItemModel::append(RestItem item)
{
    m_items.append(item);
}

void BaseRestItemModel::generateRoleNames()
{
    if (m_roleNamesIndex > 0) {
        return;
    }
	if (rowCount() == 0) {
		qErrnoWarning("BaseRestItemModel::generateRoleNames(): rowCount() == 0");
		return;
	}

    RestItem item = m_items[0];

    QStringList keys = item.keys();

    if (rowCount() > 0) {
        foreach (QString key, keys) {
            QByteArray k;
            k.append(key);
            if (!m_roleNames.key(k)) {
                m_roleNamesIndex++;
                m_roleNames[m_roleNamesIndex] = k;
            }
        }
    }
}

void BaseRestItemModel::generateDetailsRoleNames(QVariantMap item)
{
    if (m_detailRoleNamesGenerated) { return; }

    QStringList keys = item.keys();

    if (rowCount() > 0) {
        foreach (QString key, keys) {
            QByteArray k = key.toUtf8();
            if (!m_roleNames.key(k)) {
                m_roleNamesIndex++;
                m_roleNames[m_roleNamesIndex] = k;
            }
        }
    }

    m_detailRoleNamesGenerated = true;
}

int BaseRestItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.count();
}

void BaseRestItemModel::requestToReload() {
    setLoadingStatus(LoadingStatus::RequestToReload);
}

void BaseRestItemModel::forceIdle() {
    setLoadingStatus(LoadingStatus::Idle);
}

void BaseRestItemModel::setSort(QStringList sort)
{
    if (m_sort == sort)
        return;

    m_sort = sort;
    emit sortChanged(sort);
}

void BaseRestItemModel::setFilters(QVariantMap filters)
{
    if (m_filters == filters)
        return;

    m_filters = filters;
    emit filtersChanged(filters);
}

void BaseRestItemModel::setFields(QStringList fields)
{
    if (m_fields == fields)
        return;

    m_fields = fields;
    emit fieldsChanged(fields);
}

void BaseRestItemModel::setIdField(QString idField)
{
    if (m_idField == idField)
        return;

    m_idField = idField;
    emit idFieldChanged(idField);
}

void BaseRestItemModel::setApiInstance(APIBase *apiInstance)
{
    if (m_apiInstance == apiInstance)
        return;

    m_apiInstance = apiInstance;

    m_apiInstance->setAccept(accept());
    connect(m_apiInstance,SIGNAL(replyError(QNetworkReply *, QNetworkReply::NetworkError, QString)),
            this, SLOT(replyError(QNetworkReply *, QNetworkReply::NetworkError, QString)));

    emit apiInstanceChanged(apiInstance);
}

APIBase *BaseRestItemModel::apiInstance()
{
    if (m_apiInstance == nullptr) {
        return new APIBase();
    }
    return m_apiInstance;
}
