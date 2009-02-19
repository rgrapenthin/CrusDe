/***************************************************************************
 * File:        ./plugins/green/pinel_elastic_minus_thickplate.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     20.02.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @ingroup GreensFunction
 *  @ingroup Plugin 
 **/

/*@{*/	
/** \file pinel_elastic_minus_thickplate.c
 * 
 */
/*@}*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "crusde_api.h"

int (*elastic_halfspace) (double**, int, int) = NULL;
int (*thick_plate)       (double**, int, int) = NULL;

int x_pos, y_pos, z_pos;


extern const char* get_name() 	 { return "elastic minus thickplate (pinel)"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return GREEN_PLUGIN; }
extern const char* get_description() { return ""; }

//! Register parameters this Green's function claims from the input.
extern void register_parameter(){}

//! Register output fields for spatial directions this Green's function calculates.
extern void register_output_fields() 
{
	crusde_register_output_field(&x_pos, X_FIELD);	
	crusde_register_output_field(&y_pos, Y_FIELD);	
	crusde_register_output_field(&z_pos, Z_FIELD);	
}

extern void request_plugins()
{
	elastic_halfspace = crusde_request_green_plugin("elastic halfspace (pinel)");
	thick_plate       = crusde_request_green_plugin("thick plate (pinel)");
}

//! Initialize members that depend on registered values. 
extern void init(){}    


//! Not used in this plugin, left empty!
extern void run(){}

//! Clean-up before this plug-in gets unloaded. 
/*! Yet, there is nothing to free here.
 */
extern void clear(){}

//! Returns the Green's Function value at Point(x,y) at time 0.
/*! TODO!
 
 * @param x The x-Coordinate of the wanted value.
 * @param y The y-Coordinate of the wanted value.
 * 
 * @return The value found at Point[y][x]
 */
extern int get_value_at(double** result, int x, int y)
{	
	int    error = -23;
	double buffer[3];

    error = elastic_halfspace(&buffer, x, y);

	/*stop, if halfspace reports something nasty*/
	if(error != NOERROR) return error;

	error = thick_plate(result, x, y);

	/*stop, if thick plate reports something nasty*/
	if(error != NOERROR) return error;

    /*vertical displacement*/
    (*result)[z_pos] = buffer[z_pos] - (*result)[z_pos];
    /*horizontal displacement*/
    (*result)[x_pos] = buffer[x_pos] - (*result)[x_pos];
    (*result)[y_pos] = buffer[y_pos] - (*result)[y_pos];

    return NOERROR;
}
