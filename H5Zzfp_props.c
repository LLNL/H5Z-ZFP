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

#include "H5Zzfp.h"

#include "hdf5.h"

#include <stdarg.h>
#include <stdlib.h>

#define H5Z_ZFP_PUSH_AND_GOTO(MAJ, MIN, RET, MSG)     \
do                                                    \
{                                                     \
    H5Epush(H5E_DEFAULT,__FILE__,_funcname_,__LINE__, \
        H5E_ERR_CLS_g,MAJ,MIN,MSG);                   \
    retval = RET;                                     \
    goto done;                                        \
} while(0)

static herr_t H5Pset_zfp(hid_t plist, int mode, ...)
{
    static char const *_funcname_ = "H5Pset_zfp";
    static size_t ctrls_sz = sizeof(h5z_zfp_controls_t);
    unsigned int flags;
    size_t cd_nelmts = 0;
    unsigned int cd_values[1];
    h5z_zfp_controls_t *ctrls_p = 0;
    int i;
    va_list ap;
    herr_t retval;

    if (0 >= H5Pisa_class(plist, H5P_DATASET_CREATE))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_ARGS, H5E_BADTYPE, -1, "not a dataset creation property list class");

    ctrls_p = (h5z_zfp_controls_t *) malloc(ctrls_sz);
    if (0 == ctrls_p)
        H5Z_ZFP_PUSH_AND_GOTO(H5E_RESOURCE, H5E_NOSPACE, -1, "allocation failed for ZFP controls");

    va_start(ap, mode);
    ctrls_p->mode = mode;
    switch (mode)
    {
        case H5Z_ZFP_MODE_RATE:
        {
            ctrls_p->details.rate = va_arg(ap, double);
            if (0 > ctrls_p->details.rate)
                H5Z_ZFP_PUSH_AND_GOTO(H5E_ARGS, H5E_BADVALUE, -1, "rate out of range.");
            break;
        }
        case H5Z_ZFP_MODE_ACCURACY:
        {
            ctrls_p->details.acc = va_arg(ap, double);
            if (0 > ctrls_p->details.acc)
                H5Z_ZFP_PUSH_AND_GOTO(H5E_ARGS, H5E_BADVALUE, -1, "accuracy out of range.");
            break;
        }
        case H5Z_ZFP_MODE_PRECISION:
        {
            ctrls_p->details.prec = va_arg(ap, uint);
            break;
        }
        case H5Z_ZFP_MODE_EXPERT:
        {
            ctrls_p->details.expert.minbits = va_arg(ap, uint);
            ctrls_p->details.expert.maxbits = va_arg(ap, uint);
            ctrls_p->details.expert.maxprec = va_arg(ap, uint);
            ctrls_p->details.expert.minexp  = va_arg(ap, int);
            break;
        }
        default:
        {
            H5Z_ZFP_PUSH_AND_GOTO(H5E_ARGS, H5E_BADVALUE, -1, "bad ZFP mode.");
            break;
        }
    }
    va_end(ap);

    for (i = 0; i < H5Pget_nfilters(plist); i++)
    {
        H5Z_filter_t fid;
        if (0 <= (fid = H5Pget_filter(plist, i, &flags, &cd_nelmts, cd_values, 0, 0, 0))
            && fid == H5Z_FILTER_ZFP)
        {
            if (0 > H5Premove_filter(plist, H5Z_FILTER_ZFP))
                H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, -1, "Unable to remove old ZFP filter from pipeline.");
            break;
        }
    }

    if (0 > H5Pset_filter(plist, H5Z_FILTER_ZFP, H5Z_FLAG_MANDATORY, 0, 0))
        H5Z_ZFP_PUSH_AND_GOTO(H5E_PLINE, H5E_BADVALUE, -1, "Unable to put ZFP filter in pipeline.");

    if (0 == H5Pexist(plist, "zfp_controls"))
    {
        retval = H5Pinsert2(plist, "zfp_controls", ctrls_sz, ctrls_p, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        retval = H5Pset(plist, "zfp_controls", ctrls_p);
    }

    return retval;

done:

    if (ctrls_p)
        free(ctrls_p);
    va_end(ap);

    return retval;

}

herr_t H5Pset_zfp_rate(hid_t plist, double rate)
{
    return H5Pset_zfp(plist, H5Z_ZFP_MODE_RATE, rate);
}

herr_t H5Pset_zfp_precision(hid_t plist, unsigned int prec)
{
    return H5Pset_zfp(plist, H5Z_ZFP_MODE_PRECISION, prec);
}

herr_t H5Pset_zfp_accuracy(hid_t plist, double acc)
{
    return H5Pset_zfp(plist, H5Z_ZFP_MODE_ACCURACY, acc);
}

herr_t H5Pset_zfp_expert(hid_t plist, unsigned int minbits, unsigned int maxbits,
    unsigned int maxprec, int minexp)
{
    return H5Pset_zfp(plist, H5Z_ZFP_MODE_EXPERT, minbits, maxbits, maxprec, minexp);
}
