=====
Tests and Examples
=====

The tests directory contains a few simple tests of the H5Z-ZFP filter.

The test client, ``test_write.c`` is compiled a couple of different ways.
One produces ``test_write_plugin`` which demonstrates the use of this filter as
a standalone plugin. The other produces ``test_write_lib``, demonstrates the use
of the filter as an explicitly linked library. These test a simple 1D array with
and without ZFP compression using either the :ref:`generic-interface` or the
:ref:`properties-interface`.  You can use the code there as an example of using
the ZFP filter either as a plugin or as a library.
The command ``test_write_lib --help`` or ``test_write_plugin --help`` will print a
list of the example's options and how to use them.

There is a companion, ``test_read.c`` which is compiled into ``test_read_plugin``
and ``test_read_lib`` which demonstrates use of the filter reading data as a
plugin or library.

Finally, there is a Fortran test example, ``test_rw_fortran.F90``. The Fortran
test writes and reads a 2D dataset. However, the Fortran test is designed to
use the filter **only** as a library and not as a plugin. The reason for this is
that the filter controls involve passing combinations of integer and floating 
point data from Fortran callers and this can be done only through the :ref:`properties-interface`,
which by its nature requires any Fortran application to have to link with an 
implementation of that interface. Since we need to link extra code for Fortran,
we may as well also link to the filter itself alleviating the need to use the
filter as a plugin.

In addition, a number tests are performed in the Makefile which test the plugin
by using some of the HDF5 tools such as ``h5dump`` and ``h5repack``.
