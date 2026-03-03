 /***************************************************************************
 * File:        ./PluginManager.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     28.03.2007
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

#ifndef _plugin_manager_h
#define _plugin_manager_h

#include "DatabaseManager.h"
#include "XMLHandler.h"
#include "exceptions.h"
#include "Plugin.h"

#include <string>
#include <iostream>
#include <cassert>
#include <dlfcn.h>
#include <map>
 
using namespace std;

/**
 * manages a plugin database (2xml files: one in user's home directory, another in install path of GFLC, with 
 */
class PluginManager : public DatabaseManager
{

	private:
		Plugin 	 	plugin;			/**< one plugin instance that's gonna be managed */

		/**hidden copy constructor - we do not want to accidentially copy objects*/
		PluginManager(const PluginManager& x); 
		
		DOMNode* isInDB(string, string);		
		void addToDB(string filename, string dst);
		DOMNode* getEntryById(int id);
		DOMNode* getEntryById(int id, const XMLCh* tag);
		list<DOMNode*> getDependents(DOMNode *entry);
		list<string> getDependencies(DOMNode *entry);
		list<string> getParameters(DOMNode *entry);

	public:		
		PluginManager(const char* path);	/* Constructor */
		virtual ~PluginManager(){};		/* Destructor */

		//! removes plugin from database. checks before, whether any other plugin depends on ID 
		virtual bool deleteEntry(int id) throw (DatabaseError, PluginError);
		//! starts qt and the plugin manager dialog
		virtual void run();
		//! returns a formatted string for display containing all details listed for plugin 'id' in db
		virtual multimap<string, map<int, string> > getEntryMap();
		//! returns a formatted string for display containing all details listed for plugin 'id' in db
		virtual string getEntryDetails(int id);
		//! runs tests on the plugin found at 'absolute path' and adds it to the database if cleared.
		virtual void addEntry(string absolutepath, DOMNode *parent=0) throw (PluginExistsException);	/* ... */
		
		string getFilename(string, string) throw (DatabaseError);

};

#endif // _plugin_manager_h
