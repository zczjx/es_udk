#
# -= Makefile for module compile =-
#
# Usage:
# . Name this file as "Makefile";
#   Put it in the same directory as module's source code.
# . Change the ROOT definition, if necessary;
#   Make it point to the root of the software tree.
# . Define the Module Name in MOD as you like;
#   There should be NO duplicated module names in the whole project.
# . List all files want to be compiled in ASMS and/or SRCS;
#   Including files, such as header files, must NOT be listed here.
# . List all library files this module may depends on in LIBS.
# . Give a new name in SMOD, if want to include sub-directories;
#   Write the linkage information at the end of file.
#
                                                                                                                                                                         
# Destination of definition files
ROOT = 

# List of source files
SRCS = 				v4l2_camera_live_play.c \
				fb_param_test.c \
				camera_param_test.c
					
# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.

OBJS =  ${SRCS:.c= }



all :${OBJS}

	mkdir ./bin
	mv ${OBJS} ./bin


%:%.c
	echo ${CFLAGS}
	${CC} -fPIC ${CFLAGS} -L  ${TOPDIR}  -o $@  $< ${LDFLAGS} -les_udk
	
clean :
	  rm -f ${OBJS}
	  rm -rf ./bin
	  
rebuild: all
# End of common description.

# Construct of sub-modules


