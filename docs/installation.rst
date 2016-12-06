============
Installation
============

----
Prerequisites
----

* `ZFP Library <http://computation.llnl.gov/projects/floating-point-compression/download/zfp-0.5.0.tar.gz>`_ (or from `Github <https://github.com/LLNL/zfp>`_)
* `HDF5 Library <https://support.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.8.17.tar.gz>`_

^^^^
Compiling ZFP
^^^^

* There is a ``Config`` file in top-level directory of the ZFP distribution that holds ``make`` variables
  the ZFP Makefiles use. By default, this file is setup for a vanilla GNU compiler. If this is not the
  appropriate compiler, edit ``Config`` as necessary to adjust the compiler and compilation flags.
* An important flag you **will** need to adjust in order to use the ZFP library with this HDF5 filter is
  the ``BIT_STREAM_WORD_TYPE`` CPP flag. To use ZFP with H5Z-ZFP, the ZFP library **must** be compiled
  with ``BIT_STREAM_WORD_TYPE`` of ``uint8``. Typically, this is achieved by including a line in ``Config``
   of the form ``DEFS += -DBIT_STREAM_WORD_TYPE=uint8``. If you attempt to use this filter with a ZFP
  library compiled  differently from this, the  filter's ``can_apply`` method will always return
  false. This will result in silently ignoring an HDF5 client's  request to compress data with
  ZFP. Also, be sure to see :ref:`endian-issues`.
* After you have setup ``Config``, simply run ``make`` and it will build the ZFP library placing
  the library in a ``lib`` sub-directory and the necessary include files in ``inc`` sub-directory.

^^^^
Compiling HDF5
^^^^

* If you want to be able to run the fortran tests for this filter, HDF5 must be
  configured with *both* the ``--enable-fortran`` and ``--enable-fortran2003``
  configuration switches. The Fortran interface to this filter *requires* Fortran 2003
  because it uses ``ISO_C_BINDING``.

----
Compiling H5Z-ZFP
----

H5Z-ZFP is designed to be compiled both as a standalone HDF5 *plugin* and as a separate
*library* an application can explicitly link. See :ref:`plugin-vs-library`.

Once you have installed the prerequisites, you can compile H5Z-ZFP using a command-line...

| make [FC=<Fortran-compiler>] CC=<C-compiler> \
|     ZFP_HOME=<path-to-zfp> HDF5_HOME=<path-to-hdf5> \
|     PREFIX=<path-to-install>``

where ``<path-to-zfp>`` is a directory containing ZFP ``inc`` and ``lib`` dirs and
``<path-to-hdf5>`` is a directory containing HDF5 ``include`` and ``lib`` dirs.
If you don't specify a C compiler, it will try to guess one from your path. Fortran
compilation is optional. If you do not specify a Fortran compiler, it will not attempt
to build the Fortran interface.

The Makefile uses  GNU Make syntax and is designed to  work on OSX and
Linux. The filter has been tested on gcc, clang, xlc, icc and pgcc  compilers
and checked with valgrind.

The command ``make help`` will print useful information
about various make targets and variables. ``make check`` will compile everything
and run a handful of tests.

If you don't specify a ``PREFIX``, it will install to ``./install``. The installed
filter will look like...

| $(PREFIX)/include/{H5Zzfp.h,H5Zzfp_plugin.h,H5Zzfp_props.h,H5Zzfp_lib.h}
| $(PREFIX)/plugin/libh5zzfp.{so,dylib}
| $(PREFIX)/lib/libh5zzfp.a

where ``$(PREFIX)`` resolves to whatever the full path of the installation is.

To use the installed filter as an HDF5 *plugin*, you would specify, for example,
``setenv HDF5_PLUGIN_PATH $(PREFIX)/plugin``

----
H5Z-ZFP Source Code Organization
----

The source code is in two separate directories

    * ``src`` includes the ZFP filter and a few header files

        * ``H5Zzfp_plugin.h`` is an optional header file applications *may* wish
          to include because it contains several convenient macros for easily
          controlling various compression modes of the ZFP library (*rate*,
          *precision*, *accuracy*, *expert*) via the :ref:`generic-interface`. 
        * ``H5Zzfp_props.h`` is a header file that contains functions to control the
          filter using *temporary* :ref:`properties-interface`. Fortran callers are
          *required* to use this interface.
        * ``H5Zzfp_lib.h`` is a header file for applications that wish to use the filter
          explicitly as a library rather than a plugin.
        * ``H5Zzfp.h`` is an *all-of-the-above* header file for applications that don't
          care too much about separating out the above functionalities.

    * ``test`` includes various tests. In particular ``test_write.c`` includes examples
      of using both the :ref:`generic-interface` and :ref:`properties-interface`. In 
      addition, there is an example of how to use the filter from Fortran in ``test_rw_fortran.F90``.

----
Silo Integration
----

This plugin is also part of the `Silo library <https://wci.llnl.gov/simulation/computer-codes/silo>`_.
In particular, the ZFP library
itself is also embedded in Silo but is protected from appearing in Silo's
global namespace through a struct of function pointers (see `Namespaces in C <https://visitbugs.ornl.gov/projects/silo/wiki/Using_C_structs_as_a_kind_of_namespace_mechanism_to_reduce_global_symbol_bloat>`_.
If you happen to examine the source code for H5Z-ZFP, you will see some logic there
that is specific to using this plugin within Silo and dealing with ZFP as an embedded
library using this struct of function pointers wrapper. Just ignore this.
