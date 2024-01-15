#ifndef CAPI_FWK_EXTNS_THRESH_CFG_H
#define CAPI_FWK_EXTNS_THRESH_CFG_H

/**
 *   \file capi_fwk_extns_thresh_cfg.h
 *   \brief
 *        framework extension for modules to receive threshold configuration.
 *
 * Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //source/qcom/qct/platform/adsp/proj/addons_audio/spf/incs/spf/interfaces/module/capi/capi_fwk_extns_thresh_cfg.h#3 $
*/
// clang-format on

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*------------------------------------------------------------------------------
 * Include Files
 *----------------------------------------------------------------------------*/
#include "capi_types.h"

/** @addtogroup capi_fw_ext_threshold
@{ */

/** Unique identifier of the framework extension that modules use to get the
    nominal frame duration (in microseconds) from the framework.
 */
#define FWK_EXTN_THRESHOLD_CONFIGURATION 0x0A00104D

/*------------------------------------------------------------------------------
 * Parameter IDs
 *----------------------------------------------------------------------------*/

/** ID of the parameter used configure the threshold based on the performance
    mode of the graph.

    @msgpayload{fwk_extn_param_id_threshold_cfg_t}
    @table{weak__fwk__extn__param__id__threshold__cfg__t}
 */
#define FWK_EXTN_PARAM_ID_THRESHOLD_CFG 0x0A00104E

typedef struct fwk_extn_param_id_threshold_cfg_t fwk_extn_param_id_threshold_cfg_t;

/** @weakgroup weak_fwk_extn_param_id_threshold_cfg_t
@{ */
struct fwk_extn_param_id_threshold_cfg_t
{
   uint32_t duration_us;
   /**< Threshold configuration (in microseconds) based on the performance mode
        of the graph.

        The actual container frame duration might be different depending on
        other threshold modules. See #FWK_EXTN_CONTAINER_FRAME_DURATION. */
};
/** @} */ /* end_weakgroup weak_fwk_extn_param_id_threshold_cfg_t */

/** @} */ /* end_addtogroup capi_fw_ext_threshold */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef CAPI_FWK_EXTNS_THRESH_CFG_H*/
