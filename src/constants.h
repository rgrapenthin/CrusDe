/***************************************************************************
 * File:        ./constants.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     10.03.2007
 * Licence:     GPLv2
 *
 * #########################################################################
 *
 * CrusDe, simulation framework for crustal deformation studies
 * Copyright (C) 2007-2009 Ronni Grapenthin
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

#ifndef _constants_h	
#define _constants_h

#define EXPERIMENT_DIR  "experiment-db"
#define EXPERIMENT_DB   "experiment-db.xml"
#define PLUGIN_DIR      "plugins"
#define PLUGIN_DB       "plugins.xml"

#define N_LOAD_COMPS    1000

#define DIR_SEP	        "/"

extern int    const NO_TIME;
extern double const PI;			//!< Pi 		[-]
extern double const PI_half;		//!< Pi half		[-]
extern double const two_PI;		//!< two Pi 		[-]

extern short const UX;			//!< marks x-direction 
extern short const UY;			//!< marks y-direction 
extern short const UZ;			//!< marks z-direction

// errors
//extern int const NOERROR;		//!< no error occured
//extern int const ERROR_MSG;		//!< error message printed, aborting program

extern char* const VERSION;
extern char* const SVN_REVISION;

#endif // _constants_h
