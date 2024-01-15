/*========================================================================
   This file contains audio codec control island APIs.

  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header:
 //components/dev/avs.fwk/1.0/smanglam.avs.fwk.1.0.16july21_lahaina/platform/core_drv/prm_drv/cdc_ctrl_util/src/cdc_ctrl_util_island.c#1
 $
 ====================================================================== */
/*==========================================================================
 Include files
========================================================================== */
#include "cdc_ctrl_util.h"
#include "cdc_ctrl_util_i.h"

//#define CDC_CTRL_DEBUG_LOG_ISLAND 1

#define OP_IDX_ENABLE_DC 0

#define OP_IDX_DISABLE_DC 1

#define CDC_UTIL_READ_FROM_REGISTER(register_address) (*(volatile uint32_t *)register_address)
#define CDC_UTIL_WRITE_TO_REGISTER(register_address, value) ((*(volatile uint32_t *)register_address) = (value))

/*==========================================================================
  Globals
========================================================================== */
/**Global structure housing all operation structures*/
cdc_ctrl_reg_op_t cdc_ctrl_reg_op_global;

/**Pointer setup to the above info for easier access*/
cdc_ctrl_reg_op_t *cdc_ctrl_reg_op_global_ptr = &cdc_ctrl_reg_op_global;
/*==========================================================================
  Function Implementations
========================================================================== */
ar_result_t cdc_ctrl_util_op_cfg(void *handle, cdc_ctrl_util_client_op_info_t *incoming_client_op_info_ptr)
{
   ar_result_t result = AR_EOK;

   if (NULL == handle)
   {
      AR_MSG_ISLAND(DBG_ERROR_PRIO, "Handle is NULL");
      return AR_EFAILED;
   }

   // cdc_ctrl_client_info_t *client_info_ptr = (cdc_ctrl_client_info_t *)handle;
   cdc_ctrl_util_client_op_info_t *client_op_info_ptr = (cdc_ctrl_util_client_op_info_t *)incoming_client_op_info_ptr;

   switch (client_op_info_ptr->op_id)
   {
      case CDC_CTRL_UTIL_DUTY_CYCLING:
      {
         if (client_op_info_ptr->payload_size > sizeof(cdc_util_op_state_info_t))
         {
            AR_MSG_ISLAND(DBG_ERROR_PRIO, "Invalid size of payload %d", client_op_info_ptr->payload_size);
            return AR_EFAILED;
         }

         result = cdc_ctrl_perform_duty_cycling(client_op_info_ptr);
         if (AR_EOK != result)
         {
            AR_MSG_ISLAND(DBG_ERROR_PRIO, "Failed to enable duty cycling with result %d", result);
         }
         break;
      }
      default:
      {
         AR_MSG_ISLAND(DBG_ERROR_PRIO, "Unsupported op_id 0x%lx", client_op_info_ptr->op_id);
         break;
      }
   }
   return result;
}

/**Assumptions made
 * In the storage structure i.e. cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info,
 * it is assumed that registers and their operations are stored in the following order:-
 *
 * reg_info_list[0].reg_id = HW_CODEC_DIG_REG_ID_MUTE_CTRL
 * reg_info_list[0].reg_op_list[0].op_id = HW_CODEC_OP_DIG_MUTE_ENABLE
 * reg_info_list[0].reg_op_list[1].op_id = HW_CODEC_OP_DIG_MUTE_DISABLE
 *
 * reg_info_list[1].reg_id = HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE
 * reg_info_list[1].reg_op_list[0].op_id = HW_CODEC_OP_ANA_PGA_DISABLE
 * reg_info_list[1].reg_op_list[1].op_id = HW_CODEC_OP_ANA_PGA_ENABLE
 *
 * If this order is maintained, then and only then will the below indexing approach work
 */
ar_result_t cdc_ctrl_perform_duty_cycling(cdc_ctrl_util_client_op_info_t *client_op_info_ptr)
{
   ar_result_t result                  = AR_EOK;
   uint32_t    num_reg_ids             = 0;
   uint32_t    reg_virt_addr_lsw       = 0;
   uint32_t    reg_value_to_be_written = 0;
   int32_t     reg_counter             = 0;

   /**Check if there are reg IDs in the structure*/
   num_reg_ids = cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.num_reg_ids;
   if (!num_reg_ids)
   {
      AR_MSG_ISLAND(DBG_ERROR_PRIO, "num_reg_ids = %d", num_reg_ids);
      result = AR_EFAILED;
      return result;
   }

#ifdef CDC_CTRL_DEBUG_LOG_ISLAND
   AR_MSG_ISLAND(DBG_LOW_PRIO, "num_reg_ids = %d", num_reg_ids);
#endif

   /**Get the payload*/
   cdc_util_op_state_info_t *cdc_op_state_info_ptr = (cdc_util_op_state_info_t *)client_op_info_ptr->payload_start[0];

   switch (cdc_op_state_info_ptr->op_state)
   {
      case CDC_OP_ENABLE:
      {
         reg_counter = 0;

         while (reg_counter < num_reg_ids)
         {
        reg_virt_addr_lsw = cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_virt_addr_lsw;

        /**HW_CODEC_OP_DIG_MUTE_ENABLE and HW_CODEC_OP_ANA_PGA_DISABLE will be stored at 0th index*/
        reg_value_to_be_written = cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_op_list_ptr[OP_IDX_ENABLE_DC].op_value;

        /**Write value to the register*/
        #ifdef SIM
          AR_MSG_ISLAND(DBG_LOW_PRIO,"Writing value 0x%x to reg addr 0x%x reg_counter = %d", reg_value_to_be_written, reg_virt_addr_lsw, reg_counter);
        #else
          AR_MSG_ISLAND(DBG_LOW_PRIO,"Writing value 0x%x to reg addr 0x%x reg_counter = %d", reg_value_to_be_written, reg_virt_addr_lsw, reg_counter);
          CDC_UTIL_WRITE_TO_REGISTER(reg_virt_addr_lsw, reg_value_to_be_written);
        #endif

        #ifdef CDC_CTRL_DEBUG_LOG_ISLAND
          AR_MSG_ISLAND(DBG_LOW_PRIO,"reg_id = 0x%x reg_virt_addr = ox%x reg_value_to_write = 0x%x cdc_op_id = 0x%x op_id = 0x%x op_state = 0x%x", 
            cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_id,
            reg_virt_addr_lsw, reg_value_to_be_written, cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_op_list_ptr[OP_IDX_ENABLE_DC].op_id,
            client_op_info_ptr->op_id, cdc_op_state_info_ptr->op_state);
          #ifdef SIM
            AR_MSG_ISLAND(DBG_LOW_PRIO,"Reading reg value from address 0x%x",reg_virt_addr_lsw);
          #else
            AR_MSG_ISLAND(DBG_LOW_PRIO," reg_value_read = 0x%lx", CDC_UTIL_READ_FROM_REGISTER(reg_virt_addr_lsw));
          #endif
        #endif

        reg_counter++;
      }
      break;
    }
    case CDC_OP_DISABLE:
    {
      reg_counter = num_reg_ids - 1;

      while (reg_counter >= 0)
      {
        reg_virt_addr_lsw = cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_virt_addr_lsw;

        /**HW_CODEC_OP_ANA_PGA_ENABLE and HW_CODEC_OP_DIG_MUTE_DISABLE will be stored at 1st index*/
        reg_value_to_be_written = cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_op_list_ptr[OP_IDX_DISABLE_DC].op_value;

        /**Write value to the register*/
        #ifdef SIM
          AR_MSG_ISLAND(DBG_LOW_PRIO,"Writing value 0x%x to reg addr 0x%x reg_counter = %d", reg_value_to_be_written, reg_virt_addr_lsw, reg_counter);
        #else
          AR_MSG_ISLAND(DBG_LOW_PRIO,"Writing value 0x%x to reg addr 0x%x reg_counter = %d", reg_value_to_be_written, reg_virt_addr_lsw, reg_counter);
          CDC_UTIL_WRITE_TO_REGISTER(reg_virt_addr_lsw, reg_value_to_be_written);
        #endif

        #ifdef CDC_CTRL_DEBUG_LOG_ISLAND
          AR_MSG_ISLAND(DBG_LOW_PRIO,"reg_id = 0x%x reg_virt_addr = ox%x reg_value_to_write = 0x%x cdc_op_id = 0x%x op_id = 0x%x op_state = 0x%x", 
            cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_id,
            reg_virt_addr_lsw, reg_value_to_be_written, cdc_ctrl_reg_op_global_ptr->duty_cycling_reg_info.reg_info_list[reg_counter].reg_op_list_ptr[OP_IDX_DISABLE_DC].op_id,
            client_op_info_ptr->op_id, cdc_op_state_info_ptr->op_state);
          #ifdef SIM
            AR_MSG_ISLAND(DBG_LOW_PRIO,"Reading reg value from address 0x%x",reg_virt_addr_lsw);
          #else
            AR_MSG_ISLAND(DBG_LOW_PRIO," reg_value_read = 0x%lx", CDC_UTIL_READ_FROM_REGISTER(reg_virt_addr_lsw));
          #endif
        #endif

        reg_counter--;
      }
      break;
    }
    default:
    {
      AR_MSG_ISLAND(DBG_ERROR_PRIO,"Unsupported op_state %d", cdc_op_state_info_ptr->op_state);
         break;
      }
   }

   return result;
}
