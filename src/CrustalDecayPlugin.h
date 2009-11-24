/***************************************************************************
 * File:        ./CrustalDecayPlugin.h
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

#ifndef _crustal_decay_plugin_h
#define _crustal_decay_plugin_h

#include "Plugin.h"
#include "constants.h"
#include "typedefs.h"
#include <iostream>
#include <cassert>

using namespace std;

class CrustalDecayPlugin : public Plugin
{

	private:
		crustaldecay_exec_function  func_value;			/*  Pointer to loaded routine	*/
		string job_name;
		
		/**hidden copy constructor - we do not want to accidentially copy objects*/
		CrustalDecayPlugin(const CrustalDecayPlugin& x); 
		
	public:
		CrustalDecayPlugin(const string=NULL);	/* Constructor */
		virtual ~CrustalDecayPlugin();			/* Destructor */
		
		void setJobName(const string);
		const string getJobName();

		virtual void load(const string) throw (FileNotFound, LibHandleError);

		/*plug - in interface*/
		double getValueAt(unsigned int) throw (LibHandleError);
		crustaldecay_exec_function getValueFunction() throw (LibHandleError);
};

#endif // _crustal_decay_plugin_h
