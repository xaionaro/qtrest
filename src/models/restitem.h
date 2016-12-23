#ifndef RESTITEM_H
#define RESTITEM_H

#include <QVariantMap>

class RestItem {
	public:
		explicit RestItem(QVariantMap object = QVariantMap(), QString idField = "");
		QVariant value(QString key) const;
		QStringList keys() const;
		QString id() const;
		bool isUpdated() const;
		bool isValid() const;

		void update (QVariantMap value);

		bool operator==(const RestItem &other);

	private:
		QVariantMap m_object;
		QString m_idField;
		bool m_isUpdated;
		bool m_isValid;
};


#endif // RESTITEM_H
