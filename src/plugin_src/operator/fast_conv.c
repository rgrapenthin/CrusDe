/***************************************************************************
 * File:        ./plugins/operator/fast_conv.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     20.02.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup Operator Convolution Operator
 *  @ingroup Plugin 
 */
 /*@{*/
/** \file fast_conv.c
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
double		*load_in,  *green_in,  *conv_out, *green_back;
fftw_complex 	*load_out, *green_out, *conv_in;
fftw_plan 	load_plan, green_plan, conv_plan;

double 		**model_buffer, **result;

/* *_X: number of rows, *_Y: number of columns*/
int N, N_X, N_Y;
int size_x, size_y;
int x=-1, y=-1, i=0, n=-1;
unsigned long int t=0;
int dimensions=1;
int displacement_dimensions=1;

boolean is_initialized = false;

/* plugin interface */
extern void register_output_fields();
extern void register_parameter();
extern void request_plugins();
extern void init();
extern void clear();
extern void run();

extern const char* get_name() 	 { return "standard 2d convolution"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern const char* get_description() { return "Performs a fast 2D-convolution of load and Green's function based \
on FFTW3 (http://www.fftw.org)"; }
extern PluginCategory get_category() { return KERNEL_PLUGIN; }
/*!empty*/
extern void request_plugins(){}
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
   size_x = crusde_get_size_x(); // LONGITUDE
   size_y = crusde_get_size_y(); // LATITUDE

   dimensions = crusde_get_dimensions();
   displacement_dimensions = crusde_get_displacement_dimensions();
   /* find a size long enough and a power of two to allocate memory for DFT*/    
   /*get minimum values for DFT size in x and y direction*/
   /*we take the bigger*/
   if(size_x > size_y){
	N_X = 2*size_x;
 	N_Y = N_X;
   }else{
   	N_Y = 2*size_y;
	N_X = N_Y;
   }

   /* find power of 2 that is closest to N_* and greater than or equal to N_* */ 
   while(N_X>pow(2,i)){++i;}
   N_X = pow(2,i);

   N_Y = N_X;

   /* N = total number of DFT elements */
   N = N_X*N_Y;

   crusde_set_operator_space(N_X, N_Y);

/*------------------------------*/
/*Allocation of Memory		*/
/*------------------------------*/
   load_in  = (double*) fftw_malloc(sizeof(double) * N);
   load_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N); // (floor(N/2)+1));

   green_in  = (double*) fftw_malloc(sizeof(double) * N);
   green_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N); //(floor(N/2)+1));
   
   conv_in  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N); // (floor(N/2)+1));
   conv_out = (double*) fftw_malloc(sizeof(double) * N);

   model_buffer = (double**) fftw_malloc(sizeof(double*) * dimensions);
   result       = (double**) fftw_malloc(sizeof(double) * dimensions);
   green_back   = (double*) fftw_malloc(sizeof(double) * displacement_dimensions);

   n=-1;
   while(++n < dimensions){
	model_buffer[n] = (double*) fftw_malloc(sizeof(double) * N);
   	result[n] 	= (double*) fftw_malloc(sizeof(double) /* N);*/ * size_x * size_y);
	//init
	x = -1;
	while(++x < N){
		model_buffer[n][x] = 0.0;
		if(x < size_x * size_y){
			result[n][x] = 0.0;
		}
	}
   }

  
/*------------------------------*/
/*Creation of Deformation Plans	*/
/*------------------------------*/
   /* fftw_plan_dft_r2c_1d is always FFTW_FORWARD, 		*/
   /* fftw_plan_dft_c2c_1d is always FFTW_BACKWARD 		*/ 
   load_plan = fftw_plan_dft_r2c_2d(N_X, N_Y, load_in, load_out, FFTW_ESTIMATE);
   green_plan= fftw_plan_dft_r2c_2d(N_X, N_Y, green_in, green_out, FFTW_ESTIMATE);
   conv_plan = fftw_plan_dft_c2r_2d(N_X, N_Y, conv_in, conv_out, FFTW_ESTIMATE);

   /*convolution is now initialized*/   
   is_initialized = true;
}

/*! Frees memory allocated during init(). FFTW plans are destroyed.*/
extern void clear(){ 
   if(is_initialized)
   {
	   fftw_destroy_plan(green_plan);
	   fftw_destroy_plan(load_plan);   
	   fftw_destroy_plan(conv_plan);
	   
	   fftw_free(green_in);
	   fftw_free(green_out);
	   fftw_free(load_in);
	   fftw_free(load_out);
	   fftw_free(conv_in);
	   fftw_free(conv_out);

	   n=-1;
	   while(++n < dimensions){ 
		if(model_buffer[n] != NULL)
		   	fftw_free(model_buffer[n]); 
		if(result[n] != NULL)
		   	fftw_free(result[n]); 
	   }

	   fftw_free(model_buffer);   
	   fftw_free(result);   
	   fftw_free(green_back);   
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
 * CAN ONLY BE USE FOR ONE LOAD (i.e. multiple loads in one load file), to realize reuse of this 
 * operator in fast_conv_time_space - the loads want to have independent load histories!
 */
extern void run()
{   

   RESET(x,y);
      
   /* 
    * first, get the impulse response, this needs to be done only once, that's why there are two
    * double whiles ... a little inconvenient when modeling only one timestep, but effective for 
    * the other 364 ... depending on whether the Green's function is time dependent though
    */

    t = crusde_model_time();    

    if(t == 0 ){

      while(++y < N_Y){
        x = -1;
        while(++x < N_X){
            
//          if(green_initialized == false) 
//          {

            if( (x <= N_X/2) && (y <= N_Y/2) ){
			/*set quadrant we're in, in case of cylindrical:carthesian conversion in green's function*/
			crusde_set_quadrant(1);
			crusde_get_green_at(&green_back,x,y);
	    }
	    else{
			/* do origin shift (see paper)*/
			if( (x <= N_X/2 ) && (y > N_Y/2) ){
				crusde_set_quadrant(2);
				crusde_get_green_at(&green_back, x, N_Y-y);
			}
			else
			if( (x > N_X/2) && (y > N_Y/2) ){
				crusde_set_quadrant(3);
				crusde_get_green_at(&green_back, N_X-x, N_Y-y);
			}
			else
			if( (x > N_X/2) && (y <= N_Y/2) ){
				crusde_set_quadrant(4);
	 			crusde_get_green_at(&green_back, N_X-x , y);
			}
			else{
				n=-1;
				while(++n < displacement_dimensions){
					green_back[n]=0.0;
				}
			}
	    }
			
            /*copy results to model buffer*/
	    n=-1;
            while(++n<displacement_dimensions){
			model_buffer[n][x+N_X*y] = green_back[n];
	    }

	    /*third, get the load and do zero padding ... the load is the thing that is time dependent*/
            if( (x < size_x) && (y < size_y) ){
			load_in[x+N_X*y] = crusde_get_load_at(x, y);
	    }
	    else{
			load_in[x+N_X*y] = 0.0;
	    }
	 }//end while N_Y
      }//end while N_X
    
//    green_initialized=true; 

   }/*model time == 0*/
   else{
	/*second, get the load and do zero padding ... the load is the thing that is time dependent*/
	while(++y <= N_Y){
		x = -1;
		while(++x <= N_X){
			if( (x <= size_x) && (y <= size_y) ){
				load_in[x+N_X*y] = crusde_get_load_at(x, y);
			}
			//zero pad remains from first model step
		}
	}
   }

crusde_info("%s : planning FFT ...", get_name());

   /* transform green's function results and load values to frequency domain */
   fftw_execute(load_plan);
   	

   n=-1;
   while(++n < displacement_dimensions){   

   	//fill input array with values   	
	i = -1;
	while(++i < N){
		green_in[i] = model_buffer[n][i];
	}
	   
	fftw_execute(green_plan);

	/* ----------------------------------------------------------------- */
	/* convolution in space domain == multiplication in frequency domain */
	/* ----------------------------------------------------------------- */
	
	i = -1;
	while(++i < N){
		/*we're using <complex.h>*/
		/*complex multiplication is z1z2 = (a1 + ib1) (a2 + ib2) = (a1a2 - b1b2) + i(a1b2 + b1a2)*/
		conv_in[i] = (green_out[i] * load_out[i]) / N;
	}
	
	/* inverse transform of the result (back to space domain) */
	fftw_execute(conv_plan);
	
	RESET(x,y);
	/*extract region of interest from conv_result ... remove zero-pad*/
	while(++y < size_y){
		x = -1;
		while(++x < size_x){
			result[n][(int) (x+size_x*y)] = conv_out[x+N_X*y];
		}
	}

   }

   /* give result to main application */
   crusde_set_result(result);
}
