/*
    Copyright (c) 2016, Lawrence Livermore National Security, LLC.
        Produced at the Lawrence Livermore National Laboratory
           Written by Mark C. Miller, miller86@llnl.gov
               LLNL-CODE-707197 All rights reserved.

This file  is part  of H5Z-ZFP.  For details, see
https://github.com/LLNL/H5Z-ZFP.  Please  also  read  the   Additional
BSD Notice.

Redistribution and  use in  source and binary  forms, with  or without
modification, are permitted provided that the following conditions are
met:

* Redistributions  of  source code  must  retain  the above  copyright
  notice, this list of conditions and the disclaimer below.

* Redistributions in  binary form  must reproduce the  above copyright
  notice, this list of conditions  and the disclaimer (as noted below)
  in  the  documentation  and/or  other materials  provided  with  the
  distribution.

* Neither the name of the  LLNS/LLNL nor the names of its contributors
  may  be  used to  endorse  or  promote  products derived  from  this
  software without specific prior written permission.

THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
"AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Additional BSD Notice

1. This notice is required to  be provided under our contract with the
U.S. Department  of Energy (DOE).  This work was produced  at Lawrence
Livermore  National Laboratory  under  Contract No.  DE-AC52-07NA27344
with the DOE.

2.  Neither  the  United  States  Government  nor  Lawrence  Livermore
National Security, LLC nor any of their employees, makes any warranty,
express or implied, or assumes any liability or responsibility for the
accuracy, completeness,  or usefulness of  any information, apparatus,
product, or  process disclosed, or  represents that its use  would not
infringe privately-owned rights.

3.  Also,  reference  herein  to  any  specific  commercial  products,
process,  or  services  by  trade  name,  trademark,  manufacturer  or
otherwise does  not necessarily  constitute or imply  its endorsement,
recommendation,  or  favoring  by  the  United  States  Government  or
Lawrence Livermore  National Security, LLC. The views  and opinions of
authors expressed herein do not  necessarily state or reflect those of
the United States Government  or Lawrence Livermore National Security,
LLC,  and shall  not be  used for  advertising or  product endorsement
purposes.
*/

#include <stdlib.h>
#include <string.h>

/* The logic here for 'Z' and 'B' macros as well as there use within
   the code to call ZFP library methods is due to this filter being
   part of the Silo library but also supported as a stand-alone
   package. In Silo, the ZFP library is embedded inside a C struct
   to avoid pollution of the global namespace as well as collision
   with any other implementation of ZFP a Silo executable may be
   linked with. Calls to ZFP lib methods are preface with 'Z ' 
   and calls to bitstream methods with 'B ' as in

       Z zfp_stream_open(...);
       B sream_open(...);

*/

#ifdef Z
#undef Z
#endif

#ifdef B
#undef B
#endif

#ifdef AS_SILO_BUILTIN /* [ */
#include "hdf5.h"
#define USE_C_STRUCTSPACE
#include "zfp.h"
#define Z zfp.
#define B zfpbs.
#else /* ] AS_SILO_BUILTIN [ */
#include "H5PLextern.h"
#include "H5Spublic.h"
#include "zfp.h"
#include "bitstream.h"
#define Z
#define B 
#endif /* ] AS_SILO_BUILTIN */

#include "H5Zzfp.h"

/* Convenient CPP logic to capture Z version numbers as compile time string and hex number */
#define ZFP_VERSION_STR__(Maj,Min,Rel) #Maj "." #Min "." #Rel
#define ZFP_VERSION_STR_(Maj,Min,Rel)  ZFP_VERSION_STR__(Maj,Min,Rel)
#define ZFP_VERSION_STR                ZFP_VERSION_STR_(ZFP_VERSION_MAJOR,ZFP_VERSION_MINOR,ZFP_VERSION_RELEASE)

#define ZFP_VERSION_NO__(Maj,Min,Rel)  (0x0 ## Maj ## Min ## Rel)
#define ZFP_VERSION_NO_(Maj,Min,Rel)   ZFP_VERSION_NO__(Maj,Min,Rel)
#define ZFP_VERSION_NO                 ZFP_VERSION_NO_(ZFP_VERSION_MAJOR,ZFP_VERSION_MINOR,ZFP_VERSION_RELEASE)

#define H5Z_ZFP_PUSH_AND_GOTO(MAJ, MIN, RET, MSG)     \
do                                                    \
{                                                     \
    H5Epush(H5E_DEFAULT,__FILE__,_funcname_,__LINE__, \
        H5Z_ZFP_ERRCLASS,MAJ,MIN,MSG);                \
    retval = RET;                                     \
    goto done;                                        \
} while(0)

static size_t H5Z_filter_zfp   (unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[],
                                size_t nbytes, size_t *buf_size, void **buf);
static htri_t H5Z_zfp_can_apply(hid_t dcpl_id, hid_t type_id, hid_t space_id);
static herr_t H5Z_zfp_set_local(hid_t dcpl_id, hid_t type_id, hid_t space_id);

const H5Z_class2_t H5Z_ZFP[1] = {{

    H5Z_CLASS_T_VERS,       /* H5Z_class_t version          */
    H5Z_FILTER_ZFP,         /* Filter id number             */
    1,                      /* encoder_present flag         */
    1,                      /* decoder_present flag         */
    "H5Z-ZFP"               /* Filter name for debugging    */
    "-" H5Z_FILTER_ZFP_VERSION_STR
    " (ZFP-" ZFP_VERSION_STR ") "
    "github.com/LLNL/H5Z-ZFP; ",
    H5Z_zfp_can_apply,      /* The "can apply" callback     */
    H5Z_zfp_set_local,      /* The "set local" callback     */
    H5Z_filter_zfp,         /* The actual filter function   */

}};

#ifdef AS_SILO_BUILTIN
void        H5Z_zfp_register(void) { H5Zregister(H5Z_ZFP); }
#else
H5PL_type_t H5PLget_plugin_type(void) {return H5PL_TYPE_FILTER;}
const void *H5PLget_plugin_info(void) {return H5Z_ZFP;}
#endif

static hid_t H5Z_ZFP_ERRCLASS = -1;
#ifndef AS_SILO_BUILTIN
static
#endif
void H5Z_zfp_finalize(void)
{
    if (H5Z_ZFP_ERRCLASS != -1 && H5Z_ZFP_ERRCLASS != H5E_ERR_CLS_g)
        H5Eunregister_class(H5Z_ZFP_ERRCLASS);
    H5Z_ZFP_ERRCLASS = -1;
}

static void H5Z_zfp_init(void)
{
    /* Register the error class */
    if (H5Z_ZFP_ERRCLASS == -1)
    {
        if (H5Eget_class_name(H5E_ERR_CLS_g,0,0) < 0)
        {
            H5Z_ZFP_ERRCLASS = H5Eregister_class("H5Z-ZFP", "ZFP-" ZFP_VERSION_STR,
                                                 "H5Z-ZFP-" H5Z_FILTER_ZFP_VERSION_STR);
#if !defined(AS_SILO_BUILTIN) && !defined(NDEBUG)
        /* helps to eliminate resource leak for memory analysis */
            atexit(H5Z_zfp_finalize);
#endif
        }
        else
        {
            H5Z_ZFP_ERRCLASS = H5E_ERR_CLS_g;
        }
    }
}

static htri_t
H5Z_zfp_can_apply(hid_t dcpl_id, hid_t type_id, hid_t chunk_space_id)
{   
    static char const *_funcname_ = "H5Z_zfp_can_apply";
    int ndims, ndims_used = 0;
    size_t i, dsize;
    htri_t retval = 0;
    hsize_t dims[H5S_MAX_RANK];
    H5T_class_t dclass;
    hid_t native_type_id;

    H5Z_zfp_init();

    /* Disable the ZFP filter entirely if it looks like the ZFP library
       hasn't been compiled for 8-bit stream word size */
    if (B stream_word_bits != 8)
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_CANTINIT, -1,
            "ZFP lib not compiled with -DBIT_STREAM_WORD_TYPE=uint8");

    /* get datatype class, size and space dimensions */
    if (0 == (dclass = H5Tget_class(type_id)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADTYPE, -1, "bad datatype class");

    if (0 == (dsize = H5Tget_size(type_id)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADTYPE, -1, "bad datatype size");

    if (0 > (ndims = H5Sget_simple_extent_dims(chunk_space_id, dims, 0)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADTYPE, -1, "bad chunk data space");

    /* confirm ZFP library can handle this data */
    if (!(dclass == H5T_FLOAT || dclass == H5T_INTEGER))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADTYPE, 0,
            "requires datatype class of H5T_FLOAT or H5T_INTEGER");

    if (!(dsize == 4 || dsize == 8))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADTYPE, 0,
            "requires datatype size of 4 or 8");

    /* check for *USED* dimensions of the chunk */
    for (i = 0; i < ndims; i++)
    {
        if (dims[i] <= 1) continue;
        ndims_used++;
    }

    if (ndims_used == 0 || ndims_used > 3)
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, 0,
            "chunk must have only 1-3 non-unity dimensions");

    /* if caller is doing "endian targetting", disallow that */
    native_type_id = H5Tget_native_type(type_id, H5T_DIR_ASCEND);
    if (H5Tget_order(type_id) != H5Tget_order(native_type_id))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADTYPE, 0,
            "endian targetting non-sensical in conjunction with ZFP filter");

    retval = 1;

done:

    return retval;
}

static herr_t
H5Z_zfp_set_local(hid_t dcpl_id, hid_t type_id, hid_t chunk_space_id)
{   
    static char const *_funcname_ = "H5Z_zfp_set_local";
    int i, ndims, ndims_used = 0;
    size_t dsize, hdr_bits, hdr_bytes;
    size_t mem_cd_nelmts = H5Z_ZFP_CD_NELMTS_MEM;
    unsigned int mem_cd_values[H5Z_ZFP_CD_NELMTS_MEM];
    size_t hdr_cd_nelmts = H5Z_ZFP_CD_NELMTS_MAX;
    unsigned int hdr_cd_values[H5Z_ZFP_CD_NELMTS_MAX];
    unsigned int flags = 0;
    herr_t retval = 0;
    hsize_t dims[H5S_MAX_RANK], dims_used[3];
    H5T_class_t dclass;
    zfp_type zt;
    zfp_field *dummy_field = 0;
    bitstream *dummy_bstr = 0;
    zfp_stream *dummy_zstr = 0;
    int have_zfp_controls = 0;
    h5z_zfp_controls_t ctrls;

    H5Z_zfp_init();

    if (0 > (dclass = H5Tget_class(type_id)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_ARGS, H5E_BADTYPE, -1, "not a datatype");

    if (0 == (dsize = H5Tget_size(type_id)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_ARGS, H5E_BADTYPE, -1, "not a datatype");

    if (0 > (ndims = H5Sget_simple_extent_dims(chunk_space_id, dims, 0)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_ARGS, H5E_BADTYPE, -1, "not a data space");

    for (i = 0; i < ndims; i++)
    {
        if (dims[i] <= 1) continue;
        dims_used[ndims_used] = dims[i];
        ndims_used++;
    }

    /* setup zfp data type for meta header */
    if (dclass == H5T_FLOAT)
    {
        zt = (dsize == 4) ? zfp_type_float : zfp_type_double;
    }
    else if (dclass == H5T_INTEGER)
    {
        zt = (dsize == 4) ? zfp_type_int32 : zfp_type_int64;
    }
    else
    {
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADTYPE, 0,
            "datatype class must be H5T_FLOAT or H5T_INTEGER");
    }

    /* set up dummy zfp field to compute meta header */
    switch (ndims_used)
    {
        case 1: dummy_field = Z zfp_field_1d(0, zt, dims_used[0]); break;
        case 2: dummy_field = Z zfp_field_2d(0, zt, dims_used[1], dims_used[0]); break;
        case 3: dummy_field = Z zfp_field_3d(0, zt, dims_used[2], dims_used[1], dims_used[0]); break;
        default: H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, 0,
                     "requires chunks w/1,2 or 3 non-unity dims");
    }
    if (!dummy_field)
        H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "zfp_field_Xd() failed");

    /* get current cd_values and re-map to new cd_value set */
    if (0 > H5Pget_filter_by_id(dcpl_id, H5Z_FILTER_ZFP, &flags, &mem_cd_nelmts, mem_cd_values, 0, NULL, NULL))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_CANTGET, 0, "unable to get current ZFP cd_values");

    /* Handle default case when no cd_values are passed by using ZFP library defaults. */
    if (mem_cd_nelmts == 0)
    {
        /* check for filter controls in the properites */
        if (0 < H5Pexist(dcpl_id, "zfp_controls"))
        {
            if (0 > H5Pget(dcpl_id, "zfp_controls", &ctrls))
                H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_CANTGET, 0, "unable to get ZFP controls");
            have_zfp_controls = 1;
        }
        else // just use ZFP library defaults
        {
            mem_cd_nelmts = H5Z_ZFP_CD_NELMTS_MEM;
            H5Pset_zfp_expert_cdata(ZFP_MIN_BITS, ZFP_MAX_BITS, ZFP_MAX_PREC, ZFP_MIN_EXP, mem_cd_nelmts, mem_cd_values);
        }
    }
        
    /* Into hdr_cd_values, we encode ZFP library and H5Z-ZFP plugin version info at
       entry 0 and use remaining entries as a tiny buffer to write ZFP native header. */
    hdr_cd_values[0] = (unsigned int) ((ZFP_VERSION_NO<<16) | H5Z_FILTER_ZFP_VERSION_NO);
    if (0 == (dummy_bstr = B stream_open(&hdr_cd_values[1], sizeof(hdr_cd_values))))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "stream_open() failed");

    if (0 == (dummy_zstr = Z zfp_stream_open(dummy_bstr)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "zfp_stream_open() failed");

    /* Set the ZFP stream basic mode from mem_cd_values[0] */
    if (have_zfp_controls)
    {
        switch (ctrls.mode)
        {
            case H5Z_ZFP_MODE_RATE:
                Z zfp_stream_set_rate(dummy_zstr, ctrls.details.rate, zt, ndims, 0);
                break;
            case H5Z_ZFP_MODE_PRECISION:
                Z zfp_stream_set_precision(dummy_zstr, ctrls.details.prec, zt);
                break;
            case H5Z_ZFP_MODE_ACCURACY:
                Z zfp_stream_set_accuracy(dummy_zstr, ctrls.details.acc, zt);
                break;
            case H5Z_ZFP_MODE_EXPERT:
                Z zfp_stream_set_params(dummy_zstr, ctrls.details.expert.minbits,
                    ctrls.details.expert.maxbits, ctrls.details.expert.maxprec,
                    ctrls.details.expert.minexp);
                break;
            default:
                H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, 0, "invalid ZFP mode");
        }
    }
    else
    {
        switch (mem_cd_values[0])
        {
            case H5Z_ZFP_MODE_RATE:
                Z zfp_stream_set_rate(dummy_zstr, *((double*) &mem_cd_values[2]), zt, ndims, 0);
                break;
            case H5Z_ZFP_MODE_PRECISION:
                Z zfp_stream_set_precision(dummy_zstr, mem_cd_values[2], zt);
                break;
            case H5Z_ZFP_MODE_ACCURACY:
                Z zfp_stream_set_accuracy(dummy_zstr, *((double*) &mem_cd_values[2]), zt);
                break;
            case H5Z_ZFP_MODE_EXPERT:
                Z zfp_stream_set_params(dummy_zstr, mem_cd_values[2], mem_cd_values[3],
                    mem_cd_values[4], (int) mem_cd_values[5]);
                break;
            default:
                H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, 0, "invalid ZFP mode");
        }
    }

    /* Use ZFP's write_header method to write the ZFP header into hdr_cd_values array */
    if (0 == (hdr_bits = Z zfp_write_header(dummy_zstr, dummy_field, ZFP_HEADER_FULL)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_CANTINIT, 0, "unable to write header");

    /* Flush the ZFP stream */
    Z zfp_stream_flush(dummy_zstr);

    /* compute necessary hdr_cd_values size */
    hdr_bytes     = 1 + ((hdr_bits  - 1) / 8);
    hdr_cd_nelmts = 1 + ((hdr_bytes - 1) / sizeof(hdr_cd_values[0]));
    hdr_cd_nelmts++; /* for slot 0 holding version info */

    if (hdr_cd_nelmts > H5Z_ZFP_CD_NELMTS_MAX)
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, -1, "buffer overrun in hdr_cd_values");

    /* Now, update cd_values for the filter */
    if (0 > H5Pmodify_filter(dcpl_id, H5Z_FILTER_ZFP, flags, hdr_cd_nelmts, hdr_cd_values))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, 0,
            "failed to modify cd_values");

    /* cleanup the dummy ZFP stuff we used to generate the header */
    Z zfp_field_free(dummy_field); dummy_field = 0;
    Z zfp_stream_close(dummy_zstr); dummy_zstr = 0;
    B stream_close(dummy_bstr); dummy_bstr = 0;

    retval = 1;

done:

    if (dummy_field) Z zfp_field_free(dummy_field);
    if (dummy_zstr) Z zfp_stream_close(dummy_zstr);
    if (dummy_bstr) B stream_close(dummy_bstr);
    return retval;
}

static int
get_zfp_info_from_cd_values_0x0030(size_t cd_nelmts, unsigned int const *cd_values,
    uint64 *zfp_mode, uint64 *zfp_meta, H5T_order_t *swap)
{
    static char const *_funcname_ = "get_zfp_info_from_cd_values_0x0030";
    unsigned int cd_values_copy[H5Z_ZFP_CD_NELMTS_MAX];
    int retval = 0;
    bitstream *bstr = 0;
    zfp_stream *zstr = 0;
    zfp_field *zfld = 0;

    if (cd_nelmts > H5Z_ZFP_CD_NELMTS_MAX)
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_OVERFLOW, 0, "cd_nelmts exceeds max");

    /* make a copy of cd_values in case we need to byte-swap it */
    memcpy(cd_values_copy, cd_values, cd_nelmts * sizeof(cd_values[0]));

    /* treat the cd_values as a zfp bitstream buffer */
    if (0 == (bstr = B stream_open(&cd_values_copy[0], sizeof(cd_values_copy[0]) * cd_nelmts)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "opening header bitstream failed");

    if (0 == (zstr = Z zfp_stream_open(bstr)))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "opening header zfp stream failed");

    /* Allocate the field object */
    if (0 == (zfld = Z zfp_field_alloc()))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "allocating field failed");

    /* Read ZFP header */
    if (0 == (Z zfp_read_header(zstr, zfld, ZFP_HEADER_FULL)))
    {
        herr_t conv;

        /* The read may have failed due to difference in endian-ness between
           writer and reader. So, byte-swap cd_values array, rewind the stream and re-try. */
        if (H5T_ORDER_LE == (*swap = (H5Tget_order(H5T_NATIVE_UINT))))
            conv = H5Tconvert(H5T_STD_U32BE, H5T_NATIVE_UINT, cd_nelmts, cd_values_copy, 0, H5P_DEFAULT);
        else
            conv = H5Tconvert(H5T_STD_U32LE, H5T_NATIVE_UINT, cd_nelmts, cd_values_copy, 0, H5P_DEFAULT);
        if (conv < 0)
            H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, 0, "header endian-swap failed");

        Z zfp_stream_rewind(zstr);
        if (0 == (Z zfp_read_header(zstr, zfld, ZFP_HEADER_FULL)))
            H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_CANTGET, 0, "reading header failed");
    }

    /* Get ZFP stream mode and field meta */
    *zfp_mode = Z zfp_stream_mode(zstr);
    *zfp_meta = Z zfp_field_metadata(zfld);

    /* cleanup */
    Z zfp_field_free(zfld); zfld = 0;
    Z zfp_stream_close(zstr); zstr = 0;
    B stream_close(bstr); bstr = 0;
    retval = 1;

done:
    if (zfld) Z zfp_field_free(zfld);
    if (zstr) Z zfp_stream_close(zstr);
    if (bstr) B stream_close(bstr);

    return retval;
}

/* Decode cd_values for ZFP info for various versions of this filter */
static int
get_zfp_info_from_cd_values(size_t cd_nelmts, unsigned int const *cd_values,
    uint64 *zfp_mode, uint64 *zfp_meta, H5T_order_t *swap)
{
    unsigned int const h5z_zfp_version_no = cd_values[0]&0x0000FFFF;
    int retval = 0;

    H5Z_zfp_init();

    if (0x0020 <= h5z_zfp_version_no && h5z_zfp_version_no <= 0x0040)
        return get_zfp_info_from_cd_values_0x0030(cd_nelmts-1, &cd_values[1], zfp_mode, zfp_meta, swap);

    H5Epush(H5E_DEFAULT, __FILE__, "", __LINE__, H5Z_ZFP_ERRCLASS, H5E_PLINE, H5E_BADVALUE,
        "version mismatch: (file) 0x0%x <-> 0x0%x (code)", h5z_zfp_version_no, H5Z_FILTER_ZFP_VERSION_NO);

    return 0;
}

static size_t
H5Z_filter_zfp(unsigned int flags, size_t cd_nelmts,
    const unsigned int cd_values[], size_t nbytes,
    size_t *buf_size, void **buf)
{
    static char const *_funcname_ = "H5Z_filter_zfp";
    void *newbuf = 0;
    size_t retval = 0;
    int cd_vals_zfpver = (cd_values[0]>>16)&0x0000FFFF;
    H5T_order_t swap = H5T_ORDER_NONE;
    uint64 zfp_mode, zfp_meta;
    bitstream *bstr = 0;
    zfp_stream *zstr = 0;
    zfp_field *zfld = 0;

    if (0 == get_zfp_info_from_cd_values(cd_nelmts, cd_values, &zfp_mode, &zfp_meta, &swap))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_CANTGET, 0, "can't get ZFP mode/meta");

    if (flags & H5Z_FLAG_REVERSE) /* decompression */
    {
        int status;
        size_t bsize, dsize;

        /* Worry about zfp version and endian mismatch only for decompression */
        if (cd_vals_zfpver > ZFP_VERSION)
            H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_NOSPACE, 0, "ZFP lib version, "
                ZFP_VERSION_STR ", too old to decompress this data");

        /* Set up the ZFP field object */
        if (0 == (zfld = Z zfp_field_alloc()))
            H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "field alloc failed");

        Z zfp_field_set_metadata(zfld, zfp_meta);

        bsize = Z zfp_field_size(zfld, 0);
        switch (Z zfp_field_type(zfld))
        {
            case zfp_type_int32: case zfp_type_float:  dsize = 4; break;
            case zfp_type_int64: case zfp_type_double: dsize = 8; break;
            default: H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADTYPE, 0, "invalid datatype");
        }
        bsize *= dsize;

        if (NULL == (newbuf = malloc(bsize)))
            H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0,
                "memory allocation failed for ZFP decompression");

        Z zfp_field_set_pointer(zfld, newbuf);

        /* Setup the ZFP stream object */
        if (0 == (bstr = B stream_open(*buf, *buf_size)))
            H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "bitstream open failed");

        if (0 == (zstr = Z zfp_stream_open(bstr)))
            H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "zfp stream open failed");

        Z zfp_stream_set_mode(zstr, zfp_mode);

        /* Do the ZFP decompression operation */
        status = Z zfp_decompress(zstr, zfld);

        /* clean up */
        Z zfp_field_free(zfld); zfld = 0;
        Z zfp_stream_close(zstr); zstr = 0;
        B stream_close(bstr); bstr = 0;

        if (!status)
            H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_CANTFILTER, 0, "decompression failed");

	/* ZFP is an endian-independent format. It will produce correct endian-ness
           during decompress regardless of endian-ness differences between reader 
           and writer. However, the HDF5 library will not be expecting that. So,
           we need to undue the correct endian-ness here. We use HDF5's built-in
           byte-swapping here. Because we know we need only to endian-swap,
           we treat the data as unsigned. */
        if (swap != H5T_ORDER_NONE)
        {
            hid_t src = dsize == 4 ? H5T_STD_U32BE : H5T_STD_U64BE; 
            hid_t dst = dsize == 4 ? H5T_NATIVE_UINT32 : H5T_NATIVE_UINT64;
            if (swap == H5T_ORDER_BE)
                src = dsize == 4 ? H5T_STD_U32LE : H5T_STD_U64LE; 
            if (H5Tconvert(src, dst, bsize/dsize, newbuf, 0, H5P_DEFAULT) < 0)
                H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, 0, "endian-UN-swap failed");
        }

        free(*buf);
        *buf = newbuf;
        newbuf = 0;
        *buf_size = bsize; 
        retval = bsize;
    }
    else /* compression */
    {
        size_t msize, zsize;

        /* Set up the ZFP field object */
        if (0 == (zfld = Z zfp_field_alloc()))
            H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "field alloc failed");

        Z zfp_field_set_pointer(zfld, *buf);
        Z zfp_field_set_metadata(zfld, zfp_meta);

        /* Set up the ZFP stream object for real compression now */
        if (0 == (zstr = Z zfp_stream_open(0)))
            H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "zfp stream open failed");

        Z zfp_stream_set_mode(zstr, zfp_mode);
        msize = Z zfp_stream_maximum_size(zstr, zfld);

        /* Set up the bitstream object */
        if (NULL == (newbuf = malloc(msize)))
            H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0,
                "memory allocation failed for ZFP compression");

        if (0 == (bstr = B stream_open(newbuf, msize)))
            H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, 0, "bitstream open failed");

        Z zfp_stream_set_bit_stream(zstr, bstr);

        /* Do the compression */
        zsize = Z zfp_compress(zstr, zfld);

        /* clean up */
        Z zfp_field_free(zfld); zfld = 0;
        Z zfp_stream_close(zstr); zstr = 0;
        B stream_close(bstr); bstr = 0;

        if (zsize == 0)
            H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_CANTFILTER, 0, "compression failed");

        if (zsize > msize)
            H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_OVERFLOW, 0, "uncompressed buffer overrun");

        free(*buf);
        *buf = newbuf;
        newbuf = 0;
        *buf_size = zsize;
        retval = zsize;
    }

done:
    if (zfld) Z zfp_field_free(zfld);
    if (zstr) Z zfp_stream_close(zstr);
    if (bstr) B stream_close(bstr);
    if (newbuf) free(newbuf);
    return retval ;
}

#undef Z
#undef B
