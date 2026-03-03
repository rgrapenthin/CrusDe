 /***************************************************************************
 * File:        ./Dialog.cpp
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

#include <cstdlib>	//getenv

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include "Dialog.h"
#include "constants.h"

Dialog::Dialog(DatabaseManager* manager_, string title, bool addbutton) :
	manager(manager_),
	details(""),
	layout_main(),
	layout_ctrlbutt(),
	treemodel((addbutton==true? "Plugin" : "Experiment"), this ),
	treeview(this),
	current_dir("")
{
//set current directory to Home dir
	current_dir = QDir::toNativeSeparators(QDir::homePath());
	
//set name
	this->setWindowTitle(QObject::tr(title.c_str()));
	
//experiment view
	treeview.setUniformRowHeights(false);
	treeview.setModel(&treemodel);
	treeview.setAutoFillBackground(true);
	treeview.setEditTriggers(QAbstractItemView::AllEditTriggers);
	treeview.setRootIsDecorated(true);	//'+' / '-' 
	treeview.setMaximumWidth(175);
	connect(&treeview, SIGNAL( clicked(QModelIndex) ), SLOT( showDetails(QModelIndex) ) );

// details
	details.setReadOnly(true);
	details.setAcceptRichText(true);
	
	//add treeview
	layout_main.addWidget(&treeview);
	//add details
	layout_main.addWidget(&details);

//programmstart kn�pfe
	if(addbutton)
	{
		butt_ADD   = new QPushButton(  QObject::tr("Add") );
			connect( butt_ADD, SIGNAL( released() ), SLOT(addClicked()) );
			butt_ADD->resize(butt_ADD->minimumSize());
			butt_ADD->setDefault( true );
	}
	butt_DELETE = new QPushButton( QObject::tr("Delete") );
		butt_DELETE->resize(butt_DELETE->minimumSize());
		connect( butt_DELETE, SIGNAL( released() ), SLOT(deleteClicked()) );
	butt_QUIT   = new QPushButton(  QObject::tr("Close") );
		connect( butt_QUIT, SIGNAL( released() ), SLOT(quitClicked()) );
		butt_QUIT->resize(butt_QUIT->minimumSize());
		butt_QUIT->setDefault( true );

	layout_ctrlbutt.setObjectName("layout_ctrlbutt");
	layout_ctrlbutt.setSizeConstraint( QLayout::SetFixedSize );
	layout_ctrlbutt.setSizeConstraint( QLayout::SetFixedSize );
	if(addbutton)
	{
		layout_ctrlbutt.addWidget(butt_ADD);		
	}
	layout_ctrlbutt.addWidget(butt_DELETE);		
	layout_ctrlbutt.addWidget(butt_QUIT);
	layout_ctrlbutt.addStretch(200);

	//add control buttons
	layout_main.addLayout(&layout_ctrlbutt);
	
	setLayout(&layout_main);
	
	setModelData();

}

void Dialog::deleteClicked()
{
	QModelIndex current_index(treeview.currentIndex());
	int pos(-1);
	//make sure we will only delete children
	if(!treemodel.hasChildren(current_index))
		pos = current_index.row();
	
	if(pos >= 0){
		//delete from XML
		try {
			manager->deleteEntry( treemodel.getID( current_index ) );
			treemodel.removeRows(pos, 1, current_index.parent());	
			treeview.clearSelection();
			treeview.update();
			details.clear();
		}
		catch(DatabaseError e)
		{
			QMessageBox::information ( this, "There was a database error." , e.what(), 
						"I'll check on that.");
		}
		catch(runtime_error e)
		{
			QMessageBox::critical ( this, "There was a error." , e.what(), 
						"We better stop here.");
			reject();
		}
	}
}

void Dialog::addClicked()
{
	QString s = QFileDialog::getOpenFileName(
                    this,
                    "Choose an item to add",
		    current_dir,
                    "Shared Libraries (*.so )");
		    
	//cancel is not pressed
	if(!s.isEmpty()){
		//save current directory
		QDir dummy;
		current_dir = dummy.absoluteFilePath(s);

		//prepare info window
		string info("You gave me a plugin which I will now try to add to the database.\n");
		info.append("Before that, I'll run some tests on this new file.\n");
		info.append("The application might terminate (abort, end, die) in case\n");
		info.append("there's anything with that new file. You'll find an error\n");
		info.append("report on the commandline.\n");
		info.append("It's not your fault! But it means, it cannot be added.\n");
		info.append("In a more fortunate case you'll soon find it in the tree.\n");
	
		if( QMessageBox::information ( 	this, "Do not worry!", info.c_str(), 
						"Go for it!", "I'd rather do something else") == 0 )
		{
			try{
				manager->addEntry(s.toStdString());
			}
			catch (PluginExistsException e)
			{
				QMessageBox::information(this, "An Error occured!", e.what(), "Oh, that's good then.");
			}
			catch (DatabaseError e)
			{
				QMessageBox::information(this, "An Error occured!", e.what(), "Better go and install that then.");
			}
			catch (runtime_error e)
			{
				QMessageBox::critical(this, "An Error occured!", e.what(), "Well then, I'll take care on that!");
				reject();
			}
		}		
		
		setModelData();
		QModelIndex i( treemodel.indexOfLatestItem(manager->getLastAddedID()) );
		showDetails(i);
		treeview.setCurrentIndex(i);
		treeview.setExpanded(i.parent(), true);

	}
	
}

void Dialog::quitClicked()
{
	reject();
}


void Dialog::setModelData()
{
	treemodel.setupModelData(manager->getEntryMap());
	treeview.adjustSize();
}

void Dialog::showDetails(QModelIndex index)
{
	//if we're at the second level
	if(index.parent().isValid()){
 		int id(treemodel.getID(index));
		details.setHtml(QString(manager->getEntryDetails(id).c_str()));
	}
}

