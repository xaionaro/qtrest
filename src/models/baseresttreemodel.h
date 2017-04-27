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
 *   Qt/Examples/Qt-5.7/widgets/itemviews/simpletreemodel/treemodel.h
 *
 * and modified it in 2016-12-15
 */

#ifndef BASERESTTREEMODEL_H
#define BASERESTTREEMODEL_H

#include <QObject>
#include <QModelIndex>
#include <QHash>
#include "baserestitemmodel.h"
#include "resttreeitem.h"

class BaseRestTreeModel : public BaseRestItemModel
{

		Q_OBJECT

	public:
		enum SearchFlags  {
			MatchIfSubstring = 0x01,
			CaseInsensitive  = 0x02,
		};
		Q_ENUM(SearchFlags)

		Q_PROPERTY(QList<QObject*> tree READ treeAsQObjects NOTIFY treeChanged)
		const QList<RestTreeItem*> tree() const;
		const QList<QObject*> treeAsQObjects() const;

		explicit BaseRestTreeModel(QObject *parent = 0);
		~BaseRestTreeModel();

		Q_PROPERTY(QString childrenField READ childrenField WRITE setChildrenField NOTIFY childrenFieldChanged)

		QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
		Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
		//QVariant headerData(int section, Qt::Orientation orientation,
		//		int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QModelIndex index(int row, int column,
						  const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int count() const;
		RestItem *firstRestItem();
		QList<QModelIndex> getTreeItemPath(QModelIndex idx) const;
		//RestTreeItem *findTreeItemByIndex(QModelIndex idx) const;
		QString childrenField() const;
		Q_INVOKABLE QVariantMap get(const QModelIndex &index ) const;
		Q_INVOKABLE QVariantList getChildrenIndexes( QModelIndex idx ) const;
		QVariantList getIndexesByFieldValue_recursive(QString fieldName, QVariant fieldValue, QFlags<SearchFlags> searchFlags, QModelIndex parentIndex = QModelIndex() ) const;
		Q_INVOKABLE QVariantList getIndexesByFieldValue_recursive(QString fieldName, QVariant fieldValue, int searchFlags, QModelIndex parentIndex ) const;
		Q_INVOKABLE QModelIndex getRootIndex() const;
		Q_INVOKABLE QModelIndex getParentIndex(QModelIndex childIndex) const;

		bool callRecursive (bool(*foreachFunc)(RestTreeItem *item, void *arg) , void *arg);

	signals:
		void childrenFieldChanged(QString childrenField);
		void treeChanged();
		void itemChanged(RestTreeItem *item);

	public slots:
		void setChildrenField(QString childrenField);
		const RestItem *findItemById(QString id);
		RestTreeItem *findTreeItemById(QString id);
		const QModelIndex findIndexById(QString id);
		void hideAll();
		void showAll();
		void hideRecursivelyIndex(QModelIndex index);
		void showRecursivelyIndex(QModelIndex index);
		void hideIndex(QModelIndex index);
		void showIndex(QModelIndex index);
		bool isHiddenIndex(QModelIndex index) const;
		bool isClickableIndex(QModelIndex index) const;

	protected:
		virtual bool doInsertItems(QVariantList values);
		void reset();
		void updateItem(QVariantMap value);
		void modelEndInsertRows() Q_DECL_OVERRIDE;
		void addRecursiveData(RestTreeItem *curItem, const QVariantList &values, QString idFieldName, QString childrenFieldName, QModelIndex idx);

	private:
		void rebuildCache(RestTreeItem *parent = NULL);

		RestTreeItem *rootItem;
		QString m_childrenField;
		QHash<QString, RestTreeItem *> itemById;
		const QModelIndex createIndexByItem(RestTreeItem *item) const;

};

#endif // BASERESTTREEMODEL_H
