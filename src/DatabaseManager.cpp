/***************************************************************************
 * File:        ./DatabaseManager.cpp
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

#include "DatabaseManager.h"
#include "Dialog.h"

DatabaseManager::DatabaseManager(const char *db) :
	XMLHandler(db),
	guiMode(false),
	is_initialized(false),
	last_id(-1)
{
}

/**
 * read XML file and read plugins from there into a structured list
 */
void DatabaseManager::init()
{
	if(!is_initialized)
	{
		readXML();		
		is_initialized = true;
	}
}

/**
 * return filename of the database with absolute path
 */
string DatabaseManager::getDBName()
{
	return xmlFile;
}

/**
 * return id of last entry that's been added to the database
 */
int DatabaseManager::getLastAddedID()
{
	return last_id;
}

/**
 * create Dialog window and switch into guiMode, run dialog window
 */
void DatabaseManager::runDialogWindow(string title, bool addButton)
{
	//init qt Application
	QApplication app(0,0);
	//create and run dialog window 
	Dialog d(this, title, addButton);
	guiMode = true;
	d.exec();
	guiMode = false;
}

/**
 *
 */
bool DatabaseManager::inGuiMode()
{
	return guiMode;
}
