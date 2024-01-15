#ifndef CAPI_FWK_EXTNS_CONTAINER_FRAME_DURATION_H
#define CAPI_FWK_EXTNS_CONTAINER_FRAME_DURATION_H

/**
 *   \file capi_fwk_extns_container_frame_duration.h
 *   \brief
 *        framework extension for modules to receive container frame duration.
 *
 *  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*

*/
// clang-format on

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*------------------------------------------------------------------------------
 * Include Files
 *----------------------------------------------------------------------------*/
#include "capi_types.h"

/**
@{ */

/** Unique identifier of the framework extension.
 * Used by modules to get the container frame duration in us from the framework
 * See FWK_EXTN_THRESHOLD_CONFIGURATION
 */
#define FWK_EXTN_CONTAINER_FRAME_DURATION 0x0A001021

/*------------------------------------------------------------------------------
 * Parameter IDs
 *----------------------------------------------------------------------------*/

/** ID of the parameter used to set the container frame duration to modules.
    This parameter can help with internal buffer allocations.

    The modules must not raise a threshold event in response to a
    #capi_vtbl_t::set_param() call of this parameter.

    @msgpayload{fwk_extn_param_id_container_frame_duration_t}
    @table{weak__fwk__extn__param__id__container__frame__duration__t}
 */
#define FWK_EXTN_PARAM_ID_CONTAINER_FRAME_DURATION 0x0A001022

typedef struct fwk_extn_param_id_container_frame_duration_t fwk_extn_param_id_container_frame_duration_t;

/** @weakgroup weak_fwk_extn_param_id_container_frame_duration_t
@{ */
struct fwk_extn_param_id_container_frame_duration_t
{
   uint32_t duration_us;
   /**< Container frame duration in microseconds based on aggregation across
        all threshold modules. */
};
/** @} */ /* end_weakgroup weak_fwk_extn_param_id_container_frame_duration_t */

/** @} */ /* end_addtogroup capi_fw_ext_frame_duration */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef CAPI_FWK_EXTNS_CONTAINER_FRAME_DURATION_H*/
