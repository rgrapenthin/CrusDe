/***************************************************************************
 * File:        ./plugins/load_history/.c
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     29.04.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup LoadHistory Load History Functions
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file sinusoidal.c
 * Sinusoidal load history that calculates a load height depending on the
 * current time step (t), a period length (p) and the timestep when the load is supposed to
 * be at maximum (<i>d<sub>m</sub></i>):
 *
 * \f[
 *	h(t) = \frac{h_m}{2} \left[ 1 + cos( \frac{2\,\pi}{p} (t - d_m) ) \right]
 * \f]
 *
 * with <i>h<sub>m</sub></i> being the maximum load height.
 */
/*@}*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "crusde_api.h"

/*load command line parameters*/
double* p_d_max[N_LOAD_COMPS];		/*!< day of maximum load	[d]		*/
double* p_period_length[N_LOAD_COMPS];	/*!< number of days in a cycle	[d]		*/

double d_max;		/*!< day of maximum load	[d]		*/
double period_length;	/*!< number of days in a cycle	[d]		*/

int my_id = 0;

extern const char* get_name() 	     		{ return "sinusoidal"; }
extern const char* get_version()     		{ return "0.1"; }
extern const char* get_authors()     		{ return "ronni grapenthin"; }
extern PluginCategory get_category() 		{ return LOADHISTORY_PLUGIN; }
extern const char* get_description() 		{ 
	return "Sinusoidal load history that calculates a load height depending on the\
	current time step (<i>t</i>), a period length (<i>p</i>) and the timestep when the load is supposed to \
	be at maximum (<i>d<sub>m</sub></i>):<br/><br/>\
	h(t) =  h<sub>m</sub> / 2 [ 1 + cos( 2*&pi; / p * (t-d<sub>m</sub>) ) ]\
	<br/><br/>\
	with <i>h<sub>m</sub></i> being the maximum load height.\
	"; }
}

/*! empty*/
extern void request_plugins(){}
/*! empty*/
extern void register_output_fields(){}
/*! empty*/
extern void run(){}
/*! freeing mallocs*/
extern void clear(){}
/*! empty*/
extern void init(){}

/*! Register parameters this load function claims from the input.*/
extern void register_parameter()
{
	my_id = crusde_get_current_load_component();

    /* tell main program about parameters we claim from input */
	p_d_max[my_id]          = crusde_register_param_double("peak", get_category());
	p_period_length[my_id]  = crusde_register_param_double("period_length", get_category());
}

extern double get_value_at(unsigned int t)
{
	my_id = crusde_get_current_load_component();

	d_max = *p_d_max[my_id];
	period_length = *p_period_length[my_id];

	return (( 1 + cos( PI*2 / period_length * (t-d_max) ) ) / 2 )  ;
}

