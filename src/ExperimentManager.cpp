/***************************************************************************
 * File:        ./ExperimentManager.cpp
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


#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <list>

//QT-Includes
#include <QApplication>

#include "ExperimentManager.h"
#include "Dialog.h"
#include "SimulationCore.h"

ExperimentManager::ExperimentManager(const char *db) :
	DatabaseManager(db), 
	actual_experiment(NULL)
{
  	crusde_debug("%s, line: %d, ExperimentManager built for %s ", __FILE__, __LINE__, db);
}

/**
 * start gui
 */
void ExperimentManager::run()
{
	assert(is_initialized);
	//create and run the dialog
	runDialogWindow("CrusDe Experiment Manager", true);
}

/**
 * add experiment 
 */
void ExperimentManager::addEntry(string result_file, DOMNode *exp_root) 
{
	//make sure to import the new Node into the current document
	actual_experiment = doc->importNode(exp_root, true);

	addMetaData(result_file);	

	DOMElement *root = doc->getDocumentElement();
	
	try{
		if(root->hasChildNodes())
			root->insertBefore(actual_experiment, root->getFirstChild());
		else
			root->appendChild(actual_experiment);
	}
	catch(DOMException e){
		StrXML error(e.getMessage());
		crusde_warning("DOMException: %s", error.cStr());
		SimulationCore::instance()->terminate();
	}
	
	//save id
	last_id = getActualID();
	
	//increase number of experiments, meaning actual id
	const char *id = stringify(last_id + 1).c_str();
	root->setAttribute(ATTR_id.xmlStr(), XMLString::transcode( id ) );

//WRITE BACKUP OF RESULTS & OTHER FILES TO SPECIFIC DIRECTORY CHECK WHETHER ENOUGH MEMORY
	writeXML();
}

/**
 * delete experiment 
 */
bool ExperimentManager::deleteEntry(int id)
{
	DOMNode *experiment = getEntryById(id);
	
	if(experiment)
	{
		//remove files!
		try{
			DOMElement *root = doc->getDocumentElement();
			root->removeChild(experiment);
		}
		catch(DOMException e){
			StrXML error(e.getMessage());
			crusde_warning("DOMException: %s", error.cStr());
		}
		
		writeXML();
	}
	
	return true;
}


/**
 * get Experiments 
 */
multimap<string, map<int, string> > ExperimentManager::getEntryMap()
{
	multimap<string, map<int, string> > experiments;
	//get all experiments
	DOMNodeList *experiment_list = doc->getElementsByTagName(TAG_experiment.xmlStr());
	DOMNamedNodeMap *attributes = NULL;
	string experiment_metadata;
	int i(experiment_list->getLength());
	
	//we want latest experiments first
	while ( --i >= 0 )
	{
		//get all the attributes of the experiment
		attributes = experiment_list->item(i)->getAttributes();
		//get experiment name
		StrXML name(attributes->getNamedItem( ATTR_name.xmlStr() )->getNodeValue());
		//clear old metadata
		experiment_metadata.clear();
		
		//get user
		char *user = XMLString::transcode(attributes->getNamedItem( ATTR_user.xmlStr() )->getNodeValue());
		char *date = XMLString::transcode(attributes->getNamedItem( ATTR_date.xmlStr() )->getNodeValue());
		char *gmtime = XMLString::transcode(attributes->getNamedItem( ATTR_gmtime.xmlStr() )->getNodeValue());
		char *id_string = XMLString::transcode(attributes->getNamedItem( ATTR_id.xmlStr() )->getNodeValue());
		
		int id( atoi( id_string ) );
		
		//build string
		experiment_metadata.append(date).append(", ").append(gmtime).append(", ").append(user);
		
		//create subdata
		map<int, string> submap;
		submap[id] = experiment_metadata;
		
		//add to map
		experiments.insert(pair<string, map<int, string> >(name.cppStr(), submap));
		
		XMLString::release(&user);
		XMLString::release(&date);
		XMLString::release(&gmtime);
		XMLString::release(&id_string);
	}
	
	return experiments;
}

DOMNode* ExperimentManager::getEntryById(int id)
{
	DOMNodeList *experiment_list = doc->getElementsByTagName(TAG_experiment.xmlStr());
	DOMNamedNodeMap *attributes = NULL;
	
	unsigned int i(0);
	StrXML id_string(stringify(id));
	
	//find experiment with ID == id
	while ( i < experiment_list->getLength() )
	{
		//get all the attributes of the experiment
		attributes = experiment_list->item(i)->getAttributes();
		if(XMLString::compareString( attributes->getNamedItem( ATTR_id.xmlStr() )->getNodeValue(), 
					     id_string.xmlStr()) == 0 )
		{
			/* upcast to element node */
			return  experiment_list->item(i);
		}
		++i;
	}

	return NULL;
}

/**
 * get Experiments 
 */
string ExperimentManager::getEntryDetails(int id)
{
	return childrenToString(getEntryById(id));
}

string ExperimentManager::childrenToString(DOMNode *n)
{
	string children;
	
	children.append(childrenToString(n, TAG_file.xmlStr()) );
	children.append(childrenToString(n, TAG_region.xmlStr()) );
	children.append(childrenToString(n, TAG_parameter.xmlStr()) );
	children.append(childrenToString(n, TAG_plugin.xmlStr()) );
	
	return children;
}

/**
 * build a detailed string
 */
string ExperimentManager::childrenToString(DOMNode *n, const XMLCh* tag)
{
	string children;
	if( n!=NULL && n->getNodeType() == DOMNode::ELEMENT_NODE)
	{	
		DOMElement *elem = dynamic_cast<DOMElement*>(n);
		DOMElement *elem2 = dynamic_cast<DOMElement*>(n);
		assert(elem);
		DOMNodeList *nodes = NULL;
		StrXML the_tag(tag);
		
		children.append("<h4><u>").append(the_tag.cStr()).append("</u></h4>");
		
		nodes=elem->getElementsByTagName(tag);
		
		unsigned int i(0);	
	
		children.append("<table><tr><td>&nbsp;&nbsp;</td><td><table>");
		
		while(i<nodes->getLength())
		{
			if(  nodes->item(i)->getNodeType() == DOMNode::ELEMENT_NODE )
			{
				elem2 = dynamic_cast<DOMElement*>(nodes->item(i));
				StrXML tagname( elem2->getTagName() );
				StrXML name( elem2->getAttribute(ATTR_name.xmlStr()) );			
				StrXML value( elem2->getAttribute(ATTR_value.xmlStr()) );
				StrXML parent (elem2->getParentNode()->getNodeName() );
				//indent
				children.append("<tr><td><i>");
				//if plugin, add parent tagname
				if(the_tag.cppStr().compare(TAG_plugin.cppStr()) == 0 )
				{
					children.append(parent.cppStr()).append("</i></td><td>&nbsp;:&nbsp;</td><td>");
				}

				children.append(name.cppStr());
					
				if(!value.cppStr().empty())
					children.append("</td><td>&nbsp;=&nbsp;</td><td>").append(value.cppStr());
					
				children.append("</td></tr>");
			}			
			
			++i;	
		}
	}			
	return children.append("</table></td></tr></table>\n");
}


void ExperimentManager::addMetaData(string filename)
{
//  crusde_debug("%s, line: %d, ExperimentManager::addMetaData ", __FILE__, __LINE__);
		//take actual time
//		struct timeval now;
//		struct timezone here;
		
		DOMElement*  root_element = static_cast<DOMElement*>(actual_experiment);
		
		time_t the_time;
		time(&the_time);
		
		char date_str[30];
		char gmt_str[30];
		char local_str[30];
		
		strftime(date_str, 29, "%a %d %b %Y", localtime(&the_time));
		strftime(local_str, 29, "X", localtime(&the_time));
		strftime(gmt_str, 29, "%X", gmtime(&the_time));
		
		StrXML exp_name( root_element->getAttribute(ATTR_name.xmlStr()) );
		string result_backup(exp_name.cppStr());
		result_backup.append(date_str).append(local_str);
		
		StrXML attr_value(filename);
		StrXML attr_name("result");
		StrXML attr_id( stringify( getActualID() )  );
		StrXML attr_date(date_str);
		StrXML attr_localtime(local_str);
		StrXML attr_gmtime(gmt_str);
		StrXML attr_user(getenv("USER"));
  
		DOMElement*  result_element = doc->createElement(TAG_file.xmlStr());
		
		if(actual_experiment->hasChildNodes())
			actual_experiment->insertBefore(result_element, actual_experiment->getFirstChild());
		else
			actual_experiment->appendChild(result_element);
		
		result_element->setAttribute(ATTR_name.xmlStr(), attr_name.xmlStr());
		result_element->setAttribute(ATTR_value.xmlStr(), attr_value.xmlStr());

		root_element->setAttribute(ATTR_id.xmlStr(), attr_id.xmlStr());
		root_element->setAttribute(ATTR_date.xmlStr(), attr_date.xmlStr());
		root_element->setAttribute(ATTR_localtime.xmlStr(), attr_localtime.xmlStr());
		root_element->setAttribute(ATTR_gmtime.xmlStr(), attr_gmtime.xmlStr());
		root_element->setAttribute(ATTR_user.xmlStr(), attr_user.xmlStr());

}
