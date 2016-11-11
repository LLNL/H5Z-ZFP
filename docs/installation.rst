============
Installation
============

----
Prerequisites
----

* Get the `ZFP Library <http://computation.llnl.gov/projects/floating-point-compression/download/zfp-0.5.0.tar.gz>`_ or from `Github <https://github.com/LLNL/zfp>`_

    * The ZFP library **must** be compiled with the ``CPPFLAG -DBIT_STREAM_WORD_TYPE=uint8``.
      If you attempt to use this filter with a ZFP  library compiled  differently from
      this, the  filter's can_apply method will always return false. This will result
      in silently ignoring an HDF5 client's  request to compress  data with  ZFP. Also,
      see :ref:`endian-issues`

* `HDF5 Library <https://support.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.8.17.tar.gz>`_

Once you have installed the prerequisites, you can compile H5Z-ZFP using a command-line

``make CC=<compiler> ZFP_HOME=<path-to-zfp> HDF5_HOME=<path-to-hdf5>``

where ``<path-to-zfp>`` is a directory containing ZFP ``inc`` and ``lib`` dirs and
``<path-to-hdf5>`` is a directory containing HDF5 ``include`` and ``lib`` dirs.
If you don't specify a C compiler, it will try to guess one to use.

The Makefile uses  GNU Make syntax and is designed to  work on OSX and
Linux.

The command 'make help' will print useful information
about various make targets and variables.

The filter has been tested on gcc, clang, xlc, icc and pgcc  compilers
and checked with valgrind.

----
Source Code Organization
----

The source code is in three separate directories

    * ``src/plugin`` includes the ZFP filter plugin and header file, ``H5Zzfp.h``
    
        * The header file defines the symbolic filter id, ``H5Z_FILTER_ZFP``,
          as well as a set of macros for the :ref:`generic-interface`. Any
          application wanting to use the :ref:`generic-interface` will have to
          ``#include "H5Zzfp.h"``.
        * ``src/plugin/lib`` is where the compiled plugin gets installed. It is
          this path that should be set in the ``HDF5_PLUGIN_DIR`` enviornment
          variable so that you can use the plugin with any HDF5 client.

    * ``src/props`` includes the implementation of the :ref:`properties-interface` to the plugin.
      To use it, your HDF5 application needs to include the header file ``#include "H5Zzfp.h``
      and to link to ``H5Zzfp_props.o``.
    * ``test`` includes various tests. In particular ``test_write.c`` includes examples
      of using both the :ref:`generic-interface` and :ref:`properties-interface`.

----
Silo Integration
----

This plugin is also part of the `Silo library <https://wci.llnl.gov/simulation/computer-codes/silo>`_.
In particular, the ZFP library
itself is also embedded in Silo but is protected from appearing in Silo's
global namespace through a struct of function pointers (see `Namespaces in C <https://visitbugs.ornl.gov/projects/silo/wiki/Using_C_structs_as_a_kind_of_namespace_mechanism_to_reduce_global_symbol_bloat>`_.
If you happen to examine the source code for H5Z-ZFP, you will see some logic there
that is specific to using this plugin within Silo and dealing with this
struct of function pointers wrapper. Just ignore this.

----
Plugin vs. Non-Plugin Operation
----

By default, the filter is designed to be compiled for use as an HDF5 *plugin*.
When it is used as a plugin, all HDF5 applications are *required*
to *find* the plugin shared library (named ``libh5*.{so,dylib}``)
in a directory specified by the enviornment
variable, ``HDF5_PLUGIN_DIR``. Currently, the HDF5 library offers
no mechanism for applications themselves to have pre-programmed
in the directory(s) in which to search for a plugin. Applications are
then always vulnerable to an incorrectly specified or unspecified ``HDF5_PLUGIN_DIR``
environment variable.

However, when used *within* the Silo library, this filter is **not**
compiled to be used as a *plugin* and is instead can be compiled for use
as a library as a *builtin* filter.

Other applications are also free to use the filter in this way. To
do so, the filter must be compiled with the additional ``CPPFLAG``,
``-DAS_SILO_BUILTIN``. When this flag is defined, an additional
function is defined in the ``H5Zzfp.h`` header file::

   void H5Z_zfp_register(void);
   
Applications must then link directly to the compiled plugin and
call this function to make the filter available in the executable.
