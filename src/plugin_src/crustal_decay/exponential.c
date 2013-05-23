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
 * Gives the exponential decay <i>d</i> at time <i>t</i>
 * 
 * \f[
 *	d(t) = exp(-t/t_r)
 * \f]
 *
 * with <i>t<sub>r</sub></i> being the effective relaxation time.
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
	return " Gives exponential decay <i>d</i> at time <i>t</i>:<br />\
	d(t)=exp(-t/t<sup>r</sup>)<br />\
	where t<sup>r</sup> is the effective relaxation time."; }

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
    p_tR[my_id] = crusde_register_param_double("tR", get_category());
}

extern double get_value_at(unsigned int t)
{
	my_id = crusde_get_current_load_component();
	tR = *p_tR[my_id];

	return exp( -1.0 * ((double) t)/tR );
}

