include config.make

.PHONY: help all clean dist install

help:
	@echo "               This is H5Z-ZFP version $(H5Z_ZFP_VERSINFO)."
	@echo "See http://h5z-zfp.readthedocs.io/en/latest/ file for detailed information."
	@echo ""
	@echo "Typical make command is..."
	@echo ""
	@echo "    make [FC=<Fortran-compiler>] CC=<C-compiler> HDF5_HOME=<path> \\"
	@echo "    ZFP_HOME=<path> PREFIX=<path> all"
	@echo ""
	@echo "where 'path' is a dir whose children are inc/lib/bin subdirs."
	@echo "Other variables (e.g. CFLAGS, LD, etc.) can be set as usual."
	@echo "Optionally, add FC=<fortran-compiler> to include Fortran tests."
	@echo ""
	@echo "Available make targets are..."
	@echo "    all - build everything and run tests"
	@echo "    install - install compiled components"
	@echo "    clean - clean away all derived targets"
	@echo "    dist - create distribution tarfile"

all:
	cd src; $(MAKE) $(MAKEVARS) $@
	cd test; $(MAKE) $(MAKEVARS) $@

install:
	cd src; $(MAKE) $(MAKEVARS) $@

clean:
	rm -f H5Z-ZFP-$(H5Z_ZFP_VERSINFO).tar.gz
	cd src; $(MAKE) $(MAKEVARS) $@
	cd test; $(MAKE) $(MAKEVARS) $@

dist:	clean
	rm -rf H5Z-ZFP-$(H5Z_ZFP_VERSINFO) H5Z-ZFP-$(H5Z_ZFP_VERSINFO).tar.gz; \
	mkdir H5Z-ZFP-$(H5Z_ZFP_VERSINFO); \
	tar cf - --exclude ".git*" --exclude H5Z-ZFP-$(H5Z_ZFP_VERSINFO) . | tar xf - -C H5Z-ZFP-$(H5Z_ZFP_VERSINFO); \
	tar cvf - H5Z-ZFP-$(H5Z_ZFP_VERSINFO) | gzip --best > H5Z-ZFP-$(H5Z_ZFP_VERSINFO).tar.gz; \
	rm -rf H5Z-ZFP-$(H5Z_ZFP_VERSINFO);
