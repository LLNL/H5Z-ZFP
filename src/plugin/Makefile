# You should need to edit, if at all, only config.make
# Usually, this command is sufficient...
#
#     make CC=<c-compiler> HDF5_HOME=<path-to-hdf5> ZFP_HOME=<path-to-zfp>
#
# You should be able to override most options by assinging values to
# appropriate variables in the make command-line (e.g. make CFLAGS=...)
include ../../config.make

PLUGIN_SRC = H5Zzfp.c
PLUGIN_OBJ = $(PLUGIN_SRC:.c=.o)

all: plugin

plugin: $(PLUGIN_OBJ)
	rm -rf lib
	mkdir lib
	$(CC) H5Zzfp.o $(SHFLAG) -o lib/libh5zzfp.$(SOEXT) \
	    $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L$(ZFP_LIB) -L$(HDF5_LIB) $(LDFLAGS) -lzfp

install: plugin
	$(INSTALL) -d $(PREFIX)/lib/plugin
	$(INSTALL) -b $(PLUGIN_OBJ) $(PREFIX)/lib/plugin

clean:
	rm -rf lib
	rm -f $(PLUGIN_OBJ)
