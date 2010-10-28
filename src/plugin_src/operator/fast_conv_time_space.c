/***************************************************************************
 * File:        ./plugins/operator/fast_conv_time_space.c
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     01.06.2008
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup Operator Convolution Operator
 *  @ingroup Plugin 
 */
 /*@{*/
/** \file fast_conv_time_space.c
 * 
 * Implements a fast convolution using the Fast Fourier Transform (DFT). This Plugin
 * depends on the FFTW3 library, http://www.fftw3.org , which is used to allow
 * convolution in the spectral domain (see convolution theorem).
 */
/*@}*/

#include <complex.h>
#include <fftw3.h>
#include "crusde_api.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <math.h>


#define RESET(x,y) (x = y = -1)
#define POSITION(m,n) (n+(N_Y)*m)

/*variables*/
double		   *decay_in,  *history_in,  *conv_out, **spatial_result;
fftw_complex   *decay_out, *history_out, *conv_in;
fftw_plan      decay_plan, history_plan, conv_plan;

double 		   **result;

/* N_T: number of rows*/
int N_T;
int size_T=0, size_X=0, size_Y=0;

int x=-1, y=-1, i=0, n=-1;
unsigned long int t=0;
int dimensions=1;
int displacement_dimensions=1;

boolean is_initialized = false;

/* function pointer to run function of included kernel plug-in */
void (*fast_spatial_conv) () = NULL;

/* plugin interface */
extern void register_output_fields();
extern void register_parameter();
extern void request_plugins();
extern void init();
extern void clear();
extern void run();

extern const char* get_name() 	 { return "fast 3d convolution"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern const char* get_description() { return "Performs a fast 2D-convolution of load and Green's function and a 1D convolution of \
a temporal decay function with a load history based on FFTW3 (http://www.fftw.org)"; }
extern PluginCategory get_category() { return KERNEL_PLUGIN; }

/*! we re-use spatial convolution! */
extern void request_plugins()
{
	fast_spatial_conv = crusde_request_kernel_plugin("fast 2d convolution");
}
/*!empty*/
extern void register_output_fields(){}

/******************************************************************/
/***IMPLEMENTATION*************************************************/
/******************************************************************/

//! Register parameters this Kernel claims from the input.
/*! This plugin calls register_kernel_param() defined in crusde_api.h to register 
 *  references to parameters this Kernel will need to operate properly.
 *  For command line use the order of registration in this function defines the 
 *  identification of parameters in the command line string (i.e. './green -KfirstRegistered/secondRegistered/...').
 *  In case an XML is used to configure the experiment, the reference to this parameter will be identified by 
 *  the string passed as second argument to register_kernel_param().
 *
 *  This function registers the references in the following order:
 *
 *  Thus, the command line parameter string is: <tt>-K</tt>
 *
 *  @see register_kernel_param()
 *  @see register_parameter() (temp_plugin.c.tmp)
 */
extern void register_parameter(){
   /* tell main program about parameters we claim from input */
}

//! Initialization of the convolution. Allocation of memory for inputs and outputs
/**
 * The field sizes are adjusted according to the needs to avoid wrap around
 * effects in the spectal domain and allow for effective use of DFT (lenght and 
 * width are a power of 2). Memory is allocated using fftw_malloc for DFT inputs
 * and outputs (load_in, load_out, green_in, green_out, conv_in, conv_out).
 * Three FFTW plans are created for DFT transform of Green and load arrays and
 * IDFT of the convolution result back to original domain.
 */
extern void init(){
/*------------------------------*/
/*determine dimensions of DFT	*/
/*------------------------------*/
   size_X = crusde_get_size_x();	//Longitude
   size_Y = crusde_get_size_y();	//latitude
   size_T = crusde_get_timesteps();
   N_T    = 2*size_T;

   /* find power of 2 that is closest to N_* and greater than or equal to N_* */ 
   while(N_T>pow(2,i)){++i;}
   N_T = pow(2,i);

   dimensions = crusde_get_dimensions();
   displacement_dimensions = crusde_get_displacement_dimensions();

/*------------------------------*/
/*Allocation of Memory		*/
/*------------------------------*/
   decay_in  = (double*) fftw_malloc(sizeof(double) * N_T);
   decay_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N_T); // (floor(N/2)+1));

   history_in  = (double*) fftw_malloc(sizeof(double) * N_T);
   history_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N_T); //(floor(N/2)+1));
   
   conv_in  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N_T); // (floor(N/2)+1));
   conv_out = (double*) fftw_malloc(sizeof(double) * N_T);

   result       = (double**) fftw_malloc(sizeof(double) * dimensions);

   n=-1;
   while(++n < dimensions){
		result[n] = (double*) fftw_malloc(sizeof(double) * size_X * size_Y * size_T);
		x=-1;
		while(++x < size_X * size_Y * size_T){
			result[n][x] = 0.0;
		}
   }

      
/*------------------------------*/
/*Creation of Deformation Plans	*/
/*------------------------------*/
   /* fftw_plan_dft_r2c_1d is always FFTW_FORWARD, 		*/
   /* fftw_plan_dft_c2r_1d is always FFTW_BACKWARD 		*/ 
   decay_plan =  fftw_plan_dft_r2c_1d(N_T, decay_in, decay_out, FFTW_ESTIMATE);
   history_plan= fftw_plan_dft_r2c_1d(N_T, history_in, history_out, FFTW_ESTIMATE);
   conv_plan =   fftw_plan_dft_c2r_1d(N_T, conv_in, conv_out, FFTW_ESTIMATE);

   /*convolution is now initialized*/   
   is_initialized = true;
}

/*! Frees memory allocated during init(). FFTW plans are destroyed.*/
extern void clear(){ 

   if(is_initialized)
   {
	   fftw_destroy_plan(decay_plan);
	   fftw_destroy_plan(history_plan);   
	   fftw_destroy_plan(conv_plan);
	   
	   fftw_free(decay_in);
	   fftw_free(decay_out);
	   fftw_free(history_in);
	   fftw_free(history_out);
	   fftw_free(conv_in);
	   fftw_free(conv_out);

	   n=-1;
	   while(++n < dimensions){ 
		if(result[n] != NULL)
		   	fftw_free(result[n]); 
	   }
		   
	   fftw_free(result);

           is_initialized = false;
   }
}

//! Performs the fast convolution
/** 
 * Requests values for each point in the examined area from Green's function (only once when modeltime == 0) 
 * and load function (every timestep). Does the origin shifting necessary for the Green's function values.
 * The Green's function values are requested for each displacement direction the Green's function 
 * registered with the CrusDe. 
 * 
 * FFTW plan is executed for the load. Then for each Green's function displacement direction the FFTW plan 
 * is executed, a complex multiplication of the DFT results performed and then for product the FFTW plan
 * is executed (IDFT). The result of the convolution is extracted from the oversized IDFT result and 
 * cropped to the original size of the examined region.
 *
 * ATTN: result[d][x] still holds the values from the last time run was called. This way adding up the results
 * of multiple jobs is realized!
 */
extern void run()
{   

   RESET(x,y);

 	//int cast does nothing bad, since power of 2!
 
   /* 
    * first, get the impulse response, this needs to be done only once, that's why there are two
    * double while loops ... a little inconvenient when modeling only one timestep, but effective for 
    * the other 364 ... depending on whether the Green's function is time dependent though
    */

	/* 
     * - get model time and number of loads
     * - iterate through loads:
     *    - invoke the spatial respone (make sure green's function is calculated not only on time dependence)
     *    - get a pointer to the result via crusde_get_result()
     *    - do decay-history convolution for the load history that belongs to this load
     *    - multiply spatial response with time convolution and add to already existing 
	 *      responses to previous loads in results_array.
	 */

  if(crusde_model_time() == 0){

    int num_loads = crusde_get_number_of_loads();
    n = -1;

    /* iterate through loads ... */
    while(++n < num_loads)
	{
		/* set the load id we want to work with ...*/
        crusde_set_current_load_component(n);
		/* invoke the spatial response */
        fast_spatial_conv();
		/* get a pointer to the result via crusde_get_result()*/
        spatial_result = crusde_get_result();

		/* here comes the fun part - four possibilities exist 
         * for the current load component:
		 *      - 1) crustal_decay = no, load_history = no:
		 *           This is the multi load case that is run without any temporal 
		 *           modeling. It is useful if users want to keep their load models
         *           separate; even if only one timestep is modeled. 
		 *      - 2) crustal_decay = yes, load_history = no:
		 *           This is useful to simulate gradual subsidence due to static loads.
		 *           Theoretically we would do a convolution of decay function 
		 *           with a delta function. Since the delta function is the 
         *           identity operator of the convolution, we get the same
         *           decay function as a result, hence we can skip the 
		 *           convolution and multiply the spatial result directly with
         *           the decay function.
		 *      - 3) crustal_decay = no, load_history = yes:
		 *           This is useful to simulate the pure elastic response to a changing load
	     *           Same as for 2) with changed functions.
		 *      - 4) crustal_decay = yes, load_history = yes:
         *           Now we actually have to work and do a 1D convolution of those two functions.
		 */	
		
		boolean got_history = crusde_load_history_given();
		boolean got_decay   = crusde_crustal_decay_given();
	
	//CASE 1
		if( !got_decay && !got_history)
		{
			crusde_debug("%s: CASE 1\n", __FILE__);
			t = -1;	
			while(++t < size_T)
			{
				//we use conv_out, because it is used below anyway, that shortens the code
				conv_out[t]   = 1;
				crusde_debug("%s: conv_out[%lu] = %f \n", __FILE__, t, conv_out[t]);
			}
		}

	//CASE 2
		if( got_decay && !got_history)
		{
			crusde_debug("%s: CASE 2\n", __FILE__);
			t = -1;	
			while(++t < size_T)
			{
				//we use conv_out, because it is used below anyway, that shortens the code
				conv_out[t]   = crusde_get_crustal_decay_at(t);

			crusde_debug("%s: conv_out[%lu] = %f \n", __FILE__, t, conv_out[t]);
			}
		}

	//CASE 3
		if( !got_decay && got_history)
		{
			crusde_debug("%s: CASE 3\n", __FILE__);
			t = -1;				
			while(++t < size_T)
			{
				//we use conv_out, because it is used below anyway, that shortens the code
				conv_out[t]   = crusde_get_load_history_at(t);
			
				crusde_debug("%s: conv_out[%lu] = %f \n", __FILE__, t, conv_out[t]);
			}			
		}
		
	//CASE 4
		if( got_decay && got_history)
		{
			crusde_debug("%s: CASE 4\n", __FILE__);

			t = -1;				
			while(++t < N_T)
			{
				if( t < size_T ){
					decay_in[t]   = crusde_get_crustal_decay_at(t);
					history_in[t] = crusde_get_load_history_at(t);
				}
				else{
					decay_in[t]   = 0.0;
					history_in[t] = 0.0;
				}
			}

	        /* transform decay and history function results to frequency domain */
	        fftw_execute(decay_plan);
	        fftw_execute(history_plan);
   	
	        /* ----------------------------------------------------------------- */
	        /* convolution in space domain == multiplication in frequency domain */
	        /* ----------------------------------------------------------------- */
	        i = -1;
	        while(++i < N_T){
			    /*we're using <complex.h>*/
			    /*complex multiplication is z1z2 = (a1 + ib1) (a2 + ib2) = (a1a2 - b1b2) + i(a1b2 + b1a2)*/
	            conv_in[i] = (decay_out[i] * history_out[i]) / N_T;
	        }

	        /* inverse transform of the result (back to space domain) ... we got the result now*/
	        fftw_execute(conv_plan);
		} //END CASE 4

		/* conv_out now contains the values that fit the actual purpose ... and all becomes one ... */
		t = -1;

		while(++t < size_T)
		{
//	crusde_debug("%s: conv_out[%d] = %f\n", __FILE__, t, conv_out[t]);
                    int d = -1;
                    while(++d < displacement_dimensions) 
                    {
                       y=-1;
                       while(++y < size_Y)
                       {	
                          x=-1;
                          while(++x < size_X)
                          {	
                             //since we live in an ideal world we can use the principle of superposition right here.
                             result[d][t+size_T*(x+size_X*y)] += (conv_out[t] * spatial_result[d][x+size_X*y]);

//crusde_debug("t=%d, d=%d, x=%d, y=%d, size_X=%d, size_Y=%d, RESULT: %f\n", t, d, x, y, size_X, size_Y, result[d][t+size_T*(y+size_Y*x)]);
                          }
                       }
                    }
                }
	}//END while loads ...
  }// end zeroth timestep


  // now we are basically done, except that our result array is kinda goofy and
  // we follow the convention of the simulation handler being the clock, 
  // and the data output plug-ins write on every time step, we rearrange the
  // results on every timestep and give them to the main application.
  t=crusde_model_time();
 
  spatial_result = crusde_get_result();

  int d= -1;
  while(++d < displacement_dimensions) 
  {
	y=-1;
	while(++y < size_Y)
        {	
           x=-1;
           while(++x < size_X)
           {	
              spatial_result[d][x+size_X*y] = result[d][ crusde_model_time() + size_T*(x+size_X*y) ];
           }
        }
  }

  /* give result to main application */
 // crusde_set_result(spatial_result);
}
