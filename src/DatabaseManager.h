 /***************************************************************************
 * File:        ./DatabaseManager.h
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

#ifndef _database_manager_h
#define _database_manager_h

#include <string>
#include <iostream>
#include <map>

#include "exceptions.h"
#include "XMLHandler.h"

using namespace std;

/**
 * manages a database
 */
class DatabaseManager : public XMLHandler
{

	private:
		//!hidden copy constructor - we do not want to accidentially copy objects
		DatabaseManager(const DatabaseManager& x);

		bool guiMode;

	protected:
		bool is_initialized;		/**< function init() has been called */
		int last_id;
		
	public:		
		DatabaseManager(const char* path);	/* Constructor */
		virtual ~DatabaseManager(){};	/* Destructor */

	//abstract member functions
		//!start the manager
		virtual void run() = 0;
		//!add database entry
		virtual void addEntry(string ident, DOMNode* parent=0) = 0;
		//! returns a formatted string for display containing all details listed for plugin 'id' in db
		virtual string getEntryDetails(int) = 0;
		//! returns a formatted string for display containing all details listed for plugin 'id' in db
		virtual multimap<string, map<int, string> > getEntryMap()=0;
		//! removes plugin from database. checks before, whether any other plugin depends on ID 
		virtual bool deleteEntry(int) = 0;

	//implemented member functions 		
		//!initialize database 
		virtual void init();
		//!returns database name
		string getDBName();
		//! returns last id of an item added to the database
		int getLastAddedID();
		//! show dialog window;
		void runDialogWindow(string, bool);
		//! has database manager a Dialog Window open?
		bool inGuiMode();
};

#endif // _database_manager_h
