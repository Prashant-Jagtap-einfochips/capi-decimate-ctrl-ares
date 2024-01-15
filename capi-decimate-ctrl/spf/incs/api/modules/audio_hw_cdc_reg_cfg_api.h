#ifndef _AUDIO_HW_CDC_REG_CFG_API_H 
#define _AUDIO_HW_CDC_REG_CFG_API_H
/**
 * \file audio_hw_cdc_reg_cfg_api.h 
 * \brief 
 *  	 This file contains api for audio hw codec register configuration 
 * 
 * \copyright
 *  Copyright (c) 2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //source/qcom/qct/platform/adsp/proj/addons_audio/spf/incs/api/modules/audio_hw_cdc_reg_cfg_api.h#1 $
*/
// clang-format on

#include "ar_defs.h"


/**
   Param ID for audio hw codec register configuration
   1. This resource type is used for configuring the digital and analog codec register addresses.
   2. All the codec registers to controlled via PRM can be configured using this resource ID.
*/
#define PARAM_ID_RSC_HW_CODEC_REG_INFO 0x0800131B

/*****************Register IDs*****************/

/**< Enumeration for register address invalid */
#define HW_CODEC_REG_INVALID 0x0

/**< Enumeration for register address mute control */
#define HW_CODEC_DIG_REG_ID_MUTE_CTRL 0x1

/**< Enumeration for register address command FIFO write */
#define HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE 0x2

/**Max number of reg ids that can be requested/released at a command */
#define MAX_AUD_HW_CDC_REG_ID_REQ HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE
/*****************Register IDs*****************/

/*****************Operation IDs*****************/
/**< Enumeration for register operation invalid */
#define HW_CODEC_OP_INVALID 0x0

/**< Enumeration for register operation digital mute enable */
#define HW_CODEC_OP_DIG_MUTE_ENABLE 0x1

/**< Enumeration for register operation digital mute disable */
#define HW_CODEC_OP_DIG_MUTE_DISABLE 0x2

/**< Enumeration for register operation analog mute enable */
#define HW_CODEC_OP_ANA_PGA_DISABLE 0x3

/**< Enumeration for register operation analog mute disable */
#define HW_CODEC_OP_ANA_PGA_ENABLE 0x4
/*****************Operation IDs*****************/

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** Struct for codec operation info */
/* This payload is for PARAM_ID_RSC_HW_CODEC_REG_INFO
*/
struct hw_codec_op_info_t
{
   uint32_t op_id;
   /**< Supported codec operation ID
    * @values {"HW_CODEC_OP_INVALID" = 0x0,
    *          "DIG_MUTE_ENABLE" = #HW_CODEC_OP_DIG_MUTE_ENABLE,
    *          "DIG_MUTE_DISABLE" = #HW_CODEC_OP_DIG_MUTE_DISABLE,
    *          "ANA_PGA_ENABLE" = #HW_CODEC_OP_ANA_PGA_ENABLE, 
    *          "ANA_PGA_DISABLE" = #HW_CODEC_OP_ANA_PGA_DISABLE}
    */

   uint32_t op_value;
   /**< Codec operation value. 
    * Client configures the value based upon required operation on analog and digital codec register. 
    * ******Supported codec operations under each Reg ID******
    * HW_CODEC_DIG_REG_ID_MUTE_CTRL
    *    HW_CODEC_OP_DIG_MUTE_ENABLE
    *    HW_CODEC_OP_DIG_MUTE_DISABLE
    * 
    * HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE
    *    HW_CODEC_OP_ANA_PGA_ENABLE
    *    HW_CODEC_OP_ANA_PGA_DISABLE
    * */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

typedef struct hw_codec_op_info_t hw_codec_op_info_t;
/* Note: Upon successful completion of the command, result is updated in the status field of the PRM_CMD_RSP_RELEASE_HW_RSC. */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** Struct for hw codec register info  */
/* This payload is for PARAM_ID_RSC_HW_CODEC_REG_INFO
*/
struct hw_codec_reg_info_t
{
   uint32_t reg_id;
   /**< Supported codec register ID
    * @values {"HW_CODEC_REG_INVALID" = 0x0,
    *          "MUTE_CTL" = #HW_CODEC_DIG_REG_ID_MUTE_CTRL,
    *          "CMD_FIFO_WRITE" = #HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE}
    */

   uint32_t reg_addr_msw;
   /** MSW of the codec register physical address
    *  Codec register 32-bit MSW of 64-bit physical address
    **/ 

   uint32_t reg_addr_lsw;
   /** LSW of the codec register physical address
    *  Codec register 32-bit LSW of 64-bit physical address
    **/ 

   uint32_t num_codec_op;
   /** Number of codec operations supported for this register ID. 
    * For some registers, the number of supported codec operations could be zero.
    * 0 or non zero values
    **/ 

   hw_codec_op_info_t codec_op_list[0];
   /**< Variable length array of HW codec operation info.
    * Node Type: hw_codec_op_info_t */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

typedef struct hw_codec_reg_info_t hw_codec_reg_info_t;


#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** Struct for configuring the digital and analog codec register addresses  */
struct param_id_rsc_hw_codec_reg_info_req_t
{
   uint32_t num_reg_ids;
   /**< Number of HW codec register ID’s being configured */

   hw_codec_reg_info_t reg_info_list[0];
   /**< Variable length array of HW codec register info object structure. Size of the array is equal to number of register 
    * ID’s being configured. 
    * See below structure hw_codec_reg_info_t
    */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

typedef struct param_id_rsc_hw_codec_reg_info_req_t param_id_rsc_hw_codec_reg_info_req_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** Struct for release codec operation info */
/* This payload is for PARAM_ID_RSC_HW_CODEC_REG_INFO
*/
struct hw_codec_op_info_rel_t
{
   uint32_t op_id;
   /**< Supported codec operation ID
    * @values {"HW_CODEC_OP_INVALID" = 0x0,
    *          "DIG_MUTE_ENABLE" = #HW_CODEC_OP_DIG_MUTE_ENABLE,
    *          "DIG_MUTE_DISABLE" = #HW_CODEC_OP_DIG_MUTE_DISABLE,
    *          "ANA_PGA_ENABLE" = #HW_CODEC_OP_ANA_PGA_ENABLE,
    *          "ANA_PGA_DISABLE" = #HW_CODEC_OP_ANA_PGA_DISABLE}
    */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

typedef struct hw_codec_op_info_rel_t hw_codec_op_info_rel_t;
/* Note: Upon successful completion of the command, result is updated in the status field of the PRM_CMD_RSP_RELEASE_HW_RSC. */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** Struct for release of hw codec reg info  */
/* This payload is for PARAM_ID_RSC_HW_CODEC_REG_INFO
*/
struct hw_codec_reg_info_rel_t
{
   uint32_t reg_id;
   /**< Supported codec register ID
    * @values {"HW_CODEC_REG_INVALID" = 0x0,
    *          "MUTE_CTL" = #HW_CODEC_DIG_REG_ID_MUTE_CTRL,
    *          "CMD_FIFO_WRITE" = #HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE}
    */

   uint32_t num_codec_op;
   /** Number of codec operations supported for this register ID. 
    * For some registers, the number of supported codec operations could be zero.
    * 0 or non zero values
    **/ 

   hw_codec_op_info_rel_t codec_op_list[0];
   /**< Variable length array of HW codec operation info.
    * Node Type: hw_codec_op_info_rel_t */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

typedef struct hw_codec_reg_info_rel_t hw_codec_reg_info_rel_t;


#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** Struct for release of configuring the digital and analog codec register addresses  */
/* This payload is for PARAM_ID_RSC_HW_CODEC_REG_INFO
*/
struct param_id_rsc_hw_codec_reg_info_rel_t
{
   uint32_t num_reg_ids;
   /**< Number of HW codec register ID’s being configured */

    hw_codec_reg_info_rel_t reg_info_list[0];
   /**< Variable length array of HW codec register info object structure. Size of the array is equal to number of register 
    * ID’s being configured.
    * See below struct hw_codec_reg_info_rel_t
        */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

typedef struct param_id_rsc_hw_codec_reg_info_rel_t param_id_rsc_hw_codec_reg_info_rel_t;




#endif /*_AUDIO_HW_CDC_REG_CFG_API_H */