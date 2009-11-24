/***************************************************************************
 * File:        ./ModelRegion.h
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
 ****************************************************************************/

#ifndef _model_region_h
#define _model_region_h

#include <iostream>
#include <cassert>
#include <string>
#include <stdexcept>
#include <map>

using namespace std;

class ModelRegion
{
		
	private:
		const string name;
		double north_deg;
		double south_deg;
		double east_deg;
		double west_deg;
		double gridsize_deg;

		/**hidden copy constructor - we do not want to accidentially copy objects*/
		ModelRegion(const ModelRegion& x); 
		double distVincenty(double, double, double, double);
			
	public:
		ModelRegion(const string);	/*Constructor*/
		~ModelRegion();		/*Destructor */

		/*GETTERS*/
		/*********/
		double getEastDegrees();
		int    getLonDistance();
		int    getLatDistance();
		int    getGridsizeMetric();

		/*SETTERS*/
		/*********/
		void setGridsizeDegrees(double);
		void setRegionDegrees(double, double, double, double);
};

#endif // _model_region_h
