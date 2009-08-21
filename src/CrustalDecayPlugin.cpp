/***************************************************************************
 * File:        ./CrustalDecayPlugin.cpp
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     04.06.2008
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


#include "CrustalDecayPlugin.h"
#include "SimulationCore.h"

CrustalDecayPlugin::CrustalDecayPlugin(const string _name):
	Plugin(_name),
	job_name()
{

}

CrustalDecayPlugin::~CrustalDecayPlugin()
{
  crusde_debug("%s, line: %d, Plugin destroyed: %s ", __FILE__, __LINE__, name.c_str());
}

void CrustalDecayPlugin::setJobName(const string name)
{
	job_name = name;
}

const string CrustalDecayPlugin::getJobName()
{
	return job_name;
}

/*								*/
/* Open the shared library handle				*/
/*								*/
void CrustalDecayPlugin::load(string new_path) throw (FileNotFound, LibHandleError)
{
  crusde_debug("%s, line: %d, CrustalDecayPlugin %s load: %s ", __FILE__, __LINE__, name.c_str(), path.c_str());
  
  if(!new_path.empty())
  {
	Plugin::load(new_path);
	
	//assign function pointer ... 
	/* clear error flag */
	dlerror();
	/* get init address of init function in function lib 		*/ 
	
	func_value = (crustaldecay_exec_function) dlsym( LibHandle, "get_value_at");
	/* if dlsym returns NULL, print error message and leave	*/
	if( func_value == NULL ){
		throw (LibHandleError (dlerror() ) );
	}
   }
  
  
}

double CrustalDecayPlugin::getValueAt(unsigned int time_step) throw (LibHandleError)
{
	if(func_value!=NULL)
		return func_value(time_step);
	else{
		throw (LibHandleError ("CrustalDecayPlugin::getValueAt --- for some reason we got here without having loaded the library function before. SMRT!") );
	}
}

/*								*/
/* get shared library handle's exec function			*/
/*								*/
crustaldecay_exec_function CrustalDecayPlugin::getValueFunction() throw (LibHandleError)
{ 
  /* run function */
 	if( func_value != NULL ){
		 return func_value; 
	}
	else{
		throw (LibHandleError ("CrustalDecayPlugin::getValueFunction --- for some reason we got here without having loaded the library function before. SMRT!") );
	}
 
}
