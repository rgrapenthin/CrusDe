/***************************************************************************
 * File:        ./Plugin.h
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

#ifndef _plugin_h
#define _plugin_h

#include <string>
#include <iostream>
#include <cassert>
#include <dlfcn.h>

#include "crusde_api.h"
#include "typedefs.h"
#include "exceptions.h"

using namespace std;

typedef void  (*void_t)();
typedef char* (*char_t)();
typedef PluginCategory (*category_t)();

class Plugin
{
	private:	

		void_t func_void;	/*  Pointer to loaded routine	*/
		char_t func_get_string;	/*  Pointer to loaded routine	*/
		
		char_t func_get_name;
		char_t func_get_version;
		char_t func_get_authors;	
		char_t func_get_description;	
		
		category_t func_get_category;
		
		void_t func_run;	
		void_t func_register_parameter;	
		void_t func_register_output_fields;
		void_t func_request_plugins;	
		void_t func_init;	
		void_t func_release;
				
		int rc;				/*  return codes 		*/

		//!hidden copy constructor - we do not want to accidentially copy objects
		Plugin(const Plugin& x); 
		
		void_t get_dl_voidFunc(string) throw (LibHandleError);
		char_t get_dl_stringFunc(string) throw (LibHandleError);
		category_t get_categoryFunc(string) throw (LibHandleError);

	protected:
		void  *LibHandle;		/*  Handle to shared lib file	*/
		const string name;		/* identifier for plugin 	*/
		string path;			/* another id for plugin 	*/
		string category;		/* category of plugin, determined by directory*/
		bool is_released;		/* did we already release() the plugin? */
		bool is_initialized;		/* did we already init() the plugin? */
		bool is_registered;		/* did we already register_...() the plugin's parameter? */
		bool is_loaded;			/* were we able to load our library (default: false)? */
	
	public:
		Plugin(const string name);	/* Constructor */
		virtual ~Plugin();			/* Destructor */
		
		/*plugin Interface*/
		
		virtual void load(const string) throw (FileNotFound, LibHandleError);
		virtual void unload() throw (LibHandleError);
		
		virtual void run();
		virtual void registerParameter();
		virtual void registerOutputFields();
		virtual void requestPlugins();
		virtual void init();
		virtual void release();
		
		virtual void selftest();
		
		/*plugin management interface */
		
		string  getName();		/*returns name of the plugin*/
		string  getVersion();		/*returns version of the plugin*/
		string  getAuthors();		/*returns authors of the plugin*/
		string  getDescription();	/*returns description of the plugin*/
		string  getCategory();		/*returns plugin type {green|kernel|out|load} */
		PluginCategory getCategoryID();

		bool isLoaded();		
		void setCategory(string new_type);
		
		run_function getRunFunction();
};

#endif // _plugin_h
