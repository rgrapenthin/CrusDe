#################################################################################
#										#
#			           CrusDe MAKEFILE				#
#			       =====================				#
#										#
#	 author:  Ronni Grapenthin						#
#	   date:  20.02.07							#
#	license:  GPLv2								#
#										#
# CrusDe, simulation framework for crustal deformation studies			#
# Copyright (C) 2007 Ronni Grapenthin						#
#										#
# This program is free software; you can redistribute it and/or			#
# modify it under the terms of the GNU General Public License			#
# as published by the Free Software Foundation; version 2			#
# of the License.								#
# 										#
# This program is distributed in the hope that it will be useful,		#
# but WITHOUT ANY WARRANTY; without even the implied warranty of		#
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			#
# GNU General Public License for more details.					#
# 										#
# You should have received a copy of the GNU General Public License		#
# along with this program; if not, write to the Free Software			#
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#										#
#################################################################################

# --- Target name
TARGET                  := ../crusde
BIN                     := /usr/local/bin
# --- path to Qt installation
export QT_DIR           := /usr/local/Trolltech/Qt
export QT_INCLUDES      := -I$(QT_DIR)/include -I$(QT_DIR)/include/Qt -I$(QT_DIR)/include/QtAssistant -I$(QT_DIR)/include/QtCore -I$(QT_DIR)/include/QtGui -I$(QT_DIR)/include/QtXml

#--- directory information
export INCLUDE_DIRS    := -I/usr/include -I. -I$(shell pwd) -I/usr/local/xerces-c/include #-I$(shell pwd)/../libs/netcdf/include
export LIB_DIRS        := -L/usr/local/libs/xerces-c/lib -L/usr/X11R6/lib -L$(QT_DIR)/lib #-L$(shell pwd)/../libs/netcdf/lib
export PLUGIN_DIR      := ./plugin_src
export GREEN_DIR       := $(PLUGIN_DIR)/green
export LOAD_DIR        := $(PLUGIN_DIR)/load
export KERNEL_DIR      := $(PLUGIN_DIR)/operator
export DATA_DIR        := $(PLUGIN_DIR)/data_handler
export POSTPROCESS_DIR := $(PLUGIN_DIR)/postprocess
export LOADHISTORY_DIR := $(PLUGIN_DIR)/load_history
export CRUSTALDECAY_DIR := $(PLUGIN_DIR)/crustal_decay

#--- linker options, might be different from plugins', thus local
LINKER_OPT             := -lm -ldl -lstdc++ -lxerces-c -rdynamic
#LINKER_QT              := -lQtGui -lQtCore  -lQtXml -lfreetype -lfontconfig -lpthread -lXrender -lpng  -lXcursor -lXi  -lXrandr -lXinerama -lSM -lrt 
LINKER_QT              := -lQtGui -lQtCore  -lQtXml -lX11 -lrt 

#--- debug flag, empty by default, set specifically for each target below
DEBUG                  := 

#--- this directories' sources
SOURCE                 := $(shell ls *.c *.cpp 2>/dev/null)
HEADER                 := $(shell ls *.h 2>/dev/null)
OBJS                   := $(notdir $(SOURCE:%.cpp=%.o))
OBJS                   := $(notdir $(OBJS:%.c=%.o))

#--- flags for make
export M_FLAGS         := --no-print-directory

#--- Meta Object Compiler  - needed by all header files 
#--- which use the Q_OBJECT macro
MOC                    := $(QT_DIR)/bin/moc
MOC_MODULES            := $(patsubst %.h, moc_%.o, $(shell grep -l Q_OBJECT *.h))


#--- always do this ...
.PHONY: all clean help plugins

#--- compile sources (C)
.c.o:
	gcc -c -Wall $(INCLUDE_DIRS) $<

#--- compile sources (C++) 
.cpp.o:
	g++ -c -Wall $(DEBUG) $(INCLUDE_DIRS) $(QT_INCLUDES) $<

moc_%.cpp: %.h
	$(MOC) $< -o $@

#--- dependencies of the source files ... see rule for defs.h
#--- need to be included before the rules
include defs.h


# --- help  ---------------------------------------------
help: 		# hope this helps
	@echo '-----------------------------------------------------------------------------------'
	@echo 'this is the makefile for the deformation package'
	@echo "usage: make [option]"
	@echo ;
	@echo "option (without colon) = "
	@egrep '^[^:;=.]*::?[	 ]*#' [mM]akefile
	@echo '-----------------------------------------------------------------------------------'
	echo $(OBJS)

defs.h: $(SOURCE)
	@echo --------------------------------------
	gcc -MM $(INCLUDE_DIRS) $(QT_INCLUDES) $? > defs.h
	@echo --------------------------------------

all: plugins $(TARGET)
	cp $(TARGET) $(BIN)	

debug: DEBUG = -DDEBUG
debug: plugins $(TARGET)
	cp $(TARGET) $(BIN)

# ----- the targets ...
$(TARGET): defs.h $(MOC_MODULES) $(OBJS) $(HEADER) #creates the 'crusde'-binary to model deformation using green's functions.
	gcc -o $@ $(OBJS) $(MOC_MODULES) $(LIB_DIRS) $(LINKER_OPT) $(LINKER_QT)

plugins:	# creates plugins
	@echo --------------------------------------
	@echo making plugins ...	
	@make $(M_FLAGS) -C $(GREEN_DIR) all
	@make $(M_FLAGS) -C $(LOAD_DIR) all
	@make $(M_FLAGS) -C $(KERNEL_DIR) all
	@make $(M_FLAGS) -C $(DATA_DIR) all
	@make $(M_FLAGS) -C $(POSTPROCESS_DIR) all
	@make $(M_FLAGS) -C $(LOADHISTORY_DIR) all
	@make $(M_FLAGS) -C $(CRUSTALDECAY_DIR) all
	@echo plugins done ...
	@echo --------------------------------------

clean:		# cleans directories
	-rm *.o crusde defs.h moc_*
	@make $(M_FLAGS) -C $(GREEN_DIR) clean
	@make $(M_FLAGS) -C $(LOAD_DIR) clean 
	@make $(M_FLAGS) -C $(KERNEL_DIR) clean
	@make $(M_FLAGS) -C $(DATA_DIR) clean
	@make $(M_FLAGS) -C $(POSTPROCESS_DIR) clean	
	@make $(M_FLAGS) -C $(LOADHISTORY_DIR) clean
	@make $(M_FLAGS) -C $(CRUSTALDECAY_DIR) clean

