/***************************************************************************
 * File:        ./InputHandler.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     20.02.2007
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

#ifndef _input_handler_h
#define _input_handler_h

#include "XMLHandler.h"
#include <xercesc/dom/DOMElement.hpp>

#include <iostream>
#include <cassert>
#include <string>
#include <stdexcept>
#include <map>

class ParamWrapper;
class LoadFunctionElement;

XERCES_CPP_NAMESPACE_USE
using namespace std;

class InputHandler : public XMLHandler 
{
		
	private:
		int 	argc;
		char** 	argv;
		bool 	gotFile;

		DOMElement	*rootElem, *greenElem, *loadElem, *kernelElem;

		void readCommandline();
		void initDOM();
		string getAttributeValue(const XMLCh* parentNode, const XMLCh* elementNode, const XMLCh* elementName);
		string getAttributeValueByName(const XMLCh* elementNode, const XMLCh* elementName, const XMLCh* name);
		void initParamsFromDOM(DOMNode *node, multimap<string, ParamWrapper*>);
		void initParamsFromDOMNodeList(DOMNodeList* , map< unsigned int, multimap<string, ParamWrapper*> >);
		bool jobExists(string);
		bool fileExists(string);

		/**hidden copy constructor - we do not want to accidentially copy objects*/
		InputHandler(const InputHandler& x); 
			
	public:
		InputHandler(int, char**);	/*Constructor*/
		~InputHandler();		/*Destructor */

		void init();
		void initParamsFromDOM();
		
		/*GETTERS*/
		/*********/
//		string getGreenPlugin();
		map<string, string> getGreenJobMap();
		string getGreenPlugin(string);
		string getLoadPlugin();
		string getLoadPlugin(string);
//		string getLoadHistoryPlugin();
		string getLoadHistoryPlugin(string);
//		string getCrustalDecayPlugin();
		string getCrustalDecayPlugin(string);
		string getKernelPlugin();
		string getKernelPlugin(string);
		string getOutputPlugin();
		string getOutputPlugin(string);
		string getFileName(string type);
		string getResultFileName();
		double getRegion(string direction);
		double getGridSize();
		unsigned long int getTimeSteps();
        unsigned long int getTimeIncrement();
		DOMNode* getExperiment();
		list<string> getPostProcessorNameList();
        list<LoadFunctionElement*> getLoadFunctionList();

		/*SETTERS*/
		/*********/
		void setXML(bool b){/*gotFile = b;*/}
		void setList(bool b){/*doList = b;*/}
};

#endif // _input_handler_h
