/***************************************************************************
 * File:        ./ModelRegion.cpp
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     2009-11-23
 * Licence:     GPLv2
 *
 * #########################################################################
 *
 * CrusDe, simulation framework for crustal deformation studies
 * Copyright (C) 2007-today Ronni Grapenthin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * -------------------------------------------------------------------------
 * Description: stor:
 *			- check whether we're dealing with an XML file
 *			- if not, check whether we're having some params given
 ****************************************************************************/

#include "SimulationCore.h"
#include "ModelRegion.h"

#include <stdlib.h>	/*atof*/
#include <stdio.h>
#include <unistd.h>	/*getopt*/
#include <string.h>	/*strtok*/
#include <ctype.h>	/*isprint*/

#include <string>	
#include <math.h>	
#include <map>
#include <stdexcept>
#include <algorithm>

#include "info.h"
#include "constants.h"

#include "GeographicLib/GeoCoords.hpp"
#include "GeographicLib/Geodesic.hpp"

using namespace std;
using namespace GeographicLib;

ModelRegion::ModelRegion(const string the_name):
     name(the_name),
     north(0.0), south(0.0), east(0.0), west(0.0),
     gridsize(0.0),
     roi_is_geographic(false),
     gc_UL(),
     gc_LR()
{
     crusde_debug("%s, line: %d, ModelRegion built with name '%s' NW=%s, SE=%s ... gridsize=%f", __FILE__, __LINE__, name.c_str(), gc_UL.DMSRepresentation().c_str(), gc_LR.DMSRepresentation().c_str(), gridsize );
}

ModelRegion::~ModelRegion()
{
     crusde_debug("%s, line: %d, ModelRegion destroyed.", __FILE__, __LINE__);
}


//strings come directly from xml config, not altered has been done on them.
void ModelRegion::setROI(string north_s, string south_s, string east_s, string west_s)
{

	float N(0.0);
	float S(0.0);
	float E(0.0);
	float W(0.0);
	
	crusde_debug("%s, line: %d, ModelRegion::setRegionDegrees: got north=%s, south=%s, east=%s, west=%s", __FILE__, __LINE__, north_s.c_str(), south_s.c_str(), east_s.c_str(), west_s.c_str());
	
	//build string to scan with sscanf
	string NSEW(north_s);
	NSEW.append(" ").append(south_s).append(" ").append(east_s).append(" ").append(west_s);

	//do it.
	int result( sscanf(NSEW.c_str(), "%f%f%f%f", &N, &S, &E, &W) );

	//if we couldn't scan 4 floats, the content is hopefully strings, so try converting these using GeoCoords
	if( result != 4){
		crusde_debug("%s, line: %d, ModelRegion::setROI Could not convert coordinates to floats. Trying GeoCoods conversions ... ", __FILE__, __LINE__);
			//create upper left (UL) and lower right (LR) points of ROI
		try{
			//input to GeoCoords must be const, throws else
			const string ul = (north_s.append(" ")).append(west_s);
			const string lr = (south_s.append(" ")).append(east_s);
			
			crusde_debug("%s, line: %d, ModelRegion::setROI ul = %s, lr = %s ", __FILE__, __LINE__, ul.c_str(), lr.c_str());

			gc_UL.Reset(ul);
			gc_LR.Reset(lr);
			
			north = gc_UL.Latitude();
			south = gc_LR.Latitude();
			east  = gc_LR.Longitude();
			west  = gc_UL.Longitude();

			roi_is_geographic = true;
			
			crusde_debug("%s, line: %d, ModelRegion::setROI NW=(%s), SE=(%s) ", __FILE__, __LINE__, gc_UL.GeoRepresentation().c_str(), gc_LR.GeoRepresentation().c_str() );
		}
		catch (const std::exception& e) {
			//try to check whether we're just having an out of range problem
			crusde_error("Coordinate formatting problem. %s", e.what());
		}
	}
	//we got 4 floats and need to decide whether they are geographic degrees or meter values
	else{
		//check whether incoming floats are within lat-lon ranges and therefore geographic degrees
		if(	N <= 90.0  && N >= -90.0  && S <= 90.0  && S >= -90.0 && 
			E <= 360.0 && E >= -360.0 && W <= 360.0 && W >= -360.0 ) 
		{
			crusde_debug("%s, line: %d, ModelRegion::setROI INPUT COORDS assumed to be lat/lon floats ", __FILE__, __LINE__ );
			gc_UL.Reset(N, W);
			gc_LR.Reset(S, E);
			roi_is_geographic = true;
		}
		else
		{
			crusde_debug("%s, line: %d, ModelRegion::setROI INPUT COORDS assumed to be metric floats", __FILE__, __LINE__ );
			roi_is_geographic = false;
		}
		
		north = N;
		south = S;
		east  = E;
		west  = W;
	}
	
	if(north <= south){ crusde_error("In the region definition, the North coordinate ended up smaller than South. I could try swapping things around, but I'd rather have you do this.");	}
	if(east  <= west ){ crusde_error("In the region definition, the East coordinate ended up smaller than West. I could try swapping things around, but I'd rather have you do this.");	}
			
	crusde_debug("%s, line: %d, ModelRegion::setROI After coordinate reading: ", __FILE__, __LINE__ );
	crusde_info("found region of interest coordinates to be in %s: north=%f, south=%f, east=%f, west=%f", (roi_is_geographic ? "GEOGRAPHIC DEGREES" : "METERS"), north, south, east, west);
}


void ModelRegion::setGridSize(string gridsize_s)
{
	float gs(0.0);
	int result( sscanf(gridsize_s.c_str(), "%f", &gs) );
	
	if(result == 1){
		gridsize = gs;	
		crusde_debug("%s, line: %d, ModelRegion::setGridSize set gridsize to %f", __FILE__, __LINE__ , gridsize);
	}
	else{
		crusde_error("Gridsize formatting problem. Expecting float");
	}
}

/*
 * GETTERS
 */

int ModelRegion::getSizeX()
{
	if(roi_is_geographic){
		return 0.0;
	}
	else{
		assert(gridsize != 0);
		return static_cast<int>( ceil( static_cast<double>((fabs(east) - west) / gridsize ) ) ) + 1 ;
	}
}

int ModelRegion::getSizeY()
{
	if(roi_is_geographic){
		return 0.0;
	}
	else{
		assert(gridsize != 0);
		return static_cast<int>( ceil( static_cast<double>((fabs(north) - south) / gridsize ) ) ) + 1 ;
	}
}

int ModelRegion::getMinX()
{
	if(roi_is_geographic){	return 0.0;	}
	else{			return west;	}
}

int ModelRegion::getMinY()
{
	if(roi_is_geographic){	return 0.0;	}
	else{			return south;	}
}

int ModelRegion::getGridSize(){	return gridsize;}


int ModelRegion::getLonDistance()
{
/*	int dlon_south = floor( distVincenty(south_deg, west_deg, south_deg, east_deg) ); //= 716234.140 m 
	int dlon_north = ceil( distVincenty(north_deg, west_deg, north_deg, east_deg) ); //= 616418.417 m

	crusde_debug("%s, line: %d, Got dlon_south=%d (=716234), dlon_north=%d (=616419)", __FILE__, __LINE__, dlon_south, dlon_north);
	
	if(dlon_south == -1 ){
		crusde_error("%s, line: %d, ModelRegion::distVincenty did not converge for longitudinal distance south (%f,%f,%f,%f) ", 
				__FILE__, __LINE__, south_deg, west_deg, south_deg, east_deg);
	}
	if(dlon_north == -1 ){
		crusde_error("%s, line: %d, ModelRegion::distVincenty did not converge for longitudinal distance north (%f,%f,%f,%f) ", 
				__FILE__, __LINE__, south_deg, west_deg, south_deg, east_deg);
	}
	
	return (dlon_south > dlon_north ? dlon_south : dlon_north);
*/
	return 0.0;
}

int ModelRegion::getLatDistance()
{
/*
	//latitudes are the same for varying longitudes
	int dlat = ceil( distVincenty(south_deg, east_deg, north_deg, east_deg) );  //= 445969.879 m

	crusde_debug("%s, line: %d, Got dlat=%d (=445969)", __FILE__, __LINE__, dlat);

	if(dlat == -1 ){
		crusde_error("%s, line: %d, ModelRegion::distVincenty did not converge for latitudinal distance (%f,%f,%f,%f) ", 
				__FILE__, __LINE__, south_deg, east_deg, north_deg, east_deg);
	}
	
	return dlat;
*/
	return 0.0;
}


//int ModelRegion::getGridsizeMetric()
//{
/*	double avg_lat = (north_deg + south_deg)/2;
	double avg_lon = (east_deg + west_deg)/2;

	crusde_debug("%s, line: %d, ModelRegion::getGridsizeMetric(): avg_lat=%f, avg_lon=%f", __FILE__, __LINE__, avg_lat, avg_lon);


	int gridsize_lat = ceil( distVincenty(avg_lat, avg_lon, avg_lat+gridsize_deg, avg_lon) );
	int gridsize_lon = ceil( distVincenty(avg_lat, avg_lon, avg_lat, avg_lon+gridsize_deg) );

	crusde_debug("%s, line: %d, ModelRegion::getGridsizeMetric(): gridsize_lat=%d, gridsize_lon=%d, gridsize_avg=%d", __FILE__, __LINE__, gridsize_lat, gridsize_lon, 
			( int ) ceil((gridsize_lon+gridsize_lat)/2) );

	if( gridsize_lat == -1 ){
		crusde_error("%s, line: %d, ModelRegion::distVincenty did not converge for latitudinal gridsize (%f,%f,%f,%f) ", 
				__FILE__, __LINE__, avg_lat, west_deg, avg_lat, east_deg);
	}
	if( gridsize_lon == -1 ){
		crusde_error("%s, line: %d, ModelRegion::distVincenty did not converge for latitudinal gridsize (%f,%f,%f,%f) ", 
				__FILE__, __LINE__, south_deg, avg_lon, north_deg, avg_lon);
	}
	
	

	return ceil((gridsize_lon+gridsize_lat)/2);
*/
//	return 0.0;
//}


/*double ModelRegion::distVincenty(double lat1, double lon1, double lat2, double lon2)
{
  double a = 6378137.0, b = 6356752.3142,  f = 1/298.257223563;  // WGS-84 ellipsiod
  double L = (lon2-lon1) * PI / 180;				 // difference in longitude, positive east
  double U1 = atan((1-f) * tan(lat1 * PI / 180));		 // reduced latitudes
  double U2 = atan((1-f) * tan(lat2 * PI / 180));
  double sinU1 = sin(U1), cosU1 = cos(U1);
  double sinU2 = sin(U2), cosU2 = cos(U2);
  
  double lambda = L;						 // difference in longitude on auxiliary sphere

  double lambdaP, iterLimit = 100;

  double sinLambda, cosLambda, sinSigma, cosSigma, sigma, sinAlpha, cosSqAlpha, cos2SigmaM, C;

  do {
    sinLambda = sin(lambda);
    cosLambda = cos(lambda);
    
    sinSigma = sqrt((cosU2*sinLambda) * (cosU2*sinLambda) + (cosU1*sinU2-sinU1*cosU2*cosLambda) * (cosU1*sinU2-sinU1*cosU2*cosLambda));
    
    if (sinSigma==0) return 0;  // co-incident points

    cosSigma   = sinU1*sinU2 + cosU1*cosU2*cosLambda;
    sigma      = atan2(sinSigma, cosSigma);
    sinAlpha   = cosU1 * cosU2 * sinLambda / sinSigma;
    cosSqAlpha = 1 - sinAlpha*sinAlpha;
    cos2SigmaM = cosSigma - 2*sinU1*sinU2/cosSqAlpha;
    if (isnan(cos2SigmaM)) cos2SigmaM = 0;  // equatorial line: cosSqAlpha=0 (§6)

    C = f/16*cosSqAlpha*(4+f*(4-3*cosSqAlpha));
    lambdaP = lambda;
    lambda = L + (1-C) * f * sinAlpha * (sigma + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)));

  } while (fabs(lambda-lambdaP) > 1e-12 && --iterLimit>0);	//

  if (iterLimit==0) return -1;  // formula failed to converge

  double uSq = cosSqAlpha * (a*a - b*b) / (b*b);
  double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
  double B = uSq/1024 * (256+uSq*(-128+uSq*(74-47*uSq)));
  double deltaSigma = B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)- B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
  double s = b*A*(sigma-deltaSigma);
  
//  s = s.toFixed(3); // round to 1mm precision
  return s;

}
*/

