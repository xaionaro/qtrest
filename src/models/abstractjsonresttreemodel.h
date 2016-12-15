#ifndef ABSJSONRESTTREEMODEL_H
#define ABSJSONRESTTREEMODEL_H

#include "baseresttreemodel.h"

class QNetworkReply;

class AbstractJsonRestTreeModel : public BaseRestTreeModel
{
		Q_OBJECT
	public:
		explicit AbstractJsonRestTreeModel(QObject *parent = 0);

	protected:
		//parse the tree from JSON as QByteArray
		virtual QVariantList getVariantTree(QByteArray bytes);
		//virtual bool doInsertItems(QVariantList values);

	private:
		//for get details for one element
		QVariantMap getVariantMap(QByteArray bytes);
};

#endif // ABSJSONRESTTREEMODEL_H
