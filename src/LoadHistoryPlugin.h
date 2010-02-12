/***************************************************************************
 * File:        ./LoadHistoryPlugin.h
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

#ifndef _load_history_plugin_h
#define _load_history_plugin_h

#include "Plugin.h"
#include "constants.h"
#include "config.h"
#include <iostream>
#include <cassert>

using namespace std;

class LoadHistoryPlugin : public Plugin
{

	private:
		loadhistory_exec_function  func_value;			/*  Pointer to loaded routine	*/
		string job_name;
		
		/**hidden copy constructor - we do not want to accidentially copy objects*/
		LoadHistoryPlugin(const LoadHistoryPlugin& x); 
		
	public:
		LoadHistoryPlugin(const string=NULL);	/* Constructor */
		virtual ~LoadHistoryPlugin();			/* Destructor */

		void setJobName(const string);
		const string getJobName();
		
		virtual void load(const string) throw (FileNotFound, LibHandleError);

		/*plug - in interface*/
//		double constrainLoadHeight(double, int, int, int) throw (LibHandleError);
		double getValueAt(unsigned int) throw (LibHandleError);
		loadhistory_exec_function getValueFunction() throw (LibHandleError);
};

#endif // _load_history_plugin_h
