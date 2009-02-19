/***************************************************************************
 * File:        ./LoadFunction.h
 * Author:      Ronni Grapenthin, Geophysical Institute, UAF
 * Created:     2008-02-19
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

#ifndef _load_function_bin_h
#define _load_function_bin_h

#include "Plugin.h"
#include "LoadPlugin.h"
#include "LoadHistoryPlugin.h"
#include "LoadFunctionElement.h"
#include "CrustalDecayPlugin.h"
#include "constants.h"
#include "config.h"
#include <iostream>
#include <cassert>
#include <dlfcn.h>
#include <map>
#include <list>

using namespace std;

/**
 *	provides a plug-in interface, but overrides most of the functions since
 *  it holds a list of loads.
 */
class LoadFunction : public Plugin
{

	private:
		multimap<LoadPlugin*, pair<LoadHistoryPlugin*, CrustalDecayPlugin*> >::iterator   map_iter;
		multimap<LoadPlugin*, pair<LoadHistoryPlugin*, CrustalDecayPlugin*> >             pl_load_map;
		double        load_value;
		unsigned int  load_function_component;
		
		/**hidden copy constructor - we do not want to accidentially copy objects*/
		LoadFunction(const LoadPlugin& x); 

	public:
		LoadFunction(const string);		/* Constructor */
		virtual ~LoadFunction();		/* Destructor */
		
//		virtual void selftest();
		
		/*plug - in interface*/
		double  getValueAt(int, int);
		double  getHistoryValueAt(int);
		double  getCrustalDecayValueAt(int);
		boolean crustalDecayGiven();		
		boolean loadHistoryGiven();		

        void load(list<LoadFunctionElement*> );
		void unload() throw (LibHandleError);
		void registerParameter();
		void registerOutputFields();
		void requestPlugins();
		void init();
		void release();
		
//		void selftest();

};


#endif // _load_function_bin_h
