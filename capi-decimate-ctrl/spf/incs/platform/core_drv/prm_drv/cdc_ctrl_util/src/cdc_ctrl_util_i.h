/*========================================================================
This file contains audio codec control internal API declarations

Copyright (c) 2021 Qualcomm Technologies Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

$Header: //source/qcom/qct/platform/adsp/proj/addons_audio/spf/incs/platform/core_drv/prm_drv/cdc_ctrl_util/src/cdc_ctrl_util_i.h#1 $
====================================================================== */
#ifndef _CDC_CTRL_UTIL_I_H_
#define _CDC_CTRL_UTIL_I_H_
/*=======================================================================
                            Include Files
========================================================================*/
#include "ar_defs.h"
#include "cdc_ctrl_util.h"
#include "audio_hw_cdc_reg_cfg.h"
//#include "qurt.h"
//#include "hwd_devcfg.h"
#ifdef __cplusplus
extern "C"{

#endif //__cplusplus
/*=======================================================================
                                Defines
========================================================================*/
#define MAX_OF_REG_FOR_OPS(A, B, C) (A > B ? (A > C ? A : C) : (B > C ? B : C)) /**To determine the max number among three numbers*/

#define NUM_REG_FOR_DUTY_CYCLING 2

#define NUM_REG_FOR_OP_A 1 /**Rename these macros to the respective operation names when they are added in future*/

#define NUM_REG_FOR_OP_B 1 /**Rename these macros to the respective operation names when they are added in future*/

#define NUM_CDC_OPS_FOR_DC NUM_REG_FOR_DUTY_CYCLING

#define MAX_CDC_OPS_PER_REG NUM_REG_FOR_DUTY_CYCLING

/**Using it this way so that in future if multiple operations are added and they require more registers, 
 * the value can be adjusted accordingly. Refer use of this macro to learn more*/
#define MAX_REG MAX_OF_REG_FOR_OPS(NUM_REG_FOR_DUTY_CYCLING, NUM_REG_FOR_OP_A, NUM_REG_FOR_OP_B) 

/*=======================================================================
                            Data Structures
========================================================================*/
#include "spf_begin_pragma.h"
/**Client info node. 
 * This node will be added to the linked list in the below master structure
*/
typedef struct cdc_ctrl_client_info_t
{
  uint32_t client_id;
  /**Client id given to every client who will register*/

  uint32_t heap_id;
  /**Heap_id*/

  uint32_t num_ops;
  /**Number of operations the client wants to register for*/

  cdc_util_op_id_info_t op_id_list[0];
  /**Variable length list of all operation IDs the client wants to register for
   * The length of this array will be determined by the number of operations the 
   * client wants to register for
   * */ 
}cdc_ctrl_client_info_t;
#include "spf_end_pragma.h"

/**Client master structure will contain the following
 * num_clients       - number of clients registering
 * cdc_ctrl_list_ptr - Linked list of clients. 
 *                     Each node will contain a client's info
*/
typedef struct cdc_ctrl_client_info_master_t  
{
  uint32_t num_clients; 
  /**To count the total clients*/

  spf_list_node_t *cdc_ctrl_list_ptr;
  /**< Variable length array of HW codec ctrl list.
   * Node Type: cdc_ctrl_client_info_t */
}cdc_ctrl_client_info_master_t; 

/**Contains reg_op_id and reg_op_value*/
typedef struct cdc_reg_op_info_t
{
  uint32_t op_id;
  /**Codec operation ID
   * Refer to audio_hw_cdc_reg_cfg_api.h for
   * a list of all supported operation IDs
  */
  uint32_t op_value;
    /**Codec operation value*/
}cdc_reg_op_info_t;

#include "spf_begin_pragma.h"
/**Register info node will be added in the operation's master sructure*/
typedef struct cdc_ctrl_reg_info_t 
{ 
  uint32_t reg_id;
  /**Supported codec register IDs*/

  uint32_t reg_virt_addr_msw;
  /**MSW of virtual address of register*/
  
  uint32_t reg_virt_addr_lsw;
  /**LSW of virtual address of register*/

  uint32_t num_cdc_ops;
  /**Number of operations on the register*/

//  cdc_reg_op_info_t reg_op_list[0]; //malloc -sizeof(cdc_ctrl_reg_info_t + (cdc_reg_op_info_t * num_ops))
  cdc_reg_op_info_t *reg_op_list_ptr; //malloc -sizeof(cdc_ctrl_reg_info_t + (cdc_reg_op_info_t * num_ops))
  /**Variable length array storing required register opearations*/

}cdc_ctrl_reg_info_t;
#include "spf_end_pragma.h"

/**Master structure to store the reg info for duty cycling operations 
 * num_reg_ids       - Num of registers
 * num_dc_clients    - Num of clients registered for duty cycling
 * reg_info_list     - Array of register info required for duty cycling
 * 
 * Each opearation can have a master structure of their own like below and add the above nodes
*/
typedef struct duty_cycling_reg_info_t
{
  uint32_t num_reg_ids;
  /*Number of register IDs*/

  uint32_t num_dc_clients;
  /**To count the number of clients registered for duty cycling*/

 cdc_ctrl_reg_info_t reg_info_list[NUM_REG_FOR_DUTY_CYCLING];
 /**Array of register info required for duty cycling*/
}duty_cycling_reg_info_t;

/**Global structure which will contain all the globally used structures
 * Global ptr will be declared for this
*/
typedef struct cdc_ctrl_util_t
{
  cdc_ctrl_client_info_master_t cdc_ctrl_client_info_master;
  /**Client info master structure*/
  
  hw_codec_reg_info_store_t *reg_info_ptr[MAX_REG];
  /**Ptr array to store required reg info in case of NLPI opeartion*/
}cdc_ctrl_util_t;

/**Global structure which will contain all the globally used operation structures
 * Right now it only contains duty cycling op structure.
 * More structures will be added in future versions
 * Need to keep this separate from above cdc_ctrl_util_t as this is supposed to be 
 * present in island, if required whereas cdc_ctrl_util_t will not be present in island
*/
typedef struct cdc_ctrl_reg_op_t
{
  duty_cycling_reg_info_t duty_cycling_reg_info;
  /**Master structure to store the reg info for duty cycling operations*/
}cdc_ctrl_reg_op_t;

/**Register info required per operation
 * Refer to codec_op_lut_t below
*/
typedef struct reg_lut_op_info_t
{
  uint32_t reg_id;
  /**Register ID*/

  uint32_t num_cdc_ops;
  /**Num of cdc ops required for this reg ID*/

  uint32_t reg_op_list[MAX_CDC_OPS_PER_REG];
  /**Array of reg operations required for this operation
   * Refer to audio_hw_cdc_reg_cfg_api.h for a list of supported 
   * register operation IDs
  */
}reg_lut_op_info_t;

/**Codec operations lookup table
 * Will contain register info to be copied on per operation basis
 * Refer to cdc_ctrl_util_op_id for a list of supported operations
*/
typedef struct codec_op_lut_t
{
  uint32_t num_reg;
  /**Num of reg required for that operation*/

  reg_lut_op_info_t reg_info_list[MAX_REG];
  /**Array of reg info for that operation*/
 
}codec_op_lut_t;

/*=======================================================================
                            Function Prototypes
========================================================================*/
/*===========================================================================
  Function    : cdc_ctrl_init
  DESCRIPTION : To init the global struct that stores client info
===========================================================================*/
ar_result_t cdc_ctrl_init(); //todo once combined, remove these init de init and use the audio_h2_cdc_reg_cfg init for this

/*===========================================================================
  Function    : cdc_ctrl_deinit
  DESCRIPTION : To deinit the global struct that stores client info
===========================================================================*/
ar_result_t cdc_ctrl_deinit();

/*===========================================================================
  Function    : cdc_ctrl_get_op_reg_info
  DESCRIPTION : To store register info which will be later used when 
                performing operations
===========================================================================*/
ar_result_t cdc_ctrl_get_op_reg_info(uint32_t op_id, uint32_t heap_id);

/*===========================================================================
  Function    : cdc_ctrl_get_duty_cycling_reg_info
  DESCRIPTION : To store register info for duty cycling operation
===========================================================================*/
ar_result_t cdc_ctrl_get_duty_cycling_reg_info(uint32_t op_id, uint32_t heap_id);

/*===========================================================================
  Function    : cdc_ctrl_perform_duty_cycling
  DESCRIPTION : To perform duty cycling
===========================================================================*/
ar_result_t cdc_ctrl_perform_duty_cycling(cdc_ctrl_util_client_op_info_t *client_op_info_ptr);


#ifdef SIM
/*===========================================================================
  Function    : cdc_ctrl_util_print_book
  DESCRIPTION : To print the book-keeping structure
===========================================================================*/
  void cdc_ctrl_util_print_book();
#endif

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_CDC_CTRL_UTIL_I_H_
