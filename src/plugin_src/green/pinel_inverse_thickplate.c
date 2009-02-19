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
 *	G_{r}^{H}(r) = G_{r,elastic}(r) + \frac{g}{2\pi E (1-2\nu)} \int^{\infty}_0{\left[(1-\nu^2)(2\epsilon \frac{A}{D} + 4\nu)\right]
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

#include "crusde_api.h"

double buffer[3];
int x_pos, y_pos, z_pos;
int error_value;

int (*thick_plate)        (double**, int, int) = NULL;


/*external functions*/	
//--------------------------------------------------------------

extern const char* get_name() 	 { return "inverse thick plate (pinel)"; }
extern const char* get_version() { return "0.1"; }
extern const char* get_authors() { return "ronni grapenthin"; }
extern PluginCategory get_category() { return GREEN_PLUGIN; }
extern const char* get_description() { 
	return ("The negative thickplate response."); }

//! Register parameters this Green's function claims from the input.
extern void register_parameter(){}

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

//! Request necessary plugins:  crusde_request_green_plugin("elastic halfspace (pinel)")
/*!
 */
extern void request_plugins()
{
	thick_plate = crusde_request_green_plugin("thick plate (pinel)");
}

//! Initialize members that depend on registered values. 
/*!
 */
extern void init(){
	error_value = NOERROR;
}

//! Not used in this plugin, left empty!
extern void run(){}

//! Clean-up before this plug-in gets unloaded. 
/*! Free the integration workspace!
 */
extern void clear(){}

//! Returns the Green's Function value at Point(x,y)
/*!
 
 * @param x The x-Coordinate of the requested value.
 * @param y The y-Coordinate of the requested value.
 * 
 * @return The value found at Point[y][x]
 */
extern int get_value_at(double** result, int x, int y)
{
	/*get thickplate response*/
	error_value = thick_plate(result,x,y);

	/*stop, if thickplate reports something nasty*/
	if(error_value != NOERROR)	
		return error_value;

	/*vertical displacement*/
		(*result)[z_pos] *= -1.0;
	/*horizontal displacement*/
		(*result)[x_pos] *= -1.0;
		(*result)[y_pos] *= -1.0;

	return NOERROR;	
}
