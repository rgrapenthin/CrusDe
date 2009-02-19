/***************************************************************************
 * File:        ./LoadFunctionElement.cpp
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     10.06.2008
 * Licence:     GPLv2
 *
 * #########################################################################
 *
 * CrusDe, simulation framework for crustal deformation studies
 * Copyright (C) 2007 Ronni Grapenthin
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

#include "LoadFunctionElement.h"

/* class LoadFunctionElement */

LoadFunctionElement::LoadFunctionElement(const string _name) :
	load_name(_name),
	history_name(),
	history_job(),
	decay_name(),
	decay_job()
{}

LoadFunctionElement::~LoadFunctionElement()
{}

/*								*/
/* Give load contribution of current component at point (x,y) */
/*								*/
void LoadFunctionElement::setLoadName(const string name){  load_name = name;}
void LoadFunctionElement::setHistoryName(const string name){  history_name = name;}
void LoadFunctionElement::setHistoryJob(const string name){  history_job = name;}
void LoadFunctionElement::setDecayName(const string name){  decay_name = name;}
void LoadFunctionElement::setDecayJob(const string name){  decay_job = name;}

const string LoadFunctionElement::getLoadName(){ return load_name;}
const string LoadFunctionElement::getHistoryName(){ return history_name;}
const string LoadFunctionElement::getHistoryJob(){ return history_job;}
const string LoadFunctionElement::getDecayName(){ return decay_name;}
const string LoadFunctionElement::getDecayJob(){ return decay_job;}

