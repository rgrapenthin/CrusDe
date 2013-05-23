/***************************************************************************
 * File:        ./plugins/green/disk_load.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     21.02.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @ingroup LoadFunction
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file irregular_load.c
 * 
 * Read load from a file that must be in the following format:
 *
 *		Lon Lat Height
 *
 * with Lon and Lat being integers in Lambert coordinates and Height determining the load height 
 * at the point (Lon,Lat).\
 *
 * Comment lines have to start with '#'.
 *
 */
/*@}*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "crusde_api.h"

/**
 * THE LOAD --- trying to be a little memory efficient here. Instead of saving all
 * load values from the give to a gigantic array which is mainly filled with zeros, 
 * the values go into this list.
 */

typedef struct s_load_list_elem 
{
	int x;						/*x-coordinate of this load value*/
	int y;						/*y-coordinate of this load value*/
	double height;				/*the actual load (height) */
	struct s_load_list_elem * next;	/*next element in the list*/

} load_list_elem;

#define NIL ((load_list_elem *)0)

typedef struct s_load_list 
{
	load_list_elem * first; 
	load_list_elem * last;
	load_list_elem * current;
} load_list;


static load_list_elem * add_elem(load_list * list, int x, int y, double height)
{
	/*create new load list element*/
	load_list_elem * p = ( load_list_elem* ) malloc ( sizeof(load_list_elem) );

	if( p == NULL ){ crusde_bad_alloc(); }	
	
	/* set its values*/
	p->next = NIL;
	p->x = x;
	p->y = y;
	p->height = height;

	/* update end of the list */
	if(list->last == NULL)
	{
		list->last = p;
	}
	else
	{
		list->last->next = p;
		list->last = p;
	}

	if(list->first == NULL)
	{
		list->first = p;
		list->current = p;
	}

	/* give it back ... just in case ...*/
	return p;
}

/* get value load_list with point coordinates x,y*/
static double value_at(load_list * list, int x, int y)
{
	
	load_list_elem *e = list->first;

	while(e != NIL)
	{  
		if(e->x == x && e->y == y)
		{
			return e->height;
		}
		e = e->next;
	}

	return 0.0;
}


static void destroy_elems(load_list_elem * i)
{
	//must not try to alloc new memory
	if(i != NIL){
		destroy_elems(i->next);
		free(i);
	}
}

/*load user parameters*/
double *p_rho[N_LOAD_COMPS];		/**< Density of the load	[kg/m^3]	*/
char*  *p_file[N_LOAD_COMPS];		/**< load definition	*/

double rho;		                    /*!< Density of the load	[kg/m^3]	*/

/* internals */
int dS;                 			/*!< Area around point P(x,y)	[m^2]		*/
double rho_dS_const;	            /*!< rho_dS_const = rho * dS	[kg/m] 		 */
int nx=-1, ny=-1;

loadhistory_exec_function history_function[N_LOAD_COMPS];

/*load_list*/
load_list * loads[N_LOAD_COMPS];

int my_id = 0;

extern const char* get_name() 	     { return "irregular load"; }
extern const char* get_version()     { return "0.3"; }
extern const char* get_authors()     { return "ronni grapenthin"; }
extern const char* get_description() { 
	return "Read load from a file that must be in the following format:<br/><br/> \
	<center><code>Lon Lat Height</code></center><br/><br/> \
	with <code>Lon</code> and <code>Lat</code> being integers in Lambert coordinates \
	and <code>Height</code> determining the load height at point (lon,lat).\
<	In case a load history is defined for a simulation it is used to constrain the height for\
	the current time step.\
	"; }
extern PluginCategory get_category() { return LOAD_PLUGIN; }

/*!empty*/
extern void request_plugins()
{
//	disk_load = crusde_request_load_plugin("disk load");
//	disk_load2 = crusde_request_load_plugin("disk load");
}

/*!empty*/
extern void register_output_fields(){}

/*!empty*/
extern void run(){}

/*!frees load array that was read from file*/
extern void clear()
{
	destroy_elems(loads[crusde_get_current_load_component()]->first);
	free(loads[crusde_get_current_load_component()]);
}

//! Initialize members that depend on registered values. 
/*! This function <b>must not</b> be called before register_parameter() unless none of the necessary values depends on 
 *  parameters provided by the user, which are only set after they have been registered. This function is called
 *  some time <b>after</b> register_parameter().
 */
extern void init()
{
	FILE *fi;
//	int first_x=-1;
//	int prev_x=-1, prev_y=-1;
//	int dimX=0, dimY=0;
//	int inc_y;
//	boolean x_set=false, y_set=false;
	float xg,yg,zg;       		/* buffers to store values gotten from 
							   the input file.			*/

	int gridsize = crusde_get_gridsize();
	int x_min = crusde_get_min_x();
	int y_min = crusde_get_min_y();
	int x_max = x_min + crusde_get_size_x() * gridsize;
	int y_max = y_min + crusde_get_size_y() * gridsize;

	my_id = crusde_get_current_load_component();


	loads[my_id] = (load_list*) malloc (sizeof(load_list));
	
	if (loads[my_id] == NULL ) {
		crusde_error("Could not allocate enough memory for load. Reduce region of interest or increase grid cell size. You could try killing unneccesary processes, or getting more memory. Sorry, gotta stop here.");
	}	

	loads[my_id]->first = NIL;
	loads[my_id]->last = NIL;
	loads[my_id]->current = NIL;
	
        crusde_debug("fopen %s", *p_file[my_id]);

	if((fi = fopen( *p_file[my_id], "r" )) == NULL)
	{
		int n=0;
		while(n < N_LOAD_COMPS){
			crusde_warning("(%d) FILE PROBLEM ...: <%s>, addr: %X", n, *p_file[n], (unsigned int) p_file[n]);
			++n;
		}
		perror(*p_file[my_id]);
		crusde_exit(1);
	}

	/*other problems with file?*/
	if(ferror(fi) != 0)
	{
		crusde_warning("FILE PROBLEM ... <%s>", *p_file[my_id]);
		perror(*p_file[my_id]);
		crusde_exit(1);
	}

	/*determine array dimensions and increment from load file*/
//	while(fscanf(fi, "%f%f%f", &yg,&xg,&zg) != EOF)
//	{	
//		/*increment data counter*/
//		if(!x_set){
//		
//			/* if x column repeats for the first time set all possible values*/
//			if(first_x == (int) xg && nx == -1)
//			{
//				nx    = dimX;
//				inc_y = (int) yg - prev_y;
//				x_set = true;
//			}
//			else
//			{	/*inc_x can be calculated at any time but not when first_x==xg --> values are not neighbors*/
//				inc_x = (int) xg - prev_x;
//			}
//
//			/* save first coordinate for later comparisons*/
//			if(first_x == -1)
//			{
//				first_x = (int) xg;
//			}
//			/* buffer previous values for increment determination */
//			prev_x = (int) xg;
//			prev_y = (int) yg;
//			++dimX;
//	
//		}
//
//		++dimY;
//		
//	}/*while ! EOF*/
//
//	/*y dimension == number of lines with %f%f%f format over x dimension*/
//	ny    = (int) dimY/nx;
//	dS = ;

	nx = crusde_get_size_x();
	ny = crusde_get_size_y();
	dS = gridsize*gridsize;

	/*reset filepointer*/
	rewind(fi);
	//get the line
    int result       = 0;
	int line_no      = 0;
	int comment_line = 0;
	int out_of_bounds = 0;
	char *comment = "#";

	/*read values, copy to local array*/
	while( true )
        {

            char st[1024];
	    fgets( st, sizeof(st), fi );
	    if( feof(fi) )
            {
                crusde_info("(%s): read %d data sets, %d in model region, %d comments, %d lines total.", get_name(), line_no-comment_line, line_no-comment_line-out_of_bounds, comment_line, line_no);
		if ( out_of_bounds > 0){
			crusde_warning("%d (%.2f\%) load elements are outside your region of interest and NOT modeled. The results at the edges are not trustworthy. You should increase your model region!", out_of_bounds, ((double) out_of_bounds/ (double)line_no-comment_line) * 100);
		}
                break;
            }

	    ++line_no;

	    //a blank line
	    if( strlen(st) == 1 ){          ++comment_line; continue; }
	    //a comment line
	    if( !strncmp(st, comment, 1) ){ ++comment_line; continue; }

            
            //now go to work
            result = sscanf(st, "%f%f%f", &xg,&yg,&zg );
            if(result == 3)             //three matches, as expected
            {		           //go ahead and add the element

		if(xg >= x_min && xg <= x_max && yg >= y_min && yg <= y_max ){
			add_elem( loads[my_id], 
                         (int) ( ( xg - x_min) / gridsize ), 
                         (int) ( ( yg - y_min) / gridsize ),
                         zg
			);
		}
		else{
			++out_of_bounds;
		}
            }
            else
            {
               crusde_error("(%s): Formatting problem in file <%s>, line %d ", get_name(), *p_file[my_id], line_no);
            }
	}

/*	while(fscanf(fi, "%f%f%f", &xg,&yg,&zg) != EOF)
        {	
            add_elem( loads[my_id], 
                      (int) ( ( xg - x_min) / gridsize ), 
                      (int) ( ( yg - y_min) / gridsize ),
                      zg
                     );
	}
*//*while ! EOF*/

}



//! Register parameters this load function claims from the input.
/*! This function calls register_load_param() defined in crusde_api.h to register 
 *  references to parameters this Green's function will need to operate properly.
 *  In case an XML is used to configure the experiment, the reference to this parameter will be identified by 
 *  the string passed as second argument to register_green_param().
 *
 *  This function registers the references in the following order:
 * 
 *  -# rho, XML config identifier is "rho", SI-unit [kg/m^3]
 *
 *  @see register_load_param()
 *  @see register_parameter() (temp_plugin.c.tmp)
 */
extern void register_parameter()
{
	my_id = crusde_get_current_load_component();

    /* tell main program about parameters we claim from input */
    p_rho[my_id]  = crusde_register_param_double("rho", get_category());
    p_file[my_id] = crusde_register_param_string("file", get_category());
}

//! Returns the Load of a disk at Point (x,y) at time t.
/*! Computes the euclidean distance of Point (x,y) to (center_x, center_y). 
 *  If this distance is less or equal to disk_radius the load will be returned, zero otherwise.
 *  
 * @param x The x-Coordinate of the wanted value.
 * @param y The y-Coordinate of the wanted value.
 * @param t The time-Coordinate of the wanted value (set to NO_TIME in non-temporal modelling environment).
 * 
 * @return The load at Point (x,y,t).
 */
extern double get_value_at(int x, int y, int t)
{
	my_id = crusde_get_current_load_component();

	rho = *p_rho[my_id];

	dS = crusde_get_gridsize()*crusde_get_gridsize();
	rho_dS_const = rho * dS ;

/*	if(history_function[my_id] != NULL)
	{
		return ( history_function[my_id]( value_at( t) * rho_dS_const);
	}
	else
	{
*/
		return (value_at(loads[my_id], x, y) * rho_dS_const);
//	}
}

extern void set_history_function(loadhistory_exec_function history)
{
	history_function[ crusde_get_current_load_component() ] = history;
}
