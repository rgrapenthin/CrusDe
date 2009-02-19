/***************************************************************************
 * File:        ./ParamWrapper.h
 * Author:      Ronni Grapenthin, Geophysical Institute, UAF
 * Created:     2008-02-27
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

#ifndef param_wrapper_h
#define param_wrapper_h

#include <iostream>
#include <string>

#include <string.h>

using namespace std;

/**
 *	provides a plug-in interface, but overrides most of the functions since
 *  it holds a list of loads.
 */
class ParamWrapper
{

	private:
		string 	*string_param;
		double  *double_param;

		char* cStr;

	public:
		ParamWrapper();	/* Constructor */

		~ParamWrapper();		/* Destructor */
		
		bool isString();
		bool isDouble();

		void setValue(string);
		void setValue(double);

		char** newString();
		double* newDouble();

		char** stringValue();
		double* doubleValue();

};


#endif // param_wrapper_h
