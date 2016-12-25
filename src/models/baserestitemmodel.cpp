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
	return false;
}

void BaseRestItemModel::fetchMore(const QModelIndex &parent)
{
	Q_UNUSED(parent)

	switch (loadingStatus()) {
		case LoadingStatus::RequestToReload:
			setLoadingStatus(LoadingStatus::FullReloadProcessing);
			break;
		case LoadingStatus::Idle:
			setLoadingStatus(LoadingStatus::LoadMoreProcessing);
			break;
		default:
			return;
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

	qDebug() << "values.count == " << values.count();
	if ( !this->doInsertItems(values) ) {
		setLoadingStatus(LoadingStatus::Error);
		emit countChanged();
		qDebug() << "Nothing to insert! Please check your parser!" << count() << loadingStatus();
		return;
	}

    //get all role names
	RestItem *item = this->firstRestItem();
	generateRoleNames(*item);

	qDebug() << "modelEndInsertRows()…";
	this->modelEndInsertRows();

    setLoadingStatus(LoadingStatus::Idle);

    emit countChanged();
}

void BaseRestItemModel::fetchDetail(QString id)
{
    m_fetchDetailLastId = id;
	const RestItem &item = *findItemById(id);
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

QByteArray BaseRestItemModel::accept()
{
    return apiInstance()->accept();
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
	Q_UNUSED(reply)
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


void BaseRestItemModel::generateRoleNames(const RestItem &item)
{
	if (m_roleNamesIndex > 0) {
		return;
	}
	if (rowCount() == 0) {
		qErrnoWarning("BaseRestItemModel::generateRoleNames(): rowCount() == 0");
		return;
	}

	{
		QStringList fields = this->fields();
		foreach (QString field, fields) {
			QByteArray k;
			k.append(field);
			if (!m_roleNames.key(k)) {
				m_roleNamesIndex++;
				m_roleNames[m_roleNamesIndex] = k;
			}
		}
	}

	if (rowCount() > 0) {
		QStringList keys = item.keys();

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

QString BaseRestItemModel::getRoleName(int roleId) const {
	return this->m_roleNames[roleId];
}

void BaseRestItemModel::modelEndInsertRows()
{
	this->endInsertRows();
}

int BaseRestItemModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return this->m_fields.count();
}


bool BaseRestItemModel::isValidIndex(QModelIndex index) const
{
	return index.isValid();
}

bool BaseRestItemModel::isHiddenIndex(QModelIndex index) const
{
	RestItem *item = static_cast<RestItem*>(index.internalPointer());

	if (item == NULL) {
		qDebug() << "BaseRestItemModel::isHiddenIndex(): item == NULL (index == " << index << ")";
		return true;
	}
	if (!item->isValid()) {
		qDebug() << "BaseRestItemModel::isHiddenIndex(): !item->isValid() (index == " << index << ")";
		return true;
	}

	bool isHidden = item->isHidden();

	qDebug() << "BaseRestItemModel::isHiddenIndex(" << index << "): " << isHidden;
	return isHidden;
}

