/*========================================================================
   This file contains audio codec control APIs.

  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header:
 //components/dev/avs.fwk/1.0/smanglam.avs.fwk.1.0.16july21_lahaina/platform/core_drv/prm_drv/cdc_ctrl_util/src/cdc_ctrl_util.c#1
 $
 ====================================================================== */
/*==========================================================================
 Include files
========================================================================== */
#include "cdc_ctrl_util.h"
#include "cdc_ctrl_util_i.h"
#include "audio_hw_cdc_reg_cfg.h"
/*==========================================================================
  Globals
========================================================================== */
/**Global structure containing various structures
 * One of the strucutres will contain client information*/
cdc_ctrl_util_t cdc_ctrl_util_global;

/**Pointer setup to the above info for easier access*/
cdc_ctrl_util_t *cdc_ctrl_util_global_ptr = &cdc_ctrl_util_global;

#ifndef SIM // Added to avoid compilation error. Would it be available on target build?
rsc_hw_codec_reg_info_store_t  hw_codec_info_req_store_global;
rsc_hw_codec_reg_info_store_t *hw_codec_info_req_store_global_ptr = &hw_codec_info_req_store_global;
#endif

/**Global ptr to the register operation info structure
 * This is declared in island cdc_ctrl_util_island
 */
extern cdc_ctrl_reg_op_t *cdc_ctrl_reg_op_global_ptr;

/**Global ptr to the register data structure from audio_hw_cdc_reg_cfg*/
extern rsc_hw_codec_reg_info_store_t *hw_codec_info_req_store_global_ptr;

codec_op_lut_t codec_op_lookup_table[CDC_CTRL_UTIL_MAX_OP] = {
   /**CDC_CTRL_UTIL_DUTY_CYCLING lookup table*/
   { NUM_REG_FOR_DUTY_CYCLING,
     { { HW_CODEC_DIG_REG_ID_MUTE_CTRL,
         NUM_CDC_OPS_FOR_DC,
         { HW_CODEC_OP_DIG_MUTE_ENABLE, HW_CODEC_OP_DIG_MUTE_DISABLE } },
       { HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE,
         NUM_CDC_OPS_FOR_DC,
         { HW_CODEC_OP_ANA_PGA_DISABLE,
           HW_CODEC_OP_ANA_PGA_ENABLE } } } } /**For operations being added in the future, keep adding the info here*/
};
/*==========================================================================
  Function Implementations
========================================================================== */

ar_result_t cdc_ctrl_init(void)
{
#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Inside cdc_ctrl_init");
#endif

   ar_result_t result = AR_EOK;

   memset(cdc_ctrl_util_global_ptr, 0x00, sizeof(cdc_ctrl_client_info_master_t));
   memset(cdc_ctrl_reg_op_global_ptr, 0x00, sizeof(cdc_ctrl_reg_op_t));

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Exiting cdc_ctrl_init");
#endif

   return result;
}

ar_result_t cdc_ctrl_deinit(void)
{
#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Inside cdc_ctrl_deinit");
#endif

   ar_result_t result = AR_EOK;

   memset(cdc_ctrl_util_global_ptr, 0x00, sizeof(cdc_ctrl_client_info_master_t));
   memset(cdc_ctrl_reg_op_global_ptr, 0x00, sizeof(cdc_ctrl_reg_op_t));

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Exiting cdc_ctrl_deinit");
#endif

   return result;
}

ar_result_t cdc_ctrl_util_register(void **handle, cdc_ctrl_util_client_reg_info_t *client_reg_info_ptr)
{
#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Inside cdc_ctrl_util_register");
#endif

   ar_result_t result = AR_EOK;

   if (NULL == client_reg_info_ptr)
   {
      AR_MSG(DBG_ERROR_PRIO, "Client info ptr is NULL");
      result |= AR_EFAILED;
      return result;
   }
   uint32_t num_ops = 0;
   uint32_t client_node_size =
      sizeof(cdc_ctrl_client_info_t) + (sizeof(cdc_util_op_id_info_t) * client_reg_info_ptr->num_ops);

   cdc_ctrl_client_info_t *client_info_ptr =
      (cdc_ctrl_client_info_t *)posal_memory_malloc(client_node_size, client_reg_info_ptr->heap_id);
   if (NULL == client_info_ptr)
   {
      AR_MSG(DBG_ERROR_PRIO, "Failed to allocate memory for cdc ctrl client info ptr");
      result = AR_ENOMEMORY;
      return result;
   }
   memset(client_info_ptr, 0x00, client_node_size);

   /*Unique client_id*/
   client_info_ptr->client_id = (uint32_t)client_info_ptr;
   client_info_ptr->heap_id   = client_reg_info_ptr->heap_id;

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Client id 0x%x heap_id %x", client_info_ptr->client_id, client_info_ptr->heap_id);
#endif

   num_ops = client_reg_info_ptr->num_ops;
   if (num_ops > CDC_CTRL_UTIL_MAX_OP || num_ops <= 0)
   {
      AR_MSG(DBG_ERROR_PRIO, "Invalid num_ops %d", num_ops);
      posal_memory_free(client_info_ptr);
      return AR_EBADPARAM;
   }

   /**Saving the num_ops in the client node*/
   client_info_ptr->num_ops = num_ops;

   for (uint32_t i = 0; i < num_ops; i++)
   {
      if (CDC_CTRL_UTIL_MAX_OP < client_reg_info_ptr->op_id_list[i].op_id)
      {
         AR_MSG(DBG_ERROR_PRIO, "Invalid op_id 0x%x", client_reg_info_ptr->op_id_list[i].op_id);
         posal_memory_free(client_info_ptr);
         return AR_EFAILED;
      }

#ifdef CDC_CTRL_DEBUG_LOG
      AR_MSG(DBG_LOW_PRIO, "Client registering for op_id 0x%x i %d", client_reg_info_ptr->op_id_list[i].op_id, i);
#endif

      client_info_ptr->op_id_list[i].op_id = client_reg_info_ptr->op_id_list[i].op_id;

      /**Get reg_info*/
      result = cdc_ctrl_get_op_reg_info(client_info_ptr->op_id_list[i].op_id, client_info_ptr->heap_id);
      if (AR_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "Failed to get register operation info with result %d", result);
         posal_memory_free(client_info_ptr);
         return result;
      }

#ifdef CDC_CTRL_DEBUG_LOG
      AR_MSG(DBG_LOW_PRIO, "Client registered for op_id 0x%lx", client_info_ptr->op_id_list[i].op_id);
#endif
   }

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Attempting to insert client node");
   AR_MSG(DBG_LOW_PRIO, "head_ptr addr 0x%x", &cdc_ctrl_util_global_ptr->cdc_ctrl_client_info_master.cdc_ctrl_list_ptr);
   AR_MSG(DBG_LOW_PRIO, "client info ptr 0x%x", client_info_ptr);
   AR_MSG(DBG_LOW_PRIO, "Heap id 0x%x", (POSAL_HEAP_ID)client_info_ptr->heap_id);
#endif

   /*Add node to the list of clients*/
   result = spf_list_insert_tail((spf_list_node_t **)&(
                                    cdc_ctrl_util_global_ptr->cdc_ctrl_client_info_master.cdc_ctrl_list_ptr),
                                 client_info_ptr,
                                 (POSAL_HEAP_ID)client_info_ptr->heap_id,
                                 FALSE);
   if (AR_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "Failed to add node to client info list. Result = %d", result);
      posal_memory_free(client_info_ptr);
      return result;
   }

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Client node inserted");
#endif

   /*Assign the ptr to handle which will be used by the client*/
   *handle = (void *)client_info_ptr;

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Assigned handle 0x%x", *handle);
#endif

   /*Incrementing the number of clients*/
   cdc_ctrl_util_global_ptr->cdc_ctrl_client_info_master.num_clients++;

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Global client count %d", cdc_ctrl_util_global_ptr->cdc_ctrl_client_info_master.num_clients);
#endif

#ifdef SIM
   cdc_ctrl_util_print_book();
#endif

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Exiting cdc_ctrl_util_register");
#endif

   return result;
}

bool_t cdc_ctrl_util_deregister(void *handle, cdc_ctrl_util_client_reg_info_t *client_de_reg_info_ptr)
{
#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Inside cdc_ctrl_util_deregister");
#endif

   bool_t is_delete_done = TRUE;

   if (NULL == handle)
   {
      AR_MSG(DBG_ERROR_PRIO, "Received handle is NULL. Nothing to delete");
      is_delete_done = FALSE;
      return is_delete_done;
   }

   cdc_ctrl_client_info_t *client_info_ptr = (cdc_ctrl_client_info_t *)handle;

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Removing node with client_id 0x%lx", client_info_ptr->client_id);
#endif

   is_delete_done = spf_list_find_delete_node((spf_list_node_t **)&cdc_ctrl_util_global_ptr->cdc_ctrl_client_info_master
                                                 .cdc_ctrl_list_ptr,
                                              client_info_ptr,
                                              FALSE);
   if (!is_delete_done)
   {
      AR_MSG(DBG_ERROR_PRIO, "Failed to delete client node with status %d", is_delete_done);
      return is_delete_done;
   }
#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Removed node with client_id 0x%lx with status %d", client_info_ptr->client_id, is_delete_done);
#endif

   /*Decrementing the number of clients*/
   cdc_ctrl_util_global_ptr->cdc_ctrl_client_info_master.num_clients--;

#ifdef CDC_CTRL_DEBUG_LOG
   AR_MSG(DBG_LOW_PRIO, "Exiting cdc_ctrl_util_deregister");
#endif

   return is_delete_done;
}

ar_result_t cdc_ctrl_get_op_reg_info(uint32_t op_id, uint32_t heap_id)
{
   ar_result_t result = AR_EOK;

   uint32_t num_reg_ids_in_lut         = 0;
   uint32_t num_reg_ids_in_cmd_handler = 0;
   uint32_t cdc_op_index               = 0;
   uint32_t lut_reg_counter            = 0;
   uint32_t p_reg_counter              = 0;
   uint32_t reg_op_counter             = 0;
   uint32_t lut_reg_id                 = 0;
   uint32_t num_cdc_ops_to_copy        = 0;
   uint32_t cdc_op_block_size          = 0;

   /**op_id starts from 0 and operations are stored in the same order in the lookup table,
    * as they are in cdc_ctrl_util_op_id
    */
   cdc_op_index = op_id;

   num_reg_ids_in_lut = codec_op_lookup_table[cdc_op_index].num_reg;

   /**Iterate over the lookup table for the operation and do the following
    * 1. Iterate over hw_codec_info_req_store_global_ptr and compare the
    * reg_ids with the one in lookup table.
    * 2. Once a reg_id is found, check which heap is this operation requested for
    * 3. If it is default heap, store ptr to reg info
    * 4. If it is island heap, copy the reg info to cdc_ctrl_reg_op_global_ptr
    */
   while (lut_reg_counter < num_reg_ids_in_lut)
   {
#ifdef CDC_CTRL_DEBUG_LOG
      AR_MSG(DBG_LOW_PRIO, "lut_reg_counter = %d", lut_reg_counter);
#endif

      p_reg_counter = 0;

      /**Get the lookup table reg id*/
      lut_reg_id = codec_op_lookup_table[cdc_op_index].reg_info_list[lut_reg_counter].reg_id;

      /**Get the num reg ids in cmd handler*/
      num_reg_ids_in_cmd_handler = hw_codec_info_req_store_global_ptr->num_reg_ids;

      /**Num of reg ops to copy per register
       * This might vary per operation
       */
      num_cdc_ops_to_copy = codec_op_lookup_table[cdc_op_index].reg_info_list[lut_reg_counter].num_cdc_ops;

#ifdef CDC_CTRL_DEBUG_LOG
      AR_MSG(DBG_LOW_PRIO,
             "lut_reg_id 0x%x num_reg_ids_in_cmd_handler %d num_cdc_ops_to_copy %d",
             lut_reg_id,
             num_reg_ids_in_cmd_handler,
             num_cdc_ops_to_copy);
#endif

      /**1. Iterate over hw_codec_info_req_store_global_ptr and compare the
       * reg_ids with the one in lookup table.*/
      for (p_reg_counter = 0; p_reg_counter < num_reg_ids_in_cmd_handler; p_reg_counter++)
      {
#ifdef CDC_CTRL_DEBUG_LOG
         AR_MSG(DBG_LOW_PRIO, "p_reg_counter = %d", p_reg_counter);
#endif

         /**Check if the reg id in lookup table and cmd handler structure are same*/
         if (lut_reg_id != hw_codec_info_req_store_global_ptr->reg_info_list[p_reg_counter].reg_id)
         {
#ifdef CDC_CTRL_DEBUG_LOG
            AR_MSG(DBG_LOW_PRIO,
                   "lut_reg_id 0x%x and cmd_hdlr_reg_id 0x%x don't match.",
                   lut_reg_id,
                   hw_codec_info_req_store_global_ptr->reg_info_list[p_reg_counter].reg_id);
#endif

            /**If reg id dont match then we need to visit the next reg id in cmd handler*/
            continue;
         }

#ifdef CDC_CTRL_DEBUG_LOG
         AR_MSG(DBG_LOW_PRIO,
                "lut_reg_id = 0x%x cmd_hdlr_reg_id = 0x%x match",
                lut_reg_id,
                hw_codec_info_req_store_global_ptr->reg_info_list[p_reg_counter].reg_id);
#endif

         /**This is a check to see if op specific register infor has already been copied or not.
          * Since it is operation specific, a switch is required here*/
         switch (op_id)
         {
            case CDC_CTRL_UTIL_DUTY_CYCLING:
            {
               /**Return if info already received*/
               if (cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.num_dc_clients)
               {
                  AR_MSG(DBG_HIGH_PRIO,
                         "Register info is already received. num_dc_clients = %d Returning",
                         cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.num_dc_clients);
                  /**Increment to show that a client registered for DC*/
                  cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.num_dc_clients++;
                  return AR_EOK;
               }
               break;
            }
            default:
            {
               AR_MSG(DBG_ERROR_PRIO, "Unsupported op_id 0x%lx", op_id);
               return AR_EFAILED;
               break;
            }
         }

         /**2. Once a reg_id is found, check which heap is this operation requested for*/
         /**3. If it is default heap, store ptr to reg info*/
         if (POSAL_HEAP_DEFAULT == heap_id)
         {
            /**Copy the address to node containing reg info*/
            cdc_ctrl_util_global_ptr->reg_info_ptr[lut_reg_counter] =
               &hw_codec_info_req_store_global_ptr->reg_info_list[p_reg_counter];

#ifdef CDC_CTRL_DEBUG_LOG
            AR_MSG(DBG_LOW_PRIO, "Copied ptr to following reg_info");
            AR_MSG(DBG_LOW_PRIO,
                   "Reg_id = 0x%x p_add_lsw = 0x%x v_add_lsw = 0x%x",
                   cdc_ctrl_util_global_ptr->reg_info_ptr[lut_reg_counter]->reg_id,
                   cdc_ctrl_util_global_ptr->reg_info_ptr[lut_reg_counter]->reg_addr_lsw,
                   cdc_ctrl_util_global_ptr->reg_info_ptr[lut_reg_counter]->reg_virt_addr_lsw);
#endif

            /**Break because now we already got the required reg info for this iteration*/
            break;
         }
         /**4. If it is island heap, copy the reg info to cdc_ctrl_reg_op_global_ptr*/
         else
         {
            /**A switch case is required here as the data structure to be used to copy the register data will vary per
             * operation*/
            switch (op_id)
            {
               case CDC_CTRL_UTIL_DUTY_CYCLING:
               {
                  /**Copy the reg info*/
                  cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].reg_id =
                     hw_codec_info_req_store_global_ptr->reg_info_list[p_reg_counter].reg_id;
                  cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].reg_virt_addr_msw =
                     hw_codec_info_req_store_global_ptr->reg_info_list[p_reg_counter].reg_virt_addr_msw;
                  cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].reg_virt_addr_lsw =
                     hw_codec_info_req_store_global_ptr->reg_info_list[p_reg_counter].reg_virt_addr_lsw;
                  cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].num_cdc_ops =
                     hw_codec_info_req_store_global_ptr->reg_info_list[p_reg_counter].num_codec_op;

#ifdef CDC_CTRL_DEBUG_LOG
                  AR_MSG(DBG_LOW_PRIO, "Copied reg info");
                  AR_MSG(DBG_LOW_PRIO,
                         "reg_id = 0x%x virt_addr_lsw = 0x%x num_cdc_ops = 0x%x",
                         cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].reg_id,
                         cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter]
                            .reg_virt_addr_lsw,
                         cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].num_cdc_ops);
                  AR_MSG(DBG_LOW_PRIO,
                         "reg_id_A %p virt_addr_lsw_A %p",
                         &(cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].reg_id),
                         &(cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter]
                              .reg_virt_addr_lsw));
#endif

                  break;
               }
               default:
               {
                  AR_MSG(DBG_ERROR_PRIO, "Unsupported op_id 0x%lx", op_id);
                  return AR_EFAILED;
                  break;
               }
            }

            /**Stored codec op list for a particular register*/
            spf_list_node_t *cdc_op_list_head_ptr =
               hw_codec_info_req_store_global_ptr->reg_info_list[p_reg_counter].codec_op_list_ptr;
            /**NULL check for cdc_op_list_head_ptr*/
            if (NULL == cdc_op_list_head_ptr)
            {
               AR_MSG(DBG_ERROR_PRIO, "cdc_op_list_head_ptr which should contain the codec_op_list is NULL");
               return AR_EFAILED;
            }

            /**To get one op id and op value,
             * we define a ptr of node type and get the value one by one*/
            hw_codec_op_info_t *op_info_store_ptr;

            reg_op_counter    = 0;
            cdc_op_block_size = sizeof(cdc_reg_op_info_t) * num_cdc_ops_to_copy;

            cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].reg_op_list_ptr =
               (cdc_reg_op_info_t *)posal_memory_malloc(cdc_op_block_size, heap_id);
            if (NULL ==
                cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].reg_op_list_ptr)
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "Failed to allocate memory to store cdc_op info for reg_id 0x%x",
                      cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].reg_id);
               result = AR_ENOMEMORY;
               return result;
            }
            memset(cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter].reg_op_list_ptr,
                   0x00,
                   cdc_op_block_size);

            /**Iterate over the cmd handler node to copy all the op_id and op_value per register*/
            while (cdc_op_list_head_ptr && reg_op_counter < num_cdc_ops_to_copy)
            {
#ifdef CDC_CTRL_DEBUG_LOG
               AR_MSG(DBG_LOW_PRIO, "reg_op_counter = %d", reg_op_counter);
#endif

               op_info_store_ptr = (hw_codec_op_info_t *)cdc_op_list_head_ptr->obj_ptr;

               /**Check if op_id from cmd handler node matches op_id in lookup table*/
               if (codec_op_lookup_table[cdc_op_index].reg_info_list[lut_reg_counter].reg_op_list[reg_op_counter] !=
                   op_info_store_ptr->op_id)
               {
#ifdef CDC_CTRL_DEBUG_LOG
                  AR_MSG(DBG_LOW_PRIO,
                         "cmd_hdlr_op_id = 0x%x lut_op_id = 0x%x don't match",
                         op_info_store_ptr->op_id,
                         codec_op_lookup_table[cdc_op_index]
                            .reg_info_list[lut_reg_counter]
                            .reg_op_list[reg_op_counter]);
#endif

                  /**Iterate to next node*/
                  cdc_op_list_head_ptr = cdc_op_list_head_ptr->next_ptr;

                  continue;
               }

#ifdef CDC_CTRL_DEBUG_LOG
               AR_MSG(DBG_LOW_PRIO,
                      "cmd_hdlr_op_id = 0x%x lut_op_id = 0x%x match",
                      op_info_store_ptr->op_id,
                      codec_op_lookup_table[cdc_op_index].reg_info_list[lut_reg_counter].reg_op_list[reg_op_counter]);
#endif

               /**A switch case is required here as the data structure to be used to copy the register data will vary
                * per operation*/
               switch (op_id)
               {
                  case CDC_CTRL_UTIL_DUTY_CYCLING:
                  {
                     /**Copy the op_id and op_value*/
                     cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter]
                        .reg_op_list_ptr[reg_op_counter]
                        .op_id = op_info_store_ptr->op_id;
                     cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter]
                        .reg_op_list_ptr[reg_op_counter]
                        .op_value = op_info_store_ptr->op_value;

#ifdef CDC_CTRL_DEBUG_LOG
                     AR_MSG(DBG_LOW_PRIO,
                            "copied op_id 0x%x op_value 0x%x",
                            cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter]
                               .reg_op_list_ptr[reg_op_counter]
                               .op_id,
                            cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter]
                               .reg_op_list_ptr[reg_op_counter]
                               .op_value);
                     AR_MSG(DBG_LOW_PRIO,
                            "op_id_A 0x%p op_value_A %p",
                            &(cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter]
                                 .reg_op_list_ptr[reg_op_counter]
                                 .op_id),
                            &(cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[lut_reg_counter]
                                 .reg_op_list_ptr[reg_op_counter]
                                 .op_value));
#endif

                     break;
                  }
                  default:
                  {
                     AR_MSG(DBG_ERROR_PRIO, "Unsupported op_id 0x%lx", op_id);
                     return AR_EFAILED;
                     break;
                  }
               }

               /**Increment the reg_op_counter as next register op_id and value needs to be copied*/
               reg_op_counter++;

               /**Iterate to next node*/
               cdc_op_list_head_ptr = cdc_op_list_head_ptr->next_ptr;
            }
         }

         /**Break because we got the reg info for this register and now we need to increment outer loop to compare to
          * next register*/
         break;
      }

      cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.num_reg_ids++;

      /**Increment the counter to move to the next reg id*/
      lut_reg_counter++;
   }

   /**A switch case is required here as the data structure to be used to increment will vary per operation*/
   switch (op_id)
   {
      case CDC_CTRL_UTIL_DUTY_CYCLING:
      {
         /**Increment to show that a client registered for DC*/
         cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.num_dc_clients++;
         break;
      }
      default:
      {
         AR_MSG(DBG_ERROR_PRIO, "Unsupported op_id 0x%lx", op_id);
         break;
      }
   }

   return result;
}

#ifdef SIM
void cdc_ctrl_util_print_book()
{
   AR_MSG(DBG_LOW_PRIO, "Entered cdc_ctrl_util_print_book");

   uint32_t num_reg_ids = 0;
   uint32_t num_cdc_ops = 0;
   uint32_t reg_counter = 0;
   uint32_t op_counter  = 0;

   num_reg_ids = cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.num_reg_ids;

   AR_MSG(DBG_LOW_PRIO, "num_reg_ids = %d", num_reg_ids);

   for (reg_counter = 0; reg_counter < num_reg_ids; reg_counter++)
   {
      AR_MSG(DBG_LOW_PRIO, "reg_counter = %d", reg_counter);

      AR_MSG(DBG_LOW_PRIO,
             "reg_id = 0x%x v_add_l = 0x%x",
             cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_id,
             cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_virt_addr_lsw);

      AR_MSG(DBG_LOW_PRIO,
             "reg_id_A = %p v_add_l_A = %p",
             &(cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_id),
             &(cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_virt_addr_lsw));

      num_cdc_ops = cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].num_cdc_ops;
      AR_MSG(DBG_LOW_PRIO, "num_cdc_ops = %d", num_cdc_ops);

      for (op_counter = 0; op_counter < num_cdc_ops; op_counter++)
      {
         AR_MSG(DBG_LOW_PRIO, "op_counter = %d", op_counter);

         AR_MSG(DBG_LOW_PRIO,
                "op_id = 0x%x op_value = 0x%x",
                cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter]
                   .reg_op_list_ptr[op_counter]
                   .op_id,
                cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter]
                   .reg_op_list_ptr[op_counter]
                   .op_value);
         AR_MSG(DBG_LOW_PRIO,
                "op_id_A = %p op_value_A = %p",
                &(cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter]
                     .reg_op_list_ptr[op_counter]
                     .op_id),
                &(cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter]
                     .reg_op_list_ptr[op_counter]
                     .op_value));
      }
   }
   AR_MSG(DBG_LOW_PRIO, "Exiting cdc_ctrl_util_print_book");
}
#endif
