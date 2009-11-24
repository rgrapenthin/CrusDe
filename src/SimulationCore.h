/***************************************************************************
 * File:        ./SimulationCore.h
 * Author:      Ronni Grapenthin, GEOPHYSICAL INSTITUTE, UAF
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

/** 
 *  @defgroup core Core Application
 *  @ingroup core 
 **/

/*@{*/
/** \file SimulationCore.h
 * 
 *	Class definition for the SimulationCore that sets up and controls the experiment run.
 */
/*@}*/

#ifndef simulationCore_h	
#define simulationCore_h

#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <cassert>
#include <string>
#include "constants.h"
#include "typedefs.h"
#include "crusde_api.h"
#include "exceptions.h"

using namespace std;

class LoadHistoryPlugin;
class InputHandler;
class Plugin;
class DataOutPlugin;
class LoadPlugin;
class GreensFunction;
class ExperimentManager;
class PluginManager;
class LoadFunction;
class ParamWrapper;
class ModelRegion;

//! The parameter registry for plugins.
/*! 
 *  This struct holds the references to variables that are defined inside of plug-ins,
 *  but are initialized by user parameters (either command-line or XML). The 
 *  register_parameter() function ...
 */
struct s_parameters {				
	map< unsigned int, multimap< string, ParamWrapper* > > load;
	map< unsigned int, multimap< string, ParamWrapper* > > loadhistory;
	map< unsigned int, multimap< string, ParamWrapper* > > crustaldecay;
	multimap<string, ParamWrapper* > green;	/*!< name and pointer to value of green */
	multimap<string, ParamWrapper* > kernel;	/*!< name and pointer to value of kernel*/
	multimap<string, ParamWrapper* > datahandler;/*!< name and pointer to value of postprocessor*/
	multimap<string, ParamWrapper* > postprocessor;/*!< name and pointer to value of postprocessor*/
};

//! Singleton class SimulationCore
/*!
 * The SimulationCore is implemented following the Singleton Design Pattern, wich
 * makes sure one and only one instance of the class SimulationCore will exist during an application run. 
 * This makes sense for various reasons. The most important is certainly that a SimulationCore is the
 * application. 
 *
 * The main() function will get a pointer to this class by calling the instance(int, char**) function. 
 * Its main function is to build, init, run and terminate
 * all parts of the application at the right time. The SimulationCore also keeps the simulation clock. 
 * 
 */
class SimulationCore /*: 	public PluginAPI, 
			public PluginSimulationCore,*/ {
	
	InputHandler      *com_port;        /**< hard-wired input handler, no plugin*/
	ModelRegion       *model_region;    /**< manager of model region; does conversions degree to great circle dists, holds gridsize */
	ExperimentManager *exp_man;         /**< hard-wired experiment Manager, no plugin*/
	PluginManager     *plugin_man;      /**< hard-wired plugin Manager, no plugin ;)*/

	Plugin            *pl_kernel;       /**< Plug-in instance taking care on the simulation kernel */
	DataOutPlugin     *pl_out;          /**< Plug-in instance handling data output*/
	GreensFunction    *greens_function; /**< Green's Function plug in, one factor for convolution */
	LoadFunction      *load_function;   /**< Load function, holds all loads and histories*/

	unsigned int load_function_component;
		
	/*experiment setup data that needs to be available for all participants*/
	int x_west;
	int x_east;
	int y_south;
	int y_north;
	int gridsize;
	
	int modelstep;         /**< the model step. an integer that is increased with each timestep */
	int modeltime;         /**< the model time. an integer that is increased with each timestep */
	int num_timesteps;     /**< maximum number of timesteps we want to simulate */
    	int num_timeincrement; /**< ... using this stepsize  */
	int stepsize;                        /**< time increment with each model step */
	int quadrant;				
	int dimensions;		
	
	int x_index;
	int y_index;
	int z_index;
	
	map<int*, int> add_field_map;
	
	double **model_data;
	
	list<Plugin*>::iterator pl_iter;
	
	list<Plugin*> requested_plugin_list; /**< buffer for all the plugins that will be loaded by other plugins.
						       they only have to be kept in memory so the function pointer 
						       stay valid. but noone else will ever need to access them 
						       individually, only as a group to perform Plugin functions*/
	
	list<Plugin*> pl_list_postprocess;   /**< buffer for all the postprocessor plugins that might be loaded*/

	string	root_dir;

	int operator_space_x;
	int operator_space_y;
	bool operator_space_set;

	unsigned int num_load_components;

	bool quiet;
							   
	static SimulationCore 	*pSimulationCore;	/**< pointer to the only instance of SimulationCore */
	
	//!hidden constructor, the SimulationCore shall be a singleton
	SimulationCore(int argc, char** argv) throw(SeriousException);
	//!hidden copy constructor - we do not want to accidentially copy objects
	SimulationCore(const SimulationCore& x); 
	//!hidden assignment operator - we do not want to accidentially copy objects
	SimulationCore const &operator=(SimulationCore const &rvalue);

/************************/		
/**** PUBLIC SECTION ****/
/************************/		
    public:
    	static s_parameters 	s_params;			/* */

	//!Destructor	 
	~SimulationCore();
		
	void exec();
	void init();
	void terminate();
	
	void abort(string msg);
		
	/**
	 * SimulationCore::instance
	 * initializes singleton object in case it does not exist. returns the only existing 
	 * instance
	 */
	static SimulationCore* instance(int argc, char** argv) throw(){
		if(!pSimulationCore){ 
			try{
				pSimulationCore = new SimulationCore(argc, argv); 
			}
			catch(SeriousException e)
			{
				crusde_error("%s \nAborting...", e.what()); 
			}
			catch(...)
			{
				crusde_error("An unrecognized error occured while trying to build the SimulationCore.\nAborting..."); 
			}
		}
		
		return pSimulationCore;
	}

	/**
	 * SimulationCore::instance
	 * returns the only existing instance
	 */
	static SimulationCore* instance() throw() {
		if(pSimulationCore)
			return pSimulationCore;
		else{
			//must not call crusde_error, otherwise infinite loop
			cerr << "Call instance(argc, argv) first!" << endl;
			return NULL; 
		}
	}
	
	GreensFunction*	greensFunction(){ return greens_function;}
	LoadFunction*	loadFunction(){ return load_function;}
	DataOutPlugin*	dataOutPlugin(){ return pl_out;}

	/* we want to allow for multiple loads with independent parameters and all that, so they need to be identifiable*/
	unsigned int getLoadFunctionComponent();
	void setLoadFunctionComponent(unsigned int i);

	unsigned int getNumberOfLoadComponents();

	void registerParam(ParamWrapper *param, const char* name, PluginCategory category);
	
	const char* outFile();

	int sizeX();
	int sizeY();
	int sizeT();
	int gridSize();
	int minX();
	int minY();
	int modelTime();
	int modelStep();
	int stepSize();
	int xIndex();
	int yIndex();
	int zIndex();

	int getDimensions();
	int getTimesteps();
	int displacementDimensions();
//	double constrainLoadHeight(double, int);
	
	int getQuadrant();
	void setQuadrant(int);
	void setModelData(double**);
	double** getModelData();
	void runExperimentManager();
	void runPluginManager();
        void installPlugin(string);
	PluginManager* pluginManager();
	list<string> getRegisteredParameters(PluginCategory cat);
	void deleteRegistrees();
	list<string> getRequestedNames();
	void deleteRequests();

    int getOperatorSpaceX();
    int getOperatorSpaceY();
    void setOperatorSpace(int, int);
    bool operatorSpaceIsSet();
	
	string getPluginFilename(string, string) throw (DatabaseError);
	void registerOutputField(int *output_index, FieldName field);

	string currentJob();

	void setQuiet(bool);
	bool isQuiet();

	//these functions enable re-use of plug-ins within the same category
	green_exec_function          addGreenPlugin(string plugin) throw (FileNotFound, runtime_error);
	load_exec_function           addLoadPlugin(string plugin) throw (FileNotFound, runtime_error);
	run_function                 addKernelPlugin(string plugin) throw (FileNotFound, runtime_error);
	run_function                 addPostprocessorPlugin(string plugin) throw (FileNotFound, runtime_error);
	loadhistory_exec_function    addLoadHistoryPlugin(string plugin) throw (FileNotFound, runtime_error);
	crustaldecay_exec_function   addCrustalDecayPlugin(string plugin) throw (FileNotFound, runtime_error);

};

#endif // simulationCore_h
