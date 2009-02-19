/****************************************************************************
**
** Copyright (C) 2005-2006 Trolltech AS. All rights reserved.
**
** This file is part of the documentation of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
	treemodel.cpp

        Provides a simple tree model to show how to create and use hierarchical
        models.
*/

#include <QtGui>
#include <map>

#include "Qt_TreeItem.h"
#include "Qt_TreeModel.h"
#include "XMLHandler.h"

TreeModel::TreeModel(const char* id, QObject *parent)
: QAbstractItemModel(parent)
{
	rootItem = new TreeItem(-1, id);
}

TreeModel::~TreeModel()
{
	delete rootItem;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
	
	return item->data(index.column());
}

int TreeModel::getID(const QModelIndex &index) const
{
	if (!index.isValid())
		return -1;

	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
	
	return item->getID();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem *childItem = parentItem->child(row);
	
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}


bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
	TreeItem *parentItem;
    	if (!parent.isValid())
		parentItem = rootItem;
	else		
		parentItem = static_cast<TreeItem*>(parent.internalPointer());
    
    beginRemoveRows(parent, position, position+rows-1);
	parentItem->removeChild(position);
    endRemoveRows();    
	
	if(!hasChildren(parent) && parent.isValid())
	{
		removeRows(parent.row(), 1, parent.parent());
	}
	
	return true;
}

QModelIndex TreeModel::indexOfLatestItem(int id, const QModelIndex &parent)
{
	int i(0);
	TreeItem *childItem;
	
	while(i < rootItem->childCount())
	{
		int j(0);
		childItem = rootItem->child(i);
		
		QModelIndex ind = index(i, 0, QModelIndex());
		
		while(j < childItem->childCount())
		{
			QModelIndex ind2 = index(j, 0, ind);
			
			if(id == getID(ind2))
				return ind2;
			++j;
		}
		++i;
	}
	return QModelIndex();	
}

void TreeModel::setupModelData(multimap<string, map<int, string> > data)
{
	reset();

	int i(0);
	
	while(i < rootItem->childCount())
	{
		rootItem->removeChild(i);
	}
	
	QList<TreeItem*> parents;
        parents << rootItem;
	
	//iterates over multimap
	multimap<string, map<int, string> >::iterator iter = data.begin();
	multimap<string, map<int, string> >::iterator key_iter;
	pair< multimap<string, map<int, string> >::iterator, multimap<string, map<int, string> >::iterator > key_range;
					
	//add experiments to tree model
	while( iter!=data.end() )
	{
		TreeItem *t = new TreeItem(-1, iter->first, parents.last());
		//Append a new item to the current parent's list of children.
		parents.last()->appendChild(t);
		
		//now ... append all the values that have the same key as children		
		key_range = data.equal_range(iter->first);
		//all keys have values that are adresses of double variables in the repsective plugins
		//each of those variables now gets a value assigned. the same value.
		key_iter=key_range.second;
		//we want to list the contents in reverse order
		do{
			--key_iter;
			//second.begin->first is the id, while ->second is the string
			t->appendChild(new TreeItem(key_iter->second.begin()->first, key_iter->second.begin()->second, t));

		}
		while(key_iter!=key_range.first);
	
		iter = key_range.second;
	}	        
}
