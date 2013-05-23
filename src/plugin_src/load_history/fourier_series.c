/***************************************************************************
 * File:        ./plugins/load_history/fourier_series.c
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     29.04.2009
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup LoadHistory Load History Functions
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file fourier_series.c
 * Provides a Fourier Series for which bias, b_0, of a time series (y-offset), linear trend, b_1, (annual) 
 * sine and cosine, s_1, c_1, semi(annual) sine and cosine, s_2, c_2, as well as the number of days (365) in a 
 * cycle, l, can be specified.
 * \f[
 *	h(t) = b_0 + b_1 * t + c_1 * cos(2*p) + s_1 * sin(2*p) + c_2*cos(4*p) + s_2*sin(4*p)
 * \f]
 *  
 * where p = \pi* t/l ; with l is period length.
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

extern const char* get_name() 	     		{ return "fourier_series"; }
extern const char* get_version()     		{ return "0.1"; }
extern const char* get_authors()     		{ return "anthony arendt, ronni grapenthin"; }
extern PluginCategory get_category() 		{ return LOADHISTORY_PLUGIN; }
extern const char* get_description() 		{ 
	return "Provides a Fourier Series for which bias, b_0, of a time series (y-offset), linear trend, b_1, (annual)\
	sine and cosine, s_1, c_1, semi(annual) sine and cosine, s_2, c_2, as well as the number of days (365) in a\
	cycle, l, can be specified:\
	\
    h(t) = b_0 + b_1 * t + c_1 * cos(2*p) + s_1 * sin(2*p) + c_2*cos(4*p) + s_2*sin(4*p)\
    \
    where p = \pi* t/l ; with l is period length.\
 	"; }

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

	return bias + trend * t + 					//constants, linear trend
	       annual_cos*cos(2*p) + annual_sin*sin(2*p) + 		//annual terms
	       semiannual_cos*cos(4*p) + semiannual_sin*sin(4*p);	//semiannual terms
}

