/***************************************************************************
 * File:        ./crusde_api.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     20.02.2007
 * Licence:     GPLv2
 *
 * #########################################################################
 *
 * CrusDe, simulation framework for crustal deformation studies
 * Copyright (C) 2007-2009 Ronni Grapenthin
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

#ifndef _green_api_h	
#define _green_api_h

#include "constants.h"
#include "typedefs.h"
#include "stdarg.h"

#define boolean	int		/* mimic boolean */
#define true	1		/* mimic boolean true */
#define false	0		/* mimic boolean false */

#define NOT_TIME_DEPENDENT false
#define TIME_DEPENDENT     true

// 
#ifdef __cplusplus
extern "C" {
#endif

//! PluginCategory holds ID values used to identify the various plug-in types that are valid within CrusDe
typedef enum{
	LOAD_PLUGIN = 0,
	KERNEL_PLUGIN = 1,
	GREEN_PLUGIN = 2,
	DATAOUT_PLUGIN = 3,
	POSTPROCESS_PLUGIN = 4,
	LOADHISTORY_PLUGIN = 5,
	CRUSTALDECAY_PLUGIN = 6
}PluginCategory;

//! FieldName holds ID values to identify output field names
typedef enum{
	X_FIELD = 0,
	Y_FIELD = 1,
	Z_FIELD = 2,
	ADD_FIELD = 3
}FieldName;


typedef enum{
	NORTH = 0,
	SOUTH = 1,
	EAST  = 2,
	WEST  = 3
}RegionBound;

//! error and debugging functions, formatting works just like <code>printf</code>
void		crusde_error(const char* format, ...);		/** prints msg to stderr and aborts*/
void		crusde_debug(const char* format, ...);		/** prints msg to stdout if DEBUG is set*/
void		crusde_warning(const char* format, ...);	/** prints msg to stderr*/
void		crusde_info(const char* format, ...);		/** prints msg to stderr*/

double* 	crusde_register_param_double(const char* param_name, PluginCategory);
double* 	crusde_register_optional_param_double(const char* param_name, PluginCategory, double);
							/*register double parameter with CrusDe*/
//int* 	        crusde_register_param_int(const char* param_name, PluginCategory);
							/*register integer parameter with CrusDe*/
char**          crusde_register_param_string(const char* param_name, PluginCategory);
char**          crusde_register_optional_param_string(const char* param_name, PluginCategory, char*);
							/*register string parameter with CrusDe*/
void		crusde_register_output_field(int* position, FieldName);
							/*register output field with CrusDe*/
int 		crusde_get_size_x();            /*length of the region of interest*/
int 		crusde_get_size_y();            /*width of the region of interest*/
int 		crusde_get_size_t();            /*total number of time steps*/
int 		crusde_get_gridsize();          /*side length of a grid cell*/
int 		crusde_get_min_x();             /*westernmost coordinate of ROI*/
int 		crusde_get_min_y();             /*southernmost coordinate of ROI*/
int 		crusde_get_dist_to_min_lon(double lat, double lon); /*distance of lon to minimum longitude of model region */
int 		crusde_get_dist_to_min_lat(double lat, double lon); /*distance of lat to minimum latitude of model region */
int 		crusde_get_dimensions();		/*total number of output fields*/
int 		crusde_get_displacement_dimensions();	/*total number of spatial output fields (x,y,z)*/
int 		crusde_model_time();      /*current time step*/
int 		crusde_get_timesteps();   /* total number of timesteps*/
int		crusde_model_step();
int 		crusde_stepsize();              /*time increment with each model step*/
int 		crusde_get_x_index();           /*index of x-displacement values in result array*/
int 		crusde_get_y_index();           /*index of y-displacement values in result array*/
int 		crusde_get_z_index();           /*index of z-displacement values in result array*/
double 		crusde_get_bound(RegionBound);	
double*		crusde_get_gridsize_geographic();

const char* 	crusde_get_observation_file();		/*filename of points to be observed (unused)*/
const char* 	crusde_get_out_file();			/*filename for result output*/

int 		crusde_get_green_at(double** res, int x, int y);
							/*green's function coefficients at x,y, t*/
double 		crusde_get_load_at(int x, int y);/*load at x,y,t*/
double 		crusde_get_load_history_at(int t);      /*load history at t*/
double 		crusde_get_crustal_decay_at(int t);     /*crustal decay at t*/

void 		crusde_set_result(double**);		/*return pointer to model results*/
double** 	crusde_get_result();			/*pointer to model results*/
void 		crusde_set_quadrant(int);		/*get / set quadrant in coordinate system ...*/
int             crusde_get_quadrant();			/*... in which green's function is calculated*/

void 		crusde_exit(int exitcode);		/*have CrusDe terminate gracefully*/

int 		crusde_get_current_load_component();    /*get the current load component id to work with it with somewhere*/
void 		crusde_set_current_load_component(int); /*set the current load component id that is globally worked with*/
int             crusde_get_number_of_loads();

void            crusde_set_operator_space(int, int);    /* operator can set the spatial dimensions it operates on */
void            crusde_get_operator_space(int*, int*);  /* anybody else can retrieve this information */

boolean 	crusde_crustal_decay_given();			/* tells whether the user set a crustal decay function for the current load*/
boolean 	crusde_load_history_given();			/* tells whether the user set a load history function for the current load*/


/*functions to request pointers to the run time function of other plugins of the same category*/
green_exec_function         crusde_request_green_plugin(char* plugin);
load_exec_function          crusde_request_load_plugin(char* plugin);
run_function                crusde_request_kernel_plugin(char* plugin);
run_function                crusde_request_postprocessor_plugin(char* plugin);
loadhistory_exec_function   crusde_request_loadhistory_plugin(char* plugin);
crustaldecay_exec_function  crusde_request_crustaldecay_plugin(char* plugin);

	
/*void	crusde_print_green_params();
void	crusde_print_load_params();
void	crusde_print_kernel_params();
*/

/*double* crusde_get_green_array();
double* crusde_get_load_array();
*/

#ifdef __cplusplus
}
#endif

#endif // _green_api_h
