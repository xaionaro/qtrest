#ifndef ABSJSONRESTLISTMODEL_H
#define ABSJSONRESTLISTMODEL_H

#include "baserestlistmodel.h"

class QNetworkReply;

class AbstractJsonRestListModel : public BaseRestListModel
{
		Q_OBJECT
	public:
		explicit AbstractJsonRestListModel(QObject *parent = 0);

	protected:
		//for get list
		virtual QVariantList getVariantList(QByteArray bytes);
		//virtual bool doInsertItems(QVariantList values);

	private:
		//for get details for one element
		QVariantMap getVariantMap(QByteArray bytes);
};

#endif // ABSJSONRESTLISTMODEL_H
