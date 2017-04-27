#include "restitem.h"
#include <QDebug>

RestItem::RestItem(QVariantMap object, QString idField) :
	m_object(object),
	m_idField(idField),
	m_isUpdated(false),
	m_isValid(true),
	m_isHidden(false),
	m_isClickable(true)
{
}

QVariant RestItem::value(QString key) const {
    return m_object.value(key);
}

QStringList RestItem::keys() const {
    return m_object.keys();
}

QVariantMap RestItem::object() const {
	//qDebug() << this << m_object;
	return m_object;
}

QString RestItem::id() const {
    return m_object.value(m_idField).toString();
}

bool RestItem::isUpdated() const {
    return m_isUpdated;
}

bool RestItem::isValid() const
{
    return m_isValid;
}

void RestItem::update(QVariantMap value) {
    QMapIterator<QString, QVariant> i(value);
    while (i.hasNext()) {
        i.next();
        m_object.insert(i.key(), i.value());
    }
    m_isUpdated = true;
}

void RestItem::setFieldValue(QString fieldName, QVariant value) {
	this->m_object[fieldName] = value;

	return;
}

bool RestItem::operator==(const RestItem &other) {
    return id() == other.id();
}

bool RestItem::isHidden() const
{
	return this->m_isHidden;
}

void RestItem::setIsHidden(bool isHidden)
{
	this->m_isHidden = isHidden;
}


bool RestItem::isClickable() const
{
	return this->m_isClickable;
}

void RestItem::setIsClickable(bool isClickable)
{
	this->m_isClickable = isClickable;
}
