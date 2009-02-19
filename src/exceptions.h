/***************************************************************************
 * File:        ./exceptions.h
 * Author:      Ronni Grapenthin, NORVULK & HU-BERLIN
 * Created:     23.04.2007
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

/** 
 *  @defgroup Exceptions CrusDe specific exceptions
 *  @ingroup Exceptions
 **/

/*@{*/
/** \file exceptions.h
 * 
 *	Defines and implements all exception for CrusDe
 */
/*@}*/

#ifndef crusde_exceptions_h	
#define crusde_exceptions_h

#include <string>
#include <stdexcept>

//!An exception that should lead to shutdown of the application-
class SeriousException : public std::runtime_error {
 public:
   SeriousException(const std::string& s)
     : std::runtime_error(s)
     { }
};

//! Some Conversion went wrong ... serious 
class Conversion : public SeriousException {
 public:
   Conversion(const std::string& s)
     : SeriousException(s)
     { }
 };

//!A file could not be found.
class FileNotFound : public SeriousException {
 public:
   FileNotFound(const std::string& s)
     : SeriousException(s)
     { }
 };

 //!Something went wrong with Library access
class LibHandleError : public SeriousException {
 public:
   LibHandleError(const std::string& s)
     : SeriousException (s)
     { }
 };
 
//! Something's wrong with the database ... one might consider shutdown.
class DatabaseError : public std::runtime_error {
 public:
   DatabaseError(const std::string& s)
     : std::runtime_error(s)
     { }
 };
//! Some Plugin error 
class PluginError : public std::runtime_error {
 public:
   PluginError(const std::string& s)
     : std::runtime_error(s)
     { }
};

class PluginExistsException : public std::runtime_error {
 public:
   PluginExistsException(const std::string& s)
     : std::runtime_error(s)
     { }
};
#endif // crusde_exceptions_h
