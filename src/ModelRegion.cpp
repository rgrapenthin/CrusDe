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

using namespace std;

ModelRegion::ModelRegion(const string the_name):
     name(the_name),
     north_deg(0.0),
     south_deg(0.0),
     east_deg(0.0),
     west_deg(0.0),
     gridsize_metric(0.0)
{
     crusde_debug("%s, line: %d, ModelRegion built with name %s N=%f, S=%f, E=%f, W=%f ", __FILE__, __LINE__, name.c_str(), north_deg, south_deg, east_deg, west_deg );
     gridsize_deg[0] = 0.0;
     gridsize_deg[1] = 0.0;

     tmp_point[0] = 0.0;
     tmp_point[1] = 0.0;

}

ModelRegion::~ModelRegion()
{
     crusde_debug("%s, line: %d, ModelRegion destroyed.", __FILE__, __LINE__);
}

void ModelRegion::setRegionDegrees(double north, double south, double east, double west)
{

      crusde_debug("%s, line: %d, ModelRegion::setRegionDegrees: got north=%f, south=%f, east=%f, west=%f", __FILE__, __LINE__, north, south, east, west);
	
	north_deg = north;
	south_deg = south;
	east_deg = east;
	west_deg = west;
}

void ModelRegion::setGridsizeMetric(int gridsize)
{
	gridsize_metric = gridsize;
}


/*
 * GETTERS
 */

double ModelRegion::getEastDegrees(){ return east_deg;}

int ModelRegion::getLonDistance()
{
	int dlon_south = floor( distVincenty(south_deg, west_deg, south_deg, east_deg) ); //= 716234.140 m 
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
}

int ModelRegion::getLatDistance()
{

	//latitudes are the same for varying longitudes
	int dlat = ceil( distVincenty(south_deg, east_deg, north_deg, east_deg) );  //= 445969.879 m

	crusde_debug("%s, line: %d, Got dlat=%d (=445969)", __FILE__, __LINE__, dlat);

	if(dlat == -1 ){
		crusde_error("%s, line: %d, ModelRegion::distVincenty did not converge for latitudinal distance (%f,%f,%f,%f) ", 
				__FILE__, __LINE__, south_deg, east_deg, north_deg, east_deg);
	}

	return dlat;	
}


int ModelRegion::getDistMinLon(double lat, double lon)
{
	return ceil(distVincenty(lat, lon, lat, west_deg));

}

int ModelRegion::getDistMinLat(double lat, double lon)
{
	return ceil(distVincenty(lat, lon, south_deg, lon));
}

double ModelRegion::getRegionBound(RegionBound bound)
{
	if(      bound == NORTH)
		return north_deg;
	else if (bound == SOUTH)
		return south_deg;
	else if (bound == EAST)
		return east_deg;
	else if (bound == WEST)
		return west_deg;
	else
		crusde_error("%s, line: %d, ModelRegion::getRegionBound: Unrecognized region bound %d", __FILE__, __LINE__, bound);
}

double* ModelRegion::getGridsizeGeographic()
{
	double avg_lat = (north_deg + south_deg)/2;
	double avg_lon = (east_deg + west_deg)/2;

	//from midpoint find points g
	double *target_lat, *target_lon;

	//do exactly like that: get destVincenty and then assign value to gridsize_deg. Otherwise old result is being overwritten
	//we're handling pointers, right!
	target_lat = destVincenty(avg_lat, avg_lon, 180.0, gridsize_metric);
	gridsize_deg[0] = fabs(avg_lat-target_lat[0]);

	target_lon = destVincenty(avg_lat, avg_lon,  90.0, gridsize_metric);
	gridsize_deg[1] = fabs(avg_lon-target_lon[1]);

	crusde_debug("%s, line: %d, ModelRegion::getGridsizeGeographic(): gridsize_lat=%f, gridsize_lon=%f", __FILE__, __LINE__, gridsize_deg[0], gridsize_deg[1]);
	
	return gridsize_deg;
}


int ModelRegion::getGridsizeMetric()
{
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
	return (gridsize_lon+gridsize_lat)/2;
*/	

	return gridsize_metric;

}

/*
 * Calculate geodesic distance (in m) between two points specified by latitude/longitude 
 * (in numeric degrees) using Vincenty inverse formula for ellipsoids. 
 * Adapted from JavaScript solution given by Chris Veness.
 */
double ModelRegion::distVincenty(double lat1, double lon1, double lat2, double lon2)
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


/*
 * Calculate destination point given start point lat/long (numeric degrees), 
 * bearing (numeric degrees) & distance (in m).
 *
 * from: Vincenty direct formula - T Vincenty, "Direct and Inverse Solutions of Geodesics on the 
 *       Ellipsoid with application of nested equations", Survey Review, vol XXII no 176, 1975
 *       http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
 *
 * Adapted from JavaScript solution given by Chris Veness.
 */
double* ModelRegion::destVincenty(double lat1, double lon1, double brng, double dist) {
  double a = 6378137.0, b = 6356752.3142,  f = 1/298.257223563;  // WGS-84 ellipsiod
  double s = dist;
  double alpha1 = brng * PI / 180;
  double sinAlpha1 = sin(alpha1);
  double cosAlpha1 = cos(alpha1);
  
  double tanU1 = (1-f) * tan(lat1 * PI / 180 );
  double cosU1 = 1 / sqrt( (double) (1 + tanU1*tanU1) ), sinU1 = tanU1*cosU1;
  double sigma1 = atan2(tanU1, cosAlpha1);
  double sinAlpha = cosU1 * sinAlpha1;
  double cosSqAlpha = 1 - sinAlpha*sinAlpha;
  double uSq = cosSqAlpha * (a*a - b*b) / (b*b);
  double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
  double B = uSq/1024 * (256+uSq*(-128+uSq*(74-47*uSq)));
  
  double sigma  ( s / (b*A) );
  double sigmaP ( 2*PI );

  double cos2SigmaM(0), sinSigma(0), cosSigma(0), deltaSigma(0);

  while (fabs(sigma-sigmaP) > 1e-12) {
    cos2SigmaM = cos(2*sigma1 + sigma);
    sinSigma = sin(sigma);
    cosSigma = cos(sigma);
    deltaSigma = B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
    sigmaP = sigma;
    sigma = s / (b*A) + deltaSigma;
  }

  double tmp  ( sinU1*sinSigma - cosU1*cosSigma*cosAlpha1 );
  double lat2 ( atan2(sinU1*cosSigma + cosU1*sinSigma*cosAlpha1, (1-f)*sqrt( (double) sinAlpha*sinAlpha + tmp*tmp)) );
  double lambda (atan2(sinSigma*sinAlpha1, cosU1*cosSigma - sinU1*sinSigma*cosAlpha1) );
  double C ( f/16*cosSqAlpha*(4+f*(4-3*cosSqAlpha)) );
  double L ( lambda - (1-C) * f * sinAlpha * (sigma + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM))) );

  double revAz ( atan2(sinAlpha, -tmp) );  // final bearing

  tmp_point[0] = lat2*180/PI;
  tmp_point[1] = lon1+(L*180/PI);

  return tmp_point;
}

