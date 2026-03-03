/***************************************************************************
 * File:        ./LoadHistoryPlugin.cpp
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


#include "LoadHistoryPlugin.h"
#include "SimulationCore.h"

LoadHistoryPlugin::LoadHistoryPlugin(const string _name):
	Plugin(_name),
	job_name()
{

}

LoadHistoryPlugin::~LoadHistoryPlugin()
{
  crusde_debug("%s, line: %d, Plugin destroyed: %s ", __FILE__, __LINE__, name.c_str());
}

void LoadHistoryPlugin::setJobName(const string _name)
{
	job_name = _name;
}

const string LoadHistoryPlugin::getJobName()
{
	return job_name;
}

/*								*/
/* Open the shared library handle				*/
/*								*/
void LoadHistoryPlugin::load(string new_path) throw (FileNotFound, LibHandleError)
{
  crusde_debug("%s, line: %d, LoadHistoryPlugin %s load: %s ", __FILE__, __LINE__, name.c_str(), path.c_str());
  
  if(!new_path.empty())
  {
	Plugin::load(new_path);
	
	//assign function pointer ... 
	/* clear error flag */
	dlerror();
	/* get init address of init function in function lib 		*/ 
	
//	func_value = (value_t) dlsym( LibHandle, "constrain_load_height");
	func_value = (loadhistory_exec_function) dlsym( LibHandle, "get_value_at");
	/* if dlsym returns NULL, print error message and leave	*/
	if( func_value == NULL ){
		throw (LibHandleError (dlerror() ) );
	}
   }
  
  
}

double LoadHistoryPlugin::getValueAt(unsigned int time_step) throw (LibHandleError)
{
	if(func_value!=NULL)
		return func_value(time_step);
	else{
		throw (LibHandleError ("LoadHistoryPlugin::constrainLoadHeight --- for some reason we got here without having loaded the library function before. SMRT!") );
	}
}

/*								*/
/* get shared library handle's exec function			*/
/*								*/
loadhistory_exec_function LoadHistoryPlugin::getValueFunction() throw (LibHandleError)
{ 
  /* run function */
 	if( func_value != NULL ){
		 return func_value; 
	}
	else{
		throw (LibHandleError ("LoadHistoryPlugin::constrainLoadHeight --- for some reason we got here without having loaded the library function before. SMRT!") );
	}
 
}
