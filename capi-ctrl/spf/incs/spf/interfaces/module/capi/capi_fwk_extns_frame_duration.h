#ifndef CAPI_FWK_EXTNS_FRAME_DURATION_H
#define CAPI_FWK_EXTNS_FRAME_DURATION_H

/*==============================================================================
  @file capi_fwk_extns_frame_duration.h
  @brief fwk extension to take care of frame duration extension definitions
  ==============================================================================*/
  
/*==============================================================================
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

/*==============================================================================
  Edit History

  $Header:
  $

  when        who       what, where, why
  --------    ---       -------------------------------------------------------

==============================================================================*/

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
 * Used by modules to get the nominal frame duration in us from the framework
 */
#define FWK_EXTN_NOMINAL_FRAME_DURATION             0x0A001021

/*------------------------------------------------------------------------------
 * Parameter IDs
 *----------------------------------------------------------------------------*/
#define FWK_EXTN_PARAM_ID_NOMINAL_FRAME_DURATION    0x0A001022

typedef struct fwk_extn_param_id_frame_duration_t
{
   uint32_t duration_us;      /**< Nominal frame duration in microsecond based on performance mode of the graph
                                    Actual container frame duration may be different depending on other threshold modules. */

} fwk_extn_param_id_frame_duration_t;


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef CAPI_FWK_EXTNS_FRAME_DURATION_H*/
