/***************************************************************************
 * File:        ./plugins/green/pinel_hs_elastic.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     20.02.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @defgroup GreensFunction Green's Functions
 *  @ingroup Plugin 
 **/

/*@{*/
/** \file pinel_hs_elastic.c
 * 
 * Implements the 'elastic halfspace' case as described by 
 * Pinel et al (2006) for horizontal and vertical displacement:
 * \f[
 *	G_{z}(r)  = \frac{g}{\pi} \frac{(1-\nu ^2)}{E} \frac{1}{r}
 * \f]
 * \f[
 *	G_{r}(r)  = -\frac{g}{2\pi} \frac{(1+\nu)(1-2\nu)}{E} \frac{1}{r}
 * \f]
 * with \f$G_{z}(r)\f$ and \f$G_{r}(r)\f$ determining vertical and radial instantaneous displacement at point \f$r\f$, respectively.
 *
 *  The sequence in which the functions are called from the main program is as follows:
 *  -# register_parameter()
 *  -# init()
 *  -# get_value_at(int, int, int)
 *  -# clear()
 */
/*@}*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "crusde_api.h"

double* p_g;		//!< gravity 		[m/s^2]
double* p_E;		//!< Young's Modulus 	[GPa]	
double* p_nu;		//!< Poisson's ratio 	[-]	

double g;		//!< gravity 		[m/s^2]
double E;		//!< Young's Modulus 	[GPa]	
double nu;		//!< Poisson's ratio 	[-]	

double U_vert_const;	//!< Constant part of the vertical displacement equation 
double U_hori_const;	//!< Constant part of the horizontal displacement equation 

double xx;
double yy;
double theta;
double r;
int quadrant;
short sin_sign;
short cos_sign;

int x_pos, y_pos, z_pos;

double backpack[3];
	
extern const char* get_name() 	 { return "elastic halfspace (pinel)"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return GREEN_PLUGIN; }
extern const char* get_description() { 
	return "This Green's function implements the 'elastic halfspace' Earth model \
	case as described by Pinel et al. (2006) for horizontal and vertical displacement: \
	<br /><br />\
<p><i><b>\
 	G<sub>z</sub>(r)  =  g &frasl; &pi; &sdot; (1-&nu;<sup>2</sup>) &frasl; E &sdot; 1 &frasl; r \
</b></i></p> <br /><br />\
<p><i><b>\
 	G<sub>r</sub>(r)  =  - g &frasl; 2&pi; &sdot; ( (1+&nu;)(1-2&nu;) &frasl; E &sdot; 1 &frasl; r \
</b></i></p> <br /><br />\
 	with G<sub>z</sub>(r) and G<sub>r</sub>(r) determine vertical and radial displacement at \
	point r (cylindrical coordinates), respectively (filter coefficients).\
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
	p_g = crusde_register_param_double("g", get_category());
	p_E = crusde_register_param_double("E", get_category());
	p_nu = crusde_register_param_double("nu", get_category());
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
/*! This function <b>must not</b> be called before register_parameter() unless none of the necessary values depends on 
 *  parameters provided by the user, which are only set after they have been registered. This function is called
 *  some time <b>after</b> register_parameter().
 *
 * Here U_vert_const and U_hori_const are initialized as follows:
 * \f[
 *	U_{z,const}  =  \frac{g}{\pi} \frac{(1-\nu ^2)}{E}
 * \f]
 * \f[
 *	U_{r, const} =  -\frac{g}{2\pi} \frac{(1+\nu)(1-2\nu)}{E}
 * \f]
 */
extern void init()
{
	/* */
	g = *(p_g);
	E = *(p_E);
	nu = *(p_nu);
	E = E*pow(10, 9);
	U_vert_const = g/PI * ( (1.0 - nu*nu) / E );
	U_hori_const = -1 * g/(2*PI) * ( (1.0+nu) * (1-2*nu) / E );
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
 * @return error code
 */
extern int get_value_at(double** result, int x, int y)
{	
	(*result)[x_pos] = 0.0;
	(*result)[y_pos] = 0.0;
	(*result)[z_pos] = 0.0;
	
	/* CAREFUL! sqrt takes only 'double' and the autocast mechanism lead to weird results, thus do the 
	 * expansion separately and save it in double typed vars which in turn can be used in the conversion 
	 * this way using pow(x*gridsize,2) is avoided which is a function call, thus more expensive than 
	 * a basic operator.*/
	
	/* -expand index to a coordinate: x*crusde_get_gridsize() */
	xx = x*crusde_get_gridsize();
	yy = y*crusde_get_gridsize();
		
	/* -convert carthesian to cylindrical coordinates: r = srqt(x^2+y^2)*/
	double r = sqrt(xx*xx + yy*yy);
	
	if(r>0){
	/*vertical displacement*/
		(*result)[z_pos] = (U_vert_const / r);
	/*horizontal displacement*/
		quadrant = crusde_get_quadrant();
		sin_sign = 1;
		cos_sign = 1;
		
		/* get the angle of the cyllindrical coordinate */
		if(xx == 0){	theta=PI/2;		}
		else{		theta = atan(fabs(yy)/fabs(xx));}
		
		/* get the direction of the direction cosine */
		if(quadrant==2){	
			sin_sign = -1;
		}
		else if(quadrant==3){	
			sin_sign = -1;
			cos_sign = -1;
		}
		else if(quadrant==4){	
			cos_sign = -1;
		}
			
	/*horizontal displacement*/
		(*result)[x_pos] = (cos_sign * cos(theta) * U_hori_const / r);
		(*result)[y_pos] = (sin_sign * sin(theta) * U_hori_const / r);
	}
	
	return NOERROR;
}
