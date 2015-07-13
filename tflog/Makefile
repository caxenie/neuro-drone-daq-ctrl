###################################################
#
#	Makefile to compile the program
#
###################################################

PROJ_NAME     	= tflog
PATHDIR	      	= tflog
OUTPATH		  	= .
OBJ_DIR 		= obj
LIBROOTPATH	  	=

CSRCS 			= main.c
CSRCS			+= uio_manager.c


CPPSRCS 		= 

COMPILER_PATH 	= 
ARCH			=

##################################################
CC				= $(COMPILER_PATH)$(ARCH)gcc
CPP				= $(COMPILER_PATH)$(ARCH)g++
OBJCOPY			= $(COMPILER_PATH)$(ARCH)objcopy
OBJDUMP			= $(COMPILER_PATH)$(ARCH)objdump
SIZE 			= $(COMPILER_PATH)$(ARCH)size
NM 				= $(COMPILER_PATH)$(ARCH)nm
#AR			= $(COMPILER_PATH)$(COMPILER_PATH)-ar


# optimization level
#CFLAGS		= -O3

LINKFLAGS	= 

##################################################
# tell the makefile where to find the source files
##################################################
vpath %.c src
vpath %.cpp src



###################################################
# where to find the needed include files
###################################################

#include files of the project
CFLAGS += -Iinc



#include files of libraries
#CFLAGS += -I /usr/local/include/opencv

CFLAGS += -I ../tflog-libs/conns/inc
CFLAGS += -I ../tflog-libs/npoint/inc
CFLAGS += -I ../tflog-libs/px4remote/inc
CFLAGS += -I ../tflog-libs/utils/inc

###################################################
# Library paths
###################################################

#LIBPATHS = -L$(LIBROOTPATH)
#LIBPATHS += -L /usr/local/lib

LIBPATHS += -L ../tflog-libs/conns
LIBPATHS += -L ../tflog-libs/npoint
LIBPATHS += -L ../tflog-libs/px4remote
LIBPATHS += -L ../tflog-libs/utils


###################################################
# Libraries to link
# ATTENTION: add libraries also to linker group !!!
# (not needed if $(LIBS) is placed in the right manner)
###################################################

LIBS += -pthread -lrt
LIBS += -lm
LIBS += -lconns
LIBS += -lnpoint
LIBS += -lpx4remote
LIBS += -lutils


# remove all the ".c" and replace them with ".o"
OBJS = $(CSRCS:.c=.o)
OBJS += $(CPPSRCS:.cpp=.o)

OBJS_PATH = $(addprefix $(OBJ_DIR)/, $(OBJS))


all: $(OUTPATH)/$(PROJ_NAME)

%.o : %.c
	$(CPP) $(CFLAGS) -c -o $(OBJ_DIR)/$@ $^
	
%.o : %.cpp
	$(CPP) $(CFLAGS) -c -o $(OBJ_DIR)/$@ $^

# link everything to final programm	

$(OUTPATH)/$(PROJ_NAME): $(OBJS)
#	echo $(OBJS)
	$(CPP) $(LINKFLAGS) $(CFLAGS) $(LIBPATHS) -o $(OUTPATH)/$(PROJ_NAME) $(OBJS_PATH) $(LIBS)


clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(OUTPATH)/$(PROJ_NAME)




