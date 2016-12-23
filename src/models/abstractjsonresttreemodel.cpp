#include "abstractjsonresttreemodel.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QJsonDocument>

AbstractJsonRestTreeModel::AbstractJsonRestTreeModel(QObject *parent) : BaseRestTreeModel(parent)
{
	setAccept("application/json");
}

QVariantList AbstractJsonRestTreeModel::getVariantTree(QByteArray bytes)
{
	//QString str = QString::fromUtf8(bytes.data(), bytes.size());

	QJsonParseError parseError;
	QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
	QJsonArray jsonArray = document.array();

	if (parseError.error != QJsonParseError::NoError) {
		qDebug() << parseError.errorString();
	}

	qDebug() << "getVariantTree(): " << bytes;
	return jsonArray.toVariantList();
}

QVariantMap AbstractJsonRestTreeModel::getVariantMap(QByteArray bytes)
{
	//QString str = QString::fromUtf8(bytes.data(), bytes.size());

	QJsonParseError parseError;
	QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
	QJsonObject obj = document.object();

	if (parseError.error != QJsonParseError::NoError) {
		qDebug() << parseError.errorString();
	}

	qDebug() << "getVariantMap(): " << bytes;
	return obj.toVariantMap();
}

