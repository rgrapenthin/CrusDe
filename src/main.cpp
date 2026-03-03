/***************************************************************************
 * File:        ./main.cpp
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     20.02.2007
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

#include "SimulationCore.h"
#include "constants.h"
#include <map>
#include <time.h>

using namespace std;


/* initialization of public constants */
int const NO_TIME = -1;			//< flag for time independent modeling

double const PI	  	= 3.141592653589793;	//< Pi 			[-]
double const PI_half 	= PI / 2;		//< Pi_half		[-]
double const two_PI 	= PI * 2;		//< two Pi		[-]

short const UX = 0;				//< x-direction
short const UY = 1;				//< y-direction
short const UZ = 2;				//< z-direction

/*error codes*/
//int const NOERROR 	=  0;			//< no error occured (0)
//int const ERROR_MSG = -1;			//< error message printed (-1)

char* const VERSION="0.3.0";
char* const SVN_REVISION="r36";

int main( int argc, char** argv )
{

    	time_t start, finish;
    	time(&start);		

	SimulationCore *core = SimulationCore::instance(argc, argv);

	crusde_info("Welcome! ");

	try
	{	
		core->init();	
		crusde_info("Let me start your experiment.");
		core->exec();	
		core->terminate();	
	}
/*	
	catch (const INIT_EXCEPTION& e)
	{
		cerr << "Error during GFLC initialization: \n" << e.msg <<endl;
		cerr << "Aborting." <<endl;
		exit(-1);
	}

	catch (const EXEC_EXCEPTION& e)
	{
		cerr << "Error during GFLC execution: \n" << e.msg <<endl;
		cerr << "Aborting." <<endl;
		exit(-1);
	}

	catch (const KILL_EXCEPTION& e)
	{
		cerr << "Error during GFLC termination: \n"  << e.msg << endl;
		cerr << "Aborting." <<endl;
		exit(-1);
	}
*/		
        catch( const std::bad_alloc& e )
        {
                std::cout << e.what() << std::endl;
        }
	catch ( const std::exception& e)
	{
		crusde_error("Unexpected exception during CrusDe run:\n\t%s", e.what());
	}
	
	time(&finish);
	crusde_info("run time= %f s", (float)difftime(finish,start) );
	crusde_info("Terminating normally! Bye.");
		
	return 0;
}

