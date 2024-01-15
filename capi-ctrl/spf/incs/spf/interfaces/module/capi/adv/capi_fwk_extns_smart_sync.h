#ifndef _CAPI_FWK_EXTNS_SMART_SYNC_H_
#define _CAPI_FWK_EXTNS_SMART_SYNC_H_

/*==============================================================================
  @file capi_fwk_extns_smart_sync.h
  @brief This file contains CAPI Smart Sync Module Framework Extension Definitions

  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

/*==============================================================================
  Edit History

  when       who        what, where, why
  --------   ---        ------------------------------------------------------

==============================================================================*/

/*=====================================================================
  Includes
 ======================================================================*/
#include "capi_types.h"
#include "capi_fwk_extns_sync.h"

/*==============================================================================
   Constants
==============================================================================*/
/* Framework extension ID for the Smart Sync module */
#define FWK_EXTN_SMART_SYNC  0x0A00100D

/*==============================================================================
   Constants
==============================================================================*/
/* Custom event id to raise event when module wants to enable/disable threshold
   buffering. With threshold buffering disabled, the framework should invoke the
   topology whenever input data is received, regardless of whether the threshold
   amount of input data is met or not. */
#define FWK_EXTN_SMART_SYNC_EVENT_ID_ENABLE_THRESHOLD_BUFFERING   0x0A00100E

/*==============================================================================
   Type definitions
==============================================================================*/
/* Structure defined for above Property  */
typedef struct fwk_extn_smart_sync_event_id_enable_threshold_buffering_t
{
   bool_t enable_threshold_buffering;
   /**< Flag which indicates whether threshold buffering should be enabled or
        disabled. 1 for enabled, 0 for disabled.
        @values : {0..1} */
} fwk_extn_smart_sync_event_id_enable_threshold_buffering_t;

/*==============================================================================
   Constants
==============================================================================*/
/* Custom param id to inform the smart sync module of the container's external input
   threshold in microseconds. This MUST be configured before the use case is
   started because this information is needed for port synchronization. */
#define FWK_EXTN_SMART_SYNC_PARAM_ID_EXT_INPUT_THRESHOLD_US  FWK_EXTN_SYNC_PARAM_ID_EXT_INPUT_THRESHOLD_US

/*==============================================================================
   Type definitions
==============================================================================*/
/* Structure defined for above Property  */
typedef struct fwk_extn_smart_sync_param_id_ext_input_threshold_us_t
{
   uint32_t threshold_us;
   /**< The container's external input threshold in microseconds (how much data must
        be buffered at the external input before the topology is invoked).
        @values : {0..0xFFFFFFFF} */
} fwk_extn_smart_sync_param_id_ext_input_threshold_us_t;

/*==============================================================================
   Constants
==============================================================================*/
/* Custom param id to inform the smart sync module of the container's reception
   of the first proc tick. It is also set anytime there's a resync upon the first
   proc tick after it. This param has no payload. */
#define FWK_EXTN_SMART_SYNC_PARAM_ID_FIRST_PROC_TICK_NOTIF  0x0A001026


#endif /* _CAPI_FWK_EXTNS_SMART_SYNC_H_ */
