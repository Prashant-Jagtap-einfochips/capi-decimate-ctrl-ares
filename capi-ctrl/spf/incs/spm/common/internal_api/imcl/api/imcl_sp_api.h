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

#include "imcl_spm_intent_api.h"

#ifdef INTENT_ID_DUMMY_SP

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/**< Header - Any IMCL message between soft volume module and Popless equalizer
 *  will have the following header followed by the actual payload.
 *  The peers have to parse the header accordingly*/

typedef struct sp_imcl_header_t
{
   // specific purpose understandable to the IMCL peers only
   uint32_t opcode;

   // Size (in bytes) for the payload specific to the intent.
   uint32_t actual_data_len;

} sp_imcl_header_t;

#define MIN_INCOMING_IMCL_PARAM_SIZE_SP (sizeof(sp_imcl_header_t) + sizeof(intf_extn_param_id_imcl_incoming_data_t))

/* ============================================================================
   Param ID
==============================================================================*/

#define PARAM_ID_DUMMY_SP_VI_IMC_PAYLOAD 0x08001232

/*==============================================================================
   Param structure defintions
==============================================================================*/

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif // INTENT_ID_AVC

#endif /* IMCL_AVC_API_H */
