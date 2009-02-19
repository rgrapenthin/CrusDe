/***************************************************************************
 * File:        ./plugins/crustal_decay/exponential.c
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     04.06.2008
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup CrustalDecay Crustal Decay Functions
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file exponential.c
 * Sinusoidal load history that calculates a load height depending on the
 * current time step (t), a period length (p) and the timestep when the load is supposed to
 * be at maximum (<i>d<sub>m</sub></i>):
 *
 * \f[
 *	h(t) = \frac{h_m}{2} \left[ 1 + cos( \frac{2\,\pi}{p} (t - d_m) ) \right]
 * \f]
 * with <i>h<sub>m</sub></i> being the maximum load height.
 */
/*@}*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "crusde_api.h"

/*load command line parameters*/
double* p_tR[100];		/*!< effective relaxation time [yr]		*/
double tR;				/*!< effective relaxation time [yr]		*/

int my_id = 0;

extern const char* get_name() 	     		{ return "exponential"; }
extern const char* get_version()     		{ return "0.1"; }
extern const char* get_authors()     		{ return "ronni grapenthin"; }
extern PluginCategory get_category() 	    { return CRUSTALDECAY_PLUGIN; }
extern const char* get_description() 		{ 
	return " TODO "; }

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
    p_tR[my_id] = crusde_register_param_double("tR", get_category());
}

extern double get_value_at(unsigned int t)
{
	my_id = crusde_get_current_load_component();
	tR = *p_tR[my_id];

	return exp( -1.0 * ((double) t)/tR );
}

