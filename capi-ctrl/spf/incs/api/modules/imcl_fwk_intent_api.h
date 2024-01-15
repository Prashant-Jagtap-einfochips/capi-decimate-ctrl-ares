#ifndef _IMCL_FWK_INTENT_API_H_
#define _IMCL_FWK_INTENT_API_H_
/**
 *  \file imcl_fwk_intent_api.h
 * \brief
 *    This file contains all the public intent names. 
 *    The intent related structs are defined in internal header file
 * 
 * \copyright
 *   Copyright (c) 2019-2022 Qualcomm Technologies, Inc.
 *   All Rights Reserved.
 *   Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //source/qcom/qct/platform/adsp/proj/addons_audio/spf/incs/api/modules/imcl_fwk_intent_api.h#1 $
*/
// clang-format on

/*------------------------------------------------------------------------------
 *  Header Includes
 *----------------------------------------------------------------------------*/

/** @addtogroup ar_spf_mod_ctrl_port_int_ids
    Intents used by the I2S, PCM-TDM, Sample Slip, Sample Slip EC, and SPR
    modules.
*/

#include "ar_defs.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/* -----------------------------------------------------------------------
 * Type Declarations
 * ----------------------------------------------------------------------- */

/* clang-format off */

/* clang-format on */

/*==============================================================================
  Intent IDs
==============================================================================*/

/** Identifier for the IMCL intent used to send sideband data to the encoder.
    @subhead4{Supported parameter IDs}
    - #IMCL_PARAM_ID_BT_SIDEBAND_ENCODER_FEEDBACK @lstsp1
    - #IMCL_PARAM_ID_BIT_RATE_LEVEL_ENCODER_FEEDBACK
    - #IMCL_PARAM_ID_ENC_TO_CONN_PROXY_SINK
    - #IMCL_PARAM_ID_CONN_PROXY_SINK_TO_ENC
    - #IMCL_PARAM_ID_ENC_TO_CONN_PROXY_SOURCE
    - #IMCL_PARAM_ID_CONN_PROXY_SOURCE_TO_ENC
*/
#define INTENT_ID_BT_ENCODER_FEEDBACK           0x080010D7

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _IMCL_FWK_INTENT_API_H_ */
