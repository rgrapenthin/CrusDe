/***************************************************************************
 * File:        ./GreensFunctionPlugin.h
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

#ifndef _green_plugin_h
#define _green_plugin_h

#include "Plugin.h"
#include "constants.h"
#include "config.h"
#include <iostream>
#include <cassert>
#include <map>
#include <dlfcn.h>

using namespace std;

class GreenPlugin : public Plugin
{

	private:
        map<string, string> my_jobs;
		green_exec_function func_value_array;		/*  Pointer to loaded routine	*/
		/**hidden copy constructor - we do not want to accidentially copy objects*/
		GreenPlugin(const GreenPlugin& x); 

	public:
		GreenPlugin(const string=NULL);	/* Constructor */
		virtual ~GreenPlugin();		/* Destructor */
		
		virtual void load( const string ) throw (FileNotFound, LibHandleError);
//		virtual void selftest();
		
		/*plug - in interface*/
		int getValueArrayAt(double**, int, int);
		/*get pointers to exec function*/
		green_exec_function getArrayFunction();
};

#endif // _green_plugin_h
