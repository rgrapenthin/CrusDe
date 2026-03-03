/***************************************************************************
 * File:        ./plugins/data_handler/table.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     21.02.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @ingroup Plugin 
 *  @ingroup DataHandler
 **/

/*@{*/
/** \file table.c
 * 
 * This data handler writes CrusDe results to an ASCII table
 * file. A header indicates the semantics of the colomns. Data are separated by a whitespace.
 */
/*@}*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netcdf.h>
#include <assert.h>
#include <time.h>

#include "crusde_api.h"


double** data_out;
boolean data_written;
char* filename;
FILE *out_file = NULL;

int k, l, xmin, ymin, nlat, nlon, ds, dim, d, x_pos, y_pos, z_pos;

extern const char* get_name() 	 { return "table writer"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
/** return DATAOUT_PLUGIN */
extern PluginCategory get_category() { return DATAOUT_PLUGIN; }
/** empty */
extern void request_plugins(){}
/** empty */
extern void register_output_fields(){}
extern const char* get_description() { return "This data handler writes CrusDe results to an ASCII table \
file. A header indicates the semantics of the colomns. Data are separated by a whitespace. \
"; }

/*! writes data for actual time step to file
 */
extern void run()
{

   k=-1;
   int written = 0;
   while(++k<nlon){
	l = -1;
   	while(++l<nlat){
		written = 0;

		written += fprintf(out_file, "%d %d %d ", crusde_model_time(), k*ds+xmin, l*ds+ymin);

		if(x_pos >= 0)
			written += fprintf(out_file, "%f ", data_out[x_pos][k+nlon*l]);
		if(y_pos >= 0)
			written += fprintf(out_file, "%f ", data_out[y_pos][k+nlon*l]);
		if(z_pos >= 0)
			written += fprintf(out_file, "%f ", data_out[z_pos][k+nlon*l]);
   
		d = (x_pos > y_pos) ? x_pos : y_pos;
		d = (d > z_pos) ? d : z_pos;

   		while(++d < dim){
			written += fprintf(out_file, "%f ", data_out[d][k+nlon*l]);
   		}
		
		written += fprintf(out_file, "\n");
		
		if(written<0){
			crusde_error("Problems writing to file %s. Disk full?", out_file);
		}
	}
   }
   
   data_written=true;
}

/*! Clean-up before this plug-in gets unloaded. 
 */
extern void clear()
{
   if(out_file != NULL)
   {
   	fclose(out_file);
   }
   
   if(!data_written)
   {
   	/*delete empty file */
   	remove(filename);
   }

   free(filename);
}


//! Initialize members that depend on registered values. 
/*! This function <b>must not</b> be called before register_parameter() unless none of the necessary values depends on 
 *  parameters provided by the user, which are only set after they have been registered. This function is called
 *  some time <b>after</b> register_parameter().
 */
extern void init()
{
   xmin=crusde_get_min_x();
   ymin=crusde_get_min_y();
   nlon = crusde_get_size_x();
   nlat = crusde_get_size_y();	
   ds = crusde_get_gridsize();
   dim = crusde_get_dimensions();
   x_pos = crusde_get_x_index();
   y_pos = crusde_get_y_index();
   z_pos = crusde_get_z_index();

   data_out = NULL;

   const char* tmp = crusde_get_out_file();
   filename = (char*) malloc (sizeof(char*) * (strlen(tmp) + 1 ));
	
   if (filename == NULL) { crusde_bad_alloc();}
	
   strncpy(filename, tmp, strlen(tmp));
   filename[strlen(tmp)] = '\0';

   out_file = fopen(filename, "w+t");
  
   if(out_file==NULL )
   {
	crusde_error("(%s, %d): Could not open file: %s\n",  __FILE__, __LINE__, filename); 
   }
   
   //header info
   time_t the_time;
   time(&the_time);
		
   char date_str[30];
   char gmt_str[30];
		
   strftime(date_str, 29, "%a %d %b %Y", localtime(&the_time));
   strftime(gmt_str, 29, "%X", gmtime(&the_time));

   
   /*write header*/
   fprintf(out_file, "# file '%s' created by CrusDe's table writer\n", filename);
   fprintf(out_file, "#\n");
   fprintf(out_file, "#    date: %s\n", date_str);
   fprintf(out_file, "#    time: %s (GMT)\n", gmt_str);
   fprintf(out_file, "# records: %d \n", nlat*nlon);
   fprintf(out_file, "#\n");
   fprintf(out_file, "# field names (%d): time-step | x-coordinate | y-coordinate", dim+3);
   if(x_pos >= 0)
	   fprintf(out_file, " | x displacement (Ux)");
   if(y_pos >= 0)
	   fprintf(out_file, " | y displacement (Uy)");
   if(z_pos >= 0)
	   fprintf(out_file, " | z displacement (Uz)");
   
   d = (x_pos > y_pos) ? x_pos : y_pos;
   d = (d > z_pos) ? d : z_pos;

   while(++d < dim){
	fprintf(out_file, "| ADD field ");
   }
   fprintf(out_file, "\n# ------------------------------------------------------------------------------------------------------------------- \n\n");

}	


//! Register parameters this load function claims from the input.
/*! This function calls register_load_param() defined in crusde_api.h to register 
 *
 *  @see register_load_param()
 *  @see register_parameter() (temp_plugin.c.tmp)
 */
extern void register_parameter() {}

/*! set pointer to model result.
 */
 extern void set_model_data(double** data, int dim_x, int dim_y)
{  

crusde_info("%s, line: %d, netcdf::set_model_data\n", __FILE__, __LINE__);
fflush(stdout);
	
	//shallow or deep copy???
	data_out = data;
}
