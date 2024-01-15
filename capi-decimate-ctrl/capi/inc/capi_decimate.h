/* ======================================================================== */
/**
   @file capi_decimate.h

   C source file to implement CAPI Decimation example
 */

/* =========================================================================
   Copyright (c) 2015-2021 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
#ifndef CAPI_DECIMATE_H_
#define CAPI_DECIMATE_H_
#include "ar_defs.h"
#include "ar_msg.h"
#include "capi.h"
#include "audio_decimate_calib.h"

// Decimate module calibration parameters
#define CAPI_PARAM_ID_DECIMATE_ENABLE DECIMATE_PARAM_MOD_ENABLE
#define CAPI_PARAM_ID_UPDATE_DECIMATION_FACTOR DECIMATE_PARAM_DECIMATION_FACTOR

#ifdef __cplusplus
extern "C" {
#endif

capi_err_t __attribute__((visibility("default")))
capi_decimate_get_static_properties(capi_proplist_t *init_set_properties, capi_proplist_t *static_properties);
capi_err_t __attribute__((visibility("default")))
capi_decimate_init(capi_t *_pif, capi_proplist_t *init_set_properties);
#ifdef __cplusplus
}
#endif

#endif /* CAPI_DECIMATE_H_ */
