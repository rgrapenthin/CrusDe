/***************************************************************************
 * File:        ./plugins/green/alma.c
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     2009-07-08
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @ingroup GreensFunction
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file alma.c
 * 
 * Isn't truly a Green's function, but provides an interface to ALMA
 */
/*@}*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "crusde_api.h"

double *p_deg_min;
double *p_deg_max;
double *p_deg_step;
char   **p_file;
double *p_kv;
double *p_lth;
double *p_mode; 
double *p_ng;
double *p_nla;
double *p_p;
double *p_rheol;
double *p_sd;
double *p_time_min;
double *p_time_max;

int x_pos, y_pos, z_pos;

float **love_number_h;
float **love_number_k;
float **love_number_l;

double backpack[3];
	
void read_alma_output(const char* file, float** love_array);

extern const char* get_name() 	 { return "alma"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return GREEN_PLUGIN; }
extern const char* get_description() { 
	return "This plugin implements an interface to ALMA ... still in progress...\
"; }

//! Register parameters this Green's function claims from the input.
/*! This function calls crusde_register_green_param() defined in crusde_api.h to register 
 *  references to parameters this Green's function will need to operate properly.
 *  In case an XML is used to configure the experiment, the reference to this parameter will be identified by 
 *  the string passed as second argument to register_green_param().
 *
 *  This function registers the references in the following order:
 *  -# g, XML config identifier is "g" SI-unit: [m/s^2] 
 *  -# E, XML config identifier is "E" SI-unit: [GPa]
 *  -# nu XML config identifier is "nu" SI-unit: [-]
 *
 *  @see register_green_param()
 *  @see register_parameter() (temp_plugin.c.tmp)
 */
extern void register_parameter()
{
	p_deg_min  = crusde_register_param_double("deg_min", get_category());
	p_deg_max  = crusde_register_param_double("deg_max", get_category());
	p_deg_step = crusde_register_param_double("deg_step", get_category());
	p_file     = crusde_register_optional_param_string("file", get_category(), "");
	p_kv       = crusde_register_param_double("kv", get_category());
	p_lth      = crusde_register_param_double("lth", get_category());
	p_mode     = crusde_register_param_double("mode", get_category());
	p_ng       = crusde_register_param_double("ng", get_category());
	p_nla      = crusde_register_param_double("nla", get_category());
	p_p        = crusde_register_param_double("p", get_category());
	p_rheol    = crusde_register_param_double("rheol", get_category());
	p_sd       = crusde_register_param_double("sd", get_category());
	p_time_min = crusde_register_param_double("time_min", get_category());
	p_time_max = crusde_register_param_double("time_max", get_category());
}

//! Register output fields for spatial directions this Green's function calculates.
/*! This function calls crusde_register_output_field defined in crusde_api.h to register 
 *  output field in the following order:
 *
 *   -# x, y, z (horizontal-x, horizontal-y, vertical).
 */
extern void register_output_fields() 
{ 
    crusde_info("%s registers output fields...", get_name());	

    crusde_register_output_field(&x_pos, X_FIELD);	
    crusde_register_output_field(&y_pos, Y_FIELD);	
    crusde_register_output_field(&z_pos, Z_FIELD);	

    crusde_info("\t x: %d", x_pos);	
    crusde_info("\t y: %d", y_pos);	
    crusde_info("\t z: %d", z_pos);	
}

extern void request_plugins(){}

//! Initialize members that depend on registered values. 
/*! 
 */
extern void init()
{
	/* call ALMA */
	char  alma_call[255];

	if(getenv("ALMA") == NULL){
	    	crusde_error("Error: Environment variable ALMA is not defined!\n\n");
		crusde_exit(-1);
	}

	//construct alma call string
	sprintf(alma_call, "%s/alma -h -l -k -load -salz -deg %d:%d:%d -kv %d -lth %e -mode %d -ng %d -nla %d -p %d -rheol %d -sd %d -time %d:%d", 
			   getenv("ALMA"), 
			   (int) *(p_deg_min), (int) *(p_deg_step), (int) *(p_deg_max), (int) *(p_kv), *(p_lth), (int) *(p_mode), (int) *(p_ng), (int) *(p_nla), 
			   (int) *(p_p), (int) *(p_rheol), (int) *(p_sd), (int) *(p_time_min), (int) *(p_time_max) );

	crusde_info("Calling ALMA:");
	crusde_info( alma_call );

	//perform the actual system call.	
	system(alma_call);

	crusde_debug("reading ALMA output.");

        // need number of time points to alloc memory: this is p_p+1
        int times = ((int) *(p_p) )+1;
        // need number of degrees to alloc memory: ceil( (deg_max-deg_min)/deg_step)
        int degrees = (int) ceil ( (*(p_deg_max) - *(p_deg_min) + 1)/ *(p_deg_step) );

        // need memory for h, k, l
        love_number_h = (float**) malloc (sizeof(float*) * times);
        love_number_k = (float**) malloc (sizeof(float*) * times);
        love_number_l = (float**) malloc (sizeof(float*) * times);

	if(love_number_h == NULL || love_number_k == NULL || love_number_l == NULL) {crusde_bad_alloc();}
		

        int t=-1;
        while(++t < times){
            love_number_h[t] = (float*) malloc (sizeof(float) * degrees);
            love_number_k[t] = (float*) malloc (sizeof(float) * degrees);
            love_number_l[t] = (float*) malloc (sizeof(float) * degrees);

  	    if(love_number_h[t] == NULL || love_number_k[t] == NULL || love_number_l[t] == NULL) {crusde_bad_alloc();}
            
	    //init
            int n = -1;
            while(++n < degrees){
                love_number_h[t][n] = 0.0;
                love_number_k[t][n] = 0.0;
                love_number_l[t][n] = 0.0;
           }
       }

	//fill the arrays with values produced by alma.
	read_alma_output("./h.dat", love_number_h);
	read_alma_output("./k.dat", love_number_k);
	read_alma_output("./l.dat", love_number_l);
}    

//! Not used in this plugin, left empty!
extern void run(){}

//! Clean-up before this plug-in gets unloaded. 
/*! Yet, there is nothing to free here.
 */
extern void clear(){}

//! Returns the Green's Function value at Point(x,y).
/*! TODO!
 
 * @param x The x-Coordinate of the wanted value.
 * @param y The y-Coordinate of the wanted value.
 * 
 * @return The value found at Point[y][x]
 */
extern int get_value_at(double** result, int x, int y)
{	
	return NOERROR;
}


void read_alma_output(const char* file, float **love_array)
{
    FILE * fi;
    char *    alma_header  = "  #";
    const int alma_column_width = 19;
    int degrees = (int) ceil ( (*(p_deg_max) - *(p_deg_min) + 1)/ *(p_deg_step) );
    // need number of time points to alloc memory: this is p_p+1
    int times = ((int) *(p_p) )+1;
    int len          = alma_column_width * (degrees + 1) + 1;
    char buffer[alma_column_width+1];

    //allocate memory for lines that are of length that depends on number of degrees
    char * line = (char *) malloc (len * sizeof(char) );
	
    if (line == NULL){ crusde_bad_alloc(); }

    crusde_debug("Reading ALMA file <%s>, times = %d ", file, times);

    if((fi = fopen( file, "r" )) == NULL)
    {
	crusde_warning("FILE PROBLEM: <%s>", file);
 	perror(file);
	crusde_exit(1);
    }

    /*other problems with file?*/
    if(ferror(fi) != 0)
    {
	crusde_warning("FILE PROBLEM ... <%s>", file);
	perror(file);
	crusde_exit(1);
    }

    /*reset filepointer*/
    rewind(fi);

   
    /*read values, copy to local array*/
    int t = -1;
    while( true )
    { 
	 //read a line of unknown length
	 fgets( line, len, fi );
         //stop if we're at the end of the file
         if( feof(fi) ) {              				  break; }
         //ignore 'empty' lines
         if( strlen(line) <= strlen(alma_header) ) {              continue; }
         //stop once we're beyond the header            
       	 if( !strncmp(line, alma_header, strlen(alma_header) ) ){ continue; }


 	 if (line != NULL){
		 t++;		//we are at a new and relevant line which translates to a new time, hence increment it
		 int n=0;	//count the 'degrees', keep in mind though that the index != the actual degree, would have to do arithmetic to get it!

		 while(++n<=degrees)
		 {
			 //copy each value (as a string) into buffer, because we don't know how many there are we use this option instead of a dynamic
			 //fgets which would be a pain.
			 strncpy(buffer, &line[n*alma_column_width], alma_column_width);
			 //terminate the string!
			 buffer[alma_column_width] = '\0';
			 //now read the float value in the format written by alma
			 sscanf(buffer, "%E", &love_array[t][n-1] );
  	   	         //debug info, what's been read, and stored and so on.
			 crusde_debug("%d, %d read %s - got value %.8e", t, n, buffer, love_array[t][n-1]);
		 }
	 }
     }

     //free memory
     if(line!=NULL) free(line);
}
