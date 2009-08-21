/***************************************************************************
 * File:        ./ParamWrapper.cpp
 * Author:      Ronni Grapenthin, Geophysical Institute, UAF
 * Created:     2008-02-27
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

#include <cstdlib>
#include <string.h>
#include "ParamWrapper.h"
#include "crusde_api.h"

ParamWrapper::ParamWrapper() :
	string_param(NULL),	
	double_param(NULL),
	optional(false)
{}

ParamWrapper::~ParamWrapper()
{
	if(double_param!=NULL)
		delete double_param;
	if(string_param!=NULL)
    {
		delete string_param;
		free(cStr);
    }
}

char** ParamWrapper::newString(char* val)
{
	if(double_param == NULL && string_param == NULL)
	{
		string_param = new string();
		cStr  = (char*) malloc(sizeof(char*));
	}
	else
	{
		crusde_error("ParamWrapper::newString() : try creating string when double exists\n");
	}
	
	return &cStr;
}

double* ParamWrapper::newDouble(double val)
{
	if(string_param == NULL && double_param == NULL)
	{
		double_param = new double(val);
	}
	else
	{
		crusde_error("ParamWrapper::newString() : try creating double when string exists\n");
	}

	return double_param;		
}
	
void ParamWrapper::setValue(string value)
{
	if(this->isString())
	{
		*string_param = value;
		if( (cStr = (char*) realloc(cStr, sizeof(char*) * (string_param->length()+1))) == NULL )
		{
			crusde_error("ParamWrapper::setValue : cannot allocate memory\n");
		}
		else
		{		
			memcpy(cStr, string_param->c_str(), string_param->length());
			cStr[string_param->length()]='\0';
		}
	}
	else
	{
		crusde_error("ParamWrapper::setValue : try setting string when param is not string\n");
	}
}

void ParamWrapper::setValue(double value)
{
	if(isDouble())
	{
		*double_param = value;
	}
	else
	{
		crusde_error("ParamWrapper::setValue : try setting double when param is not double\n");
	}
}

void ParamWrapper::setOptional(bool value){ optional = value;}

double* ParamWrapper::doubleValue()
{
		return double_param;
}

char** ParamWrapper::stringValue()
{
		return &cStr;
}

bool ParamWrapper::isString(){ return (string_param != NULL ? true : false);}
bool ParamWrapper::isDouble(){ return (double_param != NULL ? true : false);}

bool ParamWrapper::isOptional(){ return optional;}

