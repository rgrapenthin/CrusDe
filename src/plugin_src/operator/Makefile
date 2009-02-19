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

LINKER_OPT 	:= -lfftw3 -lm -shared -lc

SOURCE	 	:= $(shell ls *.c 2>/dev/null)
OBJS 		:= $(notdir $(SOURCE:%.c=%.o))
SO_OBJS 	:= $(notdir $(OBJS:%.o=%.so))

INCLUDE_DIRS	:= -I/usr/local/fftw/api -I/usr/include -I. -I../..

.PHONY: all clean help
.SUFFIXES: .so

# --- compile sources
.c.o:
	gcc -c -fpic -Wall $(INCLUDE_DIRS) $<
	
.o.so:
	gcc $(INCLUDE_DIRS) -o $(*F).so $< $(LINKER_OPT)
	cp  ./*.so ../../../plugins/operator

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
	
