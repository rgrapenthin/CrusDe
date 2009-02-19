/***************************************************************************
 * File:        ./plugin_src/postprocess/xy2r.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     28.04.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup Postprocessor Postprocessing units
 *  @ingroup Plugin 
 */
 /*@{*/
/** \file xy2r.c
 * 
 * uses the x and y carthesian coordinates, converts them to a cylindrical 
 * coordinate and adds it to the output array.
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

extern const char* get_name() 	 { return "xy2r"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return POSTPROCESS_PLUGIN; }
extern const char* get_description() { 
	return "Calculates cylindrical coordinate <i>r = &radic; (x&sup2; + y&sup2;)</i> and\
	adds it to the output in a separate field."; 
}
/*! empty*/
extern void request_plugins(){};
/*! empty*/
extern void register_parameter(){};
/*! empty*/
extern void clear(){};

/*!registers one ADD_FIELD type output field*/
extern void register_output_fields() 
{ 
printf("xy2r registers output field...");	
	crusde_register_output_field(&my_position, ADD_FIELD);	
printf(" ... got %d\n", my_position);	
}

/*! initializes local vars*/
extern void init()
{
   NLON = crusde_get_size_x();
   NLAT = crusde_get_size_y();
}

/*! does the conversion.*/
extern void run()
{
   double** result = crusde_get_result();
   
   int x = crusde_get_x_index(); 
   int y = crusde_get_y_index(); 
   int i = -1;
   
   if( x < 0 || y < 0){
   	fprintf( stderr, "Warning: Uhm, seems like X or Y have not been modeled ... don't know what to do\n");
   }
   else
   {   
	while(++i<NLON*NLAT)
	{
		result[my_position][i] = sqrt( result[x][i]*result[x][i] + result[y][i]*result[y][i] );
	}
printf("wrote xy2r conversion to position: %d\n", my_position);
   }
}
