Welcome to H5Z-ZFP
==================

H5Z-ZFP is a compression filter for HDF5 using the
`ZFP compression library <http://computation.llnl.gov/projects/floating-point-compression>`_,
supporting lossy compression of floating point and integer data to meet bitrate, accuracy,
and/or precision targets. The filter uses the `*registered* <https://support.hdfgroup.org/services/filters.html#zfp>`_
HDF5 filter ID, ``32013``.
It supports single and double precision floating point and integer data *chunked* in 1, 2 or
3 dimensions. The filter will function on datasets of more than 3 dimensions, albiet at the
probable expense of compression performance, as long as the chunking is such that no more than
3 dimensions of a chunk are non-unity.

Contents:

.. toctree::
   :maxdepth: 1
   :glob:

   installation
   interfaces
   endian_issues
   tests
