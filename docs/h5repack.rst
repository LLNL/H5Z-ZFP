========
H5Repack
========
A convenient way to use and play with the ZFP_ filter is a *plugin* with
the HDF5_ ``h5repack`` utility using the ``-f`` filter argument to apply
ZFP to existing data in a file.

The challenge in doing this is constructing the set of *cd_values* required
by the *generic* HDF5_ filter interface, especially because of the type-punning
involved for *double* data.

**Note:** Querying an existing dataset using ``h5dump`` or ``h5ls`` to obtain
the *cd_values* stored with a ZFP_ compressed dataset
**will not provide the correct cd_values**. This is because the *cd_values*
used stored in the file are different from those used in the *generic* interface
to invoke the ZFP_ filter.

To facilitate constructing a valid ``-f`` argument to ``h5repack``, we have
created a utility program, ``print_h5repack_farg``, which is presently in the
``test`` directory and is made when other tests are made.

You can use the ``print_h5repack_farg`` utility to read a command-line
consisting of ZFP_ filter parameters and it will output the *cd_values*
needed for the ``-f`` argument to ``h5repack``. Here are some examples::

    % ./print_h5repack_farg zfpmode=1 rate=4.5
    
    Print cdvals for set of ZFP compression paramaters...
        zfpmode=1        set zfp mode (1=rate,2=prec,3=acc,4=expert)
        rate=4.5                    set rate for rate mode of filter
        acc=0               set accuracy for accuracy mode of filter
        prec=11       set precision for precision mode of zfp filter
        minbits=0          set minbits for expert mode of zfp filter
        maxbits=4171       set maxbits for expert mode of zfp filter
        maxprec=64         set maxprec for expert mode of zfp filter
        minexp=-1074        set minexp for expert mode of zfp filter
        help=0                                     this help message

    h5repack -f argument...
        -f UD=32013,4,1,0,0,1074921472

    % ./print_h5repack_farg zfpmode=3 acc=0.005
    
    Print cdvals for set of ZFP compression paramaters...
        zfpmode=3        set zfp mode (1=rate,2=prec,3=acc,4=expert)
        rate=4                      set rate for rate mode of filter
        acc=0.005           set accuracy for accuracy mode of filter
        prec=11       set precision for precision mode of zfp filter
        minbits=0          set minbits for expert mode of zfp filter
        maxbits=4171       set maxbits for expert mode of zfp filter
        maxprec=64         set maxprec for expert mode of zfp filter
        minexp=-1074        set minexp for expert mode of zfp filter
        help=0                                     this help message

    h5repack -f argument...
        -f UD=32013,4,3,0,1202590843,1064598241

    % ./print_h5repack_farg zfpmode=3 acc=0.075
    
    Print cdvals for set of ZFP compression paramaters...
        zfpmode=3        set zfp mode (1=rate,2=prec,3=acc,4=expert)
        rate=4                      set rate for rate mode of filter
        acc=0.075           set accuracy for accuracy mode of filter
        prec=11       set precision for precision mode of zfp filter
        minbits=0          set minbits for expert mode of zfp filter
        maxbits=4171       set maxbits for expert mode of zfp filter
        maxprec=64         set maxprec for expert mode of zfp filter
        minexp=-1074        set minexp for expert mode of zfp filter
        help=0                                     this help message

    h5repack -f argument...
        -f UD=32013,4,3,0,858993459,1068708659
