/*
Copyright (c) 2016, Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory
Written by Mark C. Miller, miller86@llnl.gov
LLNL-CODE-707197. All rights reserved.

This file is part of H5Z-ZFP. Please also read the BSD license
https://raw.githubusercontent.com/LLNL/H5Z-ZFP/master/LICENSE
*/

#include "test_common.h"

#ifdef H5Z_ZFP_USE_PLUGIN
#include "H5Zzfp_plugin.h"
#else
#include "H5Zzfp_lib.h"
#include "H5Zzfp_props.h"
#endif

#if ZFP_HAS_CFP
#if defined(ZFP_LIB_VERSION) && ZFP_LIB_VERSION>=0x1000
  #include "zfp/array.h"
#elif defined(ZFP_LIB_VERSION) && ZFP_LIB_VERSION>=0x053
  #include "cfparrays.h"
#else
  #error GOT HERE
#endif
#endif

#if 0
/* Populate the hyper-dimensional array with samples of a radially symmetric
   sinc() function but where certain sub-spaces are randomized through dimindx arrays */
static void
hyper_smooth_radial(void *b, int typ, int n, int ndims, int const *dims, int const *m,
    int const * const dimindx[10])
{
    int i;
    double hyper_radius = 0;
    const double amp = 10000;
    double val;

    for (i = ndims-1; i >= 0; i--)
    {
        int iar = n / m[i];
        iar = dimindx[i][iar]; /* allow for randomized shuffle of this axis */
        iar -= dims[i]/2;      /* ensure centering in middle of the array */
        n = n % m[i];
        hyper_radius += iar*iar;
    }
    hyper_radius = sqrt(hyper_radius);

    if (hyper_radius < 1e-15)
        val = amp;
    else
        val = amp * sin(0.4*hyper_radius) / (0.4*hyper_radius);

    if (typ == TYPINT)
    {
        int *pi = (int*) b;
        *pi = (int) val;
    }
    else
    {
        double *pd = (double*) b;
        *pd = val;
    }
}
#endif

static double func(int i, double arg)
{
    /* a random assortment of interesting, somewhat bounded, unary functions */
    double (*const funcs[])(double x) = {cos, j0, fabs, sin, cbrt, erf};
    int const nfuncs = sizeof(funcs)/sizeof(funcs[0]);
    return funcs[i%nfuncs](arg);
}

/* Populate the hyper-dimensional array with samples of set of separable functions
   but where certain sub-spaces are randomized through dimindx arrays */
static void
hyper_smooth_separable(void *b, int typ, int n, int ndims, int const *dims, int const *m,
    int const * const dimindx[10])
{
    int i;
    double val = 1;

    for (i = ndims-1; i >= 0; i--)
    {
        int iar = n / m[i];
        iar = dimindx[i][iar]; /* allow for randomized shuffle of this axis */
        iar -= dims[i]/2;      /* ensure centering in middle of the array */
        n = n % m[i];
        val *= func(i, (double) iar);
    }

    if (typ == TYPINT)
    {
        int *pi = (int*) b;
        *pi = (int) val;
    }
    else
    {
        double *pd = (double*) b;
        *pd = val;
    }
}

/* Produce multi-dimensional array test data with the property that it is random
   in the UNcorrelated dimensions but smooth in the correlated dimensions. This
   is achieved by randomized shuffling of the array indices used in specific
   dimensional axes of the array. */
static void *
gen_random_correlated_array(int typ, int ndims, int const *dims, int nucdims, int const *ucdims)
{
    int i, n;
    int nbyt = (int) (typ == TYPINT ? sizeof(int) : sizeof(double));
    unsigned char *buf, *buf0;
    int m[10]; /* subspace multipliers */
    int *dimindx[10];

    assert(ndims <= 10);

    /* Set up total size and sub-space multipliers */
    for (i=0, n=1; i < ndims; i++)
    {
        n *= dims[i];
        m[i] = i==0?1:m[i-1]*dims[i-1];
    }

    /* allocate buffer of suitable size (doubles or ints) */
    buf0 = buf = (unsigned char*) malloc(n * nbyt);

    /* set up dimension identity indexing (e.g. Idx[i]==i) so that
       we can randomize those dimensions we wish to have UNcorrelated */
    for (i = 0; i < ndims; i++)
    {
        int j;
        dimindx[i] = (int*) malloc(dims[i]*sizeof(int));
        for (j = 0; j < dims[i]; j++)
            dimindx[i][j] = j;
    }

    /* Randomize selected dimension indexing */
    srandom(0xDeadBeef);
    for (i = 0; i < nucdims; i++)
    {
        int j, ucdimi = ucdims[i];
        for (j = 0; j < dims[ucdimi]-1; j++)
        {
            int tmp, k = random() % (dims[ucdimi]-j);
            if (k == j) continue;
            tmp = dimindx[ucdimi][j];
            dimindx[ucdimi][j] = k;
            dimindx[ucdimi][k] = tmp;
        }
    }

    /* populate the array data */
    for (i = 0; i < n; i++)
    {
        hyper_smooth_separable(buf, typ, i, ndims, dims, m, (int const * const *) dimindx);
        buf += nbyt;
    }

    /* free dimension indexing */
    for (i = 0; i < ndims; i++)
        free(dimindx[i]);

    return buf0;
}

static void
modulate_by_time(void *data, int typ, int ndims, int const *dims, int t)
{
    int i, n;

    for (i = 0, n = 1; i < ndims; i++)
        n *= dims[i];

    if (typ == TYPINT)
    {
        int *p = (int *) data;
        for (i = 0; i < n; i++, p++)
        {
            double val = *p;
            val *= exp(0.1*t*sin(t/9.0*2*M_PI));
            *p = val;
        }
    }
    else
    {
        double *p = (double *) data;
        for (i = 0; i < n; i++, p++)
        {
            double val = *p;
            val *= exp(0.1*t*sin(t/9.0*2*M_PI));
            *p = val;
        }
    }
}

static void
buffer_time_step(void *tbuf, void *data, int typ, int ndims, int const *dims, int t)
{
    int i, n;
    int k = t % 4;
    int nbyt = (int) (typ == TYPINT ? sizeof(int) : sizeof(double));

    for (i = 0, n = 1; i < ndims; i++)
        n *= dims[i];

    memcpy((char*)tbuf+k*n*nbyt, data, n*nbyt);
}

static int read_data(char const *fname, size_t npoints, double **_buf)
{
    size_t const nbytes = npoints * sizeof(double);
    int fd;

    if (0 > (fd = open(fname, O_RDONLY))) SET_ERROR(open);
    if (0 == (*_buf = (double *) malloc(nbytes))) SET_ERROR(malloc);
    if (nbytes != (size_t) read(fd, *_buf, nbytes)) SET_ERROR(read);
    if (0 != close(fd)) SET_ERROR(close);
    return 0;
}

static hid_t setup_filter(int n, hsize_t *chunk, int zfpmode,
    double rate, double acc, unsigned int prec,
    unsigned int minbits, unsigned int maxbits, unsigned int maxprec, int minexp)
{
    hid_t cpid;

    /* setup dataset creation properties */
    if (0 > (cpid = H5Pcreate(H5P_DATASET_CREATE))) SET_ERROR(H5Pcreate);
    if (0 > H5Pset_chunk(cpid, n, chunk)) SET_ERROR(H5Pset_chunk);

#ifdef H5Z_ZFP_USE_PLUGIN

    int i;
    unsigned int cd_values[10];
    size_t cd_nelmts = 10;

    /* setup zfp filter via generic (cd_values) interface */
    if (zfpmode == H5Z_ZFP_MODE_RATE)
        H5Pset_zfp_rate_cdata(rate, cd_nelmts, cd_values);
    else if (zfpmode == H5Z_ZFP_MODE_PRECISION)
        H5Pset_zfp_precision_cdata(prec, cd_nelmts, cd_values);
    else if (zfpmode == H5Z_ZFP_MODE_ACCURACY)
        H5Pset_zfp_accuracy_cdata(acc, cd_nelmts, cd_values);
    else if (zfpmode == H5Z_ZFP_MODE_EXPERT)
        H5Pset_zfp_expert_cdata(minbits, maxbits, maxprec, minexp, cd_nelmts, cd_values);
    else if (zfpmode == H5Z_ZFP_MODE_REVERSIBLE)
        H5Pset_zfp_reversible_cdata(cd_nelmts, cd_values);
    else
        cd_nelmts = 0; /* causes default behavior of ZFP library */

    /* print cd-values array used for filter */
    printf("\n%d cd_values=", (int) cd_nelmts);
    for (i = 0; i < (int) cd_nelmts; i++)
        printf("%u%c", cd_values[i],((i==cd_nelmts-1)?'\0':','));
    printf("\n");

    /* Add filter to the pipeline via generic interface */
    if (0 > H5Pset_filter(cpid, H5Z_FILTER_ZFP, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values)) SET_ERROR(H5Pset_filter);

#else

    /* When filter is used as a library, we need to init it */
    H5Z_zfp_initialize();

    /* Setup the filter using properties interface. These calls also add
       the filter to the pipeline */
    if (zfpmode == H5Z_ZFP_MODE_RATE)
        H5Pset_zfp_rate(cpid, rate);
    else if (zfpmode == H5Z_ZFP_MODE_PRECISION)
        H5Pset_zfp_precision(cpid, prec);
    else if (zfpmode == H5Z_ZFP_MODE_ACCURACY)
        H5Pset_zfp_accuracy(cpid, acc);
    else if (zfpmode == H5Z_ZFP_MODE_EXPERT)
        H5Pset_zfp_expert(cpid, minbits, maxbits, maxprec, minexp);
    else if (zfpmode == H5Z_ZFP_MODE_REVERSIBLE)
        H5Pset_zfp_reversible(cpid);

#endif

    return cpid;
}


int main(int argc, char **argv)
{
    int retval=0;

    /* filename variables */
    char *ifile = (char *) calloc(NAME_LEN,sizeof(char));
    char *ofile = (char *) calloc(NAME_LEN,sizeof(char));

    /* sinusoid data generation variables */
    hsize_t npoints = 1024;
    double noise = 0.001;
    double amp = 17.7;
    int doint = 0;
    int highd = 0;
    int sixd = 0;
    int zfparr = 0;
    int help = 0;

    /* compression parameters (defaults taken from ZFP header) */
    int zfpmode = H5Z_ZFP_MODE_RATE;
    double rate = 4;
    double acc = 0;
    unsigned int prec = 11;
    unsigned int minbits = 0;
    unsigned int maxbits = 4171;
    unsigned int maxprec = 64;
    int minexp = -1074;
    int *ibuf = 0;
    double *buf = 0;

    /* HDF5 related variables */
    hsize_t chunk = 256;
    hid_t fid, dsid, idsid, sid, cpid;

    /* file arguments */
    strcpy(ofile, "test_zfp.h5");
    HANDLE_SEP(Usage: test_write [opt1=value1 opt2=value2])
    HANDLE_ARG(ifile,strndup(argv[i]+len2,NAME_LEN), "\"%s\"",set input filename);
    HANDLE_ARG(ofile,strndup(argv[i]+len2,NAME_LEN), "\"%s\"",set output filename);

    /* ZFP filter arguments */
    HANDLE_SEP(ZFP compression parameters)
    HANDLE_ARG(zfpmode,(int) strtol(argv[i]+len2,0,10),"%d", (1=rate,2=prec,3=acc,4=expert,5=reversible));
    HANDLE_ARG(rate,(double) strtod(argv[i]+len2,0),"%g",set rate for rate mode);
    HANDLE_ARG(acc,(double) strtod(argv[i]+len2,0),"%g",set accuracy for accuracy mode);
    HANDLE_ARG(prec,(unsigned int) strtol(argv[i]+len2,0,10),"%u",set precision for precision mode);
    HANDLE_ARG(minbits,(unsigned int) strtol(argv[i]+len2,0,10),"%u",set minbits for expert mode);
    HANDLE_ARG(maxbits,(unsigned int) strtol(argv[i]+len2,0,10),"%u",set maxbits for expert mode);
    HANDLE_ARG(maxprec,(unsigned int) strtol(argv[i]+len2,0,10),"%u",set maxprec for expert mode);
    HANDLE_ARG(minexp,(int) strtol(argv[i]+len2,0,10),"%d",set minexp for expert mode);

    /* 1D dataset arguments */
    HANDLE_SEP(1D dataset generation arguments)
    HANDLE_ARG(npoints,(hsize_t) strtol(argv[i]+len2,0,10), "%llu",set number of points for 1D dataset);
    HANDLE_ARG(noise,(double) strtod(argv[i]+len2,0),"%g",set amount of random noise in 1D dataset);
    HANDLE_ARG(amp,(double) strtod(argv[i]+len2,0),"%g",set amplitude of sinusoid in 1D dataset);
    HANDLE_ARG(chunk,(hsize_t) strtol(argv[i]+len2,0,10), "%llu",set chunk size for 1D dataset);
#if defined(ZFP_LIB_VERSION) && ZFP_LIB_VERSION>=0x051
    HANDLE_ARG(doint,(int) strtol(argv[i]+len2,0,10),"%d",also do integer 1D data);
#else
    HANDLE_ARG(doint,(int) strtol(argv[i]+len2,0,10),"%d",requires ZFP>=0.5.1);
    if (doint) retval = 2;
    doint = 0;
#endif

    /* Advanced cases */
    HANDLE_SEP(Advanced cases)
    HANDLE_ARG(highd,(int) strtol(argv[i]+len2,0,10),"%d",4D w/2D chunk example);
#if defined(ZFP_LIB_VERSION) && ZFP_LIB_VERSION>=0x054
    HANDLE_ARG(sixd,(int) strtol(argv[i]+len2,0,10),"%d",run 6D extendable example);
#else
    HANDLE_ARG(sixd,(int) strtol(argv[i]+len2,0,10),"%d",requires ZFP>=0.5.4);
    if (sixd) retval = 2;
    sixd = 0;
#endif

#if defined(ZFP_LIB_VERSION) && ZFP_LIB_VERSION>=0x054 && ZFP_HAS_CFP>0 && HDF5_HAS_WRITE_CHUNK>0
    HANDLE_ARG(zfparr,(int) strtol(argv[i]+len2,0,10),"%d",run ZFP array case using H5Dwrite_chunk);
#else
    HANDLE_ARG(zfparr,(int) strtol(argv[i]+len2,0,10),"%d",requires ZFP>=0.5.4 with CFP enabled);
    if (zfparr) retval = 2;
    zfparr = 0;
#endif

    cpid = setup_filter(1, &chunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);

    /* Put this after setup_filter to permit printing of otherwise hard to
       construct cd_values to facilitate manual invocation of h5repack */
    HANDLE_ARG(help,(int)strtol(argv[i]+len2,0,10),"%d",this help message); /* must be last for help to work */

    /* create double data to write if we're not reading from an existing file */
    if (ifile[0] == '\0')
        gen_data((size_t) npoints, noise, amp, (void**)&buf, TYPDBL);
    else
        read_data(ifile, (size_t) npoints, &buf);

    /* create integer data to write */

    if (doint)
        gen_data((size_t) npoints, noise*100, amp*1000000, (void**)&ibuf, TYPINT);

    /* create HDF5 file */
    if (0 > (fid = H5Fcreate(ofile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))) SET_ERROR(H5Fcreate);

    /* setup the 1D data space */
    if (0 > (sid = H5Screate_simple(1, &npoints, 0))) SET_ERROR(H5Screate_simple);

    /* write the data WITHOUT compression */
    if (0 > (dsid = H5Dcreate(fid, "original", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
    if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf)) SET_ERROR(H5Dwrite);
    if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);
    if (doint)
    {
        if (0 > (idsid = H5Dcreate(fid, "int_original", H5T_NATIVE_INT, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
        if (0 > H5Dwrite(idsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ibuf)) SET_ERROR(H5Dwrite);
        if (0 > H5Dclose(idsid)) SET_ERROR(H5Dclose);
    }

    /* write the data with requested compression */
    if (0 > (dsid = H5Dcreate(fid, "compressed", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
    if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf)) SET_ERROR(H5Dwrite);
    if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);
    if (doint)
    {
        if (0 > (idsid = H5Dcreate(fid, "int_compressed", H5T_NATIVE_INT, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
        if (0 > H5Dwrite(idsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ibuf)) SET_ERROR(H5Dwrite);
        if (0 > H5Dclose(idsid)) SET_ERROR(H5Dclose);
    }

    /* clean up from simple tests */
    if (0 > H5Sclose(sid)) SET_ERROR(H5Sclose);
    if (0 > H5Pclose(cpid)) SET_ERROR(H5Pclose);
    free(buf);
    if (ibuf) free(ibuf);

    /* Test high dimensional (>3D) array */
    if (highd)
    {
     /* dimension indices 0   1   2  3 */
        int dims[] = {256,128,32,16};
        int ucdims[]={1,3}; /* UNcorrleted dimensions indices */
        hsize_t hdims[] = {256,128,32,16};
        hsize_t hchunk[] = {256,1,32,1};

        buf = gen_random_correlated_array(TYPDBL, 4, dims, 2, ucdims);

        cpid = setup_filter(4, hchunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);

        if (0 > (sid = H5Screate_simple(4, hdims, 0))) SET_ERROR(H5Screate_simple);

        /* write the data WITHOUT compression */
        if (0 > (dsid = H5Dcreate(fid, "highD_original", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
        if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf)) SET_ERROR(H5Dwrite);
        if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);

        /* write the data with compression */
        if (0 > (dsid = H5Dcreate(fid, "highD_compressed", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
        if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf)) SET_ERROR(H5Dwrite);
        if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);

        /* clean up from high dimensional test */
        if (0 > H5Sclose(sid)) SET_ERROR(H5Sclose);
        if (0 > H5Pclose(cpid)) SET_ERROR(H5Pclose);
        free(buf);
    }
    /* End of high dimensional test */

    /* 6D Example */
    /* Test six dimensional, time varying array...
           ...a 3x3 tensor valued variable
           ...over a 3D+time domain.
           Dimension sizes are chosen to miss perfect ZFP block alignment.
    */
    if (sixd)
    {
        void *tbuf;
        int t, dims[] = {31,31,31,3,3}; /* a single time instance */
        int ucdims[]={3,4}; /* indices of UNcorrleted dimensions in dims (tensor components) */
        hsize_t hdims[] = {31,31,31,3,3,H5S_UNLIMITED};
        hsize_t hchunk[] = {31,31,31,1,1,4}; /* 4 non-unity, requires >= ZFP 0.5.4 */
        hsize_t hwrite[] = {31,31,31,3,3,4}; /* size/shape of any given H5Dwrite */

        /* Setup the filter properties and create the dataset */
        cpid = setup_filter(6, hchunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);

        /* Create the time-varying, 6D dataset */
        if (0 > (sid = H5Screate_simple(6, hwrite, hdims))) SET_ERROR(H5Screate_simple);
        if (0 > (dsid = H5Dcreate(fid, "6D_extendible", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
        if (0 > H5Sclose(sid)) SET_ERROR(H5Sclose);
        if (0 > H5Pclose(cpid)) SET_ERROR(H5Pclose);

        /* Generate a single buffer which we'll modulate by a time-varying function
           to represent each timestep */
        buf = gen_random_correlated_array(TYPDBL, 5, dims, 2, ucdims);

        /* Allocate the "time" buffer where we will buffer up each time step
           until we have enough to span a width of 4 */
        tbuf = malloc(31*31*31*3*3*4*sizeof(double));

        /* Iterate, writing 9 timesteps by buffering in time 4x. The last
           write will contain just one timestep causing ZFP to wind up
           padding all those blocks by 3x along the time dimension.  */
        for (t = 1; t < 10; t++)
        {
            hid_t msid, fsid;
            hsize_t hstart[] = {0,0,0,0,0,t-4}; /* size/shape of any given H5Dwrite */
            hsize_t hcount[] = {31,31,31,3,3,4}; /* size/shape of any given H5Dwrite */
            hsize_t hextend[] = {31,31,31,3,3,t}; /* size/shape of */

            /* Update (e.g. modulate) the buf data for the current time step */
            modulate_by_time(buf, TYPDBL, 5, dims, t);

            /* Buffer this timestep in memory. Since chunk size in time dimension is 4,
               we need to buffer up 4 time steps before we can issue any writes */
            buffer_time_step(tbuf, buf, TYPDBL, 5, dims, t);

            /* If the buffer isn't full, just continue updating it */
            if (t%4 && t!=9) continue;

            /* For last step, adjust time dim of this write down from 4 to just 1 */
            if (t == 9)
            {
                /* last timestep, write a partial buffer */
                hwrite[5] = 1;
                hcount[5] = 1;
            }

            /* extend the dataset in time */
            if (t > 4)
                H5Dextend(dsid, hextend);

            /* Create the memory dataspace */
            if (0 > (msid = H5Screate_simple(6, hwrite, 0))) SET_ERROR(H5Screate_simple);

            /* Get the file dataspace to use for this H5Dwrite call */
            if (0 > (fsid = H5Dget_space(dsid))) SET_ERROR(H5Dget_space);

            /* Do a hyperslab selection on the file dataspace for this write*/
            if (0 > H5Sselect_hyperslab(fsid, H5S_SELECT_SET, hstart, 0, hcount, 0)) SET_ERROR(H5Sselect_hyperslab);

            /* Write this iteration to the dataset */
            if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, msid, fsid, H5P_DEFAULT, tbuf)) SET_ERROR(H5Dwrite);
            if (0 > H5Sclose(msid)) SET_ERROR(H5Sclose);
            if (0 > H5Sclose(fsid)) SET_ERROR(H5Sclose);
        }
        if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);
        free(buf);
        free(tbuf);
    }
    /* End of 6D Example */

#if ZFP_HAS_CFP>0 && HDF5_HAS_WRITE_CHUNK>0
    /* ZFP Array Example */
    if (zfparr>0 && zfpmode==1 && rate>0)
    {
        int            dims[] = {38, 128};
        hsize_t       hdims[] = {38, 128};
       /*hsize_t hchunk_dims[] = {19, 34};*/
        hsize_t hchunk_dims[] = {38, 128};
        hsize_t hchunk_off[] = {0, 0};
#if defined(ZFP_LIB_VERSION) && ZFP_LIB_VERSION<=0x055
        cfp_array2d *origarr;
#else
        cfp_array2d origarr;
#endif

        /* Create the array data */
        buf = gen_random_correlated_array(TYPDBL, 2, dims, 0, 0);

        /* Instantiate a cfp array */
        origarr = cfp.array2d.ctor(dims[1], dims[0], rate, buf, 0);
        cfp.array2d.flush_cache(origarr);

        cpid = setup_filter(2, hchunk_dims, 1, rate, acc, prec, minbits, maxbits, maxprec, minexp);

        if (0 > (sid = H5Screate_simple(2, hdims, 0))) SET_ERROR(H5Screate_simple);

        /* write the data WITHOUT compression */
        if (0 > (dsid = H5Dcreate(fid, "zfparr_original", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
        if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf)) SET_ERROR(H5Dwrite);
        if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);

        /* write the data with compression via the filter */
        if (0 > (dsid = H5Dcreate(fid, "zfparr_compressed", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
        if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf)) SET_ERROR(H5Dwrite);
        if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);

        /* write the data direct from compressed array using H5Dwrite_chunk calls */
        if (0 > (dsid = H5Dcreate(fid, "zfparr_direct", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
        if (0 > H5Dwrite_chunk(dsid, H5P_DEFAULT, 0, hchunk_off, cfp.array2d.compressed_size(origarr), cfp.array2d.compressed_data(origarr))) SET_ERROR(H5Dwrite_chunk);

        if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);

        free(buf);
        cfp.array2d.dtor(origarr);
    }
    /* End of ZFP Array Example */
#endif

    if (0 > H5Fclose(fid)) SET_ERROR(H5Fclose);

    free(ifile);
    free(ofile);

#ifndef H5Z_ZFP_USE_PLUGIN
    /* When filter is used as a library, we need to finalize it */
    H5Z_zfp_finalize();
#endif

    H5close();

    return retval;
}
