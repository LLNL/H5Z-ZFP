============
Installation
============

Prerequisites

* `ZFP Library <http://computation.llnl.gov/projects/floating-point-compression/download/zfp-0.5.0.tar.gz`_
   or from `<https://github.com/LLNL/zfp`_

** The ZFP library **must** be compiled with the ``CPPFLAG -DBIT_STREAM_WORD_TYPE=uint8``.
   If you attempt to use this filter with a ZFP  library compiled  differently from
   this, the  filter's can_apply method will always return false. This will result
   in silently ignoring an HDF5 client's  request to compress  data with  ZFP. Also,
   see note about endian-swapping issues below.

* `HDF5 Library <https://support.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.8.17.tar.gz`_

Once you have installed the prerequisites, you can compile H5Z-ZFP using a command-line

``make CC=<compiler> ZFP_HOME=<path-to-zfp> HDF5_HOME=<path-to-hdf5>``

where ``<path-to-zfp>`` is a directory containing ZFP ``inc`` and ``lib`` dirs and
``<path-to-hdf5>`` is a directory containing HDF5 ``include`` and ``lib`` dirs.
If you don't specify a C compiler, it will try to guess one to use.

The Makefile uses  GNU Make syntax and is designed to  work on OSX and
Linux. The command,

``make CC=<C-compiler> HDF5_HOME=<hdf5-dir> ZFP_HOME=<zfp-dir> all``

where hdf5-dir is  a dir containing HDF5 include,  lib and bin subdirs
and zfp-dir is a dir containing  inc and lib subdirs, should build and
test everything. The command 'make help' will print useful information
about various make targets and variables.

The filter has been tested on gcc, clang, xlc, icc and pgcc  compilers
and checked with valgrind.
