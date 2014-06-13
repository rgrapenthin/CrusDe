/***************************************************************************
 * File:        ./plugins/load_history/fourier_series_general.c
 * Author:      Ronni Grapenthin, BSL-UC Berkeley
 * Created:     12.06.2014
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup LoadHistory Load History Functions
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file fourier_series_general.c
 * Provides the most general implementation of a Fourier Series for which bias, b_0, of a time series (y-offset), linear trend, b_1, (annual) 
 * sine and cosine, s_1, c_1, semi(annual) sine and cosine, s_2, c_2, as well as the number of days (365) in a 
 * cycle, l, can be specified.
 * \f[
 *	h(t) = b_0 + b_1 * t + c_1 * cos(2\pi*t/T_cos_1 + phi_cos_1) + s_1 * sin(2\pi*t/T_sin_1 + phi_sin_1) + <br/>
 *                         c_2 * cos(2\pi*t/T_cos_2 + phi_cos_2) + s_2 * sin(2\pi*t/T_sin_2 + phi_sin_2) + 
 *                         c_3 * cos(2\pi*t/T_cos_3 + phi_cos_3) + s_3 * sin(2\pi*t/T_sin_3 + phi_sin_3) + 
 *                         c_4 * cos(2\pi*t/T_cos_4 + phi_cos_4) + s_4 * sin(2\pi*t/T_sin_4 + phi_sin_4) + 
 *                         c_5 * cos(2\pi*t/T_cos_5 + phi_cos_5) + s_5 * sin(2\pi*t/T_sin_5 + phi_sin_5) 
 * \f]
 */

/*@}*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "crusde_api.h"

/*load command line parameters*/
double* p_bias[N_LOAD_COMPS];		/*!< bias of time series (y-offset) */
double* p_trend[N_LOAD_COMPS];		/*!< linear trend in time series */

//first sine/cosine
double* p_s_1[N_LOAD_COMPS];	    /*!< amplitude for 1st sine contribution */
double* p_c_1[N_LOAD_COMPS];	    /*!< amplitude for 1st cosine contribution */
double* p_T_sin_1[N_LOAD_COMPS];	/*!< period for 1st sine contribution */
double* p_T_cos_1[N_LOAD_COMPS];	/*!< period for 1st cosine contribution */
double* p_phi_sin_1[N_LOAD_COMPS];	/*!< phase for 1st sine contribution */
double* p_phi_cos_1[N_LOAD_COMPS];	/*!< phase for 1st cosine contribution */

//second sine/cosine
double* p_s_2[N_LOAD_COMPS];	    /*!< amplitude for 2nd sine contribution */
double* p_c_2[N_LOAD_COMPS];	    /*!< amplitude for 2nd cosine contribution */
double* p_T_sin_2[N_LOAD_COMPS];	/*!< period for 2nd sine contribution */
double* p_T_cos_2[N_LOAD_COMPS];	/*!< period for 2nd cosine contribution */
double* p_phi_sin_2[N_LOAD_COMPS];	/*!< phase for 2nd sine contribution */
double* p_phi_cos_2[N_LOAD_COMPS];	/*!< phase for 2nd cosine contribution */

//third sine/cosine
double* p_s_3[N_LOAD_COMPS];	    /*!< amplitude for 3rd sine contribution */
double* p_c_3[N_LOAD_COMPS];	    /*!< amplitude for 3rd cosine contribution */
double* p_T_sin_3[N_LOAD_COMPS];	/*!< period for 3rd sine contribution */
double* p_T_cos_3[N_LOAD_COMPS];	/*!< period for 3rd cosine contribution */
double* p_phi_sin_3[N_LOAD_COMPS];	/*!< phase for 3rd sine contribution */
double* p_phi_cos_3[N_LOAD_COMPS];	/*!< phase for 3rd cosine contribution */

//fourth sine/cosine
double* p_s_4[N_LOAD_COMPS];	    /*!< amplitude for 4th sine contribution */
double* p_c_4[N_LOAD_COMPS];	    /*!< amplitude for 4th cosine contribution */
double* p_T_sin_4[N_LOAD_COMPS];	/*!< period for 4th sine contribution */
double* p_T_cos_4[N_LOAD_COMPS];	/*!< period for 4th cosine contribution */
double* p_phi_sin_4[N_LOAD_COMPS];	/*!< phase for 4th sine contribution */
double* p_phi_cos_4[N_LOAD_COMPS];	/*!< phase for 4th cosine contribution */

//fifth sine/cosine
double* p_s_5[N_LOAD_COMPS];	    /*!< amplitude for 5th sine contribution */
double* p_c_5[N_LOAD_COMPS];	    /*!< amplitude for 5th cosine contribution */
double* p_T_sin_5[N_LOAD_COMPS];	/*!< period for 5th sine contribution */
double* p_T_cos_5[N_LOAD_COMPS];	/*!< period for 5th cosine contribution */
double* p_phi_sin_5[N_LOAD_COMPS];	/*!< phase for 5th sine contribution */
double* p_phi_cos_5[N_LOAD_COMPS];	/*!< phase for 5th cosine contribution */

double bias;            /*!< bias of time series (y-offset) */
double trend;           /*!< linear trend in time series */

//first sine/cosine
double s_1;	            /*!< amplitude for 1st sine contribution */
double c_1;	            /*!< amplitude for 1st cosine contribution */
double T_sin_1; 	    /*!< period for 1st sine contribution */
double T_cos_1;	        /*!< period for 1st cosine contribution */
double phi_sin_1;	    /*!< phase for 1st sine contribution */
double phi_cos_1;	    /*!< phase for 1st cosine contribution */

//first sine/cosine
double s_1;	            /*!< amplitude for 1st sine contribution */
double c_1;	            /*!< amplitude for 1st cosine contribution */
double T_sin_1; 	    /*!< period for 1st sine contribution */
double T_cos_1;	        /*!< period for 1st cosine contribution */
double phi_sin_1;	    /*!< phase for 1st sine contribution */
double phi_cos_1;	    /*!< phase for 1st cosine contribution */

//second sine/cosine
double s_2;	            /*!< amplitude for 2nd sine contribution */
double c_2;	            /*!< amplitude for 2nd cosine contribution */
double T_sin_2; 	    /*!< period for 2nd sine contribution */
double T_cos_2;	        /*!< period for 2nd cosine contribution */
double phi_sin_2;	    /*!< phase for 2nd sine contribution */
double phi_cos_2;	    /*!< phase for 2nd cosine contribution */

//third sine/cosine
double s_3;	            /*!< amplitude for 3rd sine contribution */
double c_3;	            /*!< amplitude for 3rd cosine contribution */
double T_sin_3; 	    /*!< period for 3rd sine contribution */
double T_cos_3;	        /*!< period for 3rd cosine contribution */
double phi_sin_3;	    /*!< phase for 3rd sine contribution */
double phi_cos_3;	    /*!< phase for 3rd cosine contribution */

//fourth sine/cosine
double s_4;	            /*!< amplitude for 4th sine contribution */
double c_4;	            /*!< amplitude for 4th cosine contribution */
double T_sin_4; 	    /*!< period for 4th sine contribution */
double T_cos_4;	        /*!< period for 4th cosine contribution */
double phi_sin_4;	    /*!< phase for 4th sine contribution */
double phi_cos_4;	    /*!< phase for 4th cosine contribution */

//fifth sine/cosine
double s_5;	            /*!< amplitude for 5th sine contribution */
double c_5;	            /*!< amplitude for 5th cosine contribution */
double T_sin_5; 	    /*!< period for 5th sine contribution */
double T_cos_5;	        /*!< period for 5th cosine contribution */
double phi_sin_5;	    /*!< phase for 5th sine contribution */
double phi_cos_5;	    /*!< phase for 5th cosine contribution */

double two_pi = 0;      /*!< 2*pi constant*/
int     my_id = 0;      /*!< the ID of the load component I am working for*/

extern const char* get_name() 	     		{ return "fourier_series_general"; }
extern const char* get_version()     		{ return "0.1"; }
extern const char* get_authors()     		{ return "ronni grapenthin"; }
extern PluginCategory get_category() 		{ return LOADHISTORY_PLUGIN; }
extern const char* get_description() 		{ 
	return "Provides a general Fourier Series with bias, b_0, of a time series (y-offset), linear trend, b_1\
    and 5 sine/cosine components, each with amplitude, period and phase specification.\
	\
 	h(t) = b_0 + b_1 * t + c_1 * cos(2pi*t/T_cos_1 + phi_cos_1) + s_1 * sin(2pi*t/T_sin_1 + phi_sin_1) +\
                           c_2 * cos(2pi*t/T_cos_2 + phi_cos_2) + s_2 * sin(2pi*t/T_sin_2 + phi_sin_2) +\
                           c_3 * cos(2pi*t/T_cos_3 + phi_cos_3) + s_3 * sin(2pi*t/T_sin_3 + phi_sin_3) +\
                           c_4 * cos(2pi*t/T_cos_4 + phi_cos_4) + s_4 * sin(2pi*t/T_sin_4 + phi_sin_4) +\
                           c_5 * cos(2pi*t/T_cos_5 + phi_cos_5) + s_5 * sin(2pi*t/T_sin_5 + phi_sin_5) \
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
	my_id  = crusde_get_current_load_component();
    two_pi = 2*PI;

    /* tell main program about parameters we claim from input */
	p_bias[my_id]           = crusde_register_optional_param_double("bias", get_category(), 0.0);
	p_trend[my_id]          = crusde_register_optional_param_double("trend", get_category(), 0.0);

    //first sine/cosine ... not optional!
    p_s_1[my_id]           = crusde_register_param_double("amp_sin_1", get_category());
    p_c_1[my_id]           = crusde_register_param_double("amp_cos_1", get_category());
    p_T_sin_1[my_id]       = crusde_register_param_double("period_sin_1", get_category());
    p_T_cos_1[my_id]       = crusde_register_param_double("period_cos_1", get_category());
    p_phi_sin_1[my_id]     = crusde_register_param_double("phase_sin_1", get_category());
    p_phi_cos_1[my_id]     = crusde_register_param_double("phase_cos_1", get_category());

    //second sine/cosine ... optional!
    p_s_2[my_id]           = crusde_register_optional_param_double("amp_sin_2", get_category(), 0.0);
    p_c_2[my_id]           = crusde_register_optional_param_double("amp_cos_2", get_category(), 0.0);
    p_T_sin_2[my_id]       = crusde_register_optional_param_double("period_sin_2", get_category(), 1.0);
    p_T_cos_2[my_id]       = crusde_register_optional_param_double("period_cos_2", get_category(), 1.0);
    p_phi_sin_2[my_id]     = crusde_register_optional_param_double("phase_sin_2", get_category(), 0.0);
    p_phi_cos_2[my_id]     = crusde_register_optional_param_double("phase_cos_2", get_category(), 0.0);

    //third sine/cosine ... optional!
    p_s_3[my_id]           = crusde_register_optional_param_double("amp_sin_3", get_category(), 0.0);
    p_c_3[my_id]           = crusde_register_optional_param_double("amp_cos_3", get_category(), 0.0);
    p_T_sin_3[my_id]       = crusde_register_optional_param_double("period_sin_3", get_category(), 1.0);
    p_T_cos_3[my_id]       = crusde_register_optional_param_double("period_cos_3", get_category(), 1.0);
    p_phi_sin_3[my_id]     = crusde_register_optional_param_double("phase_sin_3", get_category(), 0.0);
    p_phi_cos_3[my_id]     = crusde_register_optional_param_double("phase_cos_3", get_category(), 0.0);

    //fourth sine/cosine ... optional!
    p_s_4[my_id]           = crusde_register_optional_param_double("amp_sin_4", get_category(), 0.0);
    p_c_4[my_id]           = crusde_register_optional_param_double("amp_cos_4", get_category(), 0.0);
    p_T_sin_4[my_id]       = crusde_register_optional_param_double("period_sin_4", get_category(), 1.0);
    p_T_cos_4[my_id]       = crusde_register_optional_param_double("period_cos_4", get_category(), 1.0);
    p_phi_sin_4[my_id]     = crusde_register_optional_param_double("phase_sin_4", get_category(), 0.0);
    p_phi_cos_4[my_id]     = crusde_register_optional_param_double("phase_cos_4", get_category(), 0.0);

    //fifth sine/cosine ... optional!
    p_s_5[my_id]           = crusde_register_optional_param_double("amp_sin_5", get_category(), 0.0);
    p_c_5[my_id]           = crusde_register_optional_param_double("amp_cos_5", get_category(), 0.0);
    p_T_sin_5[my_id]       = crusde_register_optional_param_double("period_sin_5", get_category(), 1.0);
    p_T_cos_5[my_id]       = crusde_register_optional_param_double("period_cos_5", get_category(), 1.0);
    p_phi_sin_5[my_id]     = crusde_register_optional_param_double("phase_sin_5", get_category(), 0.0);
    p_phi_cos_5[my_id]     = crusde_register_optional_param_double("phase_cos_5", get_category(), 0.0);
}

extern double get_value_at(unsigned int t)
{
	my_id          = crusde_get_current_load_component();
	bias           = *p_bias[my_id];
	trend          = *p_trend[my_id];
    
    //first sine/cosine
    s_1            = *p_s_1[my_id];
    c_1            = *p_c_1[my_id];
    T_sin_1        = *p_T_sin_1[my_id];
    T_cos_1        = *p_T_cos_1[my_id];
    phi_sin_1      = *p_phi_cos_1[my_id];
    phi_cos_1      = *p_phi_cos_1[my_id];

    //second sine/cosine
    s_2            = *p_s_2[my_id];
    c_2            = *p_c_2[my_id];
    T_sin_2        = *p_T_sin_2[my_id];
    T_cos_2        = *p_T_cos_2[my_id];
    phi_sin_2      = *p_phi_cos_2[my_id];
    phi_cos_2      = *p_phi_cos_2[my_id];

    //third sine/cosine
    s_3            = *p_s_3[my_id];
    c_3            = *p_c_3[my_id];
    T_sin_3        = *p_T_sin_3[my_id];
    T_cos_3        = *p_T_cos_3[my_id];
    phi_sin_3      = *p_phi_cos_3[my_id];
    phi_cos_3      = *p_phi_cos_3[my_id];

    //fourth sine/cosine
    s_4            = *p_s_4[my_id];
    c_4            = *p_c_4[my_id];
    T_sin_4        = *p_T_sin_4[my_id];
    T_cos_4        = *p_T_cos_4[my_id];
    phi_sin_4      = *p_phi_cos_4[my_id];
    phi_cos_4      = *p_phi_cos_4[my_id];

    //fifth sine/cosine
    s_5            = *p_s_5[my_id];
    c_5            = *p_c_5[my_id];
    T_sin_5        = *p_T_sin_5[my_id];
    T_cos_5        = *p_T_cos_5[my_id];
    phi_sin_5      = *p_phi_cos_5[my_id];
    phi_cos_5      = *p_phi_cos_5[my_id];

	return bias                                  + trend * t                             +  //constants, linear trend
	       c_1*cos(two_pi*t/T_cos_1 + phi_cos_1) + s_1*sin(two_pi*t/T_sin_1 + phi_sin_1) +  //first components
	       c_2*cos(two_pi*t/T_cos_2 + phi_cos_2) + s_2*sin(two_pi*t/T_sin_2 + phi_sin_2) +  //first components
	       c_3*cos(two_pi*t/T_cos_3 + phi_cos_3) + s_3*sin(two_pi*t/T_sin_3 + phi_sin_3) +  //first components
	       c_4*cos(two_pi*t/T_cos_4 + phi_cos_4) + s_4*sin(two_pi*t/T_sin_4 + phi_sin_4) +  //first components
	       c_5*cos(two_pi*t/T_cos_5 + phi_cos_5) + s_5*sin(two_pi*t/T_sin_5 + phi_sin_5);   //fifth components
}

