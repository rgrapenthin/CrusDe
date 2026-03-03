/***************************************************************************
 * File:        ./plugin_src/postprocess/hori2vert_ratio.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     10.06.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @ingroup Postprocessor
 *  @ingroup Plugin 
 */
 /*@{*/
/** \file hori2vert_ratio.c
 * 
 * calculates the ratio of horizontal and vertical displacement at each point
 * and adds it to the output array
 */
/*@}*/

#include "crusde_api.h"
#include <math.h>
#include <stdlib.h>
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

extern const char* get_name() 	 { return "hori2vert-ratio"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return POSTPROCESS_PLUGIN; }
extern const char* get_description() { 
	return "Calculates the ratio of horizontal and vertical displacement at each point <i>ratio = |Uh/Uz|</i>\
 	and adds it to the output in a separate field."; 
}

/*!empty*/
extern void request_plugins(){};
/*!empty*/
extern void register_parameter(){};
/*!empty*/
extern void clear(){};

/*! registers one ADD_FIELD type field with the framework */
extern void register_output_fields() 
{ 
crusde_info("(%s) registering output field ...", get_name());	
	crusde_register_output_field(&my_position, ADD_FIELD);	
crusde_info("(%s)\t... got %d", get_name(), my_position);	
}

/*!initializes local variables*/
extern void init()
{
   NLON = crusde_get_size_x();
   NLAT = crusde_get_size_y();
}

/*! does the calculation*/
extern void run()
{
   double** result = crusde_get_result();
   
   int x = crusde_get_x_index(); 
   int y = crusde_get_y_index(); 
   int z = crusde_get_z_index(); 
   
   int i = -1;
   
   if( x < 0 || y < 0 || z < 0){
   	crusde_warning("(%s) Uhm, seems like X, Y, or Z have not been modeled ... don't know what to do.", get_name());
   }
   else
   {   
	while(++i<NLON*NLAT)
	{
		result[my_position][i] = fabs(sqrt( result[x][i]*result[x][i] + result[y][i]*result[y][i] ) / result[z][i]);
	}

    crusde_info("(%s) wrote hori2vert ratio to position: %d", get_name(), my_position);
   }
}
