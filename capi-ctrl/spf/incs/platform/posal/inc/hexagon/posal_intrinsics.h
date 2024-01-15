#ifndef _POSAL_INTRINSICS_H_
#define _POSAL_INTRINSICS_H_

/*======================================================================
Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
======================================================================== */
/**
@file posal_types.h

@brief This file contains basic types and pre processor macros.
 */

/* -----------------------------------------------------------------------
 ** Standard Types
 ** ----------------------------------------------------------------------- */
#if ((defined __hexagon__) || (defined __qdsp6__))
#include <hexagon_protos.h>
#endif

// 64 byte alignment for hexagon.
static const uint32_t CACHE_ALIGNMENT = 0x3F;

#define s32_ct1_s32(var1) Q6_R_ct1_R(var1)

/** Counts the leading ones starting from the MSB.*/
#ifndef s32_cl1_s32
#define s32_cl1_s32(var1) (Q6_R_cl1_R(var1))
#endif

/** Counts the leading zeros starting from the MSB.*/
#define s32_cl0_s32(var1) Q6_R_cl0_R(var1)

/** counts the trailing zeros. */
#define s32_get_lsb_s32(var1) Q6_R_ct0_R(var1)

#define s32_shl_s32_sat(var1, shift) Q6_R_asl_RR_sat(var1, shift)

#define s32_shr_s32_sat(var1, shift) Q6_R_asr_RR_sat(var1, shift)

#endif /* #ifndef POSAL_TYPES_H */
