#ifndef BASERESTITEMMODEL_H
#define BASERESTITEMMODEL_H

#include <QAbstractItemModel>
#include "restitem.h"
#include "detailsmodel.h"
#include "apibase.h"

class QNetworkReply;
class DetailsModel;

class BaseRestItemModel : public QAbstractItemModel
{
    Q_OBJECT
    friend DetailsModel;

public:
    BaseRestItemModel(QObject *parent = 0);

    Q_PROPERTY(APIBase *api READ apiInstance WRITE setApiInstance NOTIFY apiInstanceChanged)

    //--------------------
    //Standard HATEOAS REST API params (https://en.wikipedia.org/wiki/HATEOAS, for example: https://github.com/yiisoft/yii2/blob/master/docs/guide-ru/rest-quick-start.md)
    //Specify sorting fields
    Q_PROPERTY(QStringList sort READ sort WRITE setSort NOTIFY sortChanged)
    //Specify filters parametres
    Q_PROPERTY(QVariantMap filters READ filters WRITE setFilters NOTIFY filtersChanged)
    //Specify fields parameter
    Q_PROPERTY(QStringList fields READ fields WRITE setFields NOTIFY fieldsChanged)
    //Specify Accept header for application/json or application/xml
    Q_PROPERTY(QByteArray accept READ accept WRITE setAccept NOTIFY acceptChanged)

    //identify column name, role, last fetched detail and detailModel
    Q_PROPERTY(QString idField READ idField WRITE setIdField NOTIFY idFieldChanged)
    Q_PROPERTY(int idFieldRole READ idFieldRole)
    Q_PROPERTY(QString fetchDetailLastId READ fetchDetailLastId)
    Q_PROPERTY(DetailsModel *detailsModel READ detailsModel)

    //load status and result code
    Q_PROPERTY(LoadingStatus loadingStatus READ loadingStatus WRITE setLoadingStatus NOTIFY loadingStatusChanged)
    Q_PROPERTY(QString loadingErrorString READ loadingErrorString WRITE setLoadingErrorString NOTIFY loadingErrorStringChanged)
    Q_PROPERTY(QNetworkReply::NetworkError loadingErrorCode READ loadingErrorCode WRITE setLoadingErrorCode NOTIFY loadingErrorCodeChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

    //current status of model
    enum LoadingStatus {
        Idle,
        IdleDetails,
        RequestToReload,
        FullReloadProcessing,
        LoadMoreProcessing,
        LoadDetailsProcessing,
        Error
    };

    Q_ENUMS(LoadingStatus)

    static void declareQML();

    //Properties GET methods
    QStringList sort() const;
    LoadingStatus loadingStatus() const;
    QVariantMap filters() const;
    QString loadingErrorString() const;
    QNetworkReply::NetworkError loadingErrorCode() const;
    QStringList fields() const;
	QString idField() const;
    int idFieldRole() const;
    QString fetchDetailLastId() const;
    DetailsModel *detailsModel();
    QByteArray accept();
	virtual int count() const = 0;

	//Overwritted system methdos
	virtual QVariant data(const QModelIndex &index, int role) const = 0;
	virtual RestItem *firstRestItem() = 0;

signals:
    //Properties signals
    void countChanged();
    void sortChanged(QStringList sort);
    void loadingStatusChanged(LoadingStatus loadingStatus);
    void filtersChanged(QVariantMap filters);
    void loadingErrorStringChanged(QString loadingErrorString);
    void loadingErrorCodeChanged(QNetworkReply::NetworkError loadingErrorCode);
    void fieldsChanged(QStringList fields);
    void idFieldChanged(QString idField);
	void acceptChanged(QByteArray accept);
    void apiInstanceChanged(APIBase *apiInstance);

public slots:
    void reload();
    void fetchDetail(QString id);
    void replyError(QNetworkReply *reply, QNetworkReply::NetworkError error, QString errorString);

    void requestToReload();
    void forceIdle();

    //Overloaded system methdos
	virtual bool canFetchMore(const QModelIndex &parent) const;
	virtual void fetchMore(const QModelIndex &parent);

    //Properties public SET methods
    void setSort(QStringList sort);
    void setFilters(QVariantMap filters);
    void setFields(QStringList fields);
    void setIdField(QString idField);

    void setApiInstance(APIBase *apiInstance);

	Q_INVOKABLE void setAllValues(QVariantList values, bool fullReloadProcessing);

	bool isValidIndex(QModelIndex index) const;
	virtual bool isHiddenIndex(QModelIndex index) const;
	virtual bool isClickableIndex(QModelIndex index) const;
	virtual const RestItem *findItemById(QString id) = 0;

protected:
    //reimplement this for call specific API method GET list
    virtual QNetworkReply *fetchMoreImpl(const QModelIndex &parent) = 0;
    //reimplement this for call specific API method GET details of record by ID
    virtual QNetworkReply *fetchDetailImpl(QString id) = 0;
    //reimplenet this for prepropcessing each item before add it to model
	virtual QVariantMap preProcessItem(QVariantMap item);
    //for parse list, reimplemented in JSON and XML models
    virtual QVariantList getVariantList(QByteArray bytes) = 0;
    //for parse details for one element, reimplemented in JSON and XML models
    virtual QVariantMap getVariantMap(QByteArray bytes) = 0;
    //API instance for models for one external API service
    APIBase *apiInstance();

    //Update specific headers on updating
	virtual void updateHeadersData(QNetworkReply *reply);

    //Reset model data
	virtual void reset() = 0;

	virtual void updateItem(QVariantMap value) = 0;

    //Auto generate role names by REST keys
	void generateRoleNames(const RestItem &item);
    void generateDetailsRoleNames(QVariantMap item);

    QHash<int, QByteArray> roleNames() const;
    QHash<int, QByteArray> detailsRoleNames() const;

    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const = 0;
    virtual QModelIndex parent(const QModelIndex &child) const = 0;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual bool doInsertItems(QVariantList values) = 0;
	QString getRoleName(int roleId) const;

	virtual void preModelEndInsertRows();
	virtual void modelEndInsertRows();


protected slots:
    //Properties protected SET methods
    void fetchMoreFinished();
    void fetchDetailFinished();
    void setLoadingStatus(LoadingStatus loadingStatus);
    void setAccept(QString accept);
    void setLoadingErrorString(QString loadingErrorString);
    void setLoadingErrorCode(QNetworkReply::NetworkError loadingErrorCode);
	//void setValueByIndex( QModelIndex index, QString FieldName, QVariant FieldValue ) const;

private:
    //Properties store vars
	QHash<int, QByteArray> m_roleNames;
	int m_roleNamesIndex;
	bool m_detailRoleNamesGenerated;
    QStringList m_fields;
    QString m_idField;
    QStringList m_sort;
    LoadingStatus m_loadingStatus;
    QVariantMap m_filters;
    QString m_loadingErrorString;
    QNetworkReply::NetworkError m_loadingErrorCode;
    QString m_fetchDetailLastId;
    DetailsModel m_detailsModel;
    APIBase *m_apiInstance;
};

#endif // BASERESTITEMMODEL_H
