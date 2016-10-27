HDF5_HOME ?= /Users/miller86/visit/tpl_build/hdf5-1.8.14/build/install
ZFP_HOME ?= ../zfp-0.5.0

# Detect system type
PROCESSOR := $(shell uname -p | tr '[:upper:]' '[:lower:]')
OSNAME := $(shell uname -s | tr '[:upper:]' '[:lower:]')
OSTYPE := $(shell env | grep OSTYPE | cut -d'=' -f2- | tr '[:upper:]' '[:lower:]')
SYS_TYPE := $(shell env | grep SYS_TYPE | cut -d'=' -f2- | tr '[:upper:]' '[:lower:]')

# Common compilers
HAS_GCC := $(shell basename $$(which gcc 2>/dev/null) 2>/dev/null)
HAS_CLANG := $(shell basename $(which clang 2>/dev/null) 2>/dev/null)
HAS_ICC := $(shell basename $$(which icc 2>/dev/null) 2>/dev/null)
HAS_PGCC := $(shell basename $$(which pgcc 2>/dev/null) 2>/dev/null)
HAS_XLCR := $(shell basename $$(which xlc_r 2>/dev/null) 2>/dev/null)
HAS_BGXLCR := $(shell basename $$(which bgxlc_r 2>/dev/null) 2>/dev/null)

# If compiler isn't set, lets try to pick it
ifeq ($(CC),)
    ifeq ($(OSNAME),darwin)
        ifneq ($(strip $(HAS_CLANG)),)
            CC = $(HAS_CLANG)
	else ifneq ($(strip $(HAS_GCC)),)
            CC = $(HAS_GCC)
        endif
    else ifneq ($(findstring ppc, $(PROCESSOR),),)
        ifneq ($(strip $(HAS_BGXLCR)),)
	    CC = $(HAS_BGXLCR)
        else ifneq ($(strip $(HAS_XLCR)),)
	    CC = $(HAS_XLCR)
        else ifneq ($(strip $(HAS_GCC)),)
	    CC = $(HAS_GCC)
        endif
    else
	ifneq ($(strip $(HAS_GCC)),)
            CC = $(HAS_GCC)
	else ifneq ($(strip $(HAS_ICC)),)
            CC = $(HAS_ICC)
	else ifneq ($(strip $(HAS_PGCC)),)
            CC = $(HAS_PGCC)
        endif
    endif
endif

# If we don't have a CC by now, error out
ifeq ($(CC),)
$(error $(CC))
endif

#
# Now, setup various flags based on compiler
#
ifeq ($(CC),gcc)
    CFLAGS += -fPIC
    SOEXT ?= so
    SHFLAG ?= -shared
    PREPATH = -Wl,-rpath,
else ifeq ($(CC),clang)
    SOEXT ?= dylib
    SHFLAG ?= -dynamiclib
    PREPATH = -L
else ifeq ($(CC),icc)
    CFLAGS += -fpic
    SOEXT ?= so
    SHFLAG ?= -shared
    PREPATH = -Wl,-rpath,
else ifeq ($(CC),pgcc)
    CFLAGS += -fpic
    SOEXT ?= so
    SHFLAG ?= -shared
    PREPATH = -Wl,-rpath,
else ifeq ($(CC),xlc_r)
    CFLAGS += -qpic
    SOEXT ?= so
    SHFLAG ?= -qmkshrobj
    PREPATH = -Wl,-R,
else ifeq ($(CC),bgxlc_r)
    CFLAGS += -qpic
    SOEXT ?= so
    SHFLAG ?= -qmkshrobj
    PREPATH = -Wl,-R, 
endif
