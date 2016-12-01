# You should need to edit, if at all, only config.make
# Usually, this command is sufficient...
#
#     make CC=<c-compiler> HDF5_HOME=<path-to-hdf5> ZFP_HOME=<path-to-zfp>
#
# You should be able to override most options by assinging values to
# appropriate variables in the make command-line (e.g. make CFLAGS=...)
include ../../config.make

PROP_C_SRC = H5Zzfp_props.c
PROP_F_SRC = H5Zzfp_props_f.F90
PROP_C_OBJ = $(PROP_C_SRC:.c=.o)
PROP_F_OBJ = $(PROP_F_SRC:.F90=.o)
PROP_OBJ = $(PROP_C_OBJ)
ifneq ($(FC),)
    PROP_OBJ += $(PROP_F_OBJ)
    PROP_F_MOD = $(PROP_F_SRC:.F90=.mod)
endif

all: props

props: $(PROP_OBJ) $(PROP_F_MOD)

install: props
	$(INSTALL) -d $(PREFIX)/{lib,include}
	$(INSTALL) -b $(PROP_OBJ) $(PREFIX)/lib
	$(INSTALL) -b $(PROP_F_MOD) $(PREFIX)/include

clean:
	rm -f $(PROP_OBJ)
