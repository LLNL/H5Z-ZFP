.. _hdf5_chunking:

==============
HDF5_ Chunking
==============

HDF5_'s dataset `chunking`_ feature is a way to optimize data layout on disk
to support partial dataset reads by downstream consumers. This is all the more
important when compression filters are applied to datasets as it frees a consumer
from suffering the UNcompression of an entire dataset only to read a portion.


-------------
ZFP Chunklets
-------------

When using HDF5_ `chunking`_ with ZFP_ compression, it is important to account
for the fact that ZFP_ does its work in tiny 4\ :sup:`d` chunklets of its
own where `d` is the dataset dimension (*rank* in HDF5_ parlance). This means
that that whenever possible `chunking`_ dimensions you select in HDF5_ should be
multiples of 4. When a chunk_ dimension is not a multiple of 4, ZFP_ will wind
up with partial chunklets which it will pad with useless data reducing overall
time and space efficiency of the results. On the other hand when chunk_ dimensions
are many times a multiple 4, this overhead is also likely to be negligible.

-----------------------------
More Than 3 (or 4) Dimensions
-----------------------------

Versions of ZFP_ 0.5.3 and older support compression in only 1,2 or 3
dimensions. Versions of ZFP_ 0.5.4 and newer also support 4 dimensions.

What if you have a dataset with more dimensions than ZFP_ can compress?
You can still use the H5Z-ZFP_ filter. But, in order to do so you
are *required* to chunk_ the dataset [1]_ . Furthermore, you must select a 
chunk_ size such that no more than 3 (or 4 for ZFP_ 0.5.4 and newer)
dimensions are non-unitary (e.g. of size one). 

What analysis process should you use to select the chunk_ shape? Depending
on what you expect in the way of access patters in downstream consumers,
this can be a challenging question to answer. There are potentially two
competing interests. One is optimizing the chunk_ size and shape for access
patterns anticipated by downstream consumers. The other is optimizing the chunk_
size and shape for compression. These two interests may not be compatible
and you may have to compromise between them. We illustrate the issues and
tradeoffs using an example.

For example, suppose you have a tensor valued field (e.g. a 3x3 matrix
at every *point*) over a 4D (3 spatial dimensions and 1 time dimension),
regularly sampled domain? Conceptually, this is a 6 dimensional dataset
in HDF5_ with one of the dimensions (the *time* dimension) *extendible*.
You are free to define this as a 6 dimensional dataset in HDF5_. But, you
will also have to chunk_ the dataset. You can select any chunk_ shape
you want except that no more than 3 (or 4 for ZFP_ versions 0.5.4 and
newer) dimensions of the chunk_ can be non-unity.

In the code snipit below, we demonstrate this case. A key issue to deal
with is that because we will use ZFP_ to compress along the time dimension,
this forces us to keep in memory a sufficient number of timesteps to match
ZFP_'s chunklet size of 4. The code below iterates over 9 timesteps. Each
of the first two groups of 4 timesteps are buffered in memory. Once 4
timesteps have been buffered, we can issue an H5Dwrite call on the 6D 
dataset. But, notice that the chunk_ dimensions are such that only 4
of the 6 dimensions are non-unity. This means ZFP_ will only ever see
something to compress that is 4D.

.. include:: ../test/test_write.c
   :code: c
   :start-line: 493
   :end-line: 559
   :number-lines:

.. _chunking: https://support.hdfgroup.org/HDF5/doc/Advanced/Chunking/index.html
.. _chunk: https://support.hdfgroup.org/HDF5/doc/Advanced/Chunking/index.html

.. [1] The HDF5_ library currently requires dataset chunking anyways for
   any dataset that has any kind of filter applied.

