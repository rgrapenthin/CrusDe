/***************************************************************************
 * File:        ./ExperimentManager.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     13.04.2007
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

#ifndef _experiment_manager_h
#define _experiment_manager_h

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <map>

#include "DatabaseManager.h"

using namespace std;

/**
 * manages the experiment database of the actual user
 */
class ExperimentManager : public DatabaseManager
{

	private:
		//!hidden copy constructor - we do not want to accidentially copy objects
		ExperimentManager(const ExperimentManager& x); 
		//!hidden assignment operator - we do not want to accidentially copy objects
		ExperimentManager const &operator=(ExperimentManager const &rvalue);
		
		DOMNode 		*actual_experiment;
		
		string childrenToString(DOMNode *n);
		string childrenToString(DOMNode *n, const XMLCh* tag);
		DOMNode* getEntryById(int id);
		
	public:		
		ExperimentManager(const char*);	/* Constructor */
		virtual ~ExperimentManager(){};	/* Destructor */
		
		//!start the manager
		virtual void run();
		//!add database entry
		virtual void addEntry(string ident, DOMNode* parent=0);
		//! returns a formatted string for display containing all details listed for plugin 'id' in db
		virtual string getEntryDetails(int id);
		//! returns a formatted string for display containing all details listed for plugin 'id' in db
		virtual multimap<string, map<int, string> > getEntryMap();
		//! removes plugin from database. checks before, whether any other plugin depends on ID 
		virtual bool deleteEntry(int id);
		
		void addMetaData(string);

};

#endif // _experiment_manager_h
