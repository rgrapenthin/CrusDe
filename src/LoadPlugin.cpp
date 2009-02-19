/***************************************************************************
 * File:        ./LoadPlugin.cpp
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

#include "LoadPlugin.h"
#include "SimulationCore.h"

/* class LoadPlugin */

LoadPlugin::LoadPlugin(const string _name):
	Plugin(_name)
{

}

LoadPlugin::~LoadPlugin()
{
  Debug("%s, line: %d, Plugin destroyed: %s ", __FILE__, __LINE__, name.c_str());
}

/*								*/
/* Open the shared library handle				*/
/*								*/
void LoadPlugin::load(string new_path) throw (FileNotFound, LibHandleError)
{
  Debug("%s, line: %d, LoadPlugin %s load: %s ", __FILE__, __LINE__, name.c_str(), path.c_str());
  Plugin::load(new_path);
    
  //assign function pointer ... load functions
  /* clear error flag */
  dlerror();
  func_value = (load_exec_function) dlsym( LibHandle, "get_value_at");
  /* if dlsym returns NULL, print error message and leave	*/
  if( func_value == NULL ){
  	throw (LibHandleError (dlerror() ) );
    }
}

/*								*/
/* Use the shared library handle				*/
/*								*/
double LoadPlugin::getValueAt(int x, int y)
{ 
  assert(is_initialized);
  /* run function */
  return func_value(x,y); 
}

/*								*/
/* return pointer to exec function				*/
/*								*/
load_exec_function LoadPlugin::getValueFunction()
{ 
  /* run function */
  return func_value; 
}
