# You should need to edit, if at all, only config.make
# Usually, this command is sufficient...
#
#     make CC=<c-compiler> HDF5_HOME=<path-to-hdf5> ZFP_HOME=<path-to-zfp>
#
# You should be able to override most options by assinging values to
# appropriate variables in the make command-line (e.g. make CFLAGS=...)
include ../../config.make

PROP_SRC = H5Zzfp_props.c
PROP_OBJ = $(PROP_SRC:.c=.o)

all: props

props: $(PROP_OBJ)

clean:
	rm -f $(PROP_OBJ)
