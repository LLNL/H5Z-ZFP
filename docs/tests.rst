=====
Tests and Examples
=====

The tests directory contains a few simple tests of the H5Z-ZFP filter.

The test client, ``test_write.c`` is compiled a couple of different ways.
One target is ``test_write_plugin`` which demonstrates the use of this filter as
a standalone plugin. The other target, ``test_write_lib``, demonstrates the use
of the filter as an explicitly linked library. These test a simple 1D array with
and without ZFP compression using either the :ref:`generic-interface` (for plugin)
or the :ref:`properties-interface` (for library).  You can use the code there as an
example of using the ZFP filter either as a plugin or as a library.
The command ``test_write_lib help`` or ``test_write_plugin help`` will print a
list of the example's options and how to use them.

There is a companion, ``test_read.c`` which is compiled into ``test_read_plugin``
and ``test_read_lib`` which demonstrates use of the filter reading data as a
plugin or library. Also, the commands ``test_read_lib help`` and
``test_read_plugin help`` will print a list of the command line options.

To use the plugin examples, you need to tell the HDF5 library where to find the
H5Z-ZFP plugin with the ``HDF5_PLUGIN_PATH`` environment variable. The value you
pass is the path to the directory containing the plugin shared library.

Finally, there is a Fortran test example, ``test_rw_fortran.F90``. The Fortran
test writes and reads a 2D dataset. However, the Fortran test is designed to
use the filter **only** as a library and not as a plugin. The reason for this is
that the filter controls involve passing combinations of integer and floating 
point data from Fortran callers and this can be done only through the
:ref:`properties-interface`, which by its nature requires any Fortran application
to have to link with an implementation of that interface. Since we need to link
extra code for Fortran, we may as well also link to the filter itself alleviating
the need to use the filter as a plugin. Also, if you want to use Fortran support,
the HDF5 library must have, of coursed, been configured and built with it.

In addition, a number tests are performed in the Makefile which test the plugin
by using some of the HDF5 tools such as ``h5dump`` and ``h5repack``. Again, to
use these tools to read data compressed with the H5Z-ZFP filter, you will need
to inform the HDF5 library where to find the filter plugin. For example..

::
    env HDF5_PLUGIN_PATH=<dir> h5ls test_zfp.h5

Where ``<dir>`` is the relative or absolute path to a directory containing the
filter plugin shared library.
