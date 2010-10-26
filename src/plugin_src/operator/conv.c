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
#include <stdlib.h> 

double		*load, *green_back;
double 		**green_function, **result, **real_result;

/* *_X: number of rows, *_Y: number of columns*/
int N, N_X, N_Y;
int size_x, size_y;
int dimensions=1;
int displacement_dimensions=1;
int x=-1, xg=-1, xk=-1, y=-1, yg=-1, yk=-1,n=-1, mm=-1, nn=-1;

boolean is_initialized = false;

/* plugin interface */
extern void register_parameter();
extern void register_output_fields();
extern void init();
extern void free();
extern void run();

extern const char* get_name() 	 { return "2d convolution"; }
extern const char* get_version() { return "0.1"; }
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
extern void init(){
   size_x = crusde_get_size_x(); // LONGITUDE
   size_y = crusde_get_size_y(); // LATITUDE

   dimensions = crusde_get_dimensions();
   displacement_dimensions = crusde_get_displacement_dimensions();
   /* find a size long enough and a power of two to allocate memory for DFT*/    
   /*get minimum values for DFT size in x and y direction*/
   /*we take the bigger*/
   crusde_set_operator_space(size_x, size_y);
   
   N_X = 2*size_x;
   N_Y = 2*size_y;
   N=N_X*N_Y;
   
/*------------------------------*/
   load       = (double*) malloc(sizeof(double) * size_x*size_y);
   green_back = (double*) malloc(sizeof(double) * displacement_dimensions);

   green_function = (double**) malloc(sizeof(double*) * dimensions);
   result         = (double**) malloc(sizeof(double)  * dimensions);
   real_result    = (double**) malloc(sizeof(double)  * dimensions);

   n=-1;
   while(++n < dimensions){
	green_function[n] = (double*) malloc(sizeof(double) * size_x*size_y);
   	result[n] 	  = (double*) malloc(sizeof(double) * N);/* size_x * size_y);*/
   	real_result[n] 	  = (double*) malloc(sizeof(double) * size_x*size_y);
	//init
	x = -1;
	while(++x < size_x*size_y){
		result[n][x] = 0.0;
		real_result[n][x] = 0.0;
		green_function[n][x] = 0.0;
	}
	while(++x < N){
		result[n][x] = 0.0;
	}
   }
   
   /*convolution is now initialized*/   
   is_initialized = true;
}

/*!empty*/
extern void clear()
{
   /*if(is_initialized)
   {
	   free(load);
	   free(green_back);   
	   
	   n=-1;
	   while(++n < dimensions){ 
		if(green_function[n] != NULL)
		   	free(green_function[n]); 
		if(result[n] != NULL)
		   	free(result[n]); 
	   }

	   free(green_function);   
	   free(result);   
  }
	*/
}

/*!empty*/
extern void run()
{   
    crusde_info("Calculating Green's function and Load");
    //Get Green's function
    for(x=-size_x/2; x<size_x/2+1; ++x){
       for(y=-size_y/2; y<size_y/2+1; ++y){
	 //GF is time invariant!
         if (crusde_model_time() == 0)
         {	
	    if( (x >= 0) && (y >= 0) ){
			/*set quadrant we're in, in case of cylindrical:carthesian conversion in green's function*/
			crusde_set_quadrant(1);
	    }
	    else{
			/* do origin shift (see paper)*/
			if( (x >= 0 ) && (y < 0) ){
				crusde_set_quadrant(2);
			}
			else
			if( (x < 0 ) && (y < 0) ){
				crusde_set_quadrant(3);
			}
			else
			if( (x < 0) && (y >= 0) ){
				crusde_set_quadrant(4);
			}
	    }

	    crusde_get_green_at(&green_back, x, y);

	     /*copy results to model buffer*/
	     n=-1;
             while(++n<displacement_dimensions){
		green_function[n][x+size_x/2+size_x*(y+size_x/2)] = green_back[n];
	     }
	 }

	 /*third, get the load and do zero padding ... the load is the thing that is time dependent*/
	 load[x+size_x/2+size_x*(y+size_y/2)] = crusde_get_load_at(x+size_x/2, y+size_y/2);
       }
    }
	    
    crusde_info("Doing the Convolution ... ");
   
    // Input side algorithm: http://www.dspguide.com/ch6/3.htm
    for(x=0; x < size_x; ++x){              // rows
    for(y=0; y < size_y; ++y){              // rows
	for(xk=0; xk < size_x; ++xk){     // kernel rows
        for(yk=0; yk < size_y; ++yk){     // kernel rows
	   for(n=0; n<displacement_dimensions; ++n){
	      result[n][(x+xk)+N_X*(y+yk)] += green_function[n][xk+size_x*yk] * load[x+size_x*y];
  	   }
	}
	}
    }
    }

   for(x=0; x < size_x; ++x){
      for(y=0; y < size_y; ++y){
        for(n=0; n<displacement_dimensions; ++n){
          real_result[n][x+size_x*y] = result[n][x+size_x/2+N_X*(y+size_y/2)];
	}
      }
   }
    
/*
   //Output side algorithm, http://www.dspguide.com/ch6/4.htm
   for(x=0; x < N_X; ++x){              // rows
    for(y=0; y < N_Y; ++y){              // rows
	for(xk=0; xk < size_x; ++xk){     // kernel rows
        for(yk=0; yk < size_y; ++yk){     // kernel rows
	   mm = x-xk;
	   nn = y-yk;
	   
	   if(mm>=0 && mm <= size_x && nn>=0 && nn <= size_y){
	      for(n=0; n<displacement_dimensions; ++n){
	         result[n][x+N_X*y] += green_function[n][xk+size_x*yk]*load[mm+size_x*nn];
	      }
  	   }
	}
	}
    }
    }
   for(x=0; x < size_x; ++x){
      for(y=0; y < size_y; ++y){
        for(n=0; n<displacement_dimensions; ++n){
           real_result[n][x+size_x*y] = result[n][x+size_x/2+N_X*(y+size_y/2)];
	 //  real_result[n][x+size_x*y] = green_function[n][x+size_x*y];
	}
      }
   }
*/


crusde_set_result(real_result);
//crusde_set_result(green_function);
}
