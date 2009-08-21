/***************************************************************************
 * File:        ./XMLHandler.cpp
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

#include "XMLHandler.h"
#include "SimulationCore.h"
#include <sys/stat.h>	/*stat*/
#include <errno.h>
#include <string>

//-------------------------------------------------
// CLASS StrXML
//-------------------------------------------------

StrXML::StrXML(const char* const orig) 
	:
	cstring( XMLString::replicate( orig ) ) ,
	xmlstring( XMLString::transcode( orig ) ), 
   	cppstring(cstring)
{
	return;
}

StrXML::StrXML(const XMLCh* const orig) 
	:
	cstring( XMLString::transcode( orig ) ) ,
	xmlstring( XMLString::replicate( orig ) ), 
   	cppstring(cstring) 
{
    	return;
}

StrXML::StrXML(const string& orig) 
	:
	cstring( XMLString::replicate( orig.c_str() ) ) ,
	xmlstring( XMLString::transcode( orig.c_str() ) ), 
 	cppstring(cstring) 
{
    	return;
}
  

StrXML::~StrXML() throw()
{
	XMLString::release(&xmlstring);
    XMLString::release(&cstring);
}


const XMLCh* StrXML::xmlStr() const throw() { return xmlstring; }
    
const char* StrXML::cStr() const throw() { return cstring; }

const string StrXML::cppStr() const throw() { return cppstring; }

//-------------------------------------------------
// CLASS XMLCORE
//-------------------------------------------------

XMLCore::XMLCore()
{
	try
	{
		XMLPlatformUtils::Initialize();  // Initialize Xerces infrastructure
	}
	catch( XMLException& e )
	{
		char* message = XMLString::transcode( e.getMessage() );
		crusde_warning("XML toolkit initialization error: %s", message);
		XMLString::release( &message );
		// throw exception here to return ERROR_XERCES_INIT
	}

}

XMLCore::~XMLCore()
{
	try
	{
		XMLPlatformUtils::Terminate();  // Terminate Xerces
	}
	catch( xercesc::XMLException& e )
	{
		char* message = xercesc::XMLString::transcode( e.getMessage() );
		crusde_warning("XERCES demolition error: %s", message);
		XMLString::release( &message );
	}

	crusde_debug("%s, line: %d, XMLCore destroyed.", __FILE__, __LINE__);
}

//-------------------------------------------------
// CLASS XMLHANDLER
//-------------------------------------------------

XMLHandler::XMLHandler(const char *filename):
	XMLCore(),
	parser(),
	errReporter(),
	xmlFile(filename),
	xmlPath(xmlFile.substr(0, xmlFile.find_last_of("/"))),
	doc(NULL),
	core("Core"),
	TAG_authors("authors"),
	TAG_category("category"),
	TAG_crustaldecay("crustal_decay"),
	TAG_dateAdded("added"),
	TAG_dependency("dependency"),
	TAG_description("description"),
	TAG_experiment("experiment"),
	TAG_file("file"),
	TAG_filename("filename"),
	TAG_green("greens_function"),
	TAG_job("job"),
	TAG_kernel("kernel"),
	TAG_load("load"),
	TAG_loadfunction("load_function"),
	TAG_loadhistory("load_history"),
	TAG_name("name"),
	TAG_output("output"),
	TAG_parameter("parameter"),
	TAG_plugin("plugin"),
	TAG_postprocessor("postprocessor"),
	TAG_region("region"),	
	TAG_version("version"),	
	ATTR_date("date"),
	ATTR_gmtime("gmtime"),
	ATTR_id("id"),
	ATTR_job("job"),
	ATTR_localtime("localtime"),
	ATTR_name("name"),
	ATTR_user("user"),
	ATTR_value("value")
{
 
	// Configure DOM parser.
  	parser.setErrorHandler(&errReporter);
	parser.setValidationScheme( XercesDOMParser::Val_Auto );
	parser.setDoNamespaces( false );
	parser.setDoSchema( true );
	parser.setLoadExternalDTD( true );
	parser.setExitOnFirstFatalError(true);
	
	crusde_debug("%s, line: %d, XMLHandler built for: %s ", __FILE__, __LINE__, xmlFile.c_str());
}

XMLHandler::~XMLHandler()
{
	
	try
	{
		parser.reset();
	}
	catch( xercesc::XMLException& e )
	{
		char* message = xercesc::XMLString::transcode( e.getMessage() );
		crusde_warning("XERCES demolition error: %s", message);
		XMLString::release( &message );
	}
   
	
	crusde_debug("%s, line: %d, XMLHandler destroyed.", __FILE__, __LINE__);
}


void XMLHandler::readXML()
	throw( std::runtime_error )
{
	crusde_debug("%s, line: %d, starting XML processing: %s", __FILE__, __LINE__, xmlFile.c_str());

	// Test to see if the file is ok.
	struct stat fileStatus;

	int iretStat( stat(xmlFile.c_str(), &fileStatus) );
	
	if( iretStat == ENOENT )
		throw ( std::runtime_error("File does not exist, or file is an empty string.") );
	else if( iretStat == ENOTDIR )
		throw ( std::runtime_error("A component of the path is not a directory."));
	else if( iretStat == ELOOP )
		throw ( std::runtime_error("Too many symbolic links encountered while traversing the path."));
	else if( iretStat == EACCES )
		throw ( std::runtime_error("Permission denied."));
	else if( iretStat == ENAMETOOLONG )
		throw ( std::runtime_error("File can not be read\n"));
		        	
	//reset error count first
    errReporter.resetErrors();

    try{    
	/* reset document pool */
	parser.resetDocumentPool();
	/* parse */
	parser.parse(xmlFile.c_str());
	/* getDOM */
	doc = parser.getDocument();
	assert(doc);
    }
    catch (const XMLException& toCatch)
    {
        char* error = XMLString::transcode(toCatch.getMessage());
        crusde_warning("Error during parsing: '%s' Exception message is: \n%s", xmlFile.c_str(), error);
        XMLString::release(&error);
    }
    catch (const DOMException& toCatch)
    {
        char* error = XMLString::transcode(toCatch.getMessage());
        crusde_warning("Error during parsing: '%s' Exception message is: \n%s", xmlFile.c_str(), error);
	XMLString::release(&error);
    }
    catch (...)
    {
        crusde_warning("Unexpected exception during parsing: '%s'", const_cast<char*>( xmlFile.c_str() ) );
    }

    if (errReporter.getSawErrors())
    {
        crusde_error("Errors occurred when parsing XML file: '%s' ... Aborting. ", const_cast<char*>( xmlFile.c_str() ) );
    }
    else
    {
        crusde_debug("%s, line: %d, parsing done ....", __FILE__, __LINE__);
    }
}

void XMLHandler::writeXML(bool pretty)
	throw( std::runtime_error )
{
    crusde_debug("%s, line: %d, XMLHandler::writeXML %s", __FILE__, __LINE__, const_cast<char*>( xmlFile.c_str() ) );
	
    XMLFormatTarget *outfile = new LocalFileFormatTarget( xmlFile.c_str() );

    DOMImplementation* impl = doc->getImplementation();

    if( impl == NULL )
    {
       	throw( std::runtime_error( "DOMImplementation is null!" ) );
    }

    DOMWriter* writer = impl->createDOMWriter();

    // add spacing and such for human-readable output
    if( pretty && writer->canSetFeature( XMLUni::fgDOMWRTFormatPrettyPrint, true ) )
    {
       	writer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint , true );
    }
    writer->writeNode( outfile , *doc );

    delete(outfile);
    delete(writer);
    
    crusde_debug("%s, line: %d, XMLHandler::writeXML %s ... done", __FILE__, __LINE__, xmlFile.c_str());
}

/*returns first element with name TAG_plugin*/
DOMNode* XMLHandler::getElementByTagName(const XMLCh* tagname)
{
	
	return (doc->getElementsByTagName(tagname))->item(0);
}

DOMNodeList* XMLHandler::getElementListByTagName(const XMLCh* tagname)
{
	
	return doc->getElementsByTagName(tagname);
}

/*if node == Text Node: return TEXT (<node>TEXT</node>)*/		
string XMLHandler::getValueOfNode(DOMNode* n)
{
	string value;
	
	while (n)
	{
		if(n->hasChildNodes()){
			value = getValueOfNode(n->getFirstChild());
			if(value.length() > 0)
				return value;
		}		
		if( n->getNodeType() == DOMNode::TEXT_NODE )
		{	
			DOMText *t = static_cast<DOMText*>(n);
			
			if(!XMLChar1_0::isAllSpaces(t->getData(), t->getLength())){
				char* nodeText = XMLString::transcode(t->getData());
				value = string(nodeText); 
				XMLString::release(&nodeText);
				return value;
			}
		}
		
		n = n->getNextSibling();
	}
	
	return value;
	
}

/*if node == Element Node: find node with name='name' and return value of 'value' attr*/
string XMLHandler::getValueOfNode(DOMNode* n, const XMLCh* name)	
{
	string value;
	
	if( n->getNodeType() == DOMNode::ELEMENT_NODE)
	{			
		DOMNamedNodeMap *attributes = n->getAttributes();
		
		if( XMLString::compareIString(n->getNodeName(), TAG_parameter.xmlStr() ) == 0 && 
		    XMLString::compareIString(attributes->getNamedItem( ATTR_name.xmlStr() )->getNodeValue(), name) == 0 )
		{
			/* upcast child to element node */
			DOMElement *elem = dynamic_cast<DOMElement*>(n);
			assert (elem!=NULL);
			/* free memory of old value */				
			char* v = XMLString::transcode(elem->getAttribute( ATTR_value.xmlStr() ));
			value = v;
			XMLString::release(&v);
			return value;
		}
	}
	
	return value;

}

string XMLHandler::getChildData(DOMNode *n, const XMLCh *childname)
{
	//create a list of children with name 'childname', return the first of them.
	DOMNode *child = (static_cast<DOMElement*>(n))->getElementsByTagName(childname)->item(0);
	//get this child's value
	return getValueOfNode(child);
}


void XMLHandler::fillNodesChildDataList(DOMNode *n, XMLCh *child, list<string> &datalist)
{
	while (n)
	{
		if(n->hasChildNodes()){
			fillNodesChildDataList(n->getFirstChild(), child, datalist);
		}		
		
		if( n->getNodeType() == DOMNode::TEXT_NODE )
		{	
			DOMText *t = static_cast<DOMText*>(n);
			
			if(	!XMLChar1_0::isAllSpaces(t->getData(), t->getLength()) && 
				XMLString::compareString(t->getParentNode()->getNodeName(), child) == 0 )
			{
				StrXML nodeText(n->getTextContent());
				datalist.push_back(nodeText.cppStr());
			}
		}
		
		n = n->getNextSibling();
	}
	
}

list<string> XMLHandler::getNodesChildDataList(XMLCh *parent, XMLCh *child)
{
	assert(doc);
	
	DOMNodeList *l = doc->getElementsByTagName(parent);
	
	list<string> datalist;
	
	fillNodesChildDataList( l->item(0)->getFirstChild(), child, datalist);
	
	return datalist;
}

/**
 * get New ID
 *
 * the actual
 */
int XMLHandler::getActualID()
{
	DOMElement *root = doc->getDocumentElement();
	
	StrXML id_string(root->getAttribute( ATTR_id.xmlStr() ));
	
	int id( atoi(id_string.cStr()) );
	
	return id;
}
