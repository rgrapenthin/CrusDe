/***************************************************************************
 * File:        ./GreensFunction.h
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

#ifndef _greens_function_bin_h
#define _greens_function_bin_h

#include "Plugin.h"
#include "GreenPlugin.h"
#include "constants.h"
#include "config.h"
#include <iostream>
#include <cassert>
#include <dlfcn.h>
#include <map>

using namespace std;

/**
 *	provides a plug-in interface, but overrides most of the functions since
 *  it holds a list of greens functions.
 *
 *  we don't need an 'id' counter here! It was necessary in the LoadFunction
 *  since all the loads could and should have different parameter values 
 *  whereas the Green's functions are allowed to have only one identical 
 *  parameter set. 
 */
class GreensFunction : public Plugin
{

	private:
		map<string, GreenPlugin*>::iterator  map_iter;
		map<string, GreenPlugin*>            pl_green_map;

		/**hidden copy constructor - we do not want to accidentially copy objects*/
		GreensFunction(const GreensFunction& x); 

	public:
		GreensFunction(const string);  /* Constructor */
		virtual ~GreensFunction();     /* Destructor */
		
		void setFirstJob();
		bool nextJob();
		bool isLastJob();
		string job();

//		virtual void selftest();
		
		/*plug - in interface*/
		int getValueArrayAt(double**, int, int);

        void load(map<string, string>);
		void unload() throw (LibHandleError);
		void registerParameter();
		void registerOutputFields();
		void requestPlugins();
		void init();
		void release();
		
//		void selftest();


};


#endif // _load_function_bin_h
