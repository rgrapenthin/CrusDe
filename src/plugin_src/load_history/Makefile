#################################################################################
#										#
#			          PLUGIN MAKEFILE				#
#			       =====================				#
#										#
#	 author:  Ronni Grapenthin						#
#	   date:  20.02.2006							#
#	license:  GPL								#
#										#
#################################################################################

LINKER_OPT 	:= -lm -shared -lc -lnetcdf
SOURCE	 	:= $(shell ls *.c 2>/dev/null)
OBJS 		:= $(notdir $(SOURCE:%.c=%.o))
SO_OBJS 	:= $(notdir $(OBJS:%.o=%.so))

.PHONY: all clean help
.SUFFIXES: .so

# --- compile sources
.c.o:
	gcc -c -fpic -Wall $(INCLUDE_DIRS) $<
	
.o.so:
	gcc -o $(*F).so $< $(LIB_DIRS) $(LINKER_OPT) $(INCLUDE_DIRS) 
	cp  ./*.so ../../../plugins/load_history

include defs.h


# --- help  ---------------------------------------------
help: 		# this help
	@echo '-----------------------------------------------------------------------------------'
	@echo 'this is the makefile for the deformation package'
	@echo "usage: make [option]"
	@echo ;
	@echo "option (without colon) = "
	@egrep '^[^:;=.]*::?[	 ]*#' [mM]akefile
	@echo '-----------------------------------------------------------------------------------'

defs.h: $(SOURCE)
	@echo --------------------------------------
	gcc -MM $(INCLUDE_DIRS) $? > defs.h
	@echo --------------------------------------

all: $(SO_OBJS)	#rule to compile all plugins into a shared library each ... watch out: each source equals one library!
	

clean:		# cleans directory
	-rm *.o *.so defs.h
	
