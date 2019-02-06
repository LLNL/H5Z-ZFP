.. _hdf5_chunking:

=============
HDF5 Chunking
=============

HDF5's dataset chunking feature is a way to optimize data layout on disk
to support partial dataset reads by downstream consumers. This is all the more
important when compression filters are applied to datasets as it frees a consumer
from suffering the UNcompression of an entire dataset only to read a portion.


-------------
ZFP Chunklets
-------------

When using HDF5 chunking with ZFP compression, it is important to account
for the fact that ZFP does its work in tiny 4\ :sup:`d` chunklets of its
own where `d` is the dataset dimension (_rank_ in HDF5 parlance). This means
that that whenever possible chunking dimensions you select in HDF5 should be
multiples of 4. When a chunk dimension is not a multiple of 4, ZFP will wind
up with partial chunklets which it will pad with useless data reducing overall
time and space efficiency of the results. On the other hand when chunk dimensions
are many times a multiple 4, this overhead is also likely to be negligible.

-----------------------------
More Than 3 (or 4) Dimensions
-----------------------------

Versions of ZFP 0.5.3 and older support compression in only 1,2 or 3
dimensions. Versions of ZFP 0.5.4 and newer also support 4 dimensions.

What if you have a dataset with more dimensions than ZFP can compress?
You can still use the H5Z-ZFP filter. But, in order to do so you
are *required* to chunk the dataset [1]_. Furthermore, you must select a 
chunk size such that no more than 3 (or 4 for ZFP 0.5.4 and newer)
dimensions are non-unitary (e.g. of size one). 

What analysis process should you use to select the chunk shape? Depending
on what you expect in the way of access patters in downstream consumers,
this can be a challenging question to answer. There are potentially two
competing interests. One is optimizing the chunk size and shape for access
patterns anticipated by downstream consumers. The other is optimizing the chunk
size and shape for compression. These two interests may not be compatible
and you may have to compromise between them. We illustrate the issues and
tradeoffs using an example.

For example, suppose you have a tensor valued field (e.g. a 3x3 matrix
at every _point_) over a 4D (3 spatial dimensions and 1 time dimension),
regularly sampled domain? Conceptually, this is a 6 dimensional dataset
in HDF5 with one of the dimensions (the _time_ dimension) _extendible_.
You are free to define this as a 6 dimensional dataset in HDF5. But, you
will also have to chunk the dataset. You can select any chunk shape
you want except that no more than 3 (or 4 for ZFP versions 0.5.4 and
newer) dimensions of the chunk can be non-unity.

.. [5] The HDF5 library currently requires dataset chunking anyways for
   any dataset that has any kind of filter applied.
