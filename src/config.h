/***************************************************************************
 * File:        ./config.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     11.04.2007
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

#ifndef _config_h	
#define _config_h

typedef int (*green_exec_function)(double**, int, int);		//!< function pointer to exec function of green's function plug-in
typedef double (*load_exec_function)(int, int);				//!< function pointer to exec function of load function plug-in
typedef double (*loadhistory_exec_function)(unsigned int);	//!< function pointer to exec function of load history plug-in
typedef double (*crustaldecay_exec_function)(unsigned int);	//!< function pointer to exec function of load history plug-in
typedef void (*run_function)();						//!< function pointer to exec function of generic plug-in

#endif // _constants_h
