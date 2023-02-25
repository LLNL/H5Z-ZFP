=======================================
H5Z-ZFP and the HDF5 filter's cd_values
=======================================

The HDF5 library uses an array of values, named ``cd_values`` in formal arguments documenting various API functions, for managing *auxiliary data* for a filter.
Instances of this ``cd_values`` array are used in two subtly different ways within HDF5.

The first use is in *passing* auxiliary data for a filter from caller to the library when initially creating a dataset.
This happens *directly* in an ``H5Pset_filter()`` call.

The second use is in *persisting* auxiliary data for a filter to the dataset's object *header* in a file.
This happens *indirectly* as part of an ``H5Dcreate()`` call.

When a dataset creation property list includes a filter, the filter's ``set_local()`` method is called (see H5Zregister()(https://docs.hdfgroup.org/hdf5/develop/group___h5_z.html)) as part of the ``H5Dcreate`` call.
In the filter's ``set_local()`` method, the ``cd_values`` that were *passed* by the caller (in ``H5Pset_filter()``) are often modified (via ``H5Pmodify_filter()``) before they are *persisted* to the dataset's object header in a file.

Among other things, this design allows a filter to be generally configured for any dataset in a file but then adjusted as necesssary when it is applied to a specific dataset to handle such things as the data type, dimensions to which the filter is applied.
Long story short, the data stored in ``cd_values`` of the dataset object's header in the file are often not the same values passed by the caller when the dataset was created.

To make matters a tad more complex, the ``cd_values`` data is treated by HDF5 as an array of C typed, 4-byte, ``unsigned integer`` values.
Furthermore, regardless of endianness of the data producer, the persisted values are always stored in little-endian format in the dataset object header in the file.
Nonetheless, if the persisted ``cd_values`` data is ever retrieved (e.g. H5Pget_filter...) the HDF5 library ensures the data is returned to callers in the proper endianness.
When command-line tools like ``h5ls`` and ``h5dump`` print ``cd_values``, the data will be displayed correctly.

Handling of double precision auxiliary data via ``cd_values`` is more complicated still because a single double precision value will in almost all cases span multiple values in ``cd_values``.
Aside from the possibility of differing floating point formats between a producer and consumer, any endianness handling the HDF5 library does for 4-byte values in ``cd_values`` will certainly not ensure proper endianness handling of larger values.
It is not possible for command-line tools like ``h5ls`` and ``h5dump`` to display such data correctly.

Fortunately, the ZFP library has already been designed to handle these issues as part of the ZFP's *native* stream header.
So, H5Z-ZFP uses the ``cd_values`` that are persisted to the file to store ZFP's stream header.
It stores ZFP's stream header starting at ``&cd_values[1]``. 
``cd_values[0]`` is used to stored H5Z-ZFP filter and ZFP library and encoder version information.

This also means that H5Z-ZFP avoids the overhead of duplicating ZFP stream header in each dataset chunk.
For larger chunks, this savings is probably not too terribly significant.
