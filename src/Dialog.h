 /***************************************************************************
 * File:        ./Dialog.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     24.04.2007
 * Licence:     GPLv2
 *
 * #########################################################################
 *
 * CrusDe, simulation framework for crustal deformation studies
 * Copyright (C) 2007 Ronni Grapenthin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#ifndef _dialog_h
#define _dialog_h

#include <QTreeView>
#include <QTextEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QApplication>
#include <QPushButton>
#include <QLayout>
#include <QModelIndex>

#include "Qt_TreeModel.h"
#include "DatabaseManager.h"

class Dialog : public QDialog
{
        Q_OBJECT

    public:
       Dialog(DatabaseManager*, string title, bool addbutton);

    private slots:
	virtual void deleteClicked();
	virtual void addClicked();
	virtual void quitClicked();
	virtual void showDetails(QModelIndex);

    private:
    	void setModelData();
	
	DatabaseManager  *manager;	
	QTextEdit	details;
	QHBoxLayout 	layout_main;
	QVBoxLayout 	layout_ctrlbutt;
	TreeModel	treemodel;
	QTreeView   	treeview;
	QPushButton 	*butt_QUIT, *butt_DELETE, *butt_ADD;
	QString		current_dir; //memorizes directory last used, home directory is default
};
#endif //_dialog_h
