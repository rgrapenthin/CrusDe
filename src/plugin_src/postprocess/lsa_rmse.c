/*************************************************************************** 
 * File:        ./plugin_src/postprocess/lsa_rmse.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     29.04.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @ingroup Postprocessor
 *  @ingroup Plugin 
 */
 /*@{*/
/** \file lsa_rmse.c
 * 
 * DUMMY
 */
/*@}*/

#include "crusde_api.h"
#include <math.h>
#include <stdio.h>


/*variables*/

int NLAT, NLON; 
int my_position;

/* plugin interface */
extern void register_parameter();
extern void request_plugins();
extern void init();
extern void free();
extern void run();

extern const char* get_name() 	 { return "lsa rmse"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return POSTPROCESS_PLUGIN; }
extern const char* get_description() { 
	return "DUMMY"; 
}

/*! empty*/
extern void request_plugins(){}
/*! empty*/
extern void register_parameter(){}
/*! empty*/
extern void clear(){};
/*! empty*/
extern void register_output_fields(){}
/*! empty*/
extern void init(){}
/*! empty*/
extern void run(){printf("LSA RMSE ... RUNNING!\n");}
