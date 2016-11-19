==========
Interfaces
==========

There  are two  interfaces  to  control the  filter.  One uses  HDF5's
*generic* interface via  an array of ``unsigned int cd_values`` as is used
in `H5Pset_filter() <https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetFilter>`_. The other
uses HDF5 *unregistered* properties. You  can find examples  of writing
HDF5 data using both of these interfaces in ``test_write.c``.

The plugin is designed to respond correctly when either interface is used.

.. _generic-interface:

-----------------
Generic Interface
-----------------

For the generic interface, the following CPP macros  are defined in
the ``H5Zzfp.h`` header file::

    H5Pset_zfp_rate_cdata(double rate, size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_precision_cdata(unsigned int prec, size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_accuracy_cdata(double acc, size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_expert_cdata(unsigned int minbits, unsigned int maxbits,
                            unsigned int maxprec, int minexp,
                            size_t cd_nelmts, unsigned int *cd_vals);

These  macros  utilize *type punning* to store the relevant ZFP parameters  into  a
sufficiently large array (>=6) of ``unsigned int cd_values``. It is up to
the  caller to  then call
`H5Pset_filter() <https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetFilter>`_
with  the array  of cd_values constructed by one of these macros.

However, these  macros are only a  convenience. You do  not **need** the
``H5Zzfp.h`` header file if you want  to avoid using it. But, you are then
responsible  for setting  up  the ``cd_values``  array  correctly for  the
filter.  For reference,  the ``cd_values``  array for  this ZFP  filter is
defined like so...

+-----------+---------------------------------------------------------+
|           |                     cd_values index                     |
+-----------+--------+--------+---------+---------+---------+---------+
| ZFP mode  |     0  |    1   |    2    |    3    |    4    |    5    | 
+-----------+--------+--------+---------+---------+---------+---------+
| rate      |     1  | unused |  rateA  |  rateB  |  unused |  unused |
+-----------+--------+--------+---------+---------+---------+---------+
| precision |     2  | unused |  prec   |  unused |  unused |  unused |
+-----------+--------+--------+---------+---------+---------+---------+
| accuracy  |     3  | unused |  accA   |  accB   |  unused |  unused |
+-----------+--------+--------+---------+---------+---------+---------+
| expert    |     4  | unused |  minbits|  maxbits|  maxprec|  minexp |
+-----------+--------+--------+---------+---------+---------+---------+
                   A/B are high/low 32-bit words of a double.

Note that  the cd_values  used in the  interface to  ``H5Pset_filter()`` are
**not the same** cd_values ultimately stored  to the HDF5 dataset header
for a compressed dataset. The  values are transformed in the set_local
method to use ZFP's internal  routines for 'meta' and 'mode' data. So,
don't make the mistake of examining  the values you find in a file and
think you can use those same  values, for example, in an invokation of
h5repack.

Because of the type punning involved, the generic interface is not
suitable for Fortran callers.

.. _properties-interface:

----------
Properties Interface
----------

For the properties interface, the following functions are defined in
the ``H5Zzfp.h`` header file::

    herr_t H5Pset_zfp_rate(hid_t dcpl_id, double rate);
    herr_t H5Pset_zfp_precision(hid_t dcpl_id, unsigned int prec);
    herr_t H5Pset_zfp_accuracy(hid_t dcpl_id, double acc);
    herr_t H5Pset_zfp_expert(hid_t dcpl_id,
        unsigned int minbits, unsigned int maxbits,
        unsigned int maxprec, int minexp);

These  functions take a dataset creation property list, ``hid_t dcp_lid`` and
create  temporary (e.g.  UNregistered)  HDF5 property
list entries  to control the  ZFP filter. Calling any of these functions
removes the effects of any previous call to any one of these functions.
In addition, calling any one of these functions also has the effect of
adding the filter to the pipeline.

The properties interface  is more type-safe. However, there  is no way
for the implementation of these properties to reside within the filter
plugin itself. The properties  interface requires that the caller link
with  an   additional  object  file, ``H5Zzfp_props.o``.  The generic 
interface does not require this.

----
Fortran Interface
----

A Fortran interface based on the properties interface, described above,
has been added by Scot Breitenfeld of the HDF5 group. Currently, the
code that implements the Fortran interface is in the file ``H5Zzfc.F90``
in the ``tests`` directory. We will restructure this soon to include
the Fortran interface as ``module`` during a ``make install``.
