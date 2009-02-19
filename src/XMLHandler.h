/***************************************************************************
 * File:        ./XMLHandler.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     28.03.2007
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

#ifndef _xml_handler_h
#define _xml_handler_h

//-----------------------------------------------------------
//-------------- X M L  INCLUDES ----------------------------
//-----------------------------------------------------------
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include "DOMTreeErrorReporter.hpp"

//-----------------------------------------------------------
//-------------- S T D  INCLUDES ----------------------------
//-----------------------------------------------------------
#include <string>
#include <iostream>
#include <cassert>
#include <dlfcn.h>
#include <stdexcept>
#include <list>
#include <sstream>

#include "exceptions.h"

XERCES_CPP_NAMESPACE_USE

using namespace std;

inline std::string stringify(int x)
{
   std::ostringstream o;
   if (!(o << x))
     throw Conversion(" stringify(int) ");
   return o.str();
}

class StrXML
{
	public :
	    // -----------------------------------------------------------------------
	    //  Constructors and Destructor
	    // -----------------------------------------------------------------------
	    StrXML(const char* const orig);
	    StrXML(const XMLCh* const orig);
	    StrXML( const string& orig ) ;
	    ~StrXML() throw();
	    const XMLCh* xmlStr() const throw();
    	const char* cStr() const throw();
	    const string cppStr() const throw();
    
	private :
	    char*    cstring;
	    XMLCh*   xmlstring;
	    string   cppstring;
};

/**
 * takes care on initialization and termination of Xerces. wrapped in an object to 
 * be able to provide "ressource aquisition is initialization" in XMLHandler
 */
class XMLCore
{
	public:
        XMLCore();			/* Constructor */
       ~XMLCore();			/* Destructor */
};

/**
 */
class XMLHandler : private XMLCore
{
	private:
		XercesDOMParser parser;
		DOMTreeErrorReporter errReporter;		
		DOMElement	*rootElem;
	

		/**hidden and empty copy constructor - we do not want to accidentially copy objects*/
		XMLHandler(const XMLHandler& x); 

		void      fillNodesChildDataList(DOMNode *n, XMLCh *child, list<string> &datalist);
		
	protected:
		string xmlFile;
		string xmlPath;
		
		DOMDocument 	*doc;
		
		const StrXML core;

		const StrXML TAG_authors;
		const StrXML TAG_category;
		const StrXML TAG_crustaldecay;
		const StrXML TAG_dateAdded;
		const StrXML TAG_dependency;
		const StrXML TAG_description;
		const StrXML TAG_experiment;
		const StrXML TAG_file;
		const StrXML TAG_filename;
		const StrXML TAG_green;
		const StrXML TAG_job;
		const StrXML TAG_kernel;
		const StrXML TAG_load;
		const StrXML TAG_loadfunction;
		const StrXML TAG_loadhistory;
		const StrXML TAG_name;
		const StrXML TAG_output;
		const StrXML TAG_parameter;
		const StrXML TAG_plugin;
		const StrXML TAG_postprocessor;
		const StrXML TAG_region;	
		const StrXML TAG_version;

		const StrXML ATTR_date;
		const StrXML ATTR_gmtime;
		const StrXML ATTR_id;
		const StrXML ATTR_job;
		const StrXML ATTR_localtime;
		const StrXML ATTR_name;
		const StrXML ATTR_user;
		const StrXML ATTR_value;
						
		list<string> getNodesChildDataList(XMLCh *parent, XMLCh *child);
		string   getChildData(DOMNode *parent, const XMLCh* childname);
		void     readXML() throw( std::runtime_error );
		void     writeXML(bool pretty=false) throw( std::runtime_error );
		int 	 getActualID();
	
	public:
		         XMLHandler(const char* path);	/* Constructor */
		virtual	~XMLHandler();			/* Destructor */
		
		DOMNode* getElementByTagName(const XMLCh*);	/*returns first element with given name */
		DOMNodeList* getElementListByTagName(const XMLCh*);	/*returns all elements with given name*/
		string   getValueOfNode(DOMNode*);		/*if node == Text Node: return TEXT (<node>TEXT</node>)*/
		string   getValueOfNode(DOMNode*, const XMLCh*);/*if node == Element Node: find node with name='name' and return value of 'value' attr*/
		virtual void init()=0;

};

#endif // _xml_handler_h
