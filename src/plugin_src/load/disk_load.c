/***************************************************************************
 * File:        ./plugins/green/disk_load.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     21.02.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup LoadFunction Load Functions
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file disk_load.c
 * 
 * Implements the simple case of a disk load put on a 2 dimensional area. 
 */
/*@}*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "crusde_api.h"

/*disk command line parameters*/
double* p_height[N_LOAD_COMPS]; /**< Disk's height 		[m]		*/
double* p_radius[N_LOAD_COMPS]; /**< Disk's radius 		[m] 		*/
double* p_x[N_LOAD_COMPS];     	/**< Disk's center, x-Coordinate[-]		*/
double* p_y[N_LOAD_COMPS];      /**< Disk's center, y-Coordinate[-]		*/
double* p_rho[N_LOAD_COMPS];    /**< Density of the load	[kg/m^3]	*/

loadhistory_exec_function history_function[N_LOAD_COMPS];

/*disk internal parameters*/
double disk_height;	/**< Disk's height 		[m]		*/
double disk_radius; /**< Disk's radius 		[m] 		*/
double disk_x;     	/**< Disk's center, x-Coordinate[-]		*/
double disk_y;      /**< Disk's center, y-Coordinate[-]		*/
double rho;	        /**< Density of the load	[kg/m^3]	*/

/* internals */
//double dS;           /**< Area around point P(x,y)	[m^2]		*/
//double all_const;    /**< all_const = rho*dS*disk_height, no parts of the load depend on time	*/
//double rho_dS_const; /**< rho_dS_const = rho * dS, height depends on time		*/

int my_id =0;

extern const char* get_name() 	 { return "disk load"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern const char* get_description() { 
	return "Implements the simple case of a disk load put on a 2 dimensional area.\
	In case a load history is defined for a simulation it is used to constrain the height for\
	the current time step."; }
extern PluginCategory get_category() { return LOAD_PLUGIN; }
	
//! empty
extern void run(){}
//! empty
extern void clear()
{}

//! empty
extern void request_plugins(){}

//! empty
extern void register_output_fields(){}

//! Initialize members that depend on registered values. 
/*! This function <b>must not</b> be called before register_parameter() unless none of the necessary values depends on 
 *  parameters provided by the user, which are only set after they have been registered. This function is called
 *  some time <b>after</b> register_parameter().
 */
extern void init(){}


//! Register parameters this load function claims from the input.
/*! This function calls register_load_param() defined in crusde_api.h to register 
 *  references to parameters this Green's function will need to operate properly.
 *  For command line use the order of registration in this function defines the 
 *  identification of parameters in the command line string (i.e. './green -LfirstRegistered/secondRegistered/...').
 *  In case an XML is used to configure the experiment, the reference to this parameter will be identified by 
 *  the string passed as second argument to register_green_param().
 *
 *  This function registers the references in the following order:
 * 
 *  -# disk_height, XML config identifier is "height", SI-unit [m]
 *  -# disk_radius, XML config identifier is "radius", SI-unit [m]
 *  -# disk_x, XML config identifier is "center_x", SI-unit [m] (Lambert  *  -# disk_y, XML config identifier is "center_y", SI-unit [m] (Lambert coordinate)
 *  -# rho, XML config identifier is "rho", SI-unit [kg/m^3]
 *
 *  @see register_load_param()
 *  @see register_parameter() (temp_plugin.c.tmp)
 */
extern void register_parameter()
{
     my_id = crusde_get_current_load_component();
printf("\tMy id: %d\n", my_id);
    /* tell main program about parameters we claim from input */
    p_height[my_id] = crusde_register_param_double("height", get_category());
    p_radius[my_id] = crusde_register_param_double("radius", get_category());
    p_x[my_id]      = crusde_register_param_double("center_x", get_category());
    p_y[my_id]      = crusde_register_param_double("center_y", get_category());
    p_rho[my_id]    = crusde_register_param_double("rho", get_category());

}

//! Returns the Load of a disk at Point (x,y) at time t.
/*! Computes the euclidean distance of Point (x,y) to (center_x, center_y). 
 *  If this distance is less or equal to disk_radius the load will be returned, zero otherwise.
 *  
 * @param x The x-Coordinate of the wanted value.
 * @param y The y-Coordinate of the wanted value.
 * @param t The time-Coordinate of the wanted value (set to NO_TIME in non-temporal modelling environment).
 * 
 * @return The load at Point (x,y,t).
 */
extern double get_value_at(int x, int y)
{
	/* who am I right now?*/
	my_id = crusde_get_current_load_component();

	disk_height = *p_height[my_id];
	disk_radius = *p_radius[my_id];
	disk_x = *p_x[my_id];
	disk_y = *p_y[my_id];
	rho = *p_rho[my_id];

	disk_x -= crusde_get_min_x();
	disk_y -= crusde_get_min_y();

	/*get euclidean distance to center of disc	*/
	double xx = (double) (x*crusde_get_gridsize());
	double yy = (double) (y*crusde_get_gridsize());
	
	double dist = sqrt( (double) ((xx-disk_x)*(xx-disk_x) + (yy-disk_y)*(yy-disk_y)) );

	/*if point is outside disc: h=0.0, see below*/
	if(dist <= disk_radius)
	{
		return crusde_get_gridsize()*crusde_get_gridsize()*rho*disk_height;
	}
	
	return 0.0;
}

extern void set_history_function(loadhistory_exec_function history)
{
	history_function[ crusde_get_current_load_component() ] = history;
}

