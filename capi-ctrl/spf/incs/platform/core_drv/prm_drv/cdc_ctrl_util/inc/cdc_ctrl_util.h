/*========================================================================
This file contains audio codec control API declarations

Copyright (c) 2021 Qualcomm Technologies Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

$Header: //source/qcom/qct/platform/adsp/proj/addons_audio/spf/incs/platform/core_drv/prm_drv/cdc_ctrl_util/inc/cdc_ctrl_util.h#1 $
====================================================================== */
#ifndef _CDC_CTRL_UTIL_H_
#define _CDC_CTRL_UTIL_H_
/*=======================================================================
                            Include Files
========================================================================*/
#include "ar_defs.h"
#include "spf_list_utils.h"


#ifdef __cplusplus
extern "C"{

#endif //__cplusplus
/*=======================================================================
                                Defines
========================================================================*/
//#define CDC_CTRL_DEBUG_LOG 1
/*=======================================================================
                            Data Structures
========================================================================*/
/**List of ops supported by this utility*/
typedef enum
{
    CDC_CTRL_UTIL_DUTY_CYCLING = 0,
    CDC_CTRL_UTIL_MAX_OP = CDC_CTRL_UTIL_DUTY_CYCLING + 1
}cdc_ctrl_util_op_id;

/**Each operation will have the following states*/
typedef enum 
{
    CDC_OP_DISABLE = 0,
    CDC_OP_ENABLE = 1
}cdc_ctrl_util_op_state;

/**Will be sent by client as payload when they call cdc_ctrl_util_op_cfg*/
typedef struct cdc_util_op_state_info_t
{
    cdc_ctrl_util_op_state op_state;
    /**Codec operation state*/
}cdc_util_op_state_info_t;

/**An array of this will be sent by client.
 * This array will contain all the operation IDs
 * the client wants to register for
*/
typedef struct cdc_util_op_id_info_t
{
    cdc_ctrl_util_op_id op_id;
    /**codec operation id*/ 
}cdc_util_op_id_info_t;

#include "spf_begin_pragma.h"
/**Information sent by the client when registering with the utility*/
typedef struct cdc_ctrl_util_client_reg_info_t
{
    uint32_t heap_id;
    /**Heap_id*/

    uint32_t num_ops;
    /**Number of operations the client wants to register for*/

    cdc_util_op_id_info_t op_id_list[0];
    /**Variable length list of all operation IDs the client wants to register for
     * The length of this array will be determined by the number of operations the 
     * client wants to register for
     * */ 

}cdc_ctrl_util_client_reg_info_t;
#include "spf_end_pragma.h"

/**Information sent by the client when performing certain operations*/
typedef struct cdc_ctrl_util_client_op_info_t
{
    cdc_ctrl_util_op_id op_id;
    /**codec operation id*/

    uint32_t payload_size;
    /**Size of the payload sent by the client*/

    uint64_t payload_start[1];
    /**If any payload needs to be sent by clients, this can be used.
     * For now,the cdc_util_op_state_info_t will be sent as a payload for each operation
     * */

}cdc_ctrl_util_client_op_info_t;

/*=======================================================================
                            Function Prototypes
========================================================================*/
/*===========================================================================
    Function    : cdc_ctrl_util_register
    Description : Client registers with the utility service and gets a handle 
                    for further communication
    Params : handle              -> void handle for each client
             client_reg_info_ptr -> Info provided by the client i.e. heap_id, 
                                    num_ops to register for and op_ids
===========================================================================*/
ar_result_t cdc_ctrl_util_register(void ** handle, cdc_ctrl_util_client_reg_info_t *client_reg_info_ptr);

/*===========================================================================
    Function    : cdc_ctrl_util_deregister
    Description : Client de-registers with the utility service and the handle 
                is deleted (refer to implementation)
    Params : handle                 -> void handle for each client
             client_de_reg_info_ptr -> Info provided by the client i.e. heap_id, 
                                    num_ops to register for and op_ids
===========================================================================*/
bool_t cdc_ctrl_util_deregister(void * handle, cdc_ctrl_util_client_reg_info_t *client_de_reg_info_ptr); 

/*===========================================================================
    Function    : cdc_ctrl_util_op_cfg
    Description : Client tells the utilitycdc_ctrl_util_op_cfg service, which 
                  operation to perform
    Params : handle                      -> void handle for each client
             incoming_client_op_info_ptr -> Info provided by the client i.e. op_id,
                                            payload_size and payload associated 
                                            with that operation. For now,
                                            op_state is the only payload being sent 
                                            by the client
===========================================================================*/
ar_result_t cdc_ctrl_util_op_cfg(void * handle, cdc_ctrl_util_client_op_info_t * incoming_client_op_info_ptr);

#ifdef __cplusplus

}
#endif //__cplusplus
#endif //_CDC_CTRL_UTIL_H_