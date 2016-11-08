=============
Endian Issues
=============

The ZFP library writes an endian-independent stream.

When  reading  ZFP compressed  data  on  a  machine with  a  different
endian-ness    than   the   writer,    there   is    an   unnavoidable
inefficiency. Upon reading data back,  it  produces  the  correct endian-nessu
result regardless of
reader  and  writer  endian-ness  incompatability.  However,  the HDF5
library is expecting to read  from the file (even if through a filter)
the  endian-ness  of the data as it was stored to tye file (typically
that of  the  writer machine)  and  expects to byte-swap it before returning
to the caller. So, in the H5Z-ZFP plugin, we wind up having
to  byte-swap  a result read in a cross-endian context. That way, when
HDF5  gets the data and byte-swaps it, it will produce the correct result.
There is  an endian-ness  test in  the Makefile and two ZFP compressed
example  datasets for  big-endian  and little-endian machines to  test
that cross-endian reads/writes work correctly.

Finally, endian-targetting,  that is setting the file  datatype for an
endian-ness that is possibly  different than the native endian-ness of
the  writer,  is  currently  dis-allowed  with  H5Z-ZFP.  Under  these
conditions, the can_apply method will return 0. This constraint can be
relaxed,  at the  expense of  an additional  endian-UN-swap  pass just
prior to compression, at a future date if it becomes too onerous.
