/***************************************************************************
 * File:        ./GreensFunction.cpp
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     08.06.2008
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

#include "GreensFunction.h"
#include "SimulationCore.h"

/* class GreensFunction */

GreensFunction::GreensFunction(const string _name) : Plugin(_name),
	map_iter(),
	pl_green_map()
{}

GreensFunction::~GreensFunction()
{
	map_iter = pl_green_map.begin();
	
	while(map_iter != pl_green_map.end() ){	
		delete map_iter->second;
		++map_iter;
	}
}

/*								*/
/* Give load contribution of current component at point (x,y) */
/*								*/
int GreensFunction::getValueArrayAt(double** d, int x, int y)
{ 
	//NOTE: map_iter points to the job we are currently working with
	return (map_iter->second)->getValueArrayAt(d,x,y);
}

void GreensFunction::setFirstJob()
{
	map_iter = pl_green_map.begin();
}    

bool GreensFunction::nextJob()
{
	++map_iter;

	return (map_iter == pl_green_map.end() ? false : true);
}    

bool GreensFunction::isLastJob()
{
	map<string, GreenPlugin*>::iterator  iter( map_iter );
	
	if( (++iter) == pl_green_map.end()) 
	{
		return true;
	}

	return false;
}    

string GreensFunction::job()
{
	return map_iter->first;
}    


void GreensFunction::load(map<string, string > job_map)
{

	map<string, string>::iterator jobs_iter = job_map.begin();

	if( job_map.size() == 1 && !(jobs_iter->first).empty() )
		cerr<<"WARNING: Only one job definition given. I'll do it anyway - but you better double check!"<<endl;

	while(jobs_iter != job_map.end()){
		GreenPlugin *pl_green = new GreenPlugin( (jobs_iter->second).c_str() );

		//try loading the plugins
		try
		{
			pl_green->load( (jobs_iter->second).c_str());
		}
		catch(runtime_error e)
		{
			SimulationCore::instance()->abort(e.what());
		}
		
		pl_green_map.insert(pair<string, GreenPlugin*> (jobs_iter->first, pl_green) );
		++jobs_iter;
	}    
}

void GreensFunction::unload() throw (LibHandleError)
{
	map_iter = pl_green_map.begin();

	while(map_iter != pl_green_map.end()){
		//unload load plug-ins
		(map_iter->second)->unload();
		++map_iter;
	}
}    

void GreensFunction::registerParameter()
{
	map_iter = pl_green_map.begin();
	
	while(map_iter != pl_green_map.end()){
		//register parameter of load plug-ins
		(map_iter->second)->registerParameter();
		++map_iter;
	}
}    

void GreensFunction::registerOutputFields()
{
	map_iter = pl_green_map.begin();
	
	while(map_iter != pl_green_map.end()){
		//register additional memory
		(map_iter->second)->registerOutputFields();
		++map_iter;
	}
}    

void GreensFunction::requestPlugins()
{
	map_iter = pl_green_map.begin();
	
	while(map_iter != pl_green_map.end()){
		//Have plug-ins load their additional plug-ins 
		(map_iter->second)->requestPlugins();
		++map_iter;
	}
}    

void GreensFunction::init()
{
	map_iter = pl_green_map.begin();
	
	while(map_iter != pl_green_map.end()){
		//Have plug-ins be initialized
		(map_iter->second)->init();
		++map_iter;
	}
}    

void GreensFunction::release()
{
	map_iter = pl_green_map.begin();
	
	while(map_iter != pl_green_map.end()){
		//Have plug-ins free their memory
		(map_iter->second)->release();
		++map_iter;
	}
}    

