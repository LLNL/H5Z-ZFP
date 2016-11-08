=====
Tests
=====

The tests directory contains a few simple tests of the H5Z-ZFP filter.

``test_write.c`` writes a simple 1D array with and without ZFP compression
using both the cd_values interface methods and the properties interface
methods. You can use the code there as an example of using the ZFP filter.
Use the command ``test_write --help`` will print a list of the example's
options and how to use them.

``test_read.c`` is a simple test to re-read ZFP compressed data and compare
it to the original data.

In addition, a number tests are performed in the Makefile which test the plugin
by using some of the HDF5 tools such as ``h5dump`` and ``h5repack``.
