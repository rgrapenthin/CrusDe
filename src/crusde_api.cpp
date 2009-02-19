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

extern "C" double* crusde_register_param_double(const char* name, PluginCategory category)
{
printf("crusde_register_param_double called for: %s\n", name);
	ParamWrapper *p = new ParamWrapper();
	SimulationCore::instance()->registerParam(p, name, category);
	double* add = p->newDouble();
	return add;
}

extern "C" char** crusde_register_param_string(const char* name, PluginCategory category)
{
printf("crusde_register_param_string called for: %s\n", name);
	ParamWrapper *p = new ParamWrapper();
	SimulationCore::instance()->registerParam(p, name, category);
	char** add = p->newString();
	return add;
}

/* OUTPUT FIELD REGISTRATION */

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

/* operator can set the spatial dimensions it operates on, this is useful if somebody wants to temporarily 
   store and ACCESS spatial results - arrays are stored linearily*/
extern "C" void  crusde_set_operator_space(int nx, int ny)
{
    SimulationCore::instance()->setOperatorSpace(nx, ny);
}

/* anybody else can retrieve this information */
extern "C" void crusde_get_operator_space(int *nx, int *ny)
{
	(*nx) = SimulationCore::instance()->getOperatorSpaceX();
	(*ny) = SimulationCore::instance()->getOperatorSpaceY();
}


extern "C" int crusde_get_green_at(double **result, int x, int y)
{
    return SimulationCore::instance()->greensFunction()->getValueArrayAt(result,x,y);
}

extern "C" double crusde_get_load_at(int x, int y)
{
    return SimulationCore::instance()->loadFunction()->getValueAt(x,y);
}

extern "C" double crusde_get_load_history_at(int t)
{
    return SimulationCore::instance()->loadFunction()->getHistoryValueAt(t);
}

extern "C" int crusde_get_number_of_loads()
{
	return SimulationCore::instance()->getNumberOfLoadComponents();
}

extern "C" double crusde_get_crustal_decay_at(int t)
{
    return SimulationCore::instance()->loadFunction()->getCrustalDecayValueAt(t);
}

extern "C" int crusde_get_size_x()
{
    return SimulationCore::instance()->sizeX();
}

extern "C" int crusde_get_size_y()
{
    return SimulationCore::instance()->sizeY();
}

extern "C" int crusde_get_size_t()
{
    return SimulationCore::instance()->sizeT();
}

extern "C" int crusde_get_gridsize()
{
    return SimulationCore::instance()->gridSize();
}

extern "C" int crusde_get_dimensions()
{
    return SimulationCore::instance()->getDimensions();
}

extern "C" int crusde_get_displacement_dimensions()
{
    return SimulationCore::instance()->displacementDimensions();
}

extern "C" int crusde_get_min_x()
{
    return SimulationCore::instance()->minX();
}

extern "C" int crusde_get_min_y()
{
    return SimulationCore::instance()->minY();
}

extern "C" void crusde_set_result(double **result)
{
    SimulationCore::instance()->setModelData(result);
}

extern "C" double** crusde_get_result()
{
    return SimulationCore::instance()->getModelData();
}

extern "C" void crusde_set_quadrant(int q)
{
    SimulationCore::instance()->setQuadrant(q);
}

extern "C" int crusde_get_quadrant()
{
    return SimulationCore::instance()->getQuadrant();
}

extern "C" const char* crusde_get_out_file()
{
    return SimulationCore::instance()->outFile();
}

extern "C" void crusde_exit(int exitcode)
{
    SimulationCore::instance()->terminate();
    exit(exitcode);
}
/*
extern "C" int crusde_load_history_exists()
{
    return 0; //SimulationCore::instance()->loadHistoryPlugin()->isLoaded() ? 1 : 0;
}
*/
extern "C" int crusde_model_time()
{
    return SimulationCore::instance()->modelTime();
}

extern "C" int crusde_get_timesteps()
{
    return SimulationCore::instance()->getTimesteps();
}

extern "C" int crusde_model_step()
{
    return SimulationCore::instance()->modelStep();
}


extern "C" int crusde_stepsize()
{
    return SimulationCore::instance()->stepSize();
}

extern "C" int crusde_get_x_index()
{
    return SimulationCore::instance()->xIndex();
}

extern "C" int crusde_get_y_index()
{
    return SimulationCore::instance()->yIndex();
}

extern "C" int crusde_get_z_index()
{
    return SimulationCore::instance()->zIndex();
}

extern "C" int crusde_get_current_load_component()
{
	return SimulationCore::instance()->getLoadFunctionComponent();
}

extern "C" void crusde_set_current_load_component(int id)
{
	SimulationCore::instance()->setLoadFunctionComponent(id);
}

extern "C" int crusde_get_number_of_load_components()
{
	return SimulationCore::instance()->getNumberOfLoadComponents();
}

extern "C" boolean crusde_crustal_decay_given()
{
    return SimulationCore::instance()->loadFunction()->crustalDecayGiven();
}

extern "C" boolean crusde_load_history_given()
{
    return SimulationCore::instance()->loadFunction()->loadHistoryGiven();
}

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
