/***************************************************************************
 * File:        ./LoadFunctionElement.h
 * Author:      Ronni Grapenthin, Geophysical Institute, UAF
 * Created:     2008-06-10
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

#ifndef _load_function_element_h
#define _load_function_element_h

#include <iostream>
#include <string>
#include <map>

using namespace std;

/**
 */
class LoadFunctionElement
{

	private:
		string load_name;
		string history_name;
		string history_job;
		string decay_name;
		string decay_job;

		/**hidden copy constructor - we do not want to accidentially copy objects*/
		LoadFunctionElement(const LoadFunctionElement& x); 

	public:
		LoadFunctionElement(const string);  /* Constructor */
		virtual ~LoadFunctionElement();     /* Destructor */
		
		void setLoadName(const string);
		void setHistoryName(const string);
		void setDecayName(const string);
		void setHistoryJob(const string);
		void setDecayJob(const string);

		const string getLoadName();
		const string getHistoryName();
		const string getDecayName();
		const string getHistoryJob();
		const string getDecayJob();	
};


#endif // _load_function_element_h
