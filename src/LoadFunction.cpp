/***************************************************************************
 * File:        ./LoadFunction.cpp
 * Author:      Ronni Grapenthin, Geophysical Institute, UAF
 * Created:     2008-02-19
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

#include "LoadFunction.h"
#include "SimulationCore.h"

/* class LoadPlugin */

LoadFunction::LoadFunction(const string _name) : Plugin(_name),
	map_iter(),
	pl_load_map(),	
	load_value(0.0),
	load_function_component(0)
{

}

LoadFunction::~LoadFunction()
{
	map_iter = pl_load_map.begin();
	
	while(map_iter != pl_load_map.end() ){
		delete map_iter->first;
		//delete load history decay, if existent
		if(map_iter->second.first!=NULL)
		{
			delete map_iter->second.first;
		}
		//delete crustal decay, if existent
		if(map_iter->second.second!=NULL)
		{
			delete map_iter->second.second;
		}
		++map_iter;
	}

}

/*								*/
/* Give load contribution of current component at point (x,y) */
/*								*/
double LoadFunction::getValueAt(int x, int y)
{ 
	//NOTE: this is horribly stupid and ineffective. Probably better to change to a different STL-type?
	map_iter = pl_load_map.begin();
	unsigned int current_component = SimulationCore::instance()->getLoadFunctionComponent();
	
	while( current_component > 0 && map_iter != pl_load_map.end() ){
		++map_iter;
		--current_component;
	}    

	return (map_iter->first)->getValueAt(x,y);
}

double LoadFunction::getHistoryValueAt(int td)
{ 
	if( loadHistoryGiven() )
	{
		//NOTE: this is horribly stupid and ineffective. Probably better to change to a different STL-type?
		map_iter = pl_load_map.begin();
		unsigned int current_component = SimulationCore::instance()->getLoadFunctionComponent();
		
		while( current_component > 0 && map_iter != pl_load_map.end() ){
			++map_iter;
			--current_component;
		}    

		if(map_iter->second.first != NULL)
			return (map_iter->second.first)->getValueAt(td);
	}

	return 1.0;
}

double LoadFunction::getCrustalDecayValueAt(int td)
{ 
	if(	crustalDecayGiven()	)
	{
		//NOTE: this is horribly stupid and ineffective. Probably better to change to a different STL-type?
		map_iter = pl_load_map.begin();
		unsigned int current_component = SimulationCore::instance()->getLoadFunctionComponent();
		while( current_component > 0 && map_iter != pl_load_map.end() ){
			++map_iter;
			--current_component;
		}    

		if(map_iter->second.second != NULL)
			return (map_iter->second.second)->getValueAt(td);
	}

	return 1.0;

}


boolean LoadFunction::loadHistoryGiven()
{
	//NOTE: this is horribly stupid and ineffective. Probably better to change to a different STL-type?
	map_iter = pl_load_map.begin();
	unsigned int current_component = SimulationCore::instance()->getLoadFunctionComponent();

	while( current_component > 0 && map_iter != pl_load_map.end() ){
		++map_iter;
		--current_component;
	}    

	if(map_iter->second.first != NULL){
		if( (map_iter->second.first)->getJobName().empty() ||		// no job assigned, hence valid for all jobs
			(map_iter->second.first)->getJobName().compare( SimulationCore::instance()->currentJob() ) == 0 )
		{
			return true;
		}
		else
			return false;			
	}
	
	return false;
}

boolean LoadFunction::crustalDecayGiven()
{
	//NOTE: this is horribly stupid and ineffective. Probably better to change to a different STL-type?
	map_iter = pl_load_map.begin();
	unsigned int current_component = SimulationCore::instance()->getLoadFunctionComponent();

	while( current_component > 0 && map_iter != pl_load_map.end() ){
		++map_iter;
		--current_component;
	}    

	if(map_iter->second.second != NULL){
		if( (map_iter->second.second)->getJobName().empty() ||		// no job assigned, hence valid for all jobs
			(map_iter->second.second)->getJobName().compare( SimulationCore::instance()->currentJob() ) == 0 )
		{
			return true;
		}
		else
			return false;			
	}
	
	return false;

}

	//void LoadFunction::load(multimap<string, pair<string, string> > names)
void LoadFunction::load(list<LoadFunctionElement*> names)
{
//  change data type to something like that: 
//	list< pair<string, map<string, string> > >
//  list< pair<plug-in name, <parameter-name, parameter-value> > >
//  list.at(LOAD_POS) == load plugin		//no params, yet
//  list.at(LOAD_HISTORY_POS) == load history plugin
//  list.at(CRUSTAL_DECAY_POS) == crustal decay plugin

//  and maybe put it in an extra object for easier to understand access??
//  like LoadFunctionBin??

	list< LoadFunctionElement* >::iterator names_iter = names.begin();
	load_function_component = 0;
	
	while(names_iter != names.end()){
		
		if(load_function_component > N_LOAD_COMPS)
		{
			crusde_error("Maximum Load component number (%d) exceeded. This number is a memory saving measure. \
				      Go to constants.h, increase the number N_LOAD_COMPS to your needs, recompile and rerun.", N_LOAD_COMPS);
		}
		
		SimulationCore::instance()->setLoadFunctionComponent( load_function_component );
		LoadPlugin *pl_load = new LoadPlugin( (*names_iter)->getLoadName().c_str() );
		LoadHistoryPlugin *pl_history(NULL);
		CrustalDecayPlugin *pl_decay(NULL);

		if( !(*names_iter)->getHistoryName().empty() )
		{
			pl_history = new LoadHistoryPlugin( (*names_iter)->getHistoryName().c_str() );
			pl_history->setJobName( (*names_iter)->getHistoryJob() );
		}

		if( !(*names_iter)->getDecayName().empty() )
		{
			pl_decay = new CrustalDecayPlugin( (*names_iter)->getDecayName().c_str() );
			pl_decay->setJobName( (*names_iter)->getDecayJob() );
		}

		//try loading the plugins
		try
		{
			pl_load->load( SimulationCore::instance()->getPluginFilename("load", (*names_iter)->getLoadName().c_str()) );
			if(pl_history != NULL)
			{
				pl_history->load( SimulationCore::instance()->getPluginFilename("load_history", (*names_iter)->getHistoryName().c_str()) );
			}
			if(pl_decay != NULL)
			{
				pl_decay->load( SimulationCore::instance()->getPluginFilename("crustal_decay", (*names_iter)->getDecayName().c_str()) );
			}

		}
		catch(runtime_error e)
		{
			SimulationCore::instance()->abort(e.what());
		}
		
		pl_load_map.insert(pair<LoadPlugin*, pair<LoadHistoryPlugin*, CrustalDecayPlugin*> > (pl_load, pair<LoadHistoryPlugin*, CrustalDecayPlugin*>(pl_history, pl_decay) ) );
		++names_iter;
		++load_function_component;
	}    
}


void LoadFunction::unload() throw (LibHandleError)
{
	map_iter = pl_load_map.begin();
	load_function_component = 0;
	
	while(map_iter != pl_load_map.end()){
		SimulationCore::instance()->setLoadFunctionComponent( load_function_component );
		//unload load plug-ins
		(map_iter->first)->unload();
		//unload load history plug-ins
		if(map_iter->second.first!=NULL)
		{
			(map_iter->second.first)->unload();
		}
		//unload crustal decay plug-ins
		if(map_iter->second.second!=NULL)
		{
			(map_iter->second.second)->unload();
		}

		++map_iter;
		++load_function_component;
	}
}    

void LoadFunction::registerParameter()
{
	map_iter = pl_load_map.begin();
	load_function_component = 0;
	
	while(map_iter != pl_load_map.end()){
		SimulationCore::instance()->setLoadFunctionComponent( load_function_component );
		//register parameter of load plug-ins
		(map_iter->first)->registerParameter();
		//register parameter of load history plug-ins
		if(map_iter->second.first!=NULL)
		{
			(map_iter->second.first)->registerParameter();
		}
		//register parameter of crustal decay plug-ins
		if(map_iter->second.second!=NULL)
		{
			(map_iter->second.second)->registerParameter();
		}
		++map_iter;
		++load_function_component;
	}
}    

void LoadFunction::registerOutputFields()
{
	map_iter = pl_load_map.begin();
	load_function_component = 0;
	
	while(map_iter != pl_load_map.end()){
		SimulationCore::instance()->setLoadFunctionComponent( load_function_component );
		//register output fields of load plug-ins
		(map_iter->first)->registerOutputFields();
		//register output fields of load history plug-ins
		if(map_iter->second.first!=NULL)
		{
			(map_iter->second.first)->registerOutputFields();
		}
		//register output fields of crustal decay plug-ins
		if(map_iter->second.second!=NULL)
		{
			(map_iter->second.second)->registerOutputFields();
		}
		++map_iter;
		++load_function_component;
	}
}    

void LoadFunction::requestPlugins()
{
	map_iter = pl_load_map.begin();
	load_function_component = 0;
	
	while(map_iter != pl_load_map.end()){
		SimulationCore::instance()->setLoadFunctionComponent( load_function_component );
		//have load plug-ins request additional plug-ins
		(map_iter->first)->requestPlugins();
		//have load history plug-ins request additional plug-ins
		if(map_iter->second.first!=NULL)
		{
			(map_iter->second.first)->requestPlugins();
		}
		//have load decay plug-ins request additional plug-ins
		if(map_iter->second.second!=NULL)
		{
			(map_iter->second.second)->requestPlugins();
		}
		++map_iter;
		++load_function_component;
	}
}    

void LoadFunction::init()
{
	map_iter = pl_load_map.begin();
	load_function_component = 0;
	
	while(map_iter != pl_load_map.end()){
		SimulationCore::instance()->setLoadFunctionComponent( load_function_component );
		//init load plug-ins
		(map_iter->first)->init();
		//init load history plug-ins
		if(map_iter->second.first!=NULL)
		{
			(map_iter->second.first)->init();
		}
		//init crustal decay plug-ins
		if(map_iter->second.second!=NULL)
		{
			(map_iter->second.second)->init();
		}
		++map_iter;
		++load_function_component;
	}
}    

void LoadFunction::release()
{
	map_iter = pl_load_map.begin();
	load_function_component = 0;
	
	while(map_iter != pl_load_map.end()){
		SimulationCore::instance()->setLoadFunctionComponent( load_function_component );
		//release load history plug-ins
		if(map_iter->second.first!=NULL)
		{
			(map_iter->second.first)->release();
		}
		//release crustal decay plug-ins
		if(map_iter->second.second!=NULL)
		{
			(map_iter->second.second)->release();
		}

		//release load plug-ins
		(map_iter->first)->release();
		++map_iter;
		++load_function_component;
	}
}    

