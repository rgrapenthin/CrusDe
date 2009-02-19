/***************************************************************************
 * File:        ./plugins/operator/conv.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     20.02.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @ingroup Plugin 
 *  @ingroup Operator
 */
 /*@{*/
/** \file conv.c
 * 
 * Implements a convolution in the original domain ... someday.
 *
 */
/*@}*/

#include "crusde_api.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>


/*variables*/
double		*result;

/* *_X: number of rows, *_Y: number of columns*/
int size_x, size_y;
int x=-1, y=-1, i=0;

/* plugin interface */
extern void register_parameter();
extern void register_output_fields();
extern void init();
extern void free();
extern void run();

extern const char* get_name() 	 { return "2d convolution"; }
extern const char* get_version() { return "0.0"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern const char* get_description() { return "not implemented yet"; }
extern PluginCategory get_category() { return KERNEL_PLUGIN; }


/******************************************************************/
/***IMPLEMENTATION*************************************************/
/******************************************************************/

/*!empty*/
extern void register_parameter(){}
/*!empty*/
extern void request_plugins(){}
/*!empty*/
extern void register_output_fields(){}
/*!empty*/
extern void init(){}
/*!empty*/
extern void clear(){;}
/*!empty*/
extern void run()
{   
	while(x<=size_x){
		while(y<=size_y){
			while(false){	
			}/*while ! EOF*/
		}/*yp	*/
	}/*xp	*/
}
