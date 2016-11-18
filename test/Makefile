# You should need to edit, if at all, only config.make
# Usually, this command is sufficient...
#
#     make CC=<c-compiler> HDF5_HOME=<path-to-hdf5> ZFP_HOME=<path-to-zfp>
#
# You should be able to override most options by assinging values to
# appropriate variables in the make command-line (e.g. make CFLAGS=...)
include ../config.make

.PHONY: props plugin check patch

TEST_SRC = test_write.c test_read.c
TEST_OBJ = $(TEST_SRC:.c=.o)

EXEC = test_write test_read
ifneq ($(FC),)
EXEC += test_rw_fortran
TEST_OBJ += H5Zzfc.o test_rw_fortran.o
FTESTS = test-rate-f test-precision-f test-accuracy-f
endif

help:
	@echo "               This is H5Z-ZFP version $(H5Z_ZFP_VERSINFO)."
	@echo "           See README file for detailed information."
	@echo ""
	@echo "Typical make command is..."
	@echo ""
	@echo "    make CC=<c-compiler> HDF5_HOME=<path> ZFP_HOME=<path> check"
	@echo ""
	@echo "where 'path' is a dir whose children are inc/lib/bin subdirs."
	@echo "Other variables (e.g. CFLAGS, LD, etc.) can be set as usual."
	@echo "Optionally, add FC=<fortran-compiler> to include Fortran tests."
	@echo ""
	@echo "Available make targets are..."
	@echo "    test_write - build just the write test for zfp"
	@echo "    test_read  - build just the read test for zfp"
	@echo "    test-rate  - run ZFP rate mode tests"
	@echo "    test-accuracy - run ZFP accuracy mode tests"
	@echo "    test-precision - run ZFP precision mode tests"
	@echo "    test-h5repack - run ZFP using HDF5's h5repack (requires patch to h5repack)"
	@echo "    test_rw_fortran - Fortran write read test for zfp"
	@echo "    test-rate-f  - run Fortran ZFP rate mode tests"
	@echo "    test-accuracy-f - run Fortran ZFP accuracy mode tests"
	@echo "    test-precision-f - run Fortran ZFP precision mode tests"
	@echo "    all - run all tests"
	@echo "    clean - clean everything"

all: check

patch:
	@echo "Make sure you have patched HDF5's repack tool"

plugin:
	cd ../src/plugin; $(MAKE) $(MAKEVARS) plugin

props:
	cd ../src/props; $(MAKE) $(MAKEVARS) props

test_write: test_write.o props
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) ../src/props/H5Zzfp_props.o -L$(HDF5_LIB) $(LDFLAGS)

test_read: test_read.o plugin
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) -L$(HDF5_LIB) $(LDFLAGS)

ifneq ($(FC),)

test_rw_fortran: H5Zzfc.o test_rw_fortran.o
	$(FC) $(FCFLAGS) -o $@ $^ $(PREPATH)$(HDF5_LIB) ../src/props/H5Zzfp_props.o -L$(HDF5_LIB) $(LDFLAGS) -lhdf5_fortran
	env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_rw_fortran

%.o:%.F90
	$(FC)  -o $@ -c $< $(FCFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)


# Decrease bit rate and confirm compression ratio increases
test-rate-f: plugin test_rw_fortran 
	@for r in 32 16 8 4; do\
	    expected_ratio=$$(expr 64 \/ $$r); \
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_rw_fortran zfpmode 1 rate $$r 2>&1 1>/dev/null; \
	    actual_ratio=$$(env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5dump -H -d compressed -p test_zfp.h5 | grep COMPRESSION | cut -d':' -f1 | cut -d'(' -f2 | cut -d'.' -f1); \
	    if [[ $$expected_ratio -ne $$actual_ratio ]]; then \
	        echo "Fortran ZFP rate test failed for rate=$$r"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Fortran Rate tests Passed"

# Increase accuracy and test absolute error tolerance is within accuracy
test-accuracy-f: plugin test_rw_fortran
	@for a in 0.1 0.01 0.001 0.0001; do\
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_rw_fortran zfpmode 3 acc $$a write_only 2>&1 1>/dev/null; \
	    env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d $$a test_zfp.h5 test_zfp.h5 compressed original 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        echo "Fortran ZFP accuracy test failed for accuracy=$$a"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Fortran Accuracy tests Passed"

# Increase precision and confirm diff count for given tolerance drops
test-precision-f: plugin test_rw_fortran
	@ldiffcnt=0; \
	for p in 12 16 20 24; do\
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_rw_fortran zfpmode 2 prec $$p write_only 2>&1 1>/dev/null; \
	    diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp.h5 test_zfp.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	    if [[ $$ldiffcnt -ne 0 ]] && [[ $$diffcnt -gt $$ldiffcnt ]]; then \
	        echo "Fortran ZFP precision test failed for precision=$$p"; \
		exit 1; \
	    fi; \
	    ldiffcnt=$$diffcnt; \
	done; \
	echo "Fortran Precision tests Passed"


endif

# Decrease bit rate and confirm compression ratio increases
test-rate: plugin test_write
	@for r in 32 16 8 4; do\
	    expected_ratio=$$(expr 64 \/ $$r); \
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_write zfpmode=1 rate=$$r write_only 2>&1 1>/dev/null; \
	    actual_ratio=$$(env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5dump -H -d compressed -p test_zfp.h5 | grep COMPRESSION | cut -d':' -f1 | cut -d'(' -f2 | cut -d'.' -f1); \
	    if [[ $$expected_ratio -ne $$actual_ratio ]]; then \
	        echo "ZFP rate test failed for rate=$$r"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Rate tests Passed"

# Increase accuracy and test absolute error tolerance is within accuracy
test-accuracy: plugin test_write
	@for a in 0.1 0.01 0.001 0.0001; do\
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_write zfpmode=3 acc=$$a 2>&1 1>/dev/null; \
	    env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d $$a test_zfp.h5 test_zfp.h5 compressed original 2>&1 1>/dev/null; \
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
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_write zfpmode=2 prec=$$p 2>&1 1>/dev/null; \
	    diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp.h5 test_zfp.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
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
test-h5repack: plugin mesh.h5 patch
	@env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5repack -f UD=32013,6,3,0,3539053052,1062232653,0,0 \
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
test-endian: plugin test_zfp_le.h5 test_zfp_be.h5
	@outerr=$$(env LD_LIBRARY_PATH=$(HDF5_LIB) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d 0.00001 test_zfp_le.h5 test_zfp_be.h5 compressed compressed 2>&1); \
	if [[ $$? -ne 0 ]] || [[ -n "$$(echo $$outerr | grep 'cannot be read')" ]]; then \
	    echo "Endian test failed"; \
	    exit 1; \
	fi; \
	echo "Endian test Passed"

check: $(EXEC) $(FTESTS) test-rate test-precision test-accuracy test-endian 

clean:
	@versinfo=$$(grep '#define H5Z_FILTER_ZFP_VERSION_[MP]' $(H5Z_ZFP_BASE)/H5Zzfp.h | cut -d' ' -f3 | tr '\n' '.' | cut -d'.' -f-3); \
	rm -f $(TEST_OBJ) $(PROP_OBJ) test_zfp.h5 mesh_repack.h5 $(EXEC) *.mod
