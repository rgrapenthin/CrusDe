/***************************************************************************
 * File:        ./DataOutPlugin.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     18.03.2007
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

#ifndef _data_out_plugin_h
#define _data_out_plugin_h

#include "Plugin.h"
#include "constants.h"
#include <iostream>
#include <cassert>
#include <dlfcn.h>

using namespace std;

class DataOutPlugin : public Plugin
{

	private:
		typedef double (*value_t)(double**, int, int);
		value_t  func_value;			/*  Pointer to loaded routine	*/
		
		/**hidden copy constructor - we do not want to accidentially copy objects*/
		DataOutPlugin(const DataOutPlugin& x); 
		
	public:
		DataOutPlugin(const string=NULL);	/* Constructor */
		virtual ~DataOutPlugin();			/* Destructor */
		
		virtual void load(const string) throw (FileNotFound, LibHandleError);

		/*plug - in interface*/
		void setModelData(double**);
		void setMetaData(){};
	
};

#endif // _data_out_plugin_h
