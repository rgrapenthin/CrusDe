/***************************************************************************
 * File:        ./plugins/load_history/boxcar.c
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     12.06.2008
 * Licence:     GPLv2
 ****************************************************************************/

/** 
 *  @defgroup LoadHistory Load History Functions
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file boxcar.c
  * "Boxcar or rectangular or interval function with: \
  * 
  * \f[
  *	f(t) = 1 on t0 <= t <= t1 \
         = 0 otherwise
  * \f]
  */
/*@}*/

#include <stdio.h>
#include <math.h>

#include "crusde_api.h"

/*load command line parameters*/
double* p_start[N_LOAD_COMPS];/*!< pointer to start interval values	*/
double* p_end[N_LOAD_COMPS];	/*!< pointer to end interval values 		*/

int my_id = 0;

extern const char* get_name() 	     		{ return "boxcar"; }
extern const char* get_version()     		{ return "0.1"; }
extern const char* get_authors()     		{ return "ronni grapenthin"; }
extern PluginCategory get_category() 		{ return LOADHISTORY_PLUGIN; }
extern const char* get_description() 		{ 
	return "Boxcar or rectangular or interval function with: \
\
f(t) = 1 on t0 <= t <= t1 \
     = 0 otherwise"; 
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
    p_start[my_id] = crusde_register_param_double("t0", get_category());
    p_end[my_id] = crusde_register_param_double("t1", get_category());
}

extern double get_value_at(unsigned int t)
{
	my_id = crusde_get_current_load_component();

	if(t >= *p_start[my_id] && t <= *p_end[my_id] )
	{
		return 1.0;
	}

	return 0.0;			
}
