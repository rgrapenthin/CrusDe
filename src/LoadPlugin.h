/***************************************************************************
 * File:        ./LoadPlugin.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     09.03.2007
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

#ifndef _load_plugin_h
#define _load_plugin_h

#include "Plugin.h"
#include "constants.h"
#include "config.h"
#include <iostream>
#include <cassert>
#include <dlfcn.h>

using namespace std;

//typedef void (*value_void_t)(loadhistory_exec_function);

/**
 *	
 */
class LoadPlugin : public Plugin
{

	private:
		load_exec_function        func_value;			/*  Pointer to loaded routine	*/
//		value_void_t   set_history_func_value;	/*  pointer to loaded set history function */
		/**hidden copy constructor - we do not want to accidentially copy objects*/
		LoadPlugin(const LoadPlugin& x); 

	public:
		LoadPlugin(const string=NULL);	/* Constructor */
		virtual ~LoadPlugin();		/* Destructor */
		
		virtual void load(const string) throw (FileNotFound, LibHandleError);
//		virtual void selftest();
//		void 	setHistoryFunction(loadhistory_exec_function history_func) throw (LibHandleError);
		
		/*plug - in interface*/
		double  getValueAt(int, int);			
		
		/*get pointers to exec functions*/
		load_exec_function getValueFunction();
};

#endif // _load_plugin_h
