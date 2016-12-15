#include "baseresttreemodel.h"
#include <QtQml>

BaseRestTreeModel::BaseRestTreeModel(QObject *parent) : QAbstractTreeModel(parent), m_sort("-id"),
    m_roleNamesIndex(0), m_loadingStatus(LoadingStatus::Idle), m_detailRoleNamesGenerated(false), m_apiInstance(nullptr)
{

}

void BaseRestTreeModel::declareQML()
{
    qRegisterMetaType<DetailsModel*>("DetailsModel*");
    qmlRegisterType<Pagination>("com.github.qtrest.pagination", 1, 0, "Pagination");
}

void BaseRestTreeModel::reload()
{
    setLoadingStatus(LoadingStatus::RequestToReload);
    this->fetchMore(QModelIndex());
}

bool BaseRestTreeModel::canFetchMore(const QModelIndex &parent) const
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

void BaseRestTreeModel::fetchMore(const QModelIndex &parent)
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

bool BaseRestTreeModel::doInsertItems(const QVariantTree &values)
{
    //prepare vars
    int insertFrom = rowCount();
    int insertCount = rowCount()+values.count()-1;

    //check if we need to full reload
    if (this->loadingStatus() == LoadingStatus::FullReloadProcessing) {
        reset();
        insertFrom = rowCount();
        insertCount = values.count()-1;
    }

    //check for assertion or empty data
    if (insertCount < insertFrom) { insertCount = insertFrom; }

    if (insertCount == 0) {
        setLoadingStatus(LoadingStatus::Error);
        qDebug() << "Nothing to insert! Please check your parser!" << count() << loadingStatus();
        return false;
    }

    //append rows to model
    beginInsertRows(this->index(rowCount(), 0), insertFrom, insertCount);

    QTreeIterator<QVariant> i(values);
    while (i.hasNext()) {
        RestItem item = createItem(i.next().toMap());
        append(item);
    }

    return true;
}

void BaseRestTreeModel::fetchMoreFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (apiInstance()->checkReplyIsError(reply) || !reply->isFinished()) {
        return;
    }

    if (this->loadingStatus() == LoadingStatus::Idle) {
        return;
    }

    updateHeadersData(reply);

    QVariantTree values = getVariantTree(reply->readAll());

    if ( !this->doInsertItems(values) ) {
        emit countChanged();
        return;
    }

    //get all role names
    generateRoleNames();

    endInsertRows();

    setLoadingStatus(LoadingStatus::Idle);

    emit countChanged();
}

void BaseRestTreeModel::fetchDetail(QString id)
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

void BaseRestTreeModel::fetchDetailFinished()
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

void BaseRestTreeModel::setLoadingStatus(BaseRestTreeModel::LoadingStatus loadingStatus)
{
    if (m_loadingStatus == loadingStatus) {
        return;
    }

    m_loadingStatus = loadingStatus;
    emit loadingStatusChanged(loadingStatus);
}

void BaseRestTreeModel::setAccept(QString accept)
{
    apiInstance()->setAccept(accept);
}

void BaseRestTreeModel::setLoadingErrorString(QString loadingErrorString)
{
    if (m_loadingErrorString == loadingErrorString)
        return;

    m_loadingErrorString = loadingErrorString;
    emit loadingErrorStringChanged(loadingErrorString);
}

void BaseRestTreeModel::setLoadingErrorCode(QNetworkReply::NetworkError loadingErrorCode)
{
    if (m_loadingErrorCode == loadingErrorCode)
        return;

    m_loadingErrorCode = loadingErrorCode;
    emit loadingErrorCodeChanged(loadingErrorCode);
}

void BaseRestTreeModel::replyError(QNetworkReply *reply, QNetworkReply::NetworkError error, QString errorString)
{
    Q_UNUSED(reply)
    setLoadingErrorCode(error);
    setLoadingErrorString(errorString);
    setLoadingStatus(LoadingStatus::Error);
}

RestItem BaseRestTreeModel::createItem(QVariantMap value)
{
    return RestItem(preProcessItem(value),idField());
}

RestItem BaseRestTreeModel::findItemById(QString id)
{
    QTreeIterator<RestItem> i(m_items);
    while (i.hasNext()) {
        RestItem item = i.next();
        if (item.id() == id) {
            return item;
        }
    }
}

void BaseRestTreeModel::updateItem(QVariantMap value)
{
    RestItem item = findItemById(fetchDetailLastId());
    int row = m_items.indexOf(item);
    item.update(value);
    m_items.replace(row, item);
    emit dataChanged(index(row),index(row));
}

QVariant BaseRestTreeModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_items.count()) {
        qDebug() << "Row not found" << index.row();
        return QVariant();
    }

    RestItem item = m_items.at(index.row());
    return item.value(m_roleNames[role]);
}

QStringTree BaseRestTreeModel::sort() const
{
    return m_sort;
}

BaseRestTreeModel::LoadingStatus BaseRestTreeModel::loadingStatus() const
{
    return m_loadingStatus;
}

QVariantMap BaseRestTreeModel::filters() const
{
    return m_filters;
}

QString BaseRestTreeModel::loadingErrorString() const
{
    return m_loadingErrorString;
}

QNetworkReply::NetworkError BaseRestTreeModel::loadingErrorCode() const
{
    return m_loadingErrorCode;
}

QStringList BaseRestTreeModel::fields() const
{
    return m_fields;
}

QString BaseRestTreeModel::idField() const
{
    return m_idField;
}

int BaseRestTreeModel::idFieldRole() const
{
    QByteArray obj;
    obj.append(idField());
    return m_roleNames.key(obj);
}

QString BaseRestTreeModel::fetchDetailLastId() const
{
    return m_fetchDetailLastId;
}

DetailsModel *BaseRestTreeModel::detailsModel()
{
    return &m_detailsModel;
}

Pagination *BaseRestTreeModel::pagination()
{
    return &m_pagination;
}

QByteArray BaseRestTreeModel::accept()
{
    return apiInstance()->accept();
}

int BaseRestTreeModel::count() const
{
    return m_items.count();
}

QHash<int, QByteArray> BaseRestTreeModel::roleNames() const
{
    return m_roleNames;
}

QHash<int, QByteArray> BaseRestTreeModel::detailsRoleNames() const
{
    return m_roleNames;
}

void BaseRestTreeModel::updateHeadersData(QNetworkReply *reply)
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

void BaseRestTreeModel::reset()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

void BaseRestTreeModel::append(RestItem item)
{
    m_items.append(item);
}

void BaseRestTreeModel::generateRoleNames()
{
    if (m_roleNamesIndex > 0) {
        return;
    }

    RestItem item = m_items[0];

    QStringTree keys = item.keys();

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

void BaseRestTreeModel::generateDetailsRoleNames(QVariantMap item)
{
    if (m_detailRoleNamesGenerated) { return; }

    QStringTree keys = item.keys();

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

int BaseRestTreeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.count();
}

void BaseRestTreeModel::requestToReload() {
    setLoadingStatus(LoadingStatus::RequestToReload);
}

void BaseRestTreeModel::forceIdle() {
    setLoadingStatus(LoadingStatus::Idle);
}

void BaseRestTreeModel::setSort(QStringTree sort)
{
    if (m_sort == sort)
        return;

    m_sort = sort;
    emit sortChanged(sort);
}

void BaseRestTreeModel::setFilters(QVariantMap filters)
{
    if (m_filters == filters)
        return;

    m_filters = filters;
    emit filtersChanged(filters);
}

void BaseRestTreeModel::setFields(QStringTree fields)
{
    if (m_fields == fields)
        return;

    m_fields = fields;
    emit fieldsChanged(fields);
}

void BaseRestTreeModel::setIdField(QString idField)
{
    if (m_idField == idField)
        return;

    m_idField = idField;
    emit idFieldChanged(idField);
}

void BaseRestTreeModel::setApiInstance(APIBase *apiInstance)
{
    if (m_apiInstance == apiInstance)
        return;

    m_apiInstance = apiInstance;

    m_apiInstance->setAccept(accept());
    connect(m_apiInstance,SIGNAL(replyError(QNetworkReply *, QNetworkReply::NetworkError, QString)),
            this, SLOT(replyError(QNetworkReply *, QNetworkReply::NetworkError, QString)));

    emit apiInstanceChanged(apiInstance);
}

APIBase *BaseRestTreeModel::apiInstance()
{
    if (m_apiInstance == nullptr) {
        return new APIBase();
    }
    return m_apiInstance;
}
