include ../config.make

.PHONY: help clean plugin props

help:
	@echo "               This is H5Z-ZFP version $(H5Z_ZFP_VERSINFO)."
	@echo "           See README file for detailed information."
	@echo ""
	@echo "Typical make command is..."
	@echo ""
	@echo "    make CC=<c-compiler> HDF5_HOME=<path> ZFP_HOME=<path> plugin"
	@echo ""
	@echo "where 'path' is a dir whose children are inc/lib/bin subdirs."
	@echo "Other variables (e.g. CFLAGS, LD, etc.) can be set as usual."
	@echo ""
	@echo "Available make targets are..."
	@echo "    plugin - build the plugin"
	@echo "    props - build the properties-based interface to the plugin"
	@echo "    clean - clean away all derived targets"

all: plugin props

plugin:
	pushd plugin; $(MAKE) $(MAKEFLAGS) $@

props:
	pushd props; $(MAKE) $(MAKEFLAGS) $@

clean:
	pushd plugin; $(MAKE) $(MAKEFLAGS) $@
	pushd props; $(MAKE) $(MAKEFLAGS) $@
