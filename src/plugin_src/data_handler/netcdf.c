/***************************************************************************
 * File:        ./plugin_src/data_handler/netcdf.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     21.02.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup DataHandler Data Handler
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file netcdf.c
 *  
 * This data handler writes CrusDe results to a 4-D netcdf file following the 
 * COARDS conventions ( ftp://ftp.unidata.ucar.edu/pub/netcdf/Conventions/COARDS/coards_conventions ). 
 * The four dimensions, and so the dimension names, are:
 *
 * latitude, longitude, time, direction.
 * 
 * This plugin creates a netCDF file with either the filename given in the experiment definition
 * or the default name for output (experiment.nc) on initialization. At each time step the 
 * the model results are saved to this file for all directions (x,y,z, other). The file gets deleted
 * during unloading if no data was written.
 */
/*@}*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netcdf.h>
#include <assert.h>

#include "crusde_api.h"

/* We are writing 4D data */
#define NDIMS 4
/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {crusde_error("%s, %d: %s",  __FILE__, __LINE__, nc_strerror(e)); }

int status;
int nc_id = -1;

/* ID for each one. */
int NLAT, NLON, NTIM, NDIR; 
int lat_dimid=-1, lon_dimid=-1, t_dimid=-1, dir_dimid=-1;
int lat_varid=-1, lon_varid=-1, data_varid=-1, t_varid=-1, dir_varid=-1;
int dimids[NDIMS];
double** data_out;
boolean data_written;
char* filename;

/* Names of things. */
static const char* netcdf_history 	= "written by CrusDe\'s netCDF Plugin, direction indices are: ";
static const char* netcdf_latitude 	= "latitude";
static const char* netcdf_longitude 	= "longitude";
static const char* netcdf_longname	= "experiment name";
static const char* netcdf_time 		= "time";
static const char* netcdf_direction	= "direction";
static const char* netcdf_deg_east 	= "meters_east";
static const char* netcdf_deg_north 	= "meters_north";
static const char* netcdf_meters 	= "m";
static const char* netcdf_days 		= "days";

extern const char* get_name() 	 { return "netcdf writer"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
/** returns DATAOUT_PLUGIN */
extern PluginCategory get_category() { return DATAOUT_PLUGIN; }
/** empty */
extern void request_plugins(){}
/** empty */
extern void register_output_fields(){}

extern const char* get_description() { 
	return ("This data handler writes CrusDe results to a 4-D netcdf file following the \
	COARDS conventions ( ftp://ftp.unidata.ucar.edu/pub/netcdf/Conventions/COARDS/coards_conventions ). \
	The four dimensions, and so the dimension names, are:<br /><br />\
	\
	latitude, longitude, time, direction.<br /><br />\
	\
	The sematics of the direction indices depend highly on the order in which the Green's function \
	plug-in allocates the output fields and in which order the output generating postprocessors \
	are defined. It is tried information to write as much information as possible about this \
	to the 'history' field.<br /><br />\
	This plugin creates a netCDF file with either the filename given in the experiment definition \
	or the default name for output (experiment.nc) on initialization. At each time step the \
	the model results are saved to this file for all directions (x,y,z, other). The file gets deleted \
	during unloading if no data was written.<br /><br />\
	Tools such as 'ncview' help to directly examine the result data. The 'NCO' (netCDF Operators), e.g. \
	'ncks' help to extract fields from the result array. The following script snippet for example \
	extracts values for all latitudes and longitudes at time=0 and for direction=2 (usually vertical \
	displacement, refer to respective file history) from the file disk.nc using NOS's 'kitchen sink'. \
	awk then strips the variable names from the values so that an ascii table of the format \
	'x y z' emerges:<br /><br /> \
	\
	ncks -a -P -v crusde_result -d time,0 -d direction,2 disk.nc| <br />\
	awk '{<br />\
		split($3, y, \"=\") <br />\
		split($4, x, \"=\") <br />\
		split($5, z, \"=\"); <br />\
		<br />\
		if(x[2] != 0 && z[2] != 0)<br />\
			print x[2],y[2],z[2]<br />\
	}' > $INPUT<br />\
	"); 
}

/*! does the writing to the file
 */
extern void run()
{
assert(data_out != NULL);

crusde_info("writing to file: %s ... data_varid: %d, nc_id: %d, NDIR: %d (%d), NLAT: %d, NLON: %d", filename, data_varid, nc_id, crusde_get_dimensions(), NDIR, NLAT, NLON);
/* FOR SOME REASON THIS GIVES A SEGFAULT ON X86_64-suse-linux
   size_t start[NDIMS] = {crusde_model_step(), 0, 0, 0};
   size_t count[NDIMS] = {1, 1, NLAT, NLON};
   int n=-1;

   while(++n<NDIR)
   {
     start[1] = n;
     status = nc_put_vara_double(nc_id, data_varid, start, count, &data_out[n]);
     if (status != NC_NOERR)
     {
        ERR(status); 
     }
   }
*/
   int l=-1, k=-1, n=-1;
   size_t index[NDIMS] = {0,0,0,0};
   while(++k<NLON)
   {
	l = -1;
	while(++l<NLAT)
	{
		n=-1;
   		while(++n<NDIR)
		{
			index[0]=crusde_model_step();
			index[1]=n;
			index[2]=l;
			index[3]=k;
			status = nc_put_var1_double(nc_id, data_varid, index, &data_out[n][k+NLON*l]);
			if (status != NC_NOERR)
			{
				ERR(status); 
			}
		}
	  }
    }

   data_written = true;

}

/*! Clean-up before this plug-in gets unloaded. 
 */
extern void clear()
{

   /* Close the file. This frees up any internal netCDF resources
    * associated with the file, and flushes any buffers. */
   
   if(nc_id >= 0 )
   {
	   status = nc_close(nc_id);
	   if (status != NC_NOERR) ERR(status);
   }
   
   if(!data_written)
   {
   	/*delete empty/corupt file */
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
   int lat, lon, time, dir;
   NLON = crusde_get_size_x();
   NLAT = crusde_get_size_y();
   NTIM = crusde_get_size_t();
   NDIR = crusde_get_dimensions();
   data_out = NULL;
   const char* tmp =  crusde_get_out_file();
   filename = (char*) malloc (sizeof(char*) * (strlen(tmp)+1));
   strncpy(filename, tmp, strlen(tmp));
   filename[strlen(tmp)] = '\0';
	
   double lats[NLAT], lons[NLON];
   int times[NTIM], dirs[NDIR];
    
   /* Create the file. The NC_CLOBBER parameter tells netCDF to
    * overwrite this file, if it already exists.*/
   status = nc_create(filename, NC_CLOBBER, &nc_id);
   if (status != NC_NOERR) ERR(status);
   
   /* DO ALL THE DEFINITIONS */
   
   /* GLOBAL ATTRIBUTES */
   status = nc_put_att_text(nc_id, NC_GLOBAL, "Conventions", strlen("COARDS"), "COARDS");
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, NC_GLOBAL, "title", strlen("netcdf writer"), "netcdf writer");
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, NC_GLOBAL, "history", strlen(netcdf_history), netcdf_history);
   if (status != NC_NOERR) ERR(status);
 
/* Define the dimensions and their attributes*/
   /* LATITUDE **********************************/
   status = nc_def_dim(nc_id, netcdf_latitude, NLAT, &lat_dimid);
   if (status != NC_NOERR) ERR(status);

   /* LONGITUDE *********************************/   
   status = nc_def_dim(nc_id, netcdf_longitude, NLON, &lon_dimid);
   if (status != NC_NOERR) ERR(status);

   /* TIME *********************************/   
   status = nc_def_dim(nc_id, netcdf_time, NTIM, &t_dimid);
   if (status != NC_NOERR) ERR(status);
   
   /* DIRECTION *********************************/   
   status = nc_def_dim(nc_id, netcdf_direction, NDIR, &dir_dimid);
   if (status != NC_NOERR) ERR(status);


   /* The dimids array is used to pass the IDs of the dimensions of
    * the variable. */
   dimids[0] = t_dimid;
   dimids[1] = dir_dimid;
   dimids[2] = lat_dimid;
   dimids[3] = lon_dimid;

   /* Define coordinate netCDF variables. They will hold the
      coordinate information, that is, the latitudes and longitudes. A
      varid is returned for each.*/
   /* LONGITUDE **********************************************************/
   status = nc_def_var(nc_id, netcdf_longitude, NC_DOUBLE, 1, &lon_dimid, &lon_varid);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, lon_varid, "long_name", strlen(netcdf_longitude), netcdf_longitude);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, lon_varid, "units", strlen(netcdf_deg_east), netcdf_deg_east);
   if (status != NC_NOERR) ERR(status);

   /* LATITUDE ***********************************************************/
   status = nc_def_var(nc_id, netcdf_latitude, NC_DOUBLE, 1, &lat_dimid, &lat_varid);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, lat_varid, "long_name", strlen(netcdf_latitude), netcdf_latitude);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, lat_varid, "units", strlen(netcdf_deg_north), netcdf_deg_north);
   if (status != NC_NOERR) ERR(status);

   /* TIME ***********************************************************/
   status = nc_def_var(nc_id, netcdf_time, NC_INT, 1, &t_dimid, &t_varid);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, t_varid, "long_name", strlen(netcdf_time), netcdf_time);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, t_varid, "units", strlen(netcdf_days), netcdf_days);
   if (status != NC_NOERR) ERR(status);

   /* DIRECTION ***********************************************************/
   status = nc_def_var(nc_id, netcdf_direction, NC_INT, 1, &dir_dimid, &dir_varid);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, dir_varid, "long_name", strlen(netcdf_direction), netcdf_direction);
   if (status != NC_NOERR) ERR(status);

   /* Define the data.*/
   status = nc_def_var(nc_id, "crusde_result", NC_DOUBLE, NDIMS, dimids, &data_varid);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, data_varid, "long_name", strlen(netcdf_longname), netcdf_longname);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_att_text(nc_id, data_varid, "units", strlen(netcdf_meters), netcdf_meters);
   if (status != NC_NOERR) ERR(status);

   /* End define mode. This tells netCDF we are done defining
    * metadata. */
   status = nc_enddef(nc_id);
   if (status != NC_NOERR) ERR(status);

   /* Create lat, long, and time data arrays.*/
   double *gridsize_geographic = crusde_get_gridsize_geographic();
   for (lon = 0; lon < NLON; ++lon)
      lons[lon] = lon*gridsize_geographic[1]+crusde_get_bound(WEST);
   for (lat = 0; lat < NLAT; ++lat)
      lats[lat] = lat*gridsize_geographic[0]+crusde_get_bound(SOUTH);
   for (time = 0; time < NTIM; ++time)
      times[time] = time*crusde_stepsize();
   for (dir = 0; dir < NDIR; ++dir)
      dirs[dir] = dir;

   status = nc_put_var_double(nc_id, lat_varid, &lats[0]);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_var_double(nc_id, lon_varid, &lons[0]);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_var_int(nc_id, t_varid, &times[0]);
   if (status != NC_NOERR) ERR(status);
   status = nc_put_var_int(nc_id, dir_varid, &dirs[0]);
   if (status != NC_NOERR) ERR(status);
}


/*! empty
 *  @see register_parameter() (temp_plugin.c.tmp)
 */
extern void register_parameter() {}

/*! set pointer to modelling result
 */
extern void set_model_data(double **data, int dim_x, int dim_y)
{  

	//shallow copy, pointer to the data is sufficient
	data_out = data;
} 
