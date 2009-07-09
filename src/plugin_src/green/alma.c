/***************************************************************************
 * File:        ./plugins/green/alma.c
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     2009-07-08
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @ingroup GreensFunction
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file alma.c
 * 
 * Isn't truly a Green's function, but provides an interface to ALMA
 */
/*@}*/

#include <stdio.h>
#include <stdlib.h>
#include "crusde_api.h"

double *p_deg_min;
double *p_deg_max;
double *p_deg_step;
char   *p_file;
double *p_kv;
double *p_lth;
double *p_mode; 
double *p_ng;
double *p_nla;
double *p_p;
double *p_rheol;
double *p_sd;
double *p_time_min;
double *p_time_max;

int x_pos, y_pos, z_pos;

double backpack[3];
	
extern const char* get_name() 	 { return "alma"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return GREEN_PLUGIN; }
extern const char* get_description() { 
	return "This plugin implements an interface to ALMA\
"; }

//! Register parameters this Green's function claims from the input.
/*! This function calls crusde_register_green_param() defined in crusde_api.h to register 
 *  references to parameters this Green's function will need to operate properly.
 *  In case an XML is used to configure the experiment, the reference to this parameter will be identified by 
 *  the string passed as second argument to register_green_param().
 *
 *  This function registers the references in the following order:
 *  -# g, XML config identifier is "g" SI-unit: [m/s^2] 
 *  -# E, XML config identifier is "E" SI-unit: [GPa]
 *  -# nu XML config identifier is "nu" SI-unit: [-]
 *
 *  @see register_green_param()
 *  @see register_parameter() (temp_plugin.c.tmp)
 */
extern void register_parameter()
{
	p_deg_min  = crusde_register_param_double("deg_min", get_category());
	p_deg_max  = crusde_register_param_double("deg_max", get_category());
	p_deg_step = crusde_register_param_double("deg_step", get_category());
	p_file     = crusde_register_param_string("file", get_category());
	p_kv       = crusde_register_param_double("kv", get_category());
	p_lth      = crusde_register_param_double("lth", get_category());
	p_mode     = crusde_register_param_double("mode", get_category());
	p_ng       = crusde_register_param_double("ng", get_category());
	p_nla      = crusde_register_param_double("nla", get_category());
	p_p        = crusde_register_param_double("p", get_category());
	p_rheol    = crusde_register_param_double("rheol", get_category());
	p_sd       = crusde_register_param_double("sd", get_category());
	p_time_min = crusde_register_param_double("time_min", get_category());
	p_time_max = crusde_register_param_double("time_max", get_category());
}

//! Register output fields for spatial directions this Green's function calculates.
/*! This function calls crusde_register_output_field defined in crusde_api.h to register 
 *  output field in the following order:
 *
 *   -# x, y, z (horizontal-x, horizontal-y, vertical).
 */
extern void register_output_fields() 
{ 
printf("alma.c registers output fields...");	
	crusde_register_output_field(&x_pos, X_FIELD);	
	crusde_register_output_field(&y_pos, Y_FIELD);	
	crusde_register_output_field(&z_pos, Z_FIELD);	
printf("\t x: %d\n", x_pos);	
printf("\t y: %d\n", y_pos);	
printf("\t z: %d\n", z_pos);	
}

extern void request_plugins(){}

//! Initialize members that depend on registered values. 
/*! 
 */
extern void init()
{
	/* call ALMA */
	char  alma_call[255];

	sprintf(alma_call, "/usr/local/alma2/alma -h -l -k -load -salz -deg %d:%d:%d -kv %d -lth %e -mode %d -ng %d -nla %d -p %d -rheol %d -sd %d -time %d:%d \0", 
			   (int) *(p_deg_min), (int) *(p_deg_step), (int) *(p_deg_max), (int) *(p_kv), *(p_lth), (int) *(p_mode), (int) *(p_ng), (int) *(p_nla), 
			   (int) *(p_p), (int) *(p_rheol), (int) *(p_sd), (int) *(p_time_min), (int) *(p_time_max) );

	printf("ALMA.C: calling: <%s>\n", alma_call);
	system(alma_call);
}    

//! Not used in this plugin, left empty!
extern void run(){}

//! Clean-up before this plug-in gets unloaded. 
/*! Yet, there is nothing to free here.
 */
extern void clear(){}

//! Returns the Green's Function value at Point(x,y).
/*! TODO!
 
 * @param x The x-Coordinate of the wanted value.
 * @param y The y-Coordinate of the wanted value.
 * 
 * @return The value found at Point[y][x]
 */
extern int get_value_at(double** result, int x, int y)
{	
	return NOERROR;
}
