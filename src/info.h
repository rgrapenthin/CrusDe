/***************************************************************************
 * File:        ./info.h
 * Author:      Ronni Grapenthin, UAF/GI
 * Created:     2009-07-11
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

#ifndef _info_h	
#define _info_h

/*------------------------------------------------------------------------------*/
/* USAGE INFO									*/
/*------------------------------------------------------------------------------*/
static void usage()
{
    cerr << "\nUsage:\n"
            "    crusde <XML file | <guiOptions> | <options> >\n\n"
            "This program runs an experiment that is parameterized by an XML file, \n"
	    "or starts a gui for either the experiment manager or the plug-in manager.\n\n"
            "    guiOptions:\n"
	    "      -P|p                       run Plug-in manager.\n" 
	    "      -P|p install <plug-in.so>  Install 'plug-in.so' from command line.\n" 
	    "      -M|m                       run Experiment manager.\n\n" 
            "    Options:\n"
	    "      -h|?                       show this help.\n" 
	    "      -Q|q                       quiet run, minimal output.\n" 
	    "      -v|V                       show version information.\n" 
         << endl;
}

/*------------------------------------------------------------------------------*/
/* VERSION INFO									*/
/*------------------------------------------------------------------------------*/
static void version()
{
    cout << "\nCrusDe, Version "<<VERSION<< " (svn revision: "<<SVN_REVISION<<"),\n"
            "    compiled " << __DATE__ << ", " << __TIME__ << "\n\n"
            "Copyright (C) 2007-2009 Ronni Grapenthin and authors of individual plug-ins.\n\n"
	    "This is free software (free as in free food); see the source for conditions.\nThere is NO WARRANTY. None. You are welcome to read the source and check, though.\n"
	    "CrusDe is open source software; see http://www.gps.alaska.edu/crusde \n\n"
            "Plug-in versions: \n"
            "    TODO \n"
         << endl;
}

#endif // _info_h
