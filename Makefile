# You should need to edit, if at all, only config.make
# Usually, this command is sufficient...
#
#     make CC=<c-compiler> HDF5_HOME=<path-to-hdf5> ZFP_HOME=<path-to-zfp>
#
# You should be able to override most options by assinging values to
# appropriate variables in the make command-line (e.g. make CFLAGS=...)
include config.make

ZFP_INC = $(ZFP_HOME)/inc
ZFP_LIB = $(ZFP_HOME)/lib

HDF5_INC = $(HDF5_HOME)/include
HDF5_LIB = $(HDF5_HOME)/lib
HDF5_BIN = $(HDF5_HOME)/bin

LDFLAGS += -lhdf5

TEST_SRC = test_write.c test_read.c
TEST_OBJ = $(TEST_SRC:.c=.o)

PLUGIN_SRC = H5Zzfp.c
PLUGIN_OBJ = $(PLUGIN_SRC:.c=.o)

.c.o:
	$(CC) $< -o $@ -c $(CFLAGS) -DH5_HAVE_FILTER_ZFP -I$(ZFP_INC) -I$(HDF5_INC)

help:
	@versinfo=$$(grep '#define H5Z_FILTER_ZFP_VERSION_[MP]' H5Zzfp.h | cut -d' ' -f3 | tr '\n' '.' | cut -d'.' -f-3); \
	echo ""; \
	echo "               This is H5Z-ZFP version $$versinfo."
	@echo "           See README file for detailed information."
	@echo ""
	@echo "Typical make command is..."
	@echo ""
	@echo "    make CC=<c-compiler> HDF5_HOME=<path> ZFP_HOME=<path> plugin"
	@echo ""
	@echo "where 'path' is a dir whose children are inc/lib/bin subdirs."
	@echo "Other variables (e.g. CFLAGS, LD, etc.) can be set as usual."
	@echo ""
	@echo "The make target 'plugin' will make *just* the plugin shared library in a"
	@echo "directory named 'plugin'. Other available make targets are..."
	@echo "    test_write - build just the write test for zfp"
	@echo "    test_read  - build just the read test for zfp"
	@echo "    test-rate  - run ZFP rate mode tests"
	@echo "    test-accuracy - run ZFP accuracy mode tests"
	@echo "    test-precision - run ZFP precision mode tests"
	@echo "    test-h5repack - run ZFP using HDF5's h5repack (requires patch to h5repack)"
	@echo "    all - build everything and run all tests"
	@echo "    clean - clean everything"
	@echo "    dist - create distribution tarball"

all: check

plugin: $(PLUGIN_OBJ)
	@rm -rf plugin
	@mkdir plugin
	$(CC) H5Zzfp.o $(SHFLAG) -o plugin/libh5zzfp.$(SOEXT) \
	    $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L$(ZFP_LIB) -L$(HDF5_LIB) $(LDFLAGS) -lzfp

test_write: test_write.o
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) -L$(HDF5_LIB) $(LDFLAGS)

test_read: test_read.o
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) -L$(HDF5_LIB) $(LDFLAGS)

# Decrease bit rate and confirm compression ratio increases
test-rate: plugin test_write
	@for r in 32 16 8 4; do\
	    expected_ratio=$$(expr 64 \/ $$r); \
	    env HDF5_PLUGIN_PATH=$$(pwd)/plugin ./test_write zfpmode=1 rate=$$r 2>&1 1>/dev/null; \
	    actual_ratio=$$(env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$$(pwd)/plugin $(HDF5_BIN)/h5dump -H -d compressed -p test_zfp.h5 | grep COMPRESSION | cut -d':' -f1 | cut -d'(' -f2 | cut -d'.' -f1); \
	    if [[ $$expected_ratio -ne $$actual_ratio ]]; then \
	        echo "ZFP rate test failed for rate=$$r"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Rate tests Passed"

# Increase accuracy and test absolute error tolerance is within accuracy
test-accuracy: plugin test_write
	@for a in 0.1 0.01 0.001 0.0001; do\
	    env HDF5_PLUGIN_PATH=$$(pwd)/plugin ./test_write zfpmode=3 acc=$$a 2>&1 1>/dev/null; \
	    env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$$(pwd)/plugin $(HDF5_BIN)/h5diff -v -d $$a test_zfp.h5 test_zfp.h5 compressed original 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        echo "ZFP accuracy test failed for accuracy=$$a"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Accuracy tests Passed"

# Increase precision and confirm diff count for given tolerance drops
test-precision: plugin test_write
	@ldiffcnt=0; \
	for p in 12 16 20 24; do\
	    env HDF5_PLUGIN_PATH=$$(pwd)/plugin ./test_write zfpmode=2 prec=$$p 2>&1 1>/dev/null; \
	    diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$$(pwd)/plugin $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp.h5 test_zfp.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	    if [[ $$ldiffcnt -ne 0 ]] && [[ $$diffcnt -gt $$ldiffcnt ]]; then \
	        echo "ZFP precision test failed for precision=$$p"; \
		exit 1; \
	    fi; \
	    ldiffcnt=$$diffcnt; \
	done; \
	echo "Precision tests Passed"

#
# Uses h5repack to test ZFP filter on float and int datasets in
# 1,2,3 and 4 dimensions. Note: need to specify raw cd_values on
# command-line to h5repack. We can get these from an invokation
# of test_write which prints them in the header output. The values
# here are for accuracy mode and tolerance of 0.001.
# 
# A bug-fix patch to h5repack_parse.c is required for this test.
#
test-h5repack:
	@env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$$(pwd)/plugin $(HDF5_BIN)/h5repack -f UD=32013,6,3,0,3539053052,1062232653,0,0 \
	     -l X,Y,Z,Indexes:CHUNK=217 \
	     -l Indexes2:CHUNK=1517 \
	     -l Pressure,Pressure2,Pressure3:CHUNK=10x20x5 \
	     -l Pressure_2D:CHUNK=10x20 \
	     -l Stress,Velocity,Stress2,Velocity2,Stress3,Velocity3,VelocityZ,VelocityZ2,VelocityZ3:CHUNK=11x21x1x1 \
	     -l VelocityX_2D:CHUNK=21x31 \
	     -l XY:CHUNK=651x1 \
	     -l XYZ:CHUNK=217x1 \
	     -l XYZ2:CHUNK=1617x1 \
	     -l XYZ3:CHUNK=33x1 \
	     mesh.h5 mesh_repack.h5 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    echo "Repack command failed."; \
	    echo "Did you patch h5repack? See README."; \
	    exit 1; \
        fi; \
	orig_size=$$(ls -l mesh.h5 | tr -s ' ' | cut -d' ' -f5); \
	new_size=$$(ls -l mesh_repack.h5 | tr -s ' ' | cut -d' ' -f5); \
        ratio=$$(echo "$$orig_size\n100\n*\n$$new_size\n/p\n" | dc -); \
	if [[ $$ratio -lt 2 ]]; then \
	    echo "ZFP Repack Test failed"; \
	    exit 1; \
	fi; \
	echo "Repack test Passed"

# Diff ZFP compressed data from little endian and big endian machines
# There is a bug in h5diff that causes it to return 0 when it can't find plugin.
# We protect against that by additional check of output error text
test-endian:
	@outerr=$$(env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$$(pwd)/plugin $(HDF5_BIN)/h5diff -v -d 0.00001 test_zfp_le.h5 test_zfp_be.h5 compressed compressed 2>&1); \
	if [[ $$? -ne 0 ]] || [[ -n "$$(echo $$outerr | grep 'cannot be read')" ]]; then \
	    echo "Endian test failed"; \
	    exit 1; \
	fi; \
	echo "Endian test Passed"

check: test_read test-rate test-precision test-accuracy test-endian test-h5repack

clean:
	@versinfo=$$(grep '#define H5Z_FILTER_ZFP_VERSION_[MP]' H5Zzfp.h | cut -d' ' -f3 | tr '\n' '.' | cut -d'.' -f-3); \
	rm -vf H5Z-ZFP-$$versinfo.tar.gz
	rm -rf plugin
	rm -f $(TEST_OBJ) $(PLUGIN_OBJ) test_zfp.h5 mesh_repack.h5 test_write test_read

dist:	clean
	@versinfo=$$(grep '#define H5Z_FILTER_ZFP_VERSION_[MP]' H5Zzfp.h | cut -d' ' -f3 | tr '\n' '.' | cut -d'.' -f-3); \
	rm -rf H5Z-ZFP-$$versinfo H5Z-ZFP-$$versinfo.tar.gz; \
	mkdir H5Z-ZFP-$$versinfo; \
	tar cf - --exclude ".git*" --exclude H5Z-ZFP-$$versinfo . | tar xf - -C H5Z-ZFP-$$versinfo; \
	tar cvf - H5Z-ZFP-$$versinfo | gzip --best > H5Z-ZFP-$$versinfo.tar.gz; \
	rm -rf H5Z-ZFP-$$versinfo;
