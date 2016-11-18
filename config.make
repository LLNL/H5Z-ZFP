export SHELL = /bin/bash

ifeq ($(HDF5_HOME),)
    $(warning WARNING: HDF5_HOME not specified)
endif

ifeq ($(ZFP_HOME),)
    $(warning WARNING: ZFP_HOME not specified)
endif

MAKEVARS = ZFP_HOME=$(ZFP_HOME) HDF5_HOME=$(HDF5_HOME)

# Construct version variable depending on what dir we're in
PWD_BASE = $(shell basename $$(pwd))
ifeq ($(PWD_BASE),src)
    H5Z_ZFP_BASE := ./plugin
else ifeq ($(PWD_BASE),plugin)
    H5Z_ZFP_BASE := .
else ifeq ($(PWD_BASE),props)
    H5Z_ZFP_BASE := ../plugin
else ifeq ($(PWD_BASE),test)
    H5Z_ZFP_BASE := ../src/plugin
else ifeq ($(PWD_BASE),H5Z-ZFP)
    H5Z_ZFP_BASE := ./src/plugin
endif
H5Z_ZFP_PLUGIN := $(H5Z_ZFP_BASE)/lib
H5Z_ZFP_VERSINFO := $(shell grep '^\#define H5Z_FILTER_ZFP_VERSION_[MP]' $(H5Z_ZFP_BASE)/H5Zzfp.h | cut -d' ' -f3 | tr '\n' '.' | cut -d'.' -f-3 2>/dev/null)

# Detect system type
PROCESSOR := $(shell uname -p | tr '[:upper:]' '[:lower:]')
OSNAME := $(shell uname -s | tr '[:upper:]' '[:lower:]')
OSTYPE := $(shell env | grep OSTYPE | cut -d'=' -f2- | tr '[:upper:]' '[:lower:]')
# LLNL specific enviornment variable
SYS_TYPE := $(shell env | grep SYS_TYPE | cut -d'=' -f2- | tr '[:upper:]' '[:lower:]')

# Common C compilers
HAS_GCC := $(shell basename $$(which gcc 2>/dev/null) 2>/dev/null)
HAS_CLANG := $(shell basename $$(which clang 2>/dev/null) 2>/dev/null)
HAS_ICC := $(shell basename $$(which icc 2>/dev/null) 2>/dev/null)
HAS_PGCC := $(shell basename $$(which pgcc 2>/dev/null) 2>/dev/null)
HAS_XLCR := $(shell basename $$(which xlc_r 2>/dev/null) 2>/dev/null)
HAS_BGXLCR := $(shell basename $$(which bgxlc_r 2>/dev/null) 2>/dev/null)

# Common Fortran compilers
HAS_GFORTRAN := $(shell basename $$(which gfortran 2>/dev/null) 2>/dev/null)
HAS_IFORT := $(shell basename $$(which ifort 2>/dev/null) 2>/dev/null)
HAS_XLFR := $(shell basename $$(which xlf_r 2>/dev/null) 2>/dev/null)
HAS_BGXLFR := $(shell basename $$(which bgxlf_r 2>/dev/null) 2>/dev/null)

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

ifeq ($(FC),gfortran)
    FCFLAGS += -fPIC
else ifeq ($(FC),ifort)
    FCFLAGS += -fpic
else ifeq ($(FC),pgf90)
    FCFLAGS += -fpic
else ifeq ($(FC),xlf_r)
    FCFLAGS += -qpic
else ifeq ($(FC),bgxlf_r)
    FCFLAGS += -qpic
else ifeq ($(FC),f77)
# some makefile versions set FC=f77 if FC no value given
    FC =
endif

ZFP_INC = $(ZFP_HOME)/inc
ZFP_LIB = $(ZFP_HOME)/lib

HDF5_INC = $(HDF5_HOME)/include
HDF5_LIB = $(HDF5_HOME)/lib
HDF5_BIN = $(HDF5_HOME)/bin

LDFLAGS += -lhdf5

.c.o:
	$(CC) $< -o $@ -c $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

