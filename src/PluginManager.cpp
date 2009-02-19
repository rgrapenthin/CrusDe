 /***************************************************************************
 * File:        ./PluginManager.cpp
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


 //directory handling header
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <ios>
//QT-Includes
#include <QApplication>

#include "SimulationCore.h"
#include "PluginManager.h"

PluginManager::PluginManager(const char *db) :
	DatabaseManager(db),
	plugin("dummy")
{
	Debug("%s, line: %d, PluginManager built for %s ", __FILE__, __LINE__, db);
}

/**
 * 
 * @param name 
 * @return 
 */
string PluginManager::getFilename(string category, string name) throw (DatabaseError)
{	
//Debug("%s, line: %d, PluginManager::getFilename for %s in %s in DB: %s", __FILE__, __LINE__, name.c_str(), category.c_str(), xmlFile.c_str());
	assert(is_initialized);
	DOMNode *plugin (isInDB(name, category));

	if(!plugin)
	{
		string error("Could not find ");
		throw (DatabaseError( error.append(name).append(" in category ").append(category) ) );
	}
		
	return getChildData(plugin, TAG_filename.xmlStr());	
}

/**
 * add entry 
 */
void PluginManager::addEntry(string absolutepath, DOMNode *parent) throw (PluginExistsException)
{
	try{
		plugin.load(absolutepath);
		
		//checks for the plugin to work including all dependencies, 
		//since the requested plugins are gonna be loaded
		plugin.selftest();
		
		if( isInDB(plugin.getName(), plugin.getCategory()) != NULL)
		{
			string error("Plugin '");
			error.append(plugin.getName()).append("' already exisits in category '");
			error.append(plugin.getCategory()).append("'.");
			throw (PluginExistsException(error));
		}
					
		//copy file to repository
		string dst(getenv("CRUSDE_HOME"));
		dst.append(DIR_SEP).append(PLUGIN_DIR);
		dst.append(DIR_SEP).append( plugin.getCategory() ).append("/");
		dst.append( absolutepath.substr(absolutepath.find_last_of("/")+1, absolutepath.length()) );

		if(absolutepath.compare(dst) != 0)
		{
			std::ifstream srcStream(absolutepath.c_str());
			std::ofstream dstStream(dst.c_str());
			dstStream << srcStream.rdbuf();
		}
		
		//Add to DB
		addToDB(absolutepath, dst);
		
		
		//if that's done ... write database
		writeXML(true);	
			
		//keep some stuff in mind
		last_id = getActualID()-1;
		//SimulationCore::instance()->deleteRegistrees(plugin.getCategoryID());
		SimulationCore::instance()->deleteRegistrees();
		SimulationCore::instance()->deleteRequests();
		plugin.unload();
	}
	catch(...){
		throw;
	}
}


/**
 * delete entry
 */
bool PluginManager::deleteEntry(int id) throw (DatabaseError, PluginError)
{
	DOMNode *entry = getEntryById(id);
	string error_string("Entry not found.");
	
	if( entry )
	{
		//check for dependencies
		list<DOMNode*> dependents( getDependents( entry ) );
		
		if(!dependents.empty())
		{
			list<DOMNode*>::iterator iter = dependents.begin();
			error_string.clear();
			error_string.append("Dependency conflict with plugin(s):\n");
			
			while(iter!=dependents.end())
			{
				error_string.append("\n=>").append( getChildData( (*iter)->getParentNode(), TAG_name.xmlStr() ) );
				++iter;
			}
			
			throw( DatabaseError(error_string) );
			
			return false;
		}
	
		//get filename ...
		string filename( getChildData(entry, TAG_filename.xmlStr()) );
		
		//remove entry from DB
		try{
			DOMElement *root = doc->getDocumentElement();
			root->removeChild(entry);
		}
		catch(DOMException e){
			StrXML error(e.getMessage());
			cerr << "exception: "<<error.cppStr()<<endl;
			exit(2);
		}
		
		//in case we made it this far ... remove file
		if(remove(filename.c_str()) == 0)
		{
			// ... and write the database	
			writeXML();
			// ... and say, we did so.
			return true;
		}
		else
		{	
			error_string.clear();
			error_string.append("Could not remove file: ").append(filename).append(".\nAborting");
			throw( PluginError (error_string) );
		}
	}
	
	throw( DatabaseError(error_string) );
	return false;	
}

/**
 * get Entries 
 */
multimap<string, map<int, string> > PluginManager::getEntryMap()
{
	multimap<string, map<int, string> > entries;
	//get all plugins
	DOMNodeList *plugin_list = doc->getElementsByTagName(TAG_plugin.xmlStr());
	DOMNamedNodeMap *attributes = NULL;
	string category, name;
	int i(plugin_list->getLength());
	
	//we want latest plugin first
	while ( --i >= 0 )
	{
		//get all the attributes of the plugin, namely the id
		attributes = plugin_list->item(i)->getAttributes();
		//get category which is the key in multimap
		category = getChildData(plugin_list->item(i), TAG_category.xmlStr());
		//get plugin name which is the second value in multimap
		name = getChildData(plugin_list->item(i), TAG_name.xmlStr());
		//get id string
		char *id_string = XMLString::transcode(attributes->getNamedItem( ATTR_id.xmlStr() )->getNodeValue());
		//convert id to integer
		int id( atoi( id_string ) );
		
		//create subdata
		map<int, string> submap;
		submap[id] = name;
		
		//add to map
		entries.insert(pair<string, map<int, string> >(category, submap));
		
		XMLString::release(&id_string);
	}
	
	return entries;
}

/**
 * checks whether some plugin in the database depends on entry.
 * therefore we retrieve all 'TAG_dependency' entries and compare their contents
 * with 
 * @param entry plugin someone might depend on
 * @return list of dependent plugins
 */
list<DOMNode*> PluginManager::getDependents(DOMNode *entry)
{
	DOMNodeList *dependency_list = doc->getElementsByTagName(TAG_dependency.xmlStr());
	list<DOMNode*> dependent_list;
	
	//get Plugin name
	string name( getChildData(entry, TAG_name.xmlStr()) );
	
	unsigned int i(0);
	
	while(i < dependency_list->getLength() )
	{
		if( 	!entry->isSameNode(dependency_list->item(i)->getParentNode()) && 
			name.compare( getValueOfNode(dependency_list->item(i)) ) == 0 )
		{
			dependent_list.push_back( dependency_list->item(i) );
		}
		++i;
	}
	
	return dependent_list;
}

/**
 * checks whether some plugin in the database depends on entry.
 * therefore we retrieve all 'TAG_parameter' entries and compare their contents
 * with 
 * @param entry plugin someone might depend on
 * @return list of dependent plugins
 */
list<string> PluginManager::getParameters(DOMNode *entry)
{
	list<string> dependency_list;
	
	if( entry->getNodeType() == DOMNode::ELEMENT_NODE)
	{			
		DOMElement *elem = dynamic_cast<DOMElement*>(entry);
		DOMNodeList *dependencies = elem->getElementsByTagName(TAG_parameter.xmlStr());
		unsigned int i(0);
	
		while(i < dependencies->getLength() )
		{
			dependency_list.push_back( getValueOfNode(dependencies->item(i)) );
			++i;
		}
	}
	
	return dependency_list;
}

/**
 * checks whether some plugin in the database depends on entry.
 * therefore we retrieve all 'TAG_dependency' entries and compare their contents
 * with 
 * @param entry plugin someone might depend on
 * @return list of dependent plugins
 */
list<string> PluginManager::getDependencies(DOMNode *entry)
{
	list<string> dependency_list;
	
	if( entry->getNodeType() == DOMNode::ELEMENT_NODE)
	{			
		DOMElement *elem = dynamic_cast<DOMElement*>(entry);
		DOMNodeList *dependencies = elem->getElementsByTagName(TAG_dependency.xmlStr());
		unsigned int i(0);
	
		while(i < dependencies->getLength() )
		{
			dependency_list.push_back( getValueOfNode(dependencies->item(i)) );
			++i;
		}
	}
	
	return dependency_list;
}


DOMNode* PluginManager::getEntryById(int id)
{
	return getEntryById(id, TAG_plugin.xmlStr());
}

/**
 * to be inherited from base class
 * @param id 
 * @param tag 
 * @return 
 */
DOMNode* PluginManager::getEntryById(int id, const XMLCh* tag)
{
	DOMNodeList *entry_list = doc->getElementsByTagName(tag);
	DOMNamedNodeMap *attributes = NULL;
	
	unsigned int i(0);
	StrXML id_string(stringify(id));
	
	//find experiment with ID == id
	while ( i < entry_list->getLength() )
	{
		//get all the attributes of the experiment
		attributes = entry_list->item(i)->getAttributes();
		if(XMLString::compareString( attributes->getNamedItem( ATTR_id.xmlStr() )->getNodeValue(), 
					     id_string.xmlStr()) == 0 )
		{
			/* upcast to element node */
			return  entry_list->item(i);
		}
		++i;
	}

	return NULL;
}


/**
 * get Experiments 
 */
string PluginManager::getEntryDetails(int id)
{
	string details;
	DOMNode *plugin = getEntryById(id);
	//add name
	details.append("<b><u><h3>").append(getChildData(plugin, TAG_name.xmlStr()).append("</b></u></h5>"));
	details.append("<table><tr>\n");
	details.append("<td><b>category:&nbsp;&nbsp;</b></td><td>").append(getChildData(plugin, TAG_category.xmlStr())).append("</td>\n");
	details.append("</tr><tr>\n");
	details.append("<td><b>version:&nbsp;&nbsp;</b></td><td>").append(getChildData(plugin, TAG_version.xmlStr())).append("</td>\n");
	details.append("</tr><tr>\n");
	details.append("<td><b>author(s):&nbsp;&nbsp;</b></td><td>").append(getChildData(plugin, TAG_authors.xmlStr())).append("</td>\n");
	details.append("</tr><tr>\n");
	details.append("<td><b>filename:&nbsp;&nbsp;</b></td><td>").append(getChildData(plugin, TAG_filename.xmlStr())).append("</td>\n");
	details.append("</tr><tr>\n");
	details.append("<td><b>date added:&nbsp;&nbsp;</b></td><td>").append(getChildData(plugin, TAG_dateAdded.xmlStr())).append("</td>\n");
	details.append("</tr><tr>\n");
	details.append("<td><b>description:&nbsp;&nbsp;</b></td><td>").append(getChildData(plugin, TAG_description.xmlStr())).append("</td>\n");
	details.append("</tr><tr>\n");
	details.append("<td><b>free parameter:&nbsp;&nbsp;</b></td><td valign=\"bottom\">");
	
//get dependencies
	list<string> p_list = getParameters(plugin);
	list<string>::iterator iter = p_list.begin();
	while(iter!=p_list.end())
	{
		details.append(*iter).append("<br>");
		++iter;
	}
	details.append("</td></tr>\n");
	details.append("<tr><td><b>dependencies:&nbsp;&nbsp;</b></td><td>");
	
//get dependencies
	list<string> dependencies = getDependencies(plugin);
	iter = dependencies.begin();
	while(iter!=dependencies.end())
	{
		details.append(*iter).append("<br>");
		++iter;
	}
	details.append("</td></tr>\n");
	details.append("</table>\n");
	
	return details;
}

/**
 * 
 * @param filename 
 * @param category 
 * @return 
 */
DOMNode* PluginManager::isInDB(string filename, string category)
{
	DOMNodeList *plugin_list = doc->getElementsByTagName(TAG_plugin.xmlStr());
	
	unsigned int i(0);
	
	//find plugin that has name='name' and category='category'
	while(i < plugin_list->getLength())
	{	
		if(	filename.compare( getChildData(plugin_list->item(i), TAG_name.xmlStr())) == 0 &&
			category.compare( getChildData(plugin_list->item(i), TAG_category.xmlStr())) == 0 )
		{
			return plugin_list->item(i);
		}
		
		++i;
	}
	
	return NULL;
}

/**
 * Add plugin to plugin_list and DOM
 *
 * @param name 
 * @param category 
 * @param version 
 * @param authors 
 * @param filename 
 */
void PluginManager::addToDB(string filename, string dst)
{
	//add to DOM
	try
	{
		StrXML attr_id( stringify( getActualID() )  );
		
		// create plugin element and add children
		DOMElement *pluginElem = doc->createElement( TAG_plugin.xmlStr() );
		pluginElem->setAttribute(ATTR_id.xmlStr(), attr_id.xmlStr());

		//add name
		DOMElement *pluginChild= doc->createElement( TAG_name.xmlStr() );
		pluginChild->appendChild( doc->createTextNode( StrXML( plugin.getName() ).xmlStr() ) );
		pluginElem->appendChild(pluginChild);
		//add category
		pluginChild= doc->createElement( TAG_category.xmlStr() );
		pluginChild->appendChild(doc->createTextNode( StrXML( plugin.getCategory() ).xmlStr() ) );
		pluginElem->appendChild(pluginChild);
		//add version
		pluginChild= doc->createElement( TAG_version.xmlStr() );
		pluginChild->appendChild(doc->createTextNode( StrXML( plugin.getVersion() ).xmlStr() ) );
		pluginElem->appendChild(pluginChild);
		//add authors
		pluginChild= doc->createElement( TAG_authors.xmlStr() );
		pluginChild->appendChild(doc->createTextNode( StrXML( plugin.getAuthors() ).xmlStr() ) );
		pluginElem->appendChild(pluginChild);
		//add filename
		pluginChild= doc->createElement( TAG_filename.xmlStr() );
		pluginChild->appendChild(doc->createTextNode( StrXML( dst ).xmlStr() ) );
		pluginElem->appendChild(pluginChild);		
		//add description
		pluginChild= doc->createElement( TAG_description.xmlStr() );
		pluginChild->appendChild(doc->createTextNode( StrXML( plugin.getDescription() ).xmlStr() ) );
		pluginElem->appendChild(pluginChild);		
			
		//add date
		time_t the_time;
		char date_str[30];
		
		time(&the_time);
		strftime(date_str, 29, "%a %d %b %Y", localtime(&the_time));
				
		pluginChild= doc->createElement( TAG_dateAdded.xmlStr() );
		pluginChild->appendChild(doc->createTextNode( StrXML( date_str ).xmlStr() ) );
		pluginElem->appendChild(pluginChild);
	
	//add parameters
		//a little more work
		
		//get all requested plugin names
		list<string> requests = SimulationCore::instance()->getRegisteredParameters(plugin.getCategoryID());
		
		if(!requests.empty())
		{
			
			list<string>::iterator iter = requests.begin();
			
			//add one dependency add a time
			while(iter != requests.end())
			{
				pluginChild= doc->createElement( TAG_parameter.xmlStr() );
				pluginChild->appendChild(doc->createTextNode( StrXML( *iter ).xmlStr() ) );
				pluginElem->appendChild(pluginChild);		
				++iter;
			}
		}
	
	//add dependencies
		//a little more work
		
		//get all requested plugin names
		requests = SimulationCore::instance()->getRequestedNames();
		
		if(!requests.empty())
		{
			
			list<string>::iterator iter = requests.begin();
			
			//add one dependency add a time
			while(iter != requests.end())
			{
				pluginChild= doc->createElement( TAG_dependency.xmlStr() );
				pluginChild->appendChild(doc->createTextNode( StrXML( *iter ).xmlStr() ) );
				pluginElem->appendChild(pluginChild);		
				++iter;
			}
		}
			
		//append new plugin to root
		DOMElement *root = doc->getDocumentElement();
		root->appendChild(pluginElem);
		//increase number of plugins, meaning actual id
		StrXML new_id( stringify(getActualID() + 1) );
		root->setAttribute(ATTR_id.xmlStr(), new_id.xmlStr() ) ;

	}
	catch (const OutOfMemoryException&)
	{
		XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
	}
	catch (const DOMException& e)
	{
		XERCES_STD_QUALIFIER cerr << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;
	}
	catch (...)
	{
		XERCES_STD_QUALIFIER cerr << "An error occurred creating the document" << XERCES_STD_QUALIFIER endl;
	}
	
	
}

void PluginManager::run()
{
	assert(is_initialized);
	
	//create the dialog
	runDialogWindow("CrusDe Plugin Manager", true);
}
