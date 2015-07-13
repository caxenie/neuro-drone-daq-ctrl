##################################################
#
#	Makefile to compile a library
#
#
#	dependencies: 
#		libpthread,
# 		librt,
#		libm,
# 		libutils
#
#
##################################################


LIB_NAME		= libnpoint
OBJ_DIR 		= obj


CSRCS 			= npoint_conn.c
CSRCS 			+= npoint_logger.c
CSRCS 			+= npoint_packet.c

CPPSRCS			=


COMPILER_PATH 	= 
ARCH 			=


##################################################
#	compiling tools
##################################################
CC				= $(COMPILER_PATH)$(ARCH)gcc
CPP				= $(COMPILER_PATH)$(ARCH)g++
OBJCOPY			= $(COMPILER_PATH)$(ARCH)objcopy
OBJDUMP			= $(COMPILER_PATH)$(ARCH)objdump
SIZE 			= $(COMPILER_PATH)$(ARCH)size
NM 				= $(COMPILER_PATH)$(ARCH)nm
AR				= $(COMPILER_PATH)$(ARCH)ar
SIZE 			= $(COMPILER_PATH)$(ARCH)size


##################################################
# tell the makefile where to find the source files
##################################################
vpath %.c src
vpath %.cpp src


# compiler flags
CFLAGS = 

###################################################
# where to find the needed include files
###################################################

# include files of the project
CFLAGS += -Iinc



# include files of libraries
CFLAGS += -I../utils/inc



# remove all the ".c" and replace them with ".o"
OBJS = $(CSRCS:.c=.o)

# remove all the ".cpp" and replace them with ".o"
OBJS += $(CPPSRCS:.cpp=.o)

# build the object paths
OBJS_PATH = $(addprefix $(OBJ_DIR)/, $(OBJS))


all: $(LIB_NAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/$@ $^

%.o : %.cpp
	$(CPP) $(CFLAGS) -c -o $(OBJ_DIR)/$@ $^


$(LIB_NAME): $(OBJS)
	$(AR) -r $@.a $(OBJS_PATH)
#	$(SIZE) -t $(LIB_NAME)


clean:
#linux
	rm -f $(OBJ_DIR)/*.o
	rm -f $(LIB_NAME).a



