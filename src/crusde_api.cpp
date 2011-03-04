/***************************************************************************
 * File:        ./crusde_api.c
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
 * -------------------------------------------------------------------------
 * Description: Implements the Application Programming Interface for
 *		the green's functions modeling tool.
 ****************************************************************************/

#include <iostream>
#include <map>
#include <string>
#include <stdio.h>
#include "crusde_api.h"
#include "SimulationCore.h"
#include "DataOutPlugin.h"
#include "GreensFunction.h"
#include "LoadFunction.h"
#include "ParamWrapper.h"

using namespace std;

/* forward declaration of private functions */
//void print_params(multimap<string, ParamWrapper> m);
//void print_params_map(map<unsigned int, multimap<string, ParamWrapper> >m);

/* PARAMETER REGISTRATION */
//functions for parameter registration; all type specific functions do:
//creates param wrapper, registers it with the simulation core and returns it to the
//calling function so they can allocate memory for the right type.

//! Prints error message to stderr, exists CrusDe with code ERROR_MSG
/*!
	This function prints an error message to stderr. It works exactly like
	printf, hence complex message printing is possible. The simulation run
	will be terminated by calling crusde_exist with the error code ERROR_MSG.

    \param format A formatted string following the sprintf conventions
    \param ... Elipsis containing the values to be plugged into the format string	
    \return void
    \sa crusde_warning(), crusde_info(), crusde_debug(), crusde_exit() 
*/
extern "C" void crusde_error(const char* format, ...){
	va_list args;
	va_start( args, format );
	fprintf( stderr, "[ CrusDe ERROR ] : " );
	vfprintf( stderr, format, args );
	fprintf( stderr, "\n" );
	fprintf( stderr, "Aborting.\n" );
	va_end( args );
	fflush( stderr );

	//abort program
	crusde_exit(ERROR_MSG);
}

//! Prints warning message to stderr.
/*!
	This function prints a warning message to stderr. It works exactly like
	printf, hence complex message printing is possible. The simulation run
	will be continued.

    \param format A formatted string following the sprintf conventions
    \param ... Elipsis containing the values to be plugged into the format string	
    \return void
    \sa crusde_error(), crusde_info(), crusde_debug()
*/
extern "C" void crusde_warning(const char* format, ...){
	va_list args;
	va_start( args, format );
	fprintf( stderr, "[ CrusDe WARNING ] : " );
	vfprintf( stderr, format, args );
	fprintf( stderr, "\n" );
	va_end( args );
	fflush( stderr );
}

//! Prints information message to stdout.
/*!
	This function prints an information message to stdout. It works exactly like
	printf, hence complex message printing is possible. The simulation run
	will be continued.

    \param format A formatted string following the sprintf conventions
    \param ... Elipsis containing the values to be plugged into the format string	
    \return void
    \sa crusde_warning(), crusde_error(), crusde_debug() 
*/
extern "C" void crusde_info(const char* format, ...){
	if(!SimulationCore::instance()->isQuiet()){
		va_list args;
		va_start( args, format );
		fprintf( stdout, "[ CrusDe ] : " );
		vfprintf( stdout, format, args );
		fprintf( stdout, "\n" );
		va_end( args );
		fflush( stdout );
	}
}

//! Prints debug messages to stderr if compiled with -DDEBUG
/*!
	This function prints a debug message to stderr if CrusDe was compiled with
	flag -DDEBUG; else nothing is printed; hence debug statements can stay
	with the code. It is strongly recommended to use this function that 
	works exactly like printf, instead of printf. This way the output 
	of CrusDe is not cluttered with tests and development messages.

    \param format A formatted string following the sprintf conventions
    \param ... Elipsis containing the values to be plugged into the format string	
    \return void
    \sa crusde_warning(), crusde_info(), crusde_error()
*/
extern "C" void crusde_debug(const char* format, ...){
#ifdef DEBUG
	//cannot check for quietness, since one should be able to call it even in SimulationCore constructor.
	va_list args;
	va_start( args, format );
	fprintf( stderr, "[ CrusDe debug ] : " );
	vfprintf( stderr, format, args );
	fprintf( stderr, "\n" );
	va_end( args );
	fflush( stderr );
#endif /* defined(DEBUG) */
}


//! Register double parameter that is required by a plug-in.
/*!
	Use this function if your parameter is not optional. Be sure to define sensible Parameter names
	that convey some context of the actual parameter, e.g., 'E' for Young's modulus or 'g' for 
	gravitational acceleration. Keep in mind that the 'name' is case sensitive.

    \param name 		Name of the parameter (defines how the parameter has to show up in the XML model file)
    \param category 	Elipsis containing the values to be plugged into the format string	
    \return double*		Pointer to the memory slot allocated for the parameter (keep track if you got multiple plug-ins of the same type!)
    \sa PluginCatergory, crusde_register_optional_param_double()
 */
extern "C" double* crusde_register_param_double(const char* name, PluginCategory category)
{
	crusde_debug("crusde_register_param_double called for: %s", name);
	ParamWrapper *p = new ParamWrapper();
	p->setOptional(false);
	SimulationCore::instance()->registerParam(p, name, category);
	double* add = p->newDouble();
	return add;
}

//! Register optional double parameter for a plug-in. 
/*!
	Use this function if your parameter is optional. Be sure to define sensible Parameter names
	that convey some context of the actual parameter, e.g., 'E' for Young's modulus or 'g' for 
	gravitational acceleration. Keep in mind that the 'name' is case sensitive.

    \param name 			Name of the parameter (defines how the parameter has to show up in the XML model file)
    \param category 		Elipsis containing the values to be plugged into the format string	
	\param default_value	Obviously the value that this parameter is set to if it's not defined in the input file
    \return double*		Pointer to the memory slot allocated for the parameter (keep track if you got multiple plug-ins of the same type!)
    \sa PluginCatergory, crusde_register_param_double()
 */
extern "C" double* crusde_register_optional_param_double(const char* name, PluginCategory category, double default_value)
{
	crusde_debug("crusde_register_optional_param_double called for:%s", name);
	ParamWrapper *p = new ParamWrapper();
	p->setOptional(true);
	SimulationCore::instance()->registerParam(p, name, category);
	double* add = p->newDouble(default_value);
	return add;
}

//! Register string parameter that is required by a plug-in, e.g. if you need a file name 
/*!
	Use this function if your parameter is optional. Be sure to define sensible Parameter names
	that convey some context of the actual parameter, e.g., 'load_file' for a file that defines 
	the surface load, or 'result' for the result file. Keep in mind that the 'name' is case sensitive.

    \param name 		Name of the parameter (defines how the parameter has to show up in the XML model file)
    \param category 	Elipsis containing the values to be plugged into the format string	
    \return char**		Pointer to the memory slot allocated for the parameter (pointer to a char array (keep track if you 
						got multiple plug-ins of the same type!)
    \sa PluginCatergory, crusde_register_optional_param_string()
 */
extern "C" char** crusde_register_param_string(const char* name, PluginCategory category)
{
	crusde_debug("crusde_register_param_string called for:%s", name);
	ParamWrapper *p = new ParamWrapper();
	p->setOptional(false);
	SimulationCore::instance()->registerParam(p, name, category);
	char** add =  p->newString();
	return add;
}

//! Register optional string parameter for a plug-in, e.g. if you want to allow for an optional output file.
/*!
	Use this function if your parameter is optional. Be sure to define sensible Parameter names
	that convey some context of the actual parameter, e.g., 'load_file' for a file that defines 
	the surface load, or 'result' for the result file. Keep in mind that the 'name' is case sensitive.

    \param name 			Name of the parameter (defines how the parameter has to show up in the XML model file)
    \param category 		Elipsis containing the values to be plugged into the format string	
	\param default_value	Obviously the value that this parameter is set to if it's not defined in the input file
    \return double*		Pointer to the memory slot allocated for the parameter (keep track if you got multiple plug-ins of the same type!)
    \sa PluginCatergory, crusde_register_param_double()
 */
extern "C" char** crusde_register_optional_param_string(const char* name, PluginCategory category, char* default_value)
{
crusde_debug("crusde_register_optional_param_string called for: %s\n", name);
	ParamWrapper *p = new ParamWrapper();
	p->setOptional(true);
	SimulationCore::instance()->registerParam(p, name, category);
	char** add =  p->newString(default_value);
	return add;
}



//! Request space in the output array. This allocates necessary memory for your results.
/*!
	CrusDe manages your memory this way and also assures that your results make it into the 
	output data at the specified position. E.g., if the 'position' you get from 
	CrusDe is 5 you will find your results in column 5 of an ascii table or in the 
	5th index of a netCDF file.

    \param position 		pointer to memory slot to which the output array position will be written. has to show up in the XML model file)
    \param field 			claim one of the standard fields (e.g. deformation in X/Y/Z direction; or an additional, user defined field)
    \return void
    \sa FieldName
 */
extern "C" void crusde_register_output_field(int* position, FieldName field)
{
    return SimulationCore::instance()->registerOutputField(position, field);
}

/* PARAMETER PRINTING */
/*
extern "C" void crusde_print_green_params()
{
	print_params(SimulationCore::s_params.green);
}

extern "C" void crusde_print_load_params()
{
	print_params_map(SimulationCore::s_params.load);
}

extern "C" void crusde_print_kernel_params()
{
	print_params(SimulationCore::s_params.kernel);
}

void print_params(multimap<string , ParamWrapper> m)
{
  	multimap<string , ParamWrapper>::iterator iter = m.begin();
	
	while( iter != m.end()) 
	{
		printf("%s = %f\n", iter->first.c_str(), *(iter->second));
		++iter;
 	}
}

void print_params_map(map<unsigned int, multimap<string , ParamWrapper> > m)
{
  	map<unsigned int, multimap<string , ParamWrapper> >::iterator iter = m.begin();
	
	while( iter != m.end()) 
	{
	  	multimap<string , ParamWrapper>::iterator multi_iter = iter->second.begin();
		while( multi_iter != iter->second.end()) 
		{
//			printf("%d --- %s = %f\n", iter->first, multi_iter->first.c_str(), *(multi_iter->second));
			++multi_iter;
		}
		++iter;
 	}

}
*/

//! Operator can set the spatial dimensions it works on.
/*!
	This is useful if somebody wants to temporarily store and ACCESS spatial results - arrays are stored linearily.

    \param nx 		Number of fields in the X direction, i.e., easting
    \param ny 		Number of fields in the Y direction, i.e., northing
    \return void
    \sa crusde_get_operator_space()
 */
extern "C" void  crusde_set_operator_space(int nx, int ny)
{
    SimulationCore::instance()->setOperatorSpace(nx, ny);
}

//! Read model dimensions, i.e. Region of Interest
/*!
    \param *nx 		pointer to memory slot to which the number of fields in X direction is to be written
    \param *ny 		pointer to memory slot to which the number of fields in Y direction is to be written
    \return void
    \sa crusde_set_operator_space()
 */
extern "C" void crusde_get_operator_space(int *nx, int *ny)
{
	(*nx) = SimulationCore::instance()->getOperatorSpaceX();
	(*ny) = SimulationCore::instance()->getOperatorSpaceY();
}


//! Get value of Green's function at a specific coordinate
/*!
    \param result 	pointer to array to which the Green's function (possibly for 3 spatial dimensions) is written
    \param x,y		spatial coordinates for which the Green's function is requested
    \return Error code	
 */
extern "C" int crusde_get_green_at(double **result, int x, int y)
{
    return SimulationCore::instance()->greensFunction()->getValueArrayAt(result,x,y);
}

//! Get value of Load function at Point p(x,y).
/*!
	\param x,y 		X,Y coordinates of grid point at which the load value is sought.
    \return double 	Load at Point p(x,y).
 */
extern "C" double crusde_get_load_at(int x, int y)
{
    return SimulationCore::instance()->loadFunction()->getValueAt(x,y);
}

//! Get value of Load History function at time t.
/*!
	\param t 		Time for which the load history value is sought.
    \return double 	Load History value at time t.
 */
extern "C" double crusde_get_load_history_at(int t)
{
    return SimulationCore::instance()->loadFunction()->getHistoryValueAt(t);
}


//! Get total number of load functions defined in the XML model file
/*!
    \return int	just what the function suggests
 */
extern "C" int crusde_get_number_of_loads()
{
	return SimulationCore::instance()->getNumberOfLoadComponents();
}

//! Get value of Crustal Decay function at time t.
/*!
	\param t 		Time for which the crustal decay function value is sought.
    \return double 	Crustal decay at time t.
 */
extern "C" double crusde_get_crustal_decay_at(int t)
{
    return SimulationCore::instance()->loadFunction()->getCrustalDecayValueAt(t);
}

//! X, Easting dimension of the spatial grid
/*! \sa crusde_get_size_y(), crusde_get_size_t() */
extern "C" int crusde_get_size_x()
{
    return SimulationCore::instance()->sizeX();
}

//! Y, Northing dimension of the spatial grid
/*! \sa crusde_get_size_x(), crusde_get_size_t() */
extern "C" int crusde_get_size_y()
{
    return SimulationCore::instance()->sizeY();
}

//! Basically the number of time steps to be modeled
extern "C" int crusde_get_size_t()
{
   return SimulationCore::instance()->sizeT();
}

//! get size of spatial grid defined in XML model file
extern "C" int crusde_get_gridsize()
{
    return SimulationCore::instance()->gridSize();
}

//! total number of output dimensions
/*! \sa crusde_get_displacement_dimensions()*/
extern "C" int crusde_get_dimensions()
{
    return SimulationCore::instance()->getDimensions();
}

//! displacement dimensions that are modeled, e.g. X, Z or X,Y or Z only
/*! \sa crusde_get_dimensions()*/
extern "C" int crusde_get_displacement_dimensions()
{
    return SimulationCore::instance()->displacementDimensions();
}

//! Minimum X value, i.e. Western limit of the region of interest
/*! \sa crusde_get_min_y()*/
extern "C" int crusde_get_min_x()
{
    return SimulationCore::instance()->minX();
}

//! Minimum Y value, i.e. Southern limit of the region of interest
/*! \sa crusde_get_min_x()*/
extern "C" int crusde_get_min_y()
{
    return SimulationCore::instance()->minY();
}

//! hand model result to Simulation Core
/*! \sa crusde_get_result()*/
extern "C" void crusde_set_result(double **result)
{
    SimulationCore::instance()->setModelData(result);
}

//! retrieve model result from Simulation Core
/*! \sa crusde_set_result()*/
extern "C" double** crusde_get_result()
{
    return SimulationCore::instance()->getModelData();
}

//! Set Coordinate system quadrant (1-4) that we're currently working in
/*! 
	CrusDe is inherently cartesian in nature, if a Green's function, for example, operates in a 
	Cyllindrical world, it might be necessary to know which quadrant we're in to correctly 
	determine the direction cosine. 

	\sa crusde_get_quadrant()
*/
extern "C" void crusde_set_quadrant(int q)
{
    SimulationCore::instance()->setQuadrant(q);
}

//! Get Coordinate system quadrant (1-4) that we're currently working in
/*! \sa crusde_set_quadrant()*/
extern "C" int crusde_get_quadrant()
{
    return SimulationCore::instance()->getQuadrant();
}

//! Returns the name of the result file as defined in the XML model file
extern "C" const char* crusde_get_out_file()
{
    return SimulationCore::instance()->outFile();
}


//! Gracefully terminates CrusDe, i.e. shuts down the whole system before exiting with ExitCode 'code'
/*! \sa ExitCode */
extern "C" void crusde_exit(ExitCode code)
{
    if(SimulationCore::instance() != NULL)
	    SimulationCore::instance()->terminate();

    exit(code);
}
/*
extern "C" int crusde_load_history_exists()
{
    return 0; //SimulationCore::instance()->loadHistoryPlugin()->isLoaded() ? 1 : 0;
}
*/

//! return current model time
extern "C" int crusde_model_time()
{
    return SimulationCore::instance()->modelTime();
}

//! returns total number of time steps
extern "C" int crusde_get_timesteps()
{
    return SimulationCore::instance()->getTimesteps();
}

//! return current model step, i.e. number of iterations done (temporally)
extern "C" int crusde_model_step()
{
    return SimulationCore::instance()->modelStep();
}

//! Time increment with each model step
extern "C" int crusde_stepsize()
{
    return SimulationCore::instance()->stepSize();
}

//! Index of X-displacement value in output array
extern "C" int crusde_get_x_index()
{
    return SimulationCore::instance()->xIndex();
}

//! Index of y-displacement value in output array
extern "C" int crusde_get_y_index()
{
    return SimulationCore::instance()->yIndex();
}

//! Index of Z-displacement value in output array
extern "C" int crusde_get_z_index()
{
    return SimulationCore::instance()->zIndex();
}

//! Returns the current load function id such that the load plugins can set the parameter pointers to the respective array index
/*! \sa crusde_set_current_load_component(), crusde_get_number_of_load_components() */
extern "C" int crusde_get_current_load_component()
{
	return SimulationCore::instance()->getLoadFunctionComponent();
}

//! Update currend load component id
/*! \sa crusde_get_current_load_component(), crusde_get_number_of_load_components() */
extern "C" void crusde_set_current_load_component(int id)
{
	SimulationCore::instance()->setLoadFunctionComponent(id);
}

//! Returns the total number of load functions, i.e. the maximum load component ID
/*! \sa crusde_get_current_load_component(), crusde_get_current_load_component() */
extern "C" int crusde_get_number_of_load_components()
{
	return SimulationCore::instance()->getNumberOfLoadComponents();
}

//! Has a crustal decay function been defined?
extern "C" boolean crusde_crustal_decay_given()
{
    return SimulationCore::instance()->loadFunction()->crustalDecayGiven();
}

//! Has a load history function been defined?
extern "C" boolean crusde_load_history_given()
{
    return SimulationCore::instance()->loadFunction()->loadHistoryGiven();
}

//! A Green's function plugin can request the run time function of another Green's function plug-in to reuse its functionality
/*!
	This function implements the concept of plug-in reuse. By allowing access to the run-function of a plugin of the same 
	category, a new plugin can build on existing functionality. Instead of having to reimplement fundamental 
	behavior, CrusDe allows to request an executing function and loads the respective plugin in the background. 
	The reusing plugin does not have to worry about having to remember all the correct parameters for the 
	respective plug-in. CrusDe assures that the input file is being checked for those values and that the 
	values make it into the plugin registry and to the plugin.

	\param 	plugin					Name of the Green's function to be re-used as defined in Plugin Manager, e.g. "elastic halfspace (pinel)"
	\return green_exec_function		Function pointer to the 'run()' function of a Green's function, i.e. get_green_at ...
	\sa crusde_request_load_plugin(), crusde_request_kernel_plugin(), 
		crusde_request_postprocessor_plugin(), crusde_request_loadhistory_plugin(),
		crusde_request_crustaldecay_plugin(), PluginCategory
 */
extern "C" green_exec_function crusde_request_green_plugin(char* plugin) 
{
	try
	{
		return SimulationCore::instance()->addGreenPlugin( string(plugin) );
	}
	catch( runtime_error e)
	{
		SimulationCore::instance()->abort(e.what());
	}
	
	return NULL;
}

//! A Load function plugin can request the run time function of another Load function plug-in to reuse its functionality
/*!
	\param 	plugin					Name of the Load function to be re-used as defined in Plugin Manager, e.g. "irregular load"
	\return load_exec_function		Function pointer to the 'run()' function of a Load function, i.e. get_load_at ...
	\sa crusde_request_green_plugin(), crusde_request_kernel_plugin(), 
		crusde_request_postprocessor_plugin(), crusde_request_loadhistory_plugin(),
		crusde_request_crustaldecay_plugin(), PluginCategory
 */
extern "C" load_exec_function crusde_request_load_plugin(char* plugin) 
{
	try
	{
		return SimulationCore::instance()->addLoadPlugin( string(plugin) );
	}
	catch( runtime_error e)
	{
		SimulationCore::instance()->abort(e.what());
	}
	
	return NULL;

}

//! An Operator/Kernel plugin can request the run time function of another operator/kernel plug-in to reuse its functionality
/*!
	\param 	plugin					Name of the operator to be re-used as defined in Plugin Manager, e.g. "fast 2d convolution"
	\return run_function			Function pointer to the 'run()' function of an operator.
	\sa crusde_request_green_plugin(), crusde_request_load_plugin(), 
		crusde_request_postprocessor_plugin(), crusde_request_loadhistory_plugin(),
		crusde_request_crustaldecay_plugin(), PluginCategory
 */
extern "C" run_function crusde_request_kernel_plugin(char* plugin)
{
	try
	{
		return SimulationCore::instance()->addKernelPlugin( string(plugin) );
	}
	catch( runtime_error e)
	{
		SimulationCore::instance()->abort(e.what());
	}
	
	return NULL;
	
}

//! A Postprocessor plugin can request the run time function of another Postprocessor plug-in to reuse its functionality
/*!
	\param 	plugin					Name of the postprocessor to be re-used as defined in Plugin Manager
	\return run_function			Function pointer to the 'run()' function of a Postprocessor function.
	\sa crusde_request_green_plugin(), crusde_request_load_plugin(), 
		crusde_request_kernel_plugin(), crusde_request_loadhistory_plugin(),
		crusde_request_crustaldecay_plugin(), PluginCategory
 */
extern "C" run_function crusde_request_postprocessor_plugin(char* plugin)
{
	try
	{
		return SimulationCore::instance()->addPostprocessorPlugin( string(plugin) );
	}
	catch( runtime_error e)
	{
		SimulationCore::instance()->abort(e.what());
	}
	
	return NULL;
	
}

//! A Load history function plugin can request the run time function of another Load history function plug-in to reuse its functionality
/*!
	\param 	plugin						Name of the Load history function to be re-used as defined in Plugin Manager, e.g. "sinusoidal"
	\return loadhistory_exec_function	Function pointer to the 'run()' function of a Load history function, i.e. get_loadhistory_at ...
	\sa crusde_request_green_plugin(), crusde_request_kernel_plugin(), 
		crusde_request_postprocessor_plugin(), crusde_request_load_plugin(),
		crusde_request_crustaldecay_plugin(), PluginCategory
 */
extern "C" loadhistory_exec_function crusde_request_loadhistory_plugin(char* plugin) 
{
	try
	{
		return SimulationCore::instance()->addLoadHistoryPlugin( string(plugin) );
	}
	catch( runtime_error e)
	{
		SimulationCore::instance()->abort(e.what());
	}
	
	return NULL;

}

//! A Crustal decay function plugin can request the run time function of another Crustal decay function plug-in to reuse its functionality
/*!
	\param 	plugin							Name of the Crustal Decay function to be re-used as defined in Plugin Manager, e.g. "exponential"
	\return crustaldecay_exec_function		Funtion pointer to the 'run()' function of a Crustal decay function, i.e. get_crustal_decay_at ...
	\sa crusde_request_green_plugin(), crusde_request_kernel_plugin(), 
		crusde_request_postprocessor_plugin(), crusde_request_loadhistory_plugin(),
		crusde_request_load_plugin(), PluginCategory
 */
extern "C" crustaldecay_exec_function crusde_request_crustaldecay_plugin(char* plugin) 
{
	try
	{
		return SimulationCore::instance()->addCrustalDecayPlugin( string(plugin) );
	}
	catch( runtime_error e)
	{
		SimulationCore::instance()->abort(e.what());
	}
	
	return NULL;

}
