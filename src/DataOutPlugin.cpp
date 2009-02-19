/***************************************************************************
 * File:        ./DataOutPlugin.cpp
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

#include "DataOutPlugin.h"
#include "SimulationCore.h"

DataOutPlugin::DataOutPlugin(const string _name):
	Plugin(_name)
{

}

DataOutPlugin::~DataOutPlugin()
{
  Debug("%s, line: %d, Plugin destroyed: %s ", __FILE__, __LINE__, name.c_str());
}

/*								*/
/* Open the shared library handle				*/
/*								*/
void DataOutPlugin::load(string new_path) throw (FileNotFound, LibHandleError)
{
  Debug("%s, line: %d, DataOutPlugin %s load: %s ", __FILE__, __LINE__, name.c_str(), path.c_str());
  Plugin::load(new_path);
    
  //assign function pointer ... 
  /* clear error flag */
  dlerror();
  /* get init address of init function in function lib 		*/ 
  
  func_value = (value_t) dlsym( LibHandle, "set_model_data");
  /* if dlsym returns NULL, print error message and leave	*/
  if( func_value == NULL ){
    	throw (LibHandleError (dlerror() ) );
  }
  
  
}

/*								*/
/* Use the shared library handle				*/
/*								*/
void DataOutPlugin::setModelData(double **result)
{     
  Debug("%s, line: %d, DataOutPlugin::setModelData: %s ", __FILE__, __LINE__, name.c_str());
  /* run function */
  func_value(result, 0, 0);  
}
