/***************************************************************************
 * File:        ./plugin_src/postprocess/factor.c
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     03.07.2008
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup Postprocessor Postprocessing units
 *  @ingroup Plugin 
 */
 /*@{*/
/** \file factor.c
 * 
 *  multiplies results by a constant factor. handy for unit conversions
 */
/*@}*/

#include "crusde_api.h"
#include <math.h>
#include <stdio.h>


/*variables*/
double *p_factor;
double factor;
int dimensions;
int NLAT, NLON; 

/* plugin interface */
extern void register_parameter();
extern void request_plugins();
extern void init();
extern void free();
extern void run();

extern const char* get_name() 	 { return "factor"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return POSTPROCESS_PLUGIN; }
extern const char* get_description() { 
	return "Multiplies <b>all</b> dimensions with a constant factor to be given in the experiment definition. \
		Useful for e.g. magnitude conversions after processing.";
}
/*! empty*/
extern void request_plugins(){};
/*! empty*/
extern void register_parameter()
{
  p_factor = crusde_register_param_double("factor", get_category());
};
/*! empty*/
extern void clear(){};

/*!registers one ADD_FIELD type output field*/
extern void register_output_fields(){}

/*! initializes local vars*/
extern void init()
{
   factor	= *(p_factor);
   dimensions 	= crusde_get_dimensions();
   NLON 	= crusde_get_size_x();
   NLAT 	= crusde_get_size_y();
}

/*! does the conversion.*/
extern void run()
{
   double** result = crusde_get_result();

   int i = -1;
   int d = -1;

   if( dimensions < 1){
   	fprintf( stderr, "Warning: Uhm, seems like there is no output ... don't know what to do except for nothing. \
		          Sorry! No factor applied.\n");
   }
   else
   {   
	while(++d<dimensions)
	{
		i=-1;
		while(++i<NLON*NLAT)
		{
			result[d][i] *=  factor;
		}
printf("multiplied dimension %d with %f\n", d, factor);
	}
   }
}
