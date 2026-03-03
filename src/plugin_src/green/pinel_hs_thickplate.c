/***************************************************************************
 * File:        ./plugins/green/pinel_thickplate.c
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     18.04.2007
 * Licence:     GPL
 ****************************************************************************/

/** 
 *  @ingroup Plugin 
 *  @ingroup GreensFunction
 **/

/*@{*/
/** \file pinel_thickplate.c
 *
 * The Earth is modeled as being made of an elastic layer of arbitrary thickness <i>H</i> lying
 * over an inviscid fluid of density <i>&rho;<sub>f</sub></i>. The fully relaxed response of the Earth to
 * load changes is modeled. The fluid is considered to be in equilibrium. We calculate surface displacement
 * for the equation of equilibrium (<i>div(&sigma;)=0</i>) in Lagrangian coordinates.
 * <p>Boundary conditions:<br />
 * ....
 * </p>
 * The convention is such that a compression corresponds to a positive stress. At the lower surface of 
 * of the plate (at <i>z=H</i>) a pressure equal to the hydrostatic pressure in the fluid and no tangential
 * component are applied. Displacements are calculated by integration of Bessel functions. At the surface (<i>z=0</i>)
 * they are expressed by:
 * <p><i><b>
 * \f[
 * 	G_{z}^{H}(r) = G_{z,elastic}(r) + \frac{(1-\nu^2)g}{E\pi} \int^{\infty}_0{(\frac{B}{D} -1) J_0(\epsilon r)d\epsilon}\\
 * \f]
 * \f[
 *	G_{r}^{H}(r) = G_{r,elastic}(r) - \frac{g}{2\pi E (1-2\nu)} \int^{\infty}_0{\left[(1-\nu^2)(2\epsilon \frac{A}{D} + 4\nu)\right]
 *	J_1(\epsilon r)d\epsilon }
 * \f]
 * where \f$J_0\f$ and \f$J_1\f$ are Bessel functions of the first kind of zeroth- and first-order, respectively. 
 * \f$G_{z}^{H}(r)\f$ and \f$G_{r}^{H}(r)\f$ determine vertical and radial final relaxed displacement at 
 * point \f$r\f$, respectively.
 * The coefficients \f$A, B,\f$ and \f$D\f$ are defined by:
 * \f[
 *	A = \epsilon^2 H^2 - \nu (\cosh(2 \epsilon H) - 1) - \frac{2(1-\nu^2)}{E} g \rho_f (\nu \frac{\sinh(2 \epsilon H)}{\epsilon} + H)\\
 * \f]
 * \f[
 *	B = \frac{1}{2} \epsilon \sinh(2 \epsilon H) + \epsilon^2 H^2 + \frac{1-\nu^2}{E} g \rho_f (\cosh(2 \epsilon H) - 1)
 * \f]
 * \f[
 *	D = -\epsilon^3 H^2 + \frac{1}{2} \epsilon (\cosh(2 \epsilon H) - 1) + \frac{1-\nu^2}{E} g \rho_f (\sinh(2 \epsilon H) + 2 \epsilon H)
 * \f]
 * where c\epsilon\f$ is the variable of integration. If \f$H \to \infty\f$, then \f$\frac{A}{D} \to -\frac{2\nu}{\epsilon}\f$ and 
 * \f$\frac{A}{D} \to 1\f$ which gives the solution of the elastic half-space.
 *
 * The Hankel transform integral is calculated in the bounds \f$\[0 .. 0.003\]\f$ using gsl_integration_qag of the 
 * GNU Scientific Library (http://www.gnu.org/software/gsl/).
*/
/*@}*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//gnu scientific lib includes
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_statistics_double.h>

#include "crusde_api.h"

#define SIZE_W		100000

double *p_g;		//!< gravity 				[m/s^2]
double *p_E;		//!< Young's Modulus 			[GPa]	
double *p_nu;		//!< Poisson's ratio 			[-]	
double *p_rho_f;		//!< Density of the inviscid fluid 	[kg/m^3]	
double *p_H;		//!< thickness of the elastic plate	[m]

double g;		//!< gravity 				[m/s^2]
double E;		//!< Young's Modulus 			[GPa]	
double nu;		//!< Poisson's ratio 			[-]	
double rho_f;		//!< Density of the inviscid fluid 	[kg/m^3]	
double H;		//!< thickness of the elastic plate	[m]

double U_vert_const;	//!< Constant part of the vertical displacement equation 
double U_hori_const;	//!< Constant part of the horizontal displacement equation 

double xx                   = 0.0;
double yy                   = 0.0;	
double theta                = 0.0;
double r                    = 0.0;
double local_r              = 0.0;
double result_j0            = -1.0;
double result_j1            = -1.0;
double error                = 0.0;
int quadrant                = -1;
short sin_sign              = -23;
short cos_sign              = -23;
double one_minus_nu_squared = 0.0;
double H_squared            = 0.0;
double H2                   = 0.0;
double g_rho_f_E_term       = 0.0;
double g_rho_f_E_term2      = 0.0;
double nu4                  = 0.0;
double sin_h                = 0.0;
double cos_h                = 0.0;
double A, B, D;
double global_e             = 0.0;

int x_pos, y_pos, z_pos;

gsl_integration_workspace * w = NULL;
gsl_function j0_integral;
gsl_function j1_integral;
/*gsl_function init_h_LUT;
gsl_function init_v_LUT;
*/	
//double *LUT_vertical, *LUT_horizontal;
double lower_integral_bound, upper_integral_bound;
//int eps_index = 0;

int i=0;
//double lower_bound=0.0;
//double upper_bound=0.0;
//double sum_r=0.0;
//double sum_z=0.0;
//double v[10];
//double vH[10];
//int nc=0;

/*local functions*/

//--------------------------------------------------------------

/*initializes lookup-table for vertical and horizontal integral functions*/
/*double init_vertical_LUT(double e, void *params)
{
	sin_h = sinh(2*e*H);
	cos_h = cosh(2*e*H);

	//constants from Pinel et.al. (2007)
	B = 0.5*e*sin_h + e*e*H + ( (1.0-nu*nu)/E )*g*rho_f*(cos_h-1.0); 
	D = -1.0*e*e*e*H*H + 0.5*e*(cos_h-1.0) + ( (1.0-nu*nu)/E )*g*rho_f*(sin_h+2.0*e*H); 

	if(D!=0){
		LUT_vertical[eps_index] = B/D - 1.0;
	}
	else{
		LUT_vertical[eps_index] = 0.0;
	}
	local_r = *((double *) params);	
	
	return ( LUT_vertical[eps_index++] * gsl_sf_bessel_J0(e*local_r) ) ;

}
*/
/*
double init_horizontal_LUT(double e, void *params)
{
	sin_h = sinh(2*e*H);
	cos_h = cosh(2*e*H);

	//constants from Pinel et.al. (2007)
	A = e*e*H*H - nu*(cos_h-1.0) - ( (2.0 * (1.0-nu*nu)) / E ) * g*rho_f*(nu*(sin_h/e)+H); 
	D = -1.0*e*e*e*H*H + 0.5*e*(cos_h-1.0) + ( (1.0-nu*nu)/E )*g*rho_f*(sin_h+2.0*e*H); 

	if(D!=0){
		LUT_horizontal[eps_index] =  (1-nu*nu)*(2.0*e*A/D + nu*4.0);
	}
	else{
		LUT_horizontal[eps_index] 	= 0.0;
	}
	
	local_r = *((double *) params);	
	
	return ( LUT_horizontal[eps_index++] *  gsl_sf_bessel_J1(e*local_r) ) ;

}
*/

double zeroth_order_bessel_integral(double e, void *params)
{
	//cast and dereference
	local_r = *((double *) params);
	sin_h = sinh(2*e*H);
	cos_h = cosh(2*e*H);

	/* constants from Pinel et.al. (2007)*/	
	B = 0.5*e*sin_h + e*e*H + ( (1.0-nu*nu)/E )*g*rho_f*(cos_h-1.0); 
	D = -1.0*e*e*e*H*H + 0.5*e*(cos_h-1.0) + ( (1.0-nu*nu)/E )*g*rho_f*(sin_h+2.0*e*H); 

	if(D!=0){
		return (B/D - 1.0 ) * gsl_sf_bessel_J0(e*local_r);
	}
	else{
		return 0.0;
	}

//	return ( LUT_vertical[eps_index++] * gsl_sf_bessel_J0(e*local_r) ) ;
}

double first_order_bessel_integral(double e, void *params)
{	
	//cast and dereference
	local_r = *((double *) params);	
	sin_h = sinh(2.0*e*H);
	cos_h = cosh(2.0*e*H);

	/* constants from Pinel et.al. (2007)*/	
	A = e*e*H*H - nu*(cos_h-1.0) - ( (2.0 * (1.0-nu*nu)) / E ) * g*rho_f*(nu*(sin_h/e)+H); 
	D = -1.0*e*e*e*H*H + 0.5*e*(cos_h-1.0) + ( (1.0-nu*nu)/E )*g*rho_f*(sin_h+2.0*e*H); 

	if(D!=0.0){
		return  (1.0-nu*nu)*(2.0*e*A/D + nu*4.0) * gsl_sf_bessel_J1(e*local_r);
	}
	else{
		return 0.0;
	}
	
//	return ( LUT_horizontal[eps_index++] *  gsl_sf_bessel_J1(e*local_r) ) ;
}	



double f (double x, void * params) {
       double alpha = *(double *) params;
       double f = log(alpha*x) / sqrt(x);
       return f;
}

/*external functions*/	
//--------------------------------------------------------------

extern const char* get_name() 	 { return "thick plate (pinel)"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return GREEN_PLUGIN; }
extern const char* get_description() { 
	return ("The Earth is modeled as being made of an elastic layer of arbitrary thickness <i>H</i> lying \
	over an inviscid fluid of density <i>&rho;<sub>f</sub></i>. The fully relaxed response of the Earth to \
	load changes is modeled. The fluid is considered to be in equilibrium. We calculate surface displacement \
	for the equation of equilibrium (<i>div(&sigma;)=0</i>) in Lagrangian coordinates. \
	<p>Boundary conditions:<br /> \
	.... \
	</p> \
	The convention is such that a compression corresponds to a positive stress. At the lower surface of \
	of the plate (at <i>z=H</i>) a pressure equal to the hydrostatic pressure in the fluid and no tangential \
	component are applied. Displacements are calculated by integration of Bessel functions. At the surface (<i>z=0</i>) \
	they are expressed by: \
<p><i><b>\
 	G<sub>z</sub><sup>H</sup>(r)  =  G<sub>z, elastic</sub>(r)  &sdot; (1-&nu;<sup>2</sup> g) &frasl; (E &pi;) &sdot; \
					&int;<sub>0</sub><sup>&infin;</sup> (B &frasl; D - 1) &sdot; J<sub>0</sub>(&epsilon;r) d&epsilon; \
</b></i></p> <br /><br />\
<p><i><b>\
 	G<sub>r</sub><sup>H</sup>(r)  =  G<sub>r, elastic</sub>(r)  &sdot; g &frasl; (2&pi; E (1-2&nu;)) &sdot; \
					&int;<sub>0</sub><sup>&infin;</sup> [ (1-&nu;<sup>2</sup>) &sdot; \
					(2&epsilon; A &frasl; D + 4&nu;) ] &sdot; J<sub>1</sub>(&epsilon;r) d&epsilon; \
</b></i></p> <br /><br />\
 	with G<sub>z</sub>(r) and G<sub>r</sub>(r) determine vertical and radial elastic and \
	G<sub>v</sub><sup>H</sup>(r) and G<sub>r</sub><sup>H</sup>(r) determine vertical and \
	horizontal displacement at point r (cylindrical coordinates), respectively (filter coefficients). \
	Coefficients A,B,D are to be looked up in the original paper, for the expressions are too complex \
	provide them in this poor layout.\
"); }

//! Register parameters this Green's function claims from the input.
/*! This function calls register_green_param() defined in crusde_api.h to register 
 *  references to parameters this Green's function will need to operate properly.
 *  In case an XML is used to configure the experiment, the reference to this parameter will be identified by 
 *  the string passed as second argument to register_green_param().
 *
 *  This function registers the references in the following order:
 *  -# g, XML config identifier is "g" SI-unit: [m/s^2] 
 *  -# E, XML config identifier is "E" SI-unit: [GPa]
 *  -# nu, XML config identifier is "nu" SI-unit: [-]
 *  -# rho_f, XML config identifier is "rho_f" SI-unit: [kg/m^3]
 *  -# H, XML config identifier is "H" SI-unit: [m]
 *
 *  @see register_green_param()
 *  @see register_parameter() (temp_plugin.c.tmp)
 */
extern void register_parameter()
{
	p_g     = crusde_register_param_double("g", get_category());
 	p_E     = crusde_register_param_double("E", get_category());
	p_nu    = crusde_register_param_double("nu", get_category()); 
	p_rho_f = crusde_register_param_double("rho_f", get_category()); 
	p_H     = crusde_register_param_double("H", get_category()); 
}

//! Register output fields for spatial directions this Green's function calculates.
/*! This function calls crusde_register_output_field defined in crusde_api.h to register 
 *  output field in the following order:
 *
 *   -# x, y, z (horizontal-x, horizontal-y, vertical).
 */
extern void register_output_fields() 
{ 
	crusde_register_output_field(&x_pos, X_FIELD);	
	crusde_register_output_field(&y_pos, Y_FIELD);	
	crusde_register_output_field(&z_pos, Z_FIELD);	
}

//! Request necessary plugins:
/*!
 */
extern void request_plugins(){}

//! Initialize members that depend on registered values. 
/*!
 */
extern void init()
{ 
	g     = *p_g;		//!< gravity 				[m/s^2]
	E     = *p_E;		//!< Young's Modulus 			[GPa]	
	nu    = *p_nu;		//!< Poisson's ratio 			[-]	
	rho_f = *p_rho_f;	//!< Density of the inviscid fluid 	[kg/m^3]	
	H     = *p_H;		//!< thickness of the elastic plate	[m]

	/* */
	E 			= E*pow(10, 9);
	U_vert_const 		= g/PI * ( (1.0 - nu*nu) / E );
	U_hori_const 		= -1.0 * g / (2*PI * E * (1-2*nu) );

	/*allocate gsl integration workspace*/   
	w = gsl_integration_workspace_alloc (SIZE_W);
	
	j0_integral.function = &zeroth_order_bessel_integral;
	j0_integral.params = &r;
	
	j1_integral.function = &first_order_bessel_integral;
	j1_integral.params = &r;

/*	init_v_LUT.function = &init_vertical_LUT;
	init_v_LUT.params = &r;
		
	init_h_LUT.function = &init_horizontal_LUT;
	init_h_LUT.params = &r;
*/   
    /* make integration more efficient ... create a lookup map for constant parts*/
    /*---------------------------------------------------------------------------*/
//	double res, error;
    	
	/*set upper and lower integration bound */
	lower_integral_bound = 0;
	upper_integral_bound = 0.003;
	
	/*allocate memory for lookup-tables*/
//	LUT_vertical 	= (double*) malloc (sizeof(double) * SIZE_W);
//	LUT_horizontal 	= (double*) malloc (sizeof(double) * SIZE_W);
	
	/*now the lookup-tables can be initialized */
//	eps_index = 0;	//eps_index must be resetted before each integration because it is incremenet inside the integral functions*/
//	gsl_integration_qag (&init_v_LUT, lower_integral_bound, upper_integral_bound, 0, 5e-7, SIZE_W, GSL_INTEG_GAUSS61, w, &res, &error);
//crusde_info("%s: vertical init done ... ", get_name());	
//	eps_index = 0;	//eps_index must be resetted before each integration because it is incremenet inside the integral functions*/
//	gsl_integration_qag (&init_h_LUT, lower_integral_bound, upper_integral_bound, 0, 5e-7, SIZE_W, GSL_INTEG_GAUSS61, w, &res, &error);
//crusde_info("%s: horizontal init done ... ", get_name());	

	/*good night and good luck ... */	   
}

//! Not used in this plugin, left empty!
extern void run(){}

//! Clean-up before this plug-in gets unloaded. 
/*! Free the integration workspace!
 */
extern void clear()
{
//	free(LUT_vertical);
//	free(LUT_horizontal);
	if(w != NULL){
	 	gsl_integration_workspace_free (w);
		w = NULL;
	}
}

//! Returns the Green's Function value at Point(x,y)
/*!
 
 * @param x The x-Coordinate of the requested value.
 * @param y The y-Coordinate of the requested value.
 * 
 * @return The value found at Point[y][x]
 */
extern int get_value_at(double** result, int x, int y)
{
    double error;	/* numerical error in integration*/

	(*result)[x_pos] = 0.0;
	(*result)[y_pos] = 0.0;
	(*result)[z_pos] = 0.0;

	/*well, work then ... */

	/* -expand index to a coordinate: x*crusde_get_gridsize() */
	xx = x*crusde_get_gridsize();
	yy = y*crusde_get_gridsize();
		
	/* -convert carthesian to cylindrical coordinates: r = srqt(x^2+y^2)*/
	r = sqrt(xx*xx + yy*yy);
       
	if(r>0){
	/*horizontal displacement*/
		quadrant = crusde_get_quadrant();
		sin_sign = 1;
		cos_sign = 1;
		
		/* get the angle of the cyllindrical coordinate */
		if(xx == 0){	theta=PI/2;		}
		else{		theta = atan(fabs(yy)/fabs(xx));	}
		
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
			
	/*hankel transforms*/
		//eps_index = 0;
		gsl_integration_qag (&j0_integral, lower_integral_bound, upper_integral_bound, 0, 5e-7, SIZE_W, GSL_INTEG_GAUSS61, w, &result_j0, &error);
		//eps_index = 0;
		gsl_integration_qag (&j1_integral, lower_integral_bound, upper_integral_bound, 0, 5e-7, SIZE_W, GSL_INTEG_GAUSS61, w, &result_j1, &error);

	/*vertical displacement*/
		(*result)[z_pos] = U_vert_const *  result_j0 ;
	/*horizontal displacement*/
		(*result)[x_pos] = cos_sign * cos(theta) * U_hori_const * result_j1;
		(*result)[y_pos] = sin_sign * sin(theta) * U_hori_const * result_j1;
	}	

	return NOERROR;	
}

