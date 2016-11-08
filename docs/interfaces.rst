==========
Interfaces
==========

There  are two  interfaces  to  control the  filter.  One uses  HDF5's
standard interface via  an array of ``unsigned int cd_values``. The other
uses HDF5  unregistered properties. You  can find examples  of writing
HDF5 data using both of these interfaces in test_write.c.

For the ``cd_values``  interface, the following CPP macros  are defined in
``H5Zzfp.h`` header file::

    H5Pset_zfp_rate_cdata(double rate, size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_precision_cdata(unsigned int prec, size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_accuracy_cdata(double acc, size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_expert_cdata(unsigned int minbits, unsigned int maxbits,
                            unsigned int maxprec, int minexp,
                            size_t cd_nelmts, unsigned int *cd_vals);

These  macros  simply  store   the  relevant  ZFP  parameters  into  a
sufficiently large array (>=6) of ``unsigned int cd_values``. It is up to
the  caller to  then call  ``H5Pset_filter`` with  the array  of cd_values
constructed by these macros.

For the properties interface, the following functions are defined::

    herr_t H5Pset_zfp_rate(hid_t dcpl_id, double rate);
    herr_t H5Pset_zfp_precision(hid_t dcpl_id, unsigned int prec);
    herr_t H5Pset_zfp_accuracy(hid_t dcpl_id, double acc);
    herr_t H5Pset_zfp_expert(hid_t dcpl_id, unsigned int minbits, unsigned int maxbits,
                                            unsigned int maxprec, int minexp);

These  functions create  temporary (e.g.  UNregistered)  HDF5 property
list entries  to control the  ZFP filter and also  take responsibility
for adding the filter to the pipeline.

The properties interface  is more type-safe. However, there  is no way
for the implementation of these properties to reside within the filter
plugin itself. The properties  interface requires that the caller link
with  an   additional  object  file, ``H5Zzfp_props.o``.  The  cd_values
interface does not require this.

However, these  macros are only a  convenience. You do  not **need** the
``H5Zzfp.h`` header file if you want  to avoid using it. But, you are then
responsible  for setting  up  the cd_values  array  correctly for  the
filter.  For reference,  the cd_values  array for  this ZFP  filter is
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

Note that  the cd_values  used in the  interface to  H5Pset_filter are
**not** **the** **same** cd_values ultimately stored  to the HDF5 dataset header
for a compressed dataset. The  values are transformed in the set_local
method to use ZFP's internal  routines for 'meta' and 'mode' data. So,
don't make the mistake of examining  the values you find in a file and
think you can use those same  values, for example, in an invokation of
h5repack.
