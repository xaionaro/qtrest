/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/* Dmitry Yu Okunev <dyokunev@ut.mephi.ru>:
 *
 * Originally I copied this file:
 *   Qt/Examples/Qt-5.7/widgets/itemviews/simpletreemodel/treeitem.cpp
 *
 * and modified it in 2016-12-15
 */

#include <QStringList>
#include <QDebug>

#include "resttreeitem.h"

RestTreeItem::RestTreeItem(QVariantMap object, QString idField, RestTreeItem *parent, QObject *qParent) :
	QObject(qParent),
	RestItem(object, idField),
	m_childItems(QList<RestTreeItem*>()),
	m_isOpen(false),
	m_parentItem(parent)
{
}

RestTreeItem::~RestTreeItem()
{
	qDeleteAll(this->childItems());
}

const QList<RestTreeItem *> &RestTreeItem::childItems() const{
	return m_childItems;
}

const QList<QObject *> RestTreeItem::childItemsAsQObject() const{
	QList<QObject *> res;
	res.reserve(m_childItems.count());
	for(auto i : m_childItems)
		res.append(i);
	return res;
}

void RestTreeItem::addChildItem(RestTreeItem *childItem){
	childItem->setParent(this);
	m_childItems.append(childItem);
	emit childItemsChanged();
	if(m_childItems.count() == 1)
		emit hasChildChanged();
}

bool RestTreeItem::isOpen() const{
	return m_isOpen;
}

void RestTreeItem::setIsOpen(bool isOpen){
	if(isOpen != m_isOpen){
		m_isOpen = isOpen;
		emit isOpenChanged();
	}
}

bool RestTreeItem::hasChild() const{
	return !m_childItems.isEmpty();
}


RestTreeItem *RestTreeItem::child(int row)
{
	return this->childItems().value(row);
}

/*QVariant RestTreeItem::data(int column) const
{
    return m_itemData.value(column);
}*/

RestTreeItem *RestTreeItem::parentItem() const
{
    return m_parentItem;
}

int RestTreeItem::row() const
{
    if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<RestTreeItem*>(this));

    return 0;
}

RestItem *RestTreeItem::restItem() {
	return dynamic_cast<RestItem *>(this);
}

/*void RestTreeItem::addRecursiveData(const QVariantList &values, QString idFieldName, QString childrenFieldName) {
	QListIterator<QVariant> i(values);
	while ( i.hasNext() ) {
		QVariantMap   fields = i.next().toMap();
		RestTreeItem *item   = new RestTreeItem(fields, idFieldName, this);
		//qDebug() << "id == " << fields[idFieldName];
		QVariant childrenField = fields[childrenFieldName];
		if (childrenField.isValid()) {
			QVariantList children = childrenField.toList();
			item->addRecursiveData(children, idFieldName, childrenFieldName);
		}

		this->addChildItem(item);
		emit item->contentChanged();
	}
	emit this->childItemsChanged();
}*/

int RestTreeItem::childCount() const
{
	return this->childItems().count();
}

void RestTreeItem::setIsHiddenRecursively(bool isHiddenValue) {
	int childCountValue = this->childCount();

	this->setIsHidden(isHiddenValue);

	for (int i = 0; i < childCountValue; i++) {
		RestTreeItem *childPtr = this->child(i);
		childPtr->setIsHiddenRecursively(isHiddenValue);
	}

	return;
}

void RestTreeItem::hideRecursively() {
	this->show();
	setIsHiddenRecursively(true);
}
void RestTreeItem::showRecursively() {
	this->hide();
	setIsHiddenRecursively(false);
}

void RestTreeItem::setIsHiddenToRoot(bool isHiddenValue) {
	RestTreeItem *item = this;

	do {
		item->setIsHidden(isHiddenValue);
		item = item->parentItem();
	} while(item != NULL);

	return;
}

void RestTreeItem::hide() {
	this->setIsHidden(true);
}
void RestTreeItem::show() {
	this->setIsHiddenToRoot(false);
}

bool RestTreeItem::callRecursive(bool(*foreachFunc)(RestTreeItem *, void *), void *arg ) {
	if (!foreachFunc(this, arg)) {
		return false;
	}

	QMutableListIterator<RestTreeItem*> iter( this->m_childItems );
	while ( iter.hasNext() ) {
			RestTreeItem *item = iter.next();
			if (!item->callRecursive(foreachFunc, arg)) {
				return false;
			}
	}

	return true;
}
