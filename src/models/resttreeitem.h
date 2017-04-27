/* Dmitry Yu Okunev <dyokunev@ut.mephi.ru>:
 *
 * Originally I copied this file:
 *   Qt/Examples/Qt-5.7/widgets/itemviews/simpletreemodel/treeitem.h
 *
 * and modified it in 2016-12-15
 */

#ifndef RESTTREEITEM_H
#define RESTTREEITEM_H

#include <QList>
#include <QVariant>

#include "restitem.h"

class RestTreeItem : public QObject, public RestItem
{
		Q_OBJECT

	public:
		explicit RestTreeItem(QVariantMap object = QVariantMap(), QString idField = "", RestTreeItem *parentItem = 0, QObject *qParent = 0);
		~RestTreeItem();

		void appendChild(RestTreeItem *child);

		RestTreeItem *child(int row);
		//QList<RestTreeItem*> children();
		QVariant data(int column) const;
		int row() const;
		RestTreeItem *parentItem() const;
		RestItem *restItem();
		void addRecursiveData(const QVariantList &values, QString idFieldName = "id", QString childrenFieldName = "children");

		Q_PROPERTY(QList<QObject*> childItems READ childItemsAsQObject NOTIFY childItemsChanged)
		const QList<RestTreeItem *> &childItems() const;
		const QList<QObject *> childItemsAsQObject() const;
		void addChildItem(RestTreeItem * childItem);
		int childCount() const;

		Q_PROPERTY(bool isOpen READ isOpen WRITE setIsOpen NOTIFY isOpenChanged)
		bool isOpen() const;
		void setIsOpen(bool isOpen);

		Q_PROPERTY(bool hasChild READ hasChild NOTIFY hasChildChanged)
		bool hasChild() const;

		bool callRecursive (bool(*foreachFunc)(RestTreeItem *item, void *arg) , void *arg);


	signals:
		void contentChanged();
		void childItemsChanged();
		void isOpenChanged();
		void hasChildChanged();

	public slots:
		void hideRecursively();
		void showRecursively();
		void hide();
		void show();
		void setIsHiddenRecursively(bool isHiddenValue);
		void setIsHiddenToRoot(bool isHiddenValue);

	private:
		QList<RestTreeItem*> m_childItems;
		bool m_isOpen;
		RestTreeItem *m_parentItem;
};

#endif // RESTTREEITEM_H
