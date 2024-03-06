/*
Copyright (c) 2016, Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory
Written by Mark C. Miller, miller86@llnl.gov
LLNL-CODE-707197. All rights reserved.

This file is part of H5Z-ZFP. Please also read the BSD license
https://raw.githubusercontent.com/LLNL/H5Z-ZFP/master/LICENSE 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;

#include "H5Zzfp_lib.h"
#include "zfp.h"

static int version_lt(
    int three,
    unsigned int diga1,
    unsigned int diga2,
    unsigned int diga3,
    unsigned int diga4,
    unsigned int digb1,
    unsigned int digb2,
    unsigned int digb3,
    unsigned int digb4)
{
    if (diga1 > digb1) return 0;
    if (diga1 < digb1) return 1;
    /* diga1 == digb1 */
    if (diga2 > digb2) return 0;
    if (diga2 < digb2) return 1;
    /* diga2 == digb2 */
    if (diga3 > digb3) return 0;
    if (diga3 < digb3) return 1;
    /* diga3 == digb3 */
    if (three)         return 0;
    if (diga4 > digb4) return 0;
    if (diga4 < digb4) return 1;
    /* diga4 == digb4 */
    return 0;
}

static void snprint_version(
    char vstr[32],
    int three,
    unsigned int dig1,
    unsigned int dig2,
    unsigned int dig3,
    unsigned int dig4)
{
    if (three)
        snprintf(vstr, 32, "%d.%d.%d", dig1, dig2, dig3);
    else
        snprintf(vstr, 32, "%d.%d.%d (%d)", dig1, dig2, dig3, dig4);
}

int main(int argc, char **argv)
{
    int i;

    unsigned int cd_val0;
    unsigned int cd_vals[10];

    if (argc <= 1) /* read from stdin */
    {
        scanf("%u ", &cd_val0);

        i = 0;
        while (scanf("%u ", &cd_vals[i]))
            i++;
    }
    else
    {
        cd_val0 = strtol(argv[1],0,10);

        for (i = 2; i < argc; i++)
            cd_vals[i-2] = (unsigned int) strtol(argv[i],0,10);
        i -= 2;
    }

{
    int q;
    printf("cd_val0 = %u\n", cd_val0);
    for (q = 0; q < i; q++)
        printf("cd_vals[%d] = %u\n", q, cd_vals[q]);
}

    unsigned int zfpdig1 = (cd_val0>>(16+12))&0xF;
    unsigned int zfpdig2 = (cd_val0>>(16+ 8))&0xF;
    unsigned int zfpdig3 = (cd_val0>>(16+ 4))&0xF;
    unsigned int zfpdig4 = (cd_val0>>(16+ 0))&0xF;

    unsigned int zfpcodec  = (cd_val0>>12)&0xF;
    unsigned int h5zfpdig1 = (cd_val0>> 8)&0xF;
    unsigned int h5zfpdig2 = (cd_val0>> 4)&0xF;
    unsigned int h5zfpdig3 = (cd_val0>> 0)&0xF;

    if (version_lt(1,h5zfpdig1,h5zfpdig2,h5zfpdig3,0,1,1,0,0))
    {
        zfpdig1 = zfpdig2;
        zfpdig2 = zfpdig3;
        zfpdig3 = zfpdig4;
        zfpdig4 = 0;
    }

    char zfpvstr[32];
    if (zfpdig1 >= 1)
        snprint_version(zfpvstr, 0, zfpdig1, zfpdig2, zfpdig3, zfpdig4);
    else
        snprint_version(zfpvstr, 1, zfpdig1, zfpdig2, zfpdig3, 0);

    char h5zfpvstr[32];
    snprint_version(h5zfpvstr, 1, h5zfpdig1, h5zfpdig2, h5zfpdig3, 0);

    int guess = 0;
    if (zfpcodec == 0)
    {
        if (version_lt(0,zfpdig1,zfpdig2,zfpdig3,zfpdig4,0,0,5,0))
            zfpcodec = 4;
        else if (version_lt(0,zfpdig1,zfpdig2,zfpdig3,zfpdig4,1,0,0,0))
            zfpcodec = zfpdig2;
        else if (version_lt(0,1,0,0,0,zfpdig1,zfpdig2,zfpdig3,zfpdig4))
            guess = zfpcodec = 5;
        else
            zfpcodec = 5; /* this is zfp 1.0.0 */
    }

    printf("H5Z-ZFP plugin version %s\n", h5zfpvstr);
    printf("ZFP library version %s (codec = %d%s)\n", zfpvstr, zfpcodec, guess?" guess":"");

{
    zfp_mode zm;

    // argc-2: one for program name and one for for first cd_val value
    bitstream *dummy_bstr = stream_open(&cd_vals[0], argc-2);
    zfp_stream *dummy_zstr = zfp_stream_open(dummy_bstr);
    zfp_field *zfld = zfp_field_alloc();
    zfp_read_header(dummy_zstr, zfld, ZFP_HEADER_FULL);

    size_t dims[4];
    zfp_field_size(zfld, dims);
    int ndims = (int) zfp_field_dimensionality(zfld);

    // now, query stream for info you seek...
    zm = zfp_stream_compression_mode(dummy_zstr);
    double rate = zfp_stream_rate(dummy_zstr, 1);
    double accuracy = zfp_stream_accuracy(dummy_zstr);
    uint precision = zfp_stream_precision(dummy_zstr);
    unsigned int minbits, maxbits, maxprec;
    int minexp;
    zfp_stream_params(dummy_zstr, &minbits, &maxbits, &maxprec, &minexp);
    zfp_stream_close(dummy_zstr);
    stream_close(dummy_bstr);
    printf("argc=%d, mode=%d, rate=%g, acc=%g, prec=%u\n", argc, (int)zm, rate, accuracy, precision);
    printf("minbits=%u, maxbits=%u, maxprec=%u, minexp=%d\n", minbits, maxbits, maxprec, minexp);
    printf("ndims = %d", ndims);
    for (int i = 0; i < ndims; i++)
        printf(", dims[%d] = %d", i, (int) dims[i]);
    printf("\n");
    zfp_field_free(zfld);
}

    return 0;
}
