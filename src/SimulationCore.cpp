/***************************************************************************
 * File:        ./SimulationCore.cpp
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

#include	"SimulationCore.h"
#include	"LoadHistoryPlugin.h"
#include	"DataOutPlugin.h"
#include	"ExperimentManager.h"
#include	"typedefs.h"
#include	"Plugin.h"
#include	"GreensFunction.h"
#include	"LoadPlugin.h"
#include	"InputHandler.h"
#include	"PluginManager.h"
#include	"LoadFunction.h"
#include	"ParamWrapper.h"

#include 	<iostream>

#include 	<stdarg.h>
#include 	<math.h>

//! public struct keeping the command line arguments
s_parameters SimulationCore::s_params = {};

//! static Singleton initialisieren
SimulationCore* SimulationCore::pSimulationCore=0;

SimulationCore::SimulationCore(int argc, char** argv) throw (SeriousException) :
	com_port(NULL), exp_man(NULL), plugin_man(NULL), 
	pl_kernel(NULL), pl_out(NULL), greens_function(NULL), 
        load_function(NULL), load_function_component(0),
	x_west(0), x_east(0), y_south(0), y_north(0), 
	gridsize(0), modelstep(0), modeltime(0), num_timesteps(1), num_timeincrement(1),
	stepsize(1), quadrant(1), dimensions(0),
	x_index(-1), y_index(-1), z_index(-1),
	model_data(NULL),
	pl_iter(),
	requested_plugin_list(),
	pl_list_postprocess(),
	root_dir("/"),
	operator_space_x(0), operator_space_y(0), operator_space_set(false),
	num_load_components(0),
	quiet(false)
{
	if(getenv("CRUSDE_HOME") == NULL)
	    	throw (SeriousException ( string("Error: Environment variable CRUSDE_HOME is not defined!") ) );

	root_dir = string(getenv("CRUSDE_HOME"));

	/*create integral parts*/
	com_port        = new InputHandler(argc, argv);
	exp_man         = new ExperimentManager(
				string(root_dir).append(DIR_SEP).append(EXPERIMENT_DIR).append(DIR_SEP).append(EXPERIMENT_DB).c_str()
			      );
	plugin_man      = new PluginManager(
				string(root_dir).append(DIR_SEP).append(PLUGIN_DIR).append(DIR_SEP).append(PLUGIN_DB).c_str()
			      );

	/*create plugins*/
	greens_function = new GreensFunction("greens function");
	load_function   = new LoadFunction("load function"); //holds load, load history, crustal decay
	pl_out          = new DataOutPlugin("output");
	pl_kernel       = new Plugin("modeling kernel");
}

SimulationCore::~SimulationCore()
{
	delete pl_kernel;
	delete pl_out;
	delete load_function;
	delete greens_function;
	delete com_port;
}

void SimulationCore::init() //throw INIT_EXCEPTION
{
	
    // ------------------
    // build DOM from XML input, or process command line args
    // ------------------

	com_port->init();	
	exp_man->init();
	plugin_man->init();
    
    // ------------------
    // set setup variables ... important to set these BEFORE plugins get initialized!
    // ------------------

//	crusde_debug("%s, line: %d, got north=%f, south=%f, east=%f, west=%f", __FILE__, __LINE__, com_port->getRegion("north"), com_port->getRegion("south"), com_port->getRegion("east"), com_port->getRegion("west"));


	//the region of interest
	x_west  =  com_port->getRegion("west");
	x_east  =  com_port->getRegion("east");
	y_south =  com_port->getRegion("south");
	y_north =  com_port->getRegion("north");
	
	//gridsize and time parameters
	gridsize          = com_port->getGridSize();
	num_timesteps     = com_port->getTimeSteps();	
	num_timeincrement = com_port->getTimeIncrement();	


    // ------------------
    // Have plugins load their libaries
    // ------------------	
	try
	{
	    // load everything that belongs to the greens function (all jobs and respective checking)
		greens_function->load( com_port->getGreenJobMap() );
	    // load everything that belongs to the load function (load [, load history, crustal decay function])
		load_function->load(com_port->getLoadFunctionList());
		// the rest ...
		pl_kernel->load( com_port->getKernelPlugin() );
		pl_out->load( com_port->getOutputPlugin() );
	}
	catch(runtime_error e)
	{
		abort(e.what());
	}

    // ------------------
    // create all postprocessors
    // ------------------
	
	list<string> names = com_port->getPostProcessorNameList();
	list<string>::iterator post_iter = names.begin();

	while(post_iter != names.end()){
		Plugin *pl = new Plugin( (*post_iter).c_str() );
		//try loading the plugin	
		try
		{
			pl->load( plugin_man->getFilename("postprocess", (*post_iter)) );
		}
		catch(runtime_error e)
		{
			abort(e.what());
		}
		
		pl_list_postprocess.push_back( pl );
		++post_iter;
	}    

    
    // ------------------
    // Have plugins register their parameters
    // ------------------

	greens_function->registerParameter();
	load_function->registerParameter();
	pl_kernel->registerParameter();
	pl_out->registerParameter();
	//postprocessors
	pl_iter = pl_list_postprocess.begin();
	while(pl_iter != pl_list_postprocess.end()){
		(*pl_iter)->registerParameter();
		++pl_iter;
	}

    // ------------------
    // Have plugins allocate their output memory
    // ------------------

	greens_function->registerOutputFields();
	pl_iter = pl_list_postprocess.begin();
	while(pl_iter != pl_list_postprocess.end()){
		(*pl_iter)->registerOutputFields();
		++pl_iter;
	}

    // ------------------
    // Have plugins get access to other plugins
    // ------------------

   	greens_function->requestPlugins();
	load_function->requestPlugins();
   	pl_kernel->requestPlugins();
   	pl_out->requestPlugins();
	//postprocessors
	pl_iter = pl_list_postprocess.begin();
	while(pl_iter != pl_list_postprocess.end()){
		(*pl_iter)->requestPlugins();
		++pl_iter;
	}

	//until length does not change, have requested Plug-ins request all their plug-ins
	unsigned int requested_size(0);
crusde_info(">>>>>>>>>>> ITERATING THROUGH requested_plugin_list .... ");
	do{
		requested_size = requested_plugin_list.size();

		pl_iter = requested_plugin_list.begin();
		while(pl_iter != requested_plugin_list.end()){
crusde_info(">>>>>>>>>>> %s", (*pl_iter)->getName().c_str());

			(*pl_iter)->requestPlugins();
			++pl_iter;
		}

crusde_info(">>>>>>>>>>> requested_size=%d, requested_plugin_list.size()=%d", requested_size, requested_plugin_list.size() );

	}while(requested_size < requested_plugin_list.size());

	//call all the startup functions of the requested plug-ins
	pl_iter = requested_plugin_list.begin();
	while(pl_iter != requested_plugin_list.end()){
		(*pl_iter)->registerParameter();
		(*pl_iter)->registerOutputFields();
		++pl_iter;
	}


    // ------------------
    // init Parameters that are now known to the registry
    // ------------------
	
	com_port->initParamsFromDOM();

    // ------------------
    // now the registered values are set and we can truly initialize the plugins 
    // ------------------
	
	//CRUCIAL: init of kernel before green's functions is a guarantee!!! 
    //since the kernel sets the DFT matrix size during init! DON'T CHANGE!
	pl_kernel->init();

	//NOW the rest
	greens_function->init();
	load_function->init();
	pl_out->init();

	//postprocessors
	pl_iter = pl_list_postprocess.begin();
	while(pl_iter != pl_list_postprocess.end()){
		(*pl_iter)->init();
		++pl_iter;
	}
	
	//requested plugins
	pl_iter = requested_plugin_list.begin();
	while(pl_iter != requested_plugin_list.end()){
		(*pl_iter)->init();
		++pl_iter;
	}

    if( !operatorSpaceIsSet() )
	{
		printf("%s, %d: WARNING OPERATOR SPACE NOT SET BY KERNEL\n", __FILE__, __LINE__);
    }


}

void SimulationCore::terminate() //throw KILL_EXCEPTION
{
    // ------------------
    // Have plugins release their resources
    // ------------------
	pl_iter = requested_plugin_list.begin();
	while(pl_iter != requested_plugin_list.end()){
		(*pl_iter)->release();
		++pl_iter;
	}

	greens_function->release();
	load_function->release();
	pl_kernel->release();
	pl_out->release();

	//postprocessors
	pl_iter = pl_list_postprocess.begin();
	while(pl_iter != pl_list_postprocess.end()){
		(*pl_iter)->release();
		++pl_iter;
	}
    // ------------------
    // Have plugins unload their libraries
    // ------------------
	try
	{
		greens_function->unload();
		load_function->unload();
		pl_kernel->unload();
		pl_out->unload();
		//postprocessors
		pl_iter = pl_list_postprocess.begin();
		while(pl_iter != pl_list_postprocess.end()){
			(*pl_iter)->unload();
			++pl_iter;
		}

		pl_iter = requested_plugin_list.begin();
		while(pl_iter != requested_plugin_list.end()){
			(*pl_iter)->unload();
			++pl_iter;
		}
	}
	catch(runtime_error e)
	{
		abort(e.what());
	}
    // ------------------
    // destroy singleton object
    // ------------------

	delete pSimulationCore;
}

/*! go!go!go!
 */
void SimulationCore::exec() //throw EXEC_EXCEPTION
{

     crusde_info("-------------------------: ");

/* PSEUDOCODE ...
   list *jobs = com_port->getJobs();
   if(jobs->length() == 1)
   {
      nothing in particular must be done, (maybe print warning that only one job is defined)
   }
   if(jobs->length() > 1)
   {
      setCurrentJob(job
   }
*/
     greens_function->setFirstJob();

     do{
          crusde_info("Working on job: %s", greens_function->job().c_str());

          modelstep=0;
          modeltime=0;

          while(modelstep < num_timesteps)
          {
               crusde_info("STEP: %d, TIME: %d", modelstep, modeltime);
               crusde_info("operator starts ... ");
               //convolve load with response function 
               pl_kernel->run();

               //run postprocessors ... only on last job. 
               if(greens_function->isLastJob())
               {
                    crusde_info("postprocessors start ... "); 
                    pl_iter = pl_list_postprocess.begin();
                    while(pl_iter != pl_list_postprocess.end())
                    {
                         (*pl_iter)->run();
                         ++pl_iter;
                    }

                    crusde_info("result handler starts ... ");
                    //run the output function
                    pl_out->run();

               }

               crusde_info("-------------------------");
               //another timestep??
               modeltime += num_timeincrement;
               ++modelstep;
          }
     } while(greens_function->nextJob() == true);

     crusde_info("Starting the experiment manager ... ");
     //if we're getting this far the experiment was a success ... memorize it!
     exp_man->addEntry(outFile(), com_port->getExperiment());	
}

/**
 * 
 * @param cat Plugin category
 * @return list of string IDs of registered Parameters
 */
list<string> SimulationCore::getRegisteredParameters(PluginCategory category)
{
	list<string> registeredParams;
	multimap<string, ParamWrapper* > p_map;
	
	if(category == LOAD_PLUGIN){
		p_map = SimulationCore::s_params.load[getLoadFunctionComponent()];
	} 
	else if(category == KERNEL_PLUGIN){
		p_map = SimulationCore::s_params.kernel;
	} 
	else if(category == GREEN_PLUGIN){
		p_map = SimulationCore::s_params.green;
	} 
	else if(category == DATAOUT_PLUGIN){
		p_map = SimulationCore::s_params.datahandler;
	} 
	else if(category == POSTPROCESS_PLUGIN){
		p_map = SimulationCore::s_params.postprocessor;
	}
	else if(category == LOADHISTORY_PLUGIN){
		p_map = SimulationCore::s_params.loadhistory[getLoadFunctionComponent()];
	}
	else if(category == CRUSTALDECAY_PLUGIN){
		p_map = SimulationCore::s_params.crustaldecay[getLoadFunctionComponent()];
	} else {
		crusde_warning("In SimulationCore::getRegisteredParameters: unknown category: %d", category); 
	}

  	multimap<string , ParamWrapper* >::iterator iter = p_map.begin();
	
	while( iter != p_map.end()) 
	{
		registeredParams.push_back(iter->first);
		++iter;
 	}
	
	registeredParams.sort();
	registeredParams.unique();	//deletes consecutive duplicate elements, needs sort before,
			
	return registeredParams;	
}
/**
 * deletes all parameters of a specific category
 *
 * @param cat plugin category
 */
void SimulationCore::deleteRegistrees()
{
	
	SimulationCore::s_params.load.clear();
	SimulationCore::s_params.kernel.clear();
	SimulationCore::s_params.green.clear();
	SimulationCore::s_params.datahandler.clear();
	SimulationCore::s_params.postprocessor.clear();
	SimulationCore::s_params.loadhistory.clear();
	SimulationCore::s_params.crustaldecay.clear();
}


list<string> SimulationCore::getRequestedNames()
{
	list<string> requestedNames;
	
	pl_iter = requested_plugin_list.begin();
	while(pl_iter != requested_plugin_list.end()){
		requestedNames.push_back((*pl_iter)->getName());
		++pl_iter;
	}
	
	return requestedNames;	
}

void SimulationCore::deleteRequests()
{
	pl_iter = requested_plugin_list.begin();
	while(pl_iter != requested_plugin_list.end()){
		(*pl_iter)->unload();
		delete(*pl_iter);
		++pl_iter;
	}
	
	requested_plugin_list.clear();
}


/*
 * a green plugin was requested for usage by another green plugin.
 * thus, it needs to be created, loaded, has it's parameters registerd.
 * it is added to the general plugin storage and this way kept in memory
 * and accessible to call init, unload, and other plugin functions at it.
 * it returns a pointer to the green's functions execution function.
 */
green_exec_function SimulationCore::addGreenPlugin(string plugin) throw (FileNotFound, runtime_error)
{
     crusde_info("ADDING GREEN PLUGIN TO requested_plugin_list");
		
     try{
          GreenPlugin *pl = new GreenPlugin(plugin);
          string path(com_port->getGreenPlugin(plugin));
 	
          string error_string("Could not find plugin ");
	
          if(path.empty())
          {
               error_string.append("'").append(plugin).append("'\n");
               error_string.append("Not installed?\nMisspelled?\nCheck Database!\n");
               throw ( FileNotFound(error_string) );
          }
		
          pl->load( path );
          pl->registerParameter();
          pl->registerOutputFields();

          requested_plugin_list.push_back(pl);
		
          return pl->getArrayFunction();

     }
     catch( ... )
     {
          throw;
     }
}

/*
 * a load plugin was requested for usage by another green plugin.
 * thus, it needs to be created, loaded, have it's parameters registerd.
 * it is added to the general plugin storage and this way kept in memory
 * and accessible to call init, unload, and other plugin functions at it.
 * it returns a pointer to the load functions execution function.
 */
load_exec_function SimulationCore::addLoadPlugin(string plugin) throw (FileNotFound, runtime_error)
{
	try{
		LoadPlugin *pl = new LoadPlugin(plugin);
		string path(com_port->getLoadPlugin(plugin));
		string error_string("Could not find plugin ");
	
		if(path.empty())
		{
			error_string.append("'").append(plugin).append("'\n");
			error_string.append("Not installed?\nMisspelled?\nCheck Database!\n");
			throw ( FileNotFound(error_string) );
		}
	
		pl->load( path );
		pl->registerParameter();
		
		requested_plugin_list.push_back(pl);
		
		return pl->getValueFunction();

	}
	catch( ... ){
		throw;
	}
	
}

/*
 * a kernel plugin was requested for usage by another green plugin.
 * thus, it needs to be created, loaded, has it's parameters registerd.
 * it is added to the general plugin storage and this way kept in memory
 * and accessible to call init, unload, and other plugin functions at it.
 * it returns a pointer to the load functions execution function.
 */
run_function SimulationCore::addKernelPlugin(string plugin) throw (FileNotFound, runtime_error)
{
	try
	{
		Plugin *pl = new Plugin(plugin);
		string path(com_port->getKernelPlugin(plugin));
		string error_string("Could not find plugin ");
		
		if(path.empty())
		{
			error_string.append("'").append(plugin).append("'\n");
			error_string.append("Not installed?\nMisspelled?\nCheck Database!\n");
			throw ( FileNotFound(error_string) );
		}
			
		pl->load( path );
		pl->registerParameter();
		
		requested_plugin_list.push_back(pl);
		
		return pl->getRunFunction();
	}
	catch(...){
		throw;
	}
}

	
run_function SimulationCore::addPostprocessorPlugin(string plugin) throw (FileNotFound, runtime_error)
{
	try
	{
		Plugin *pl = new Plugin(plugin);
		string path( plugin_man->getFilename("postprocess", plugin));
	
		string error_string("Could not find plugin ");
		
		if(path.empty())
		{
			error_string.append("'").append(plugin).append("'\n");
			error_string.append("Not installed?\nMisspelled?\nCheck Database!\n");
			throw ( FileNotFound(error_string) );
		}
			
		pl->load( path );
		pl->registerParameter();
		
		requested_plugin_list.push_back(pl);
		
		return pl->getRunFunction();
	}
	catch(...){
		throw;
	}
}

loadhistory_exec_function SimulationCore::addLoadHistoryPlugin(string plugin) throw (FileNotFound, runtime_error)
{
	try{
		//create new Load History Plugin
		LoadHistoryPlugin *pl = new LoadHistoryPlugin(plugin);
		//find path to plug-in based on its name
		string path(com_port->getLoadHistoryPlugin(plugin));

		//error treatment
		string error_string("Could not find plugin ");
	
		if(path.empty())
		{
			error_string.append("'").append(plugin).append("'\n");
			error_string.append("Not installed?\nMisspelled?\nCheck Database!\n");
			throw ( FileNotFound(error_string) );
		}
	
		//load dynamic library and have it register its parameters
		pl->load( path );
		pl->registerParameter();
		//put into list of all requested plugins
		requested_plugin_list.push_back(pl);
		
		//return the execution function of the plugin.
		return pl->getValueFunction();

	}
	catch( ... ){
		throw;
	}
	
}

crustaldecay_exec_function SimulationCore::addCrustalDecayPlugin(string plugin) throw (FileNotFound, runtime_error)
{
	try{
		//create new Load History Plugin
		CrustalDecayPlugin *pl = new CrustalDecayPlugin(plugin);
		//find path to plug-in based on its name
		string path(com_port->getCrustalDecayPlugin(plugin));

		//error treatment
		string error_string("Could not find plugin ");
	
		if(path.empty())
		{
			error_string.append("'").append(plugin).append("'\n");
			error_string.append("Not installed?\nMisspelled?\nCheck Database!\n");
			throw ( FileNotFound(error_string) );
		}
	
		//load dynamic library and have it register its parameters
		pl->load( path );
		pl->registerParameter();
		//put into list of all requested plugins
		requested_plugin_list.push_back(pl);
		
		//return the execution function of the plugin.
		return pl->getValueFunction();

	}
	catch( ... ){
		throw;
	}
	
}


void SimulationCore::registerOutputField(int *output_index, FieldName field)
{
	//check which field is gonna be set and whether or not it is already claimed by someone else:
	if(field == X_FIELD){
		if(x_index < 0){
			x_index = dimensions;
		}
		else{
			(*output_index) = x_index;
			return;
		}
	}
	else
	if(field == Y_FIELD){
		if(y_index < 0){
			y_index = dimensions;
		}
		else{
			(*output_index) = y_index;
			return;
		}
	}
	else
	if(field == Z_FIELD){
		if(z_index < 0){
			z_index = dimensions;
		}
		else{
			(*output_index) = z_index;
			return;
		}
	
	}
	else
	if(field == ADD_FIELD){
		//check whether the requesting address has already been assinged an output field
		map<int*,int>::iterator iter = add_field_map.find(output_index);
  		if( iter != add_field_map.end() ) 
		{
	    		crusde_warning("Seems like address '%d' has already requested memory. Is there one plugin trying to do its job twice? We will NOT give additional memory to that address.", iter->first);
			return;
 		}
		else
		{
		//else memorize that address for later...	
 			add_field_map[output_index] = dimensions;
		}
	}
	else
	{
		abort("Trying to set Invalid field. Use X_FIELD, Y_FIELD, Z_FIELD, or ADD_FIELD only!");
	}

	//set field's position in output array
	(*output_index) = dimensions;
	
	//increase output dimension
	++dimensions;	
}

string SimulationCore::currentJob()
{
	return greens_function->job();
}
void SimulationCore::setModelData(double **result)
{ 
	model_data = result;
	pl_out->setModelData(result);
}

double** SimulationCore::getModelData()
{ 
	return model_data;
}

int SimulationCore::getTimesteps()
{
	return num_timesteps;
}

const char* SimulationCore::outFile()
{
	string name(com_port->getResultFileName());
	
	if(name.empty())
		return "experiment.nc";
	else
		return name.c_str();
}

int SimulationCore::sizeX()
{
	return static_cast<int>( ceil( static_cast<double>((x_east - x_west) / gridsize ) ) ) + 1 ;
}

int SimulationCore::sizeY()
{
	return static_cast<int>( ceil( static_cast<double>((y_north - y_south) / gridsize ) ) ) + 1 ;
}

int SimulationCore::sizeT()
{
	return num_timesteps;
}

int SimulationCore::minX()
{
	return x_west;
}

int SimulationCore::minY()
{
	return y_south;
}

int SimulationCore::gridSize()
{
	return gridsize;
}

int SimulationCore::stepSize()
{
	return stepsize;
}

int SimulationCore::modelTime()
{
	return modeltime;
}

int SimulationCore::modelStep()
{
	return modelstep;
}

int SimulationCore::xIndex()
{
	return x_index;
}

int SimulationCore::yIndex()
{
	return y_index;
}

int SimulationCore::zIndex()
{
 	return z_index;
}

int SimulationCore::getQuadrant()
{
	return quadrant;
}

void SimulationCore::setQuadrant(int q)
{
	quadrant = q;
}

int SimulationCore::getDimensions()
{

	return dimensions;
}

int SimulationCore::displacementDimensions()
{
	int displacements = 0;
	
	if(x_index >= 0)
		++displacements;
	if(y_index >= 0)
		++displacements;
	if(z_index >= 0)
		++displacements;

	return displacements;
}


int SimulationCore::getOperatorSpaceX()
{
	return operator_space_x;
}

int SimulationCore::getOperatorSpaceY()
{
	return operator_space_y;
}

void SimulationCore::setOperatorSpace(int nx, int ny)
{
	operator_space_x = ny;
	operator_space_y = ny;

	operator_space_set = true;
}

bool SimulationCore::operatorSpaceIsSet()
{
	return operator_space_set;
}

string SimulationCore::getPluginFilename(string category, string name) throw(DatabaseError)
{
	string backpack("yes, I'm emty");
	try{
	    backpack = plugin_man->getFilename(category, name);
	}
	catch(DatabaseError e){
	    if(plugin_man->inGuiMode())
		throw(e);
	    else
		abort(e.what());
	}

        return backpack;
}


PluginManager* SimulationCore::pluginManager()
{
	return plugin_man;
}


void SimulationCore::runExperimentManager()
{
	exp_man->init();
	exp_man->run();
	delete pSimulationCore;
	exit(0);
}

void SimulationCore::runPluginManager()
{
	plugin_man->init();
	plugin_man->run();
	delete pSimulationCore;
	exit(0);
}

void SimulationCore::installPlugin(string plugin)
{
	plugin_man->init();
	try{
		plugin_man->addEntry(plugin);
	}
	catch (PluginExistsException e)
	{
		crusde_warning("%s.", e.what());
	}
	catch (DatabaseError e)
	{
		crusde_warning("%s.", e.what());
	}
	catch (runtime_error e)
	{
		crusde_error("%s.", e.what());
	}

	delete pSimulationCore;
	exit(0);
}

void SimulationCore::abort(string msg)
{
	cerr<<msg<<"\nAborting..."<<endl;
	delete this;
	exit(2);
}
/**
 * stick parameter that's being registered by a plug-in into the parameter registry
 */
void SimulationCore::registerParam(ParamWrapper *param, const char* name, PluginCategory category)
{
	if(category == LOAD_PLUGIN){
		s_params.load[ SimulationCore::instance()->getLoadFunctionComponent() ].insert( pair< string, ParamWrapper* > (string(name), param) );
	}
	else if(category == LOADHISTORY_PLUGIN){
		SimulationCore::s_params.loadhistory[ SimulationCore::instance()->getLoadFunctionComponent() ].insert( pair< string, ParamWrapper* > (string(name), param) );
	} 
	else if(category == CRUSTALDECAY_PLUGIN){
		SimulationCore::s_params.crustaldecay[ SimulationCore::instance()->getLoadFunctionComponent() ].insert( pair< string, ParamWrapper* > (string(name), param) );
	} 
	else if(category == KERNEL_PLUGIN){
		SimulationCore::s_params.kernel.insert( pair< string, ParamWrapper* > (string(name), param) );
	} 
	else if(category == GREEN_PLUGIN){
		SimulationCore::s_params.green.insert( pair< string, ParamWrapper* > (string(name), param) );
	} 
	else if(category == DATAOUT_PLUGIN){
		SimulationCore::s_params.datahandler.insert( pair< string, ParamWrapper* > (string(name), param) );
	} 
	else if(category == POSTPROCESS_PLUGIN){
		SimulationCore::s_params.postprocessor.insert( pair< string, ParamWrapper* > (string(name), param) );
	} else {
		crusde_error("In SimulationCore::registerParam: unknown category: %d", category); 
	}
}

/**
 * sets value of the load component that is currently to be worked with to given id
 * keeps also track of the total number of load components (max id == number of load components)
 */
void SimulationCore::setLoadFunctionComponent(unsigned int id)
{ 
	load_function_component = id; 

	if(id>num_load_components)
	{ 
		num_load_components = id;
	}
}

/**
 * returns the id of the load component we are currently working with
 */
unsigned int SimulationCore::getLoadFunctionComponent()
{ 
	return load_function_component;
}

/** 
 * returns total number of load components, id's start at zero!
 */
unsigned int SimulationCore::getNumberOfLoadComponents()
{
	return num_load_components+1;
}

/** 
 * sets a flag for quiet, i.e. no output to commandline, runs
 */
void SimulationCore::setQuiet(bool flag)
{
	quiet = flag;
}

/** 
 * returns a flag for quiet, i.e. no output to commandline, runs
 */
bool SimulationCore::isQuiet()
{
	return quiet;
}
