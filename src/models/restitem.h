#ifndef RESTITEM_H
#define RESTITEM_H

#include <QVariantMap>

class RestItem {
	public:
		explicit RestItem(QVariantMap object = QVariantMap(), QString idField = "");
		QVariant value(QString key) const;
		QVariantMap object() const;
		QStringList keys() const;
		QString id() const;
		bool isUpdated() const;
		bool isValid() const;

		void update (QVariantMap value);

		bool operator==(const RestItem &other);
		bool isHidden() const;
		bool isClickable() const;

		void setIsHidden(bool isHiddenValue);
		void setIsClickable(bool isClickableValue);

		//virtual void hide();
		//virtual void show();

	private:
		QVariantMap m_object;
		QString m_idField;
		bool m_isUpdated;
		bool m_isValid;
		bool m_isHidden;
		bool m_isClickable;
};


#endif // RESTITEM_H
