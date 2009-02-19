/***************************************************************************
 * File:        ./Plugin.cpp
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

#include "exceptions.h"
#include "Plugin.h"
#include "SimulationCore.h"
#include <stdlib.h>

Plugin::Plugin(const string _name):
	func_void(NULL),
	func_get_string(NULL),
	func_get_name(NULL),
	func_get_version(NULL),
	func_get_authors(NULL),
	func_run(NULL),
	func_register_parameter(NULL),
	func_request_plugins(NULL),
	func_init(NULL),
	func_release(NULL),
	rc(-1),
	name(_name),
	path(),
	category(),
	is_released(false),
	is_initialized(false),
	is_registered(false),
	is_loaded(false)	
{
  Debug("%s, line: %d, Plugin built: %s", __FILE__, __LINE__, name.c_str());
}

Plugin::~Plugin()
{
  Debug("%s, line: %d, Plugin destroyed: %s ", __FILE__, __LINE__, name.c_str());
}

/*								*/
/* Open the shared library handle				*/
/*								*/
void Plugin::load(string new_path) throw (FileNotFound,  LibHandleError)
{
  
  if(new_path.empty())
  {
  	throw (FileNotFound ("loading of plugin failed. filename is empty.") );
  }
  
  path.assign( new_path );
  
  Debug("%s, line: %d, Plugin %s load: %s ", __FILE__, __LINE__, name.c_str(), path.c_str());
  // clear error flag 
  dlerror();
  LibHandle = dlopen(path.c_str(), RTLD_LAZY);
  if( LibHandle  == NULL ) {
    	throw (LibHandleError (dlerror() ) );
  }
  
  //assign function pointer ... load functions
  func_get_name 	= get_dl_stringFunc("get_name");
  func_get_version 	= get_dl_stringFunc("get_version");
  func_get_authors 	= get_dl_stringFunc("get_authors");
  func_get_description  = get_dl_stringFunc("get_description");
  func_get_category	= get_categoryFunc("get_category");
  
  func_run		= get_dl_voidFunc("run");
  func_register_parameter= get_dl_voidFunc("register_parameter");	
  func_register_output_fields= get_dl_voidFunc("register_output_fields");	
  func_request_plugins	= get_dl_voidFunc("request_plugins");	
  func_init		= get_dl_voidFunc("init");	
  func_release		= get_dl_voidFunc("clear");

  is_loaded = true;  
}

/*								*/
/* Close the shared library handle				*/
/*								*/
void Plugin::unload() throw (LibHandleError)
{
  if(!is_loaded)
  	return;

  Debug("%s, line: %d, Plugin %s unload. ", __FILE__, __LINE__, name.c_str());

  /* clear memory */
  release();
  
  /* clear error flag */
  dlerror();

  rc = dlclose(LibHandle);

  if( rc ){
    	throw (LibHandleError (dlerror() ) );
  }
}

category_t Plugin::get_categoryFunc(string func_name) throw (LibHandleError)
{
//    Debug("%s, line: %d, Plugin::dl_voidFunc %s: %s ", __FILE__, __LINE__, func_name.c_str(), name.c_str());
    /* clear error flag */
    dlerror();
    /* get init address of init function in function lib 		*/ 
    func_get_category = (category_t) dlsym( LibHandle, func_name.c_str());
    /* if dlsym returns NULL, print error message and leave	*/
    if( func_get_category == NULL ){
    	throw (LibHandleError (dlerror() ) );
    }

    /* return function pointer*/
    return func_get_category;
}

void_t Plugin::get_dl_voidFunc(string func_name) throw (LibHandleError)
{
//    Debug("%s, line: %d, Plugin::dl_voidFunc %s: %s ", __FILE__, __LINE__, func_name.c_str(), name.c_str());
    /* clear error flag */
    dlerror();
    /* get init address of init function in function lib 		*/ 
    func_void = (void_t) dlsym( LibHandle, func_name.c_str());
    /* if dlsym returns NULL, print error message and leave	*/
    if( func_void == NULL ){
    	throw (LibHandleError (dlerror() ) );
    }

    /* return function pointer*/
    return func_void;

}

char_t Plugin::get_dl_stringFunc(string func_name) throw (LibHandleError)
{
//    Debug("%s, line: %d, Plugin::dl_stringFunc %s: %s ", __FILE__, __LINE__, func_name.c_str(), name.c_str());
    /* clear error flag */
    dlerror();
    /* get init address of init function in function lib 		*/ 
    func_get_string = (char_t) dlsym( LibHandle, func_name.c_str() );
    /* if dlsym returns NULL, print error message and leave	*/
    if( func_get_string == NULL ){
    	throw (LibHandleError (dlerror() ) );
    }

    /* run function and return a string*/
    return func_get_string;
}

/*								*/
/* register parameters of the shared library handle		*/
/*								*/
void Plugin::registerParameter()
{ 
   if(is_loaded)
   {
	cout << "  Plugin::registerParameter() for: "<<name<<endl;
	if(!is_registered){
		func_register_parameter();    
		/* fly the flag */
		is_registered = true;
	}
	else
	{
		cerr << "Warning: Plugin::registerParameter() called at least twice, skipping repeated registration!"<< endl;
	}
   }
}

/*								*/
/* alloc space in output			 		*/
/*								*/
void Plugin::registerOutputFields()
{ 
   if(is_loaded)
   {
	assert(is_registered);
   	func_register_output_fields();
   }
}

/*								      */
/* give the shared library handle a chance to distribute its work load*/
/*								      */
void Plugin::requestPlugins()
{ 
   if(is_loaded)
   {
	assert(is_registered);
   	func_request_plugins();    
   }
}


/*								*/
/* Init the shared library handle				*/
/*								*/
void Plugin::init()
{ 
   if(is_loaded)
   {
	assert(is_registered);
	
	if(!is_initialized){
		func_init();
		/* fly the flag */
		is_initialized = true;
	}
	else
	{
			cerr << "Warning: Plugin::init() called at least twice, skipping repeated initialization!"<< endl;
	}
   }	
}


/*								*/
/* Exec the shared library handle				*/
/*								*/
void Plugin::run()
{ 
   if(is_loaded)
   {
	assert(is_initialized);
   	func_run();
   }
}

/*								*/
/* Release the shared library handle				*/
/*								*/
void Plugin::release()
{ 
   if(is_loaded)
   {
	if(!is_released)
	{
		is_released = true;
		func_release();
	}
   }
}

string Plugin::getName()
{ 
	try{
		return string(func_get_name());
	}
	catch(...){
		throw;
	}
		
}

string Plugin::getVersion()
{ 
	try{
		return string(func_get_version());
	}
	catch(...){
		throw;
	}
}

string Plugin::getAuthors()
{ 
	try{
		return string(func_get_authors());
	}
	catch(...){
		throw;
	}
}

string Plugin::getDescription()
{
	try{
		return string(func_get_description());
	}
	catch(...){
		throw;
	}
}

string Plugin::getCategory()
{ 
	PluginCategory cat = func_get_category();
  
	if(cat == LOAD_PLUGIN){
  		return string("load");
  	}else
  	if(cat == GREEN_PLUGIN){
  		return string("green");
  	}else
  	if(cat == KERNEL_PLUGIN){
  		return string("operator");
  	}else
  	if(cat == DATAOUT_PLUGIN){
  		return string("data_handler");
	}else
  	if(cat == POSTPROCESS_PLUGIN){
  		return string("postprocess");
	}else
  	if(cat == LOADHISTORY_PLUGIN){
  		return string("load_history");
	}else
  	if(cat == CRUSTALDECAY_PLUGIN){
  		return string("crustal_decay");
  	}else
		cerr<<"category not found."<<endl;
    
    return string();
}

PluginCategory Plugin::getCategoryID()
{ 
	return func_get_category();
}

void Plugin::setCategory(string new_cat)
{ 
  category=new_cat;
}

/*									*/
/* test shared library handle for existence of necessary functions	*/
/*									*/
void Plugin::selftest()
{ 
	try
	{
		string value;  
		
		cout << "Testing plugin: " << path << endl;
		
		value = getName();
		if( value.empty() ) cout << "\n\tfound empty name!\n";
		value = getVersion();
		if( value.empty() ) cout << "\n\tfound empty version! (yes, empty version number)\n";
		value = getAuthors();
		if( value.empty() ) cout << "\n\tfound empty authors! meaning: could not find any authors!\n";
		value = getDescription();
		if( value.empty() ) cout << "\n\tfound empty description! meaning: could not find any description!\n";
		value = getCategory();
		if( value.empty() ) cout << "\n\tcategory not found! meaning: could not find any category!\n";
		
		cout << "trying to have parameters registered ...\n";
		registerParameter(); 
		cout << "seems to work ok"<<endl;
		
		cout << "trying to have plugins requested ...\n";
		try{		
			requestPlugins();
		}
		catch(DatabaseError e){
			cout << "CRAP: "<< e.what() << endl;
		}
			
		cout << "seems to work ok."<<endl;
		
		//check on dependencies.  
		cout << "Very well! Basic tests passed!" << endl;
	}
	catch( ... ){
		throw;
	}
	
    
}

run_function Plugin::getRunFunction()
{ 
  /* run function */
  return func_run; 
}

/**tells whether Plugin::load() was called and succeeded*/
bool Plugin::isLoaded()
{
  return is_loaded;
}
