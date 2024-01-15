#ifndef IMCL_AVC_API_H
#define IMCL_AVC_API_H

/**
  @file imcl_avc_api.h

  @brief defines the Intent IDs for communication over Inter-Module Control
  Links (IMCL) between SP Tx and Rx Modules

*/
/*==========================================================================
 * Copyright (c) 2019-2021 Qualcomm Technologies Incorporated
 * All rights reserved
 * Qualcomm Technologies. Proprietary and Confidential
 * =========================================================================*/
/*===========================================================================
  Edit History

  when          who         what, where, why
  --------    -------      --------------------------------------------------

=============================================================================*/
#include "ar_defs.h"

#define INTENT_ID_DECIMATE_CONTROL 0x18001231

#ifdef INTENT_ID_DECIMATE_CONTROL

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/**< Header - Any IMCL message between soft volume module and Popless equalizer
 *  will have the following header followed by the actual payload.
 *  The peers have to parse the header accordingly*/

typedef struct decimate_imcl_header_t
{
   // specific purpose understandable to the IMCL peers only
   uint32_t opcode;

   // Size (in bytes) for the payload specific to the intent.
   uint32_t actual_data_len;

} decimate_imcl_header_t;

#define MIN_INCOMING_IMCL_PARAM_SIZE_DECIMATE (sizeof(decimate_imcl_header_t) + sizeof(intf_extn_param_id_imcl_incoming_data_t))

/* ============================================================================
   Param ID
==============================================================================*/

#define PARAM_ID_DECIMATE_CONTROL_IMC_PAYLOAD 0x18001232

/*==============================================================================
   Param structure defintions
==============================================================================*/

typedef struct capi_decimate_control_data_payload_t
{

   unsigned int decimation_factor;
   /**< @h2xmle_description {Controls audio decimation}
        @h2xmle_default     {0x0000}
        @h2xmle_range       {0..0xFFFFFFFF}

        @h2xmle_policy      {Basic} */
} capi_decimate_control_data_payload_t;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif // INTENT_ID_AVC

#endif /* IMCL_AVC_API_H */
