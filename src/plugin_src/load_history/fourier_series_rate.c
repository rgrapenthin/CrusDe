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
 * with <i>h<sub>m</sub></i> being the maximum load height.
 
	h(t) = p[0] + p[1]*x + p[2]*cos(2.0*pi*x) + p[3]*sin(2.0*pi*x) + \
                      p[4]*cos(4.0*pi*x) + p[5]*sin(4.0*pi*x) 
 
 */
/*@}*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "crusde_api.h"

/*load command line parameters*/
double* p_bias[N_LOAD_COMPS];		/*!< bias of time series (y-offset) */
double* p_trend[N_LOAD_COMPS];		/*!< linear trend in time series */
double* p_annual_sin[N_LOAD_COMPS];	/*!< coefficient for annual sine contribution */
double* p_annual_cos[N_LOAD_COMPS];	/*!< coefficient for annual cosine contribution  */
double* p_semiannual_sin[N_LOAD_COMPS];	/*!< coefficient for semiannual sine contribution   */
double* p_semiannual_cos[N_LOAD_COMPS];	/*!< coefficient for semiannual cosine contribution   */
double* p_period_length[N_LOAD_COMPS];	/*!< number of days in a cycle	[d]		*/

double bias;		/*!< bias of time series (y-offset) */
double trend;		/*!< linear trend in time series */
double annual_sin;	/*!< coefficient for annual sine contribution */
double annual_cos;	/*!< coefficient for annual cosine contribution  */
double semiannual_sin;	/*!< coefficient for semiannual sine contribution   */
double semiannual_cos;	/*!< coefficient for semiannual cosine contribution   */

double p;	/*!< place holder*/
int my_id = 0;  /*!< the ID of the load component I am working for*/

extern const char* get_name() 	     		{ return "fourier_series_rate"; }
extern const char* get_version()     		{ return "0.1"; }
extern const char* get_authors()     		{ return "anthony arendt, ronni grapenthin"; }
extern PluginCategory get_category() 		{ return LOADHISTORY_PLUGIN; }
extern const char* get_description() 		{ 
	return "Sinusoidal load history that calculates a load height depending on the\
	current time step (<i>t</i>), a period length (<i>p</i>) and the timestep when the load is supposed to \
	be at maximum (<i>d<sub>m</sub></i>):<br/><br/>\
	h(t) =  h<sub>m</sub> / 2 [ 1 + cos( 2*&pi; / p * (t-d<sub>m</sub>) ) ]\
	<br/><br/>\
	with <i>h<sub>m</sub></i> being the maximum load height.\
	"; }

/*! empty*/
extern void request_plugins(){}
/*! empty*/
extern void register_output_fields(){}
/*! empty*/
extern void run(){}
/*! freeing mallocs*/
extern void clear()
{}

/*! empty*/
extern void init(){}

/*! Register parameters this load function claims from the input.*/
extern void register_parameter()
{
	my_id = crusde_get_current_load_component();

    /* tell main program about parameters we claim from input */
	p_bias[my_id]           = crusde_register_param_double("bias", get_category());
	p_trend[my_id]          = crusde_register_param_double("trend", get_category());
	p_annual_sin[my_id]     = crusde_register_param_double("annual_sin", get_category());
	p_annual_cos[my_id]     = crusde_register_param_double("annual_cos", get_category());
	p_semiannual_sin[my_id] = crusde_register_param_double("semiannual_sin", get_category());
	p_semiannual_cos[my_id] = crusde_register_param_double("semiannual_cos", get_category());
	p_period_length[my_id]  = crusde_register_param_double("period_length", get_category());
}

extern double get_value_at(unsigned int t)
{
	my_id          = crusde_get_current_load_component();
	bias           = *p_bias[my_id];
	trend          = *p_trend[my_id];
	annual_sin     = *p_annual_sin[my_id];
	annual_cos     = *p_annual_cos[my_id];
	semiannual_sin = *p_semiannual_sin[my_id];
	semiannual_cos = *p_semiannual_cos[my_id];
	//eliminate some calculations
	p              = (PI* t) / (*p_period_length[my_id]);

	return trend - 					//constants, linear trend
	       annual_cos*sin(2*p) + annual_sin*cos(2*p) - 		//annual terms
	       semiannual_cos*sin(4*p) + semiannual_sin*cos(4*p);	//semiannual terms
}

