/***************************************************************************
 * File:        ./InputHandler.cpp
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
 * -------------------------------------------------------------------------
 * Description: Reads commandline:
 *			- check whether we're dealing with an XML file
 *			- if not, check whether we're having some params given
 ****************************************************************************/

#include "SimulationCore.h"
#include "InputHandler.h"

#include <stdlib.h>	/*atof*/
#include <unistd.h>	/*getopt*/
#include <string.h>	/*strtok*/
#include <ctype.h>	/*isprint*/

#include <string>	
#include <map>
#include <stdexcept>
#include <algorithm>

#include "PluginManager.h"
#include "ParamWrapper.h"
#include "LoadFunctionElement.h"
#include "info.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

InputHandler::InputHandler(int Argc, char *Argv[]):
     XMLHandler("dummy"),
     argc(Argc),
     isXML(false)
{
     argv = Argv;
     crusde_debug("%s, line: %d, InputHandler built for: %s ", __FILE__, __LINE__, argv[0]);
}

InputHandler::~InputHandler()
{
     crusde_debug("%s, line: %d, InputHandler destroyed.", __FILE__, __LINE__);
}

void InputHandler::init()
{
     int n(1);
	
     if (n == argc)
     {
          usage();
          exit(2);
     }
	
     /* first level argument parsing to determine basic switches		*/
     /* rest will be done in input handler, where it will be possible	*/
     /* to set the plugin parameters right away.				*/
     while (n < argc )
     {
          // break at param that has no dash and following arguments contains ".xml" which means a xml-file is to be processed
          if (argv[n][0] != '-' && (string(argv[n]).find(".xml", 0) != string::npos) )
          {
               isXML = true;
               break;
          }

          // help request
          if (!strncmp(argv[n], "-?", 2) || !strncmp(argv[n], "-h", 2) )
          {
               usage();
               exit(2);
          }

          // quiet run
          if (!strncmp(argv[n], "-q", 2) || !strncmp(argv[n], "-Q", 2) )
          {
               SimulationCore::instance()->setQuiet(true);
          }

          ++n;
     }
	
     /* in case of xml processing there should be only 1 argument left, 	*/
     /* if experiment is set via command line, none should be left.		*/
     if ( (isXML && n != argc - 1) /*||  (!isXML && n != argc)*/ )
     {
          usage();
          exit(2);
     }

     if(isXML)
     {
          xmlFile = string(argv[n]);
     }	

     /* decide whether to process an xml input file or to have a closer look	*/
     /* at the command line arguments and built a DOM from there.		*/	
     if(isXML)
     {	
          try
          {
               readXML();
          }
          catch ( std::runtime_error e)
          {
               crusde_warning("Error in InitXML: %s", e.what());
          }
     }
     else
     {
          readCommandline();
     }

}

void InputHandler::readCommandline()
{
     crusde_debug("%s, line: %d, read command line", __FILE__, __LINE__);

     char c;
     char delims[]=":";      /* delimiter in command line strings	*/
     char *result=NULL;      /* dummy used in string processing	*/
     int i(0);

     string install("install");

     /* process command line option and set variables*/     
     while ((c = getopt (argc, argv, "pPmMvVqQ")) != -1)
     {
	++i;

          /* get first parameter, used in all cases */
          result = strtok( optarg, delims );

          /* figure out where we at in the parameter space */
          switch (c)
          {
               case 'q': //quiet run
               case 'Q': //quiet run
                    SimulationCore::instance()->setQuiet(true);
		    break

               case 'm': //start experiment manager
               case 'M': //start experiment manager
                    SimulationCore::instance()->runExperimentManager();
                    break;

               case 'p': //start plugin manager
               case 'P': //start plugin manager
		    if(argc == 2){ //run GUI Manager if no further arguments given
	                    SimulationCore::instance()->runPluginManager();
                    }else{         //otherwise check what's requested
                        //INSTALL
                        if(!install.compare( argv[i+1] )) {
                            //if we want to install something, we need another argument
                            if(argc >= i+2){
                                 crusde_info("Trying to install plugin: %s", argv[i+2]);
                                 SimulationCore::instance()->installPlugin(argv[i+2]);
                            }else{
                                 crusde_error("Plugin file missing! Call: crusde -p install <plugin.so>");
                            }
                        }
		    }
                    break;

               case 'v': //show version
               case 'V': //show Version
                    version();
                    break;

               default:
                    abort();
          }
     }
}


void InputHandler::initParamsFromDOM()
{
     assert(doc);

     crusde_debug("%s, line: %d, initParamsFromDOM().", __FILE__, __LINE__);

     DOMElement *root = doc->getDocumentElement();

     initParamsFromDOM(root->getElementsByTagName(TAG_green.xmlStr())->item(0), SimulationCore::s_params.green);
     initParamsFromDOMNodeList(root->getElementsByTagName( TAG_load.xmlStr()), SimulationCore::s_params.load);
     initParamsFromDOM(root->getElementsByTagName(TAG_kernel.xmlStr())->item(0), SimulationCore::s_params.kernel);
     initParamsFromDOM(root->getElementsByTagName(TAG_postprocessor.xmlStr())->item(0), SimulationCore::s_params.postprocessor);
     if(root->getElementsByTagName(TAG_loadhistory.xmlStr())->getLength() > 0)
          initParamsFromDOMNodeList(root->getElementsByTagName(TAG_loadhistory.xmlStr()), SimulationCore::s_params.loadhistory);
     if(root->getElementsByTagName(TAG_crustaldecay.xmlStr())->getLength() > 0)
          initParamsFromDOMNodeList(root->getElementsByTagName(TAG_crustaldecay.xmlStr()), SimulationCore::s_params.crustaldecay);
}

void InputHandler::initParamsFromDOMNodeList(DOMNodeList *nodelist, map< unsigned int, multimap<string, ParamWrapper*> > registry)
{
     map< unsigned int, multimap<string, ParamWrapper*> >::iterator iter = registry.begin();
     unsigned int counter(0);
	
     //walk through the input list, this way warnings are triggered if something is not registered
     while(counter < nodelist->getLength())
     {
          if(iter != registry.end() )
          {
               initParamsFromDOM(nodelist->item(counter), iter->second);
               ++iter;
          }
          else
          {
               initParamsFromDOM(nodelist->item(counter), multimap<string, ParamWrapper*>() );
          }
          ++counter;
     }
}

void InputHandler::initParamsFromDOM(DOMNode *node, multimap<string, ParamWrapper*> registry)
{

     assert(node);
     char *tmp = XMLString::transcode(node->getNodeName());
     crusde_debug("%s, line: %d, initParamsFromDOM for node: %s.", __FILE__, __LINE__, tmp);
     XMLString::release(&tmp);

     DOMNode *child = node->getFirstChild();
     DOMNamedNodeMap *attributes = NULL;
     DOMNode *attr = NULL;

     multimap<string, ParamWrapper*>::iterator key_iter;
     pair< multimap<string, ParamWrapper*>::iterator, multimap<string, ParamWrapper*>::iterator > key_range;

     list<string> params_set;

     int count_keys(-1);
     unsigned int count_set(0);
     int empty_param(0);

     while (child)
     {
          if( child->getNodeType() == DOMNode::ELEMENT_NODE)
          {
               attributes = child->getAttributes();
               attr = attributes->getNamedItem(ATTR_name.xmlStr());

               char* name = XMLString::transcode(attr->getNodeValue());

               if( XMLString::compareIString(child->getNodeName(), TAG_parameter.xmlStr()) == 0 )
               {			
                    //get number of keys that equal name
                    count_keys = static_cast<int>( registry.count(string(name)) );
                    //if there is anything in the registry ... 
                    if( count_keys > 0 )
                    {		
                         //equal_range gives two results: an iterator to the first and last element with key==name
                         key_range = registry.equal_range(string(name));
                         //all keys have values that are adresses of double variables in the repsective plugins
                         //each of those variables now gets a value assigned. the same value.
                         for ( key_iter=key_range.first; key_iter != key_range.second; ++key_iter)
                         {
                              //get the value from the DOM
                              StrXML value(attributes->getNamedItem(ATTR_value.xmlStr())->getNodeValue());
                              //write it into the variable that's strored at key_iter->second
                              if( (key_iter->second)->isString() )
                              {
                                   (key_iter->second)->setValue( value.cppStr() );
                              }
                              else if( (key_iter->second)->isDouble() )
                              {
                                   (key_iter->second)->setValue( static_cast<double>( atof(value.cStr()) ) );
                              }

                              ++count_set;
                         }
                         //memorize the key that was set only once
                         params_set.push_back(key_range.first->first);
                    }
                    else //if not found there might be a spelling mistake either in input, or plugin, or both :)
                    {
                         StrXML paramName(attributes->getNamedItem(ATTR_name.xmlStr())->getNodeValue());
                         crusde_warning("Parameter %s (coming from the experiment definition) not registered! Misspelled in XML file or Plug-in definition?",paramName.cStr());
                    }
               }

               XMLString::release(&name);         	
          }
		
          child = child->getNextSibling();
     }
	
     //two cases are possible for leftover parameters: 
     //     i)  they are optional parameters - check registry for that
     //     ii) they are unitialized - oh well, we gotta stop there
     if(count_set < registry.size())
     {
          multimap<string, ParamWrapper*>::iterator map_iter = registry.begin();
          list<string>::iterator found_iter;
          string out_string("Error: Parameters that remain uninitialized: \n");		

          while (map_iter != registry.end())
          {
               if(!map_iter->second->isOptional())
               {
                    found_iter = params_set.begin();
                    //as long as we're not at the end and could not find the parameter in the list of set 
                    //parameters, continue looking.
                    while( found_iter != params_set.end() && (*found_iter).compare(map_iter->first) != 0  ) 
                    {
                         ++found_iter;
                    } 	
                    //if we're at the end, the parameter was not in out list ... tell the user.			
                    if(found_iter==params_set.end() ) 
                    {
                         ++empty_param;
                         out_string.append("\t");
                         out_string.append(map_iter->first);
                         out_string.append("\n");
                    }
               }
		
               //get to the next unique value in the registry ... avoid printing multiple keys twice.
               map_iter = ( registry.equal_range(map_iter->first) ).second;
          }

          if(empty_param > 0) 
          {
               crusde_error("%s, Aborting.", out_string.c_str());
          }
     }		
}	

string InputHandler::getAttributeValueByName(const XMLCh* elementNode, const XMLCh* attribute, const XMLCh* name)
{

     crusde_debug("%s, line: %d, InputHandler::getAttributeValueByName(%s) name = %s ", __FILE__, __LINE__, XMLString::transcode(elementNode),
     XMLString::transcode(name));
     assert(doc);

     DOMElement *root = doc->getDocumentElement();
     DOMNode *child = root->getFirstChild();
     DOMNamedNodeMap *attributes = NULL;
		
     while (child)
     {
          if(child != NULL)
               if( child->getNodeType() == DOMNode::ELEMENT_NODE )
               {
                    if(child->hasAttributes())
                    {
                         attributes = child->getAttributes();

                         if( XMLString::compareIString( child->getNodeName(), elementNode) == 0 && 
                             XMLString::compareIString( attributes->getNamedItem(ATTR_name.xmlStr())->getNodeValue(), name) == 0 )
                         {
                              char *val = XMLString::transcode(attributes->getNamedItem(attribute)->getNodeValue());
                              string value(val);
                              XMLString::release(&val);
                              return value;
                         }
                    }
               }

          child = child->getNextSibling();
     }
	
     return string();
}


/**
 * extract's the value of an attribute and returns it:
 *
 * <parentNode>
 *	<elementNode attribute="returnstring" />
 * </parentNode>
 *
 * the first parentNode found in the document is used. thus, it is expected to be unique.
 *
 * @param parentNode 
 * @param elementNode 
 * @param attribute 
 * @return 
 */
string InputHandler::getAttributeValue(const XMLCh* parentNode, const XMLCh* elementNode, const XMLCh* attribute)
{
crusde_debug("%s, line: %d, InputHandler::getAttributeValue(%s) ", __FILE__, __LINE__, XMLString::transcode(elementNode));

	DOMElement *root = doc->getDocumentElement();
	DOMNodeList *node_list = root->getElementsByTagName(parentNode);
	
	/*if element does not exist, return emptry string*/
	if(node_list->getLength() == 0)
		return string();
		
	DOMNode *child = node_list->item(0)->getFirstChild();

	DOMNamedNodeMap *attributes = NULL;
		
	while (child)
	{
		if( child->getNodeType() == DOMNode::ELEMENT_NODE)
	       	{
			attributes = child->getAttributes();

			if( XMLString::compareIString(child->getNodeName(), elementNode) == 0 )
			{
				char *val = XMLString::transcode(attributes->getNamedItem(attribute)->getNodeValue());
				string value(val);
				XMLString::release(&val);
				return value;
			}
			
		}
		child = child->getNextSibling();
	}
	return string();
}

string InputHandler::getGreenPlugin(string name)
{
	try{
		return SimulationCore::instance()->pluginManager()->getFilename("green", name);		
	}
	catch( ... ){
		throw;
	}
}

string InputHandler::getLoadPlugin()
{
	try{
		string name(getAttributeValue(TAG_load.xmlStr(), TAG_plugin.xmlStr(), ATTR_name.xmlStr()));
printf("%s, line: %d, InputHandler::getLoadPlugin() name = %s ", __FILE__, __LINE__, name.c_str());
		return SimulationCore::instance()->pluginManager()->getFilename("load", name);
	}
	catch( ... ){
		throw;
	}
}

string InputHandler::getLoadPlugin(string name)
{
	try{
		return SimulationCore::instance()->pluginManager()->getFilename("load", name);
	}
	catch( ... ){
		throw;
	}
}
/* TODO: delete
string InputHandler::getLoadHistoryPlugin()
{
	try{
		string name(getAttributeValue(TAG_loadhistory.xmlStr(), TAG_plugin.xmlStr(), ATTR_name.xmlStr()));
		if(!name.empty())
			return SimulationCore::instance()->pluginManager()->getFilename("load_history", name);
		else{
			return string();
		}
	}
	catch( ... ){
		throw;
	}
}
*/
string InputHandler::getLoadHistoryPlugin(string name)
{
	try{
		return SimulationCore::instance()->pluginManager()->getFilename("load_history", name);
	}
	catch( ... ){
		throw;
	}
}
/* TODO: delete
string InputHandler::getCrustalDecayPlugin()
{
	try{
		string name(getAttributeValue(TAG_loadhistory.xmlStr(), TAG_plugin.xmlStr(), ATTR_name.xmlStr()));
		if(!name.empty())
			return SimulationCore::instance()->pluginManager()->getFilename("crustal_decay", name);
		else{
			return string();
		}
	}
	catch( ... ){
		throw;
	}
}
*/
string InputHandler::getCrustalDecayPlugin(string name)
{
	try{
		return SimulationCore::instance()->pluginManager()->getFilename("crustal_decay", name);
	}
	catch( ... ){
		throw;
	}
}

string InputHandler::getKernelPlugin()
{
	try{
		string name(getAttributeValue(TAG_kernel.xmlStr(), TAG_plugin.xmlStr(), ATTR_name.xmlStr()));
		return SimulationCore::instance()->pluginManager()->getFilename("operator", name);
	}
	catch( ... ){
		throw;
	}
}

string InputHandler::getKernelPlugin(string name)
{
	try{
		return SimulationCore::instance()->pluginManager()->getFilename("operator", name);
	}
	catch( ... ){
		throw;
	}
}

string InputHandler::getOutputPlugin()
{
	try{
		string name(getAttributeValue(TAG_output.xmlStr(), TAG_plugin.xmlStr(), ATTR_name.xmlStr()));
		return SimulationCore::instance()->pluginManager()->getFilename("data_handler", name);
	}
	catch( ... ){
		throw;
	}

}

string InputHandler::getOutputPlugin(string name)
{
	try{
		return SimulationCore::instance()->pluginManager()->getFilename("data_handler", name);
	}
	catch( ... ){
		throw;
	}
}

string InputHandler::getFileName(string type)
{	
	StrXML t(type);
	return getAttributeValueByName(TAG_file.xmlStr(), ATTR_value.xmlStr(), t.xmlStr());
	
}

string InputHandler::getResultFileName()
{	
	StrXML r("result");
	return getAttributeValueByName(TAG_file.xmlStr(), ATTR_value.xmlStr(), r.xmlStr());
	
}

int InputHandler::getRegion(string direction)
{
	StrXML d(direction);
	return atoi(getAttributeValueByName(TAG_region.xmlStr(), ATTR_value.xmlStr(), d.xmlStr()).c_str());
}

int InputHandler::getGridSize()
{
	StrXML g("gridsize");
	return atoi(getAttributeValueByName(TAG_parameter.xmlStr(), ATTR_value.xmlStr(), g.xmlStr()).c_str());
}

unsigned long int InputHandler::getTimeSteps()
{
	StrXML t("timesteps");
	string timesteps(getAttributeValueByName(TAG_parameter.xmlStr(), ATTR_value.xmlStr(), t.xmlStr()));
	if(!timesteps.empty())
		return atol(timesteps.c_str());
	else
		return 1;
}

unsigned long int InputHandler::getTimeIncrement()
{
	StrXML t("timestep_size");
	string timestep_size(getAttributeValueByName(TAG_parameter.xmlStr(), ATTR_value.xmlStr(), t.xmlStr()));
	if(!timestep_size.empty())
		return atol(timestep_size.c_str());
	else
		return 1;
}

DOMNode* InputHandler::getExperiment()
{
	return doc->getDocumentElement();
}

list<string> InputHandler::getPostProcessorNameList()
{
	list<string> names;
	
	//get the postprocessor node 
	DOMElement *elem = dynamic_cast<DOMElement*>(getElementByTagName(TAG_postprocessor.xmlStr()));
	//check whether there is one
	if(!elem)
	{
		crusde_info("No postprocessors found.");
		return names;
	}
	//if so, grab all plugins it owns.
	DOMNodeList *plugins = elem->getElementsByTagName(TAG_plugin.xmlStr());
	unsigned int i(0);
	
	//if there are plugins ... 
	while(plugins && i < plugins->getLength() )
	{
		if( plugins->item(i)->getNodeType() == DOMNode::ELEMENT_NODE)
		{			
			/* upcast child to element node */
			DOMElement *elem = dynamic_cast<DOMElement*>( plugins->item(i) );
			StrXML name(elem->getAttribute(ATTR_name.xmlStr()));
			//... add names to list for later lookup.
			names.push_back(name.cppStr());
		}
		++i;		
	}
	
	//return filled list
	return names;
}

bool InputHandler::jobExists(string job_name)
{
	//get all load functions
	DOMNodeList *job_list = getElementListByTagName(TAG_job.xmlStr());
	
	if(	job_list->getLength() == 0)
	{
		crusde_warning("No jobs found, although %s is referenced from a load.");
		return false;
	}

	unsigned int i(0);

	while(i < job_list->getLength() )
    {
		if( job_list->item(i)->getNodeType() == DOMNode::ELEMENT_NODE)
		{			
			DOMElement *job_elem = dynamic_cast<DOMElement*>( job_list->item(i) );
			StrXML name(job_elem->getAttribute(ATTR_name.xmlStr()));

			if( (name.cppStr()).compare(job_name) == 0)
			{
				return true;
			}
		}
		++i;
	}

	return false;	
}


/**
 * returns a map that contains load functions. a load is mapped to a pair consiits load_history
 */
list<LoadFunctionElement*> InputHandler::getLoadFunctionList()
{
     list<LoadFunctionElement*> load_function;

     //Load function elem we try to fill ... multiple times :)
     LoadFunctionElement *lf_elem;

     //get all load functions
     DOMNodeList *load_function_list = getElementListByTagName(TAG_loadfunction.xmlStr());

     //check whether there is any load defined
     if( load_function_list->getLength() == 0 )
     {
          crusde_info("No load functions found.");
          return load_function;
     }

     //if so, we want to get the load functions' load and the load history definitions
     //and put it all in a nice map to keep track of which belongs where.
     unsigned int i(0);

     crusde_debug("%s, line: %d, load_function_list has length: %d", __FILE__, __LINE__, load_function_list->getLength());
    
     while(i < load_function_list->getLength() )
     {
          if( load_function_list->item(i)->getNodeType() == DOMNode::ELEMENT_NODE)
          {			
               DOMElement *load_function_elem = dynamic_cast<DOMElement*>( load_function_list->item(i) );

               //get the load node 
               //-------------------
               DOMElement *load_elem = dynamic_cast<DOMElement*>( load_function_elem->getElementsByTagName(TAG_load.xmlStr())->item(0) );

               //we need a load
               if(!load_elem)
               {
                    crusde_warning("No load in load-function #%d found", i);
                    return load_function;
               }
			         
               //get the load-plugin
               DOMElement *load_plugin_elem = dynamic_cast<DOMElement*>(load_elem->getElementsByTagName(TAG_plugin.xmlStr())->item(0));

               //we need a load plugin
               if(!load_plugin_elem)
               {
                    crusde_warning("No load plug-in in load-function #%d found", i);
                    return load_function;
               }

               StrXML load_name( load_plugin_elem->getAttribute(ATTR_name.xmlStr()) );

               //we got something create new load function elem, and give it the load name
               lf_elem = new LoadFunctionElement( load_name.cppStr() );

               //get the history node 
               //--------------------
               DOMElement *history_elem = dynamic_cast<DOMElement*>( load_function_elem->getElementsByTagName(TAG_loadhistory.xmlStr())->item(0) );
               //a history is not necessary
               if(history_elem) 
               {
                    //get the history-plugin
                    DOMElement *history_plugin_elem = dynamic_cast<DOMElement*>( history_elem->getElementsByTagName(TAG_plugin.xmlStr())->item(0) );

                    //we need a history plug-in once the history is defined				
                    if(!history_plugin_elem) 
                    { 
                         crusde_warning("No load history plugin in load history definition #%d found", i);
                         return load_function;
                    }	
                    StrXML history_name ( history_plugin_elem->getAttribute(ATTR_name.xmlStr()) ); 
                    //add history plug-in to load element
                    lf_elem->setHistoryName( history_name.cppStr() );

                    //add history elem job attribute
                    StrXML job_name( history_elem->getAttribute(ATTR_job.xmlStr()  ));

                    if( !job_name.cppStr().empty())
                    {
                         crusde_debug("%s, line: %d, History %s only for job: %s", __FILE__, __LINE__, history_name.cStr(), job_name.cStr() );

                         if( jobExists(job_name.cppStr()) == true)
                         {
                              lf_elem->setHistoryJob( job_name.cppStr() );
                         }
                         else
                         {
                              crusde_warning("Job name given for load history '%s': %s was not defined as a job. Check spelling?", history_name.cStr(), job_name.cStr() );
                              return load_function;
                         }
                    }
					
               }

               //get the decay node 
               //--------------------
               DOMElement *decay_elem = dynamic_cast<DOMElement*>( load_function_elem->getElementsByTagName(TAG_crustaldecay.xmlStr())->item(0) );
               //a decay is not necessary
               if(decay_elem) 
               {
                    //get the decay-plugin
                    DOMElement *decay_plugin_elem = dynamic_cast<DOMElement*>( decay_elem->getElementsByTagName(TAG_plugin.xmlStr())->item(0) );

                    //we need a history plug-in once the history is defined				
                    if(!decay_plugin_elem) 
                    { 
                         crusde_warning("No crustal decay plugin in crustal decay definition #%d found.", i);
                         return load_function;
                    }	
                    StrXML decay_name( decay_plugin_elem->getAttribute( ATTR_name.xmlStr() ) ); 
                    //add to load element
                    lf_elem->setDecayName( decay_name.cppStr() );

                    //add history elem job attribute
                    StrXML job_name( decay_elem->getAttribute( ATTR_job.xmlStr() ) );

                    if( !job_name.cppStr().empty())
                    {
                         //crusde_debug("%s, line: %d, History %s only for job: %s", __FILE__, __LINE__, decay_name.cStr(), job_name.cStr() );
                         if( jobExists(job_name.cppStr()) == true)
                         {
                              lf_elem->setDecayJob( job_name.cppStr() );
                         }
                         else
                         {
                              crusde_warning("Job name given for crustal decay function '%s': %s was not defined as a job. Check spelling?", decay_name.cStr(), job_name.cStr());
                              return load_function;
                         }
                    }
               }

               //add load function elem to list which will be returned
               //---------------------------------
               load_function.push_back(lf_elem);

          }

          ++i;
     }

     //return filled map
     return load_function;
}

map<string, string> InputHandler::getGreenJobMap()
{

     map<string, string> green_job_map;
	
     //get the list of jobs that are described within the greens function definition.
     DOMElement  *green_elem = dynamic_cast<DOMElement*>( getElementByTagName(TAG_green.xmlStr()) );
     DOMNodeList *job_list = green_elem->getElementsByTagName(TAG_job.xmlStr());

     //if there are no jobs defined, we're well off: we have to get the green's function plug-in name and
     //insert only a <NULL, path_to_plugin> pair into the map. A NULL value for a job name 
     //shall be the indicator that no jobs are defined; hence only one green's function plug-in must be loaded.
     if(job_list == NULL || job_list->getLength() == 0)
     {
          string name( getAttributeValue(TAG_green.xmlStr(), TAG_plugin.xmlStr(), ATTR_name.xmlStr()) );
          string path( (SimulationCore::instance()->pluginManager())->getFilename(string("green"), name) );

          green_job_map.insert(pair<string, string > (string(), path) );

          return green_job_map;
     }


     unsigned int i(0);

     crusde_debug("%s, line: %d, job_list has length: %d", __FILE__, __LINE__, job_list->getLength());
     
     while(i < job_list->getLength() )
     {
          if( job_list->item(i)->getNodeType() == DOMNode::ELEMENT_NODE)
          {			
               DOMElement *job_elem = dynamic_cast<DOMElement*>( job_list->item(i) );

               //check whether job is already in map
               //-----------------------------------
               //get job name
               StrXML job_name(job_elem->getAttribute(ATTR_name.xmlStr()));

               map< string, string>::iterator iter = green_job_map.begin();
               //walk through the input list, this way warnings are triggered if something is not registered
               while(iter != green_job_map.end() )
               {
                    if((iter->first).compare(job_name.cppStr()) == 0)
                    {
                         crusde_error("Job %s defined twice! First time with Plug-in: %s", job_name.cStr(), iter->second.c_str()); 
                    }
                    ++iter;
               }

               //get the plug-in node
               //-------------------
               //get the green-plugin
               DOMElement *green_plugin_elem = dynamic_cast<DOMElement*>( job_elem->getElementsByTagName(TAG_plugin.xmlStr())->item(0) );

               //we need a load plugin
               if(!green_plugin_elem)
               {
                    crusde_error("No green plugin in job definition #%d found.", i);
               }
               //get plug-in name and plug-in path
               StrXML green_name(green_plugin_elem->getAttribute(ATTR_name.xmlStr()));
               string path( (SimulationCore::instance()->pluginManager())->getFilename(string("green"), green_name.cppStr()) );

               //add to map which will be returned
               //---------------------------------
               green_job_map.insert(pair<string,string>( job_name.cppStr(), path ) );
               crusde_debug("%s, line: %d, Found Job: '%s' with plug-in '%s' (%s)", __FILE__, __LINE__, job_name.cStr(), green_name.cStr(), path.c_str());
          }
          ++i;
     }

     //return filled map
     return green_job_map;
}

