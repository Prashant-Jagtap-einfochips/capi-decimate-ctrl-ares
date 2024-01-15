/*========================================================================
  This file contains audio codec register configuration specific API declarations

  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header: //source/qcom/qct/platform/adsp/proj/addons_audio/spf/incs/platform/core_drv/prm_drv/cdc_ctrl_util/inc/audio_hw_cdc_reg_cfg.h#1 $
 ====================================================================== */
 
 #ifndef _AUDIO_HW_CDC_REG_CFG_H_
 #define _AUDIO_HW_CDC_REG_CFG_H_

 /*=======================================================================
                              Include Files
  ========================================================================*/
  #include "audio_hw_cdc_reg_cfg_api.h"
  #include "spf_list_utils.h"

#ifdef __cplusplus
extern "C" 
{
  #endif //__cplusplus

  /*=======================================================================
                                Defines
    ========================================================================*/
  #define AUDIO_HW_CDC_REG_CFG_REQUEST 1
  #define AUDIO_HW_CDC_REG_CFG_RELEASE 0
 // #define AUDIO_HW_CDC_REG_CFG_DEBUG_LOG 1
  #define REGISTER_SIZE 4096
  /*=======================================================================
                                Data Structures
    ========================================================================*/

  /** Struct for hw codec register info  */
  /* This payload is for PARAM_ID_RSC_HW_CODEC_REG_INFO
  */
  typedef struct hw_codec_reg_info_store_t
  {
    uint32_t reg_id;
    /**< Supported codec register ID
      * @values {"HW_CODEC_REG_INVALID" = 0x0,
      *          "MUTE_CTL" = #HW_CODEC_DIG_REG_ID_MUTE_CTRL,
      *          "CMD_FIFO_WRITE" = #HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE}
      */

    bool_t is_cfg_received;
    /*Boolean flag to check if config received for the reg id or not*/

    bool_t are_all_cdc_ops_removed;
    /*Boolean flag to check if all codec ops for the reg id are removed or not*/

    uint32_t reg_addr_msw;
    /** MSW of the codec register physical address
      *  Codec register 32-bit MSW of 64-bit physical address
    **/ 

    uint32_t reg_addr_lsw; //todo add corresponding vars for virt addr and save it here itself
    /** LSW of the codec register physical address
      *  Codec register 32-bit LSW of 64-bit physical address
    **/ 

    uint32_t reg_virt_addr_msw;
    /** MSW of the codec register virtual address
      *  Codec register 32-bit MSW of 64-bit virtual address
    **/

    uint32_t reg_virt_addr_lsw;
    /** LSW of the codec register virtual address
      *  Codec register 32-bit LSW of 64-bit virtual address
    **/

    uint32_t num_codec_op;
    /** Number of codec operations supported for this register ID. 
      * For some registers, the number of supported codec operations could be zero.
      * 0 or non zero values
      **/ 

    spf_list_node_t *codec_op_list_ptr;
    /**< Variable length array of HW codec operation info.
      * Node Type: hw_codec_op_info_t */
  }hw_codec_reg_info_store_t;

  /** Struct for configuring the digital and analog codec register addresses  */
  typedef struct rsc_hw_codec_reg_info_store_t
  {
    uint32_t num_reg_ids;
    /**< Number of HW codec register ID’s being configured */

    hw_codec_reg_info_store_t reg_info_list[MAX_AUD_HW_CDC_REG_ID_REQ];
    /**< Variable length array of HW codec register info object structure. Size of the array is equal to number of register 
      * ID’s being configured. 
      * See below structure hw_codec_reg_info_t
      */
  }rsc_hw_codec_reg_info_store_t;

  /*=======================================================================
                                Function Prototypes
    ========================================================================*/
    ar_result_t audio_hw_cdc_reg_cfg_init();

    ar_result_t audio_hw_cdc_reg_cfg_deinit();

    ar_result_t audio_hw_cdc_reg_cfg_add_cdc_op_to_list(hw_codec_op_info_t op_info, spf_list_node_t **codec_op_list_ptr, POSAL_HEAP_ID heap_id);

    ar_result_t audio_hw_cdc_reg_cfg_release_cdc_op_list(spf_list_node_t **codec_op_list_ptr);

    ar_result_t audio_hw_cdc_reg_cfg_remove_cdc_op_from_list(spf_list_node_t **codec_op_info_rel_ptr, spf_list_node_t **codec_op_list_ptr, uint32_t cdc_op_id);

    static inline uint32_t audio_hw_cdc_reg_cfg_id_to_index(uint32_t reg_id)
    {
      return reg_id - 1;
    }

  #ifdef __cplusplus
}
#endif //__cplusplus

#endif //_AUDIO_HW_CDC_REG_CFG_H_