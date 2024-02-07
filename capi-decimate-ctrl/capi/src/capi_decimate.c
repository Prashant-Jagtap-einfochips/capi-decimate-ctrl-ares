/* ======================================================================== */
/**
   @file capi_decimate.cpp

   C source file to implement the Audio Post Processor Interface for
   DECIMATE example
 */

/* =========================================================================
   Copyright (c) 2015-2021 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef _DEBUG
#define _DEBUG
#endif
#include "capi_decimate.h"
#include "capi_decimate_utils.h"
#include "assert.h"
#include <hexagon_protos.h>
#include "capi_decimate_block.h"


#include "capi_cmn_imcl_utils.h"
#include "capi_intf_extn_imcl.h"
#include "capi_cmn_ctrl_port_list.h"
#include "imcl_decimate_api.h"

/*------------------------------------------------------------------------
 * Static declarations
 * -----------------------------------------------------------------------*/

static capi_err_t capi_decimate_process(capi_t *_pif, capi_stream_data_t *input[], capi_stream_data_t *output[]);

static capi_err_t capi_decimate_end(capi_t *_pif);

static capi_err_t capi_decimate_set_param(capi_t *                _pif,
                                          uint32_t                param_id,
                                          const capi_port_info_t *port_info_ptr,
                                          capi_buf_t *            params_ptr);

static capi_err_t capi_decimate_get_param(capi_t *                _pif,
                                          uint32_t                param_id,
                                          const capi_port_info_t *port_info_ptr,
                                          capi_buf_t *            params_ptr);

static capi_err_t capi_decimate_set_properties(capi_t *_pif, capi_proplist_t *props_ptr);

static capi_err_t capi_decimate_get_properties(capi_t *_pif, capi_proplist_t *props_ptr);

capi_err_t DecimateRx_imc_set_param_handler(capi_decimate_t *me_ptr, capi_buf_t *intent_buf_ptr); //handles the sampling frequency value received over imc

/** Virtual table for capi_decimate
 *  */
static capi_vtbl_t vtbl = { capi_decimate_process,        capi_decimate_end,
                            capi_decimate_set_param,      capi_decimate_get_param,
                            capi_decimate_set_properties, capi_decimate_get_properties };

/*------------------------------------------------------------------------
  Function name: capi_decimate_get_static_properties
  DESCRIPTION: Function to get the static properties of DECIMATE module
  -----------------------------------------------------------------------*/
capi_err_t capi_decimate_get_static_properties(capi_proplist_t *init_set_properties, capi_proplist_t *static_properties)
{
   capi_err_t capi_result = CAPI_EOK;

   if (NULL != static_properties)
   {
      capi_result = capi_decimate_process_get_properties((capi_decimate_t *)NULL, static_properties);
      if (CAPI_FAILED(capi_result))
      {
         return capi_result;
      }
   }

   if (NULL != init_set_properties)
   {
      AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: get static properties ignoring init_set_properties!");
   }

   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: capi_decimate_init
  DESCRIPTION: Initialize the CAPI Decimate module and library.
  This function can allocate memory.
  -----------------------------------------------------------------------*/
capi_err_t capi_decimate_init(capi_t *_pif, capi_proplist_t *init_set_properties)
{
   capi_err_t capi_result = CAPI_EOK;

   if (NULL == _pif || NULL == init_set_properties)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "CAPI DECIMATE: "
             "Init received bad pointer, 0x%lx, 0x%lx",
             (uint32_t)_pif,
             (uint32_t)init_set_properties);

      return CAPI_EBADPARAM;
   }

   int8_t *         ptr           = (int8_t *)_pif;
   capi_decimate_t *me_ptr        = (capi_decimate_t *)ptr;
   uint32_t         decimate_size = 0;

   // Each block must begin with 8-byte aligned memory.
   decimate_size = decimate_align_to_8_byte(sizeof(capi_decimate_t)) +
                   decimate_align_to_8_byte(TEMP_BUF_SIZE_IN_SAMPLES * sizeof(int16_t))    // For L-channel
                   + decimate_align_to_8_byte(TEMP_BUF_SIZE_IN_SAMPLES * sizeof(int16_t)); // For R-channel

   // Initialize the memory to 0
   memset((void *)me_ptr, 0, decimate_size);

   /* Shift the memory pointer by size of capi_decimate_t
      for other members of the structure.
    */
   ptr += decimate_align_to_8_byte(sizeof(capi_decimate_t));

   // Allocate memory to output samples after decimation
   me_ptr->out_samples = (uint32_t *)ptr;
   memset(me_ptr->out_samples, 0, sizeof(uint32_t));
   
   // Disable the module by default
   me_ptr->enable_flag                 = 0;
   me_ptr->lib_config.decimatest_frame = 1;

   capi_decimate_init_media_fmt(me_ptr);
   me_ptr->vtbl.vtbl_ptr = &vtbl;

   me_ptr->decimation_factor = 1;

   // should contain  EVENT_CALLBACK_INFO, PORT_INFO
   capi_result = capi_decimate_process_set_properties(me_ptr, init_set_properties);

   // Ignoring non-fatal error code.
   capi_result ^= (capi_result & CAPI_EUNSUPPORTED);
   if (CAPI_FAILED(capi_result))
   {
      return capi_result;
   }

   //Initialize the control port list
#if 1
   capi_cmn_ctrl_port_list_init(&me_ptr->ctrl_port_info);
#endif

   AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Init done!");
   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: capi_decimate_process
  DESCRIPTION: Processes an input buffer and generates an output buffer.
  -----------------------------------------------------------------------*/
static capi_err_t capi_decimate_process(capi_t *_pif, capi_stream_data_t *input[], capi_stream_data_t *output[])
{

   capi_err_t       capi_result = CAPI_EOK;
   capi_decimate_t *me_ptr      = (capi_decimate_t *)(_pif);
#if 0
   if (((me_ptr->threshold_in_bytes / me_ptr->input_media_fmt->format.num_channels) !=
        input[0]->buf_ptr->actual_data_len) &&
       (!input[0]->flags.end_of_frame))
   {
      // IF input bytes is less than threshold than we return with eneedmore. THis tells framework that we need more
      // data to process this frame.  But if EOS is set than framework wont be able to send more data, in that case we
      // must either consume the data or drop it.
      AR_MSG(DBG_HIGH_PRIO,
             "CAPI DECIMATE: Input bytes = %lu,input[0]->buf_ptr->actual_data_len = %lu, max_data_len = %lu, me_ptr->threshold_in_bytes = %lu",
             input[0]->buf_ptr->actual_data_len,input[1]->buf_ptr->actual_data_len,
             input[0]->buf_ptr->max_data_len, me_ptr->threshold_in_bytes);
      AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Input bytes less than threshold, returning without processing");
      return CAPI_ENEEDMORE;
   }

#endif
   uint32_t decimation_factor = me_ptr->decimation_factor;
   assert(me_ptr);
   assert(input);
   assert(output);
   AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Process call for Decimate module");
   // Data pointers and sizes
   // Sample count from one channel should be sufficient
   int16_t *lin_ptr            = NULL;
   int16_t *rin_ptr            = NULL;
   int16_t *lout_ptr           = NULL;
   int16_t *rout_ptr           = NULL;
   uint32_t offset_into_inbuf  = 0; // Offset for input buffer
   uint32_t offset_into_outbuf = 0; // Offset for output buffer
   /* The processing block size should be a minimum of
    *   1. input_actual_data_length/decimation_factor
    *   2. output_max_data_length
    *
    *   Also, the input_actual_data_length and ooutput_max_data_length
    *   is in bytes. Assuming bits per sample is 16 or 2 bytes per sample,
    *   num of samples are calculated by bitwise shifting by 1 byte to the right.
    * */
   uint32_t num_samples = Q6_R_min_RR((input[0]->buf_ptr->actual_data_len >> 1) / decimation_factor,
                                      (output[0]->buf_ptr->max_data_len >> 1));

   // Input and output buffers for L channel processing
   lin_ptr  = (int16_t *)input[0]->buf_ptr[0].data_ptr;
   lout_ptr = (int16_t *)output[0]->buf_ptr[0].data_ptr;

   // Input and output buffers for R channel processing
   if (2 == output[0]->bufs_num)
   {
      rin_ptr  = (int16_t *)input[0]->buf_ptr[1].data_ptr;
      rout_ptr = (int16_t *)output[0]->buf_ptr[1].data_ptr;
   }

   if (1 == me_ptr->enable_flag)
   {
      // Primary processing
      uint32_t num_samples_for_this_iteration = 0;
      uint32_t rem_num_samples                = decimation_factor * num_samples;

      do
      {
         num_samples_for_this_iteration = Q6_R_min_RR(rem_num_samples, TEMP_BUF_SIZE_IN_SAMPLES);

         decimate_block(lin_ptr + offset_into_inbuf,
                        num_samples_for_this_iteration,
                        me_ptr->out_samples,
                        decimation_factor,
                        lout_ptr + offset_into_outbuf);

         if (2 == output[0]->bufs_num)
         {
            decimate_block(rin_ptr + offset_into_inbuf,
                           num_samples_for_this_iteration,
                           me_ptr->out_samples,
                           decimation_factor,
                           rout_ptr + offset_into_outbuf);
         }

         // Check if any more input data is left to be processed
         // Update Offset into Input buffer, so we begin reading from that
         // point in the next iteration (if any).
         offset_into_inbuf += num_samples_for_this_iteration;
         offset_into_outbuf += *me_ptr->out_samples;
         rem_num_samples -= num_samples_for_this_iteration;
      } while (rem_num_samples);

      // Update actual data length for output buffer
      input[0]->buf_ptr[0].actual_data_len  = ((decimation_factor * num_samples) << 1);
      output[0]->buf_ptr[0].actual_data_len = offset_into_outbuf << 1;
      if (2 == output[0]->bufs_num)
      {
         output[0]->buf_ptr[1].actual_data_len = offset_into_outbuf << 1;
         input[0]->buf_ptr[1].actual_data_len  = ((decimation_factor * num_samples) << 1);
      }
   }
   else /* This case will never be hit because if enable_flag is not 1, module will
         * raise an event to service to disable the module from PP chain.
         * It's a dummy code for copying data from input to output.
         */
   {
      memcpy(output[0]->buf_ptr[0].data_ptr, input[0]->buf_ptr[0].data_ptr, input[0]->buf_ptr->actual_data_len);
      if (2 == output[0]->bufs_num)
      {
         memcpy(output[0]->buf_ptr[1].data_ptr, input[0]->buf_ptr[1].data_ptr, input[0]->buf_ptr->actual_data_len);
      }
      // Update actual data length for output buffer
      output[0]->buf_ptr[0].actual_data_len = input[0]->buf_ptr->actual_data_len;
      if (2 == output[0]->bufs_num)
      {
         output[0]->buf_ptr[1].actual_data_len = input[0]->buf_ptr->actual_data_len;
      }
   }

   // Copy flags from input to output
   output[0]->flags = input[0]->flags;

   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: capi_decimate_end
  DESCRIPTION: Returns the library to the uninitialized state and frees the
  memory that was allocated by init(). This function also frees the virtual
  function table.
  -----------------------------------------------------------------------*/
static capi_err_t capi_decimate_end(capi_t *_pif)
{
   capi_err_t capi_result = CAPI_EOK;
   if (NULL == _pif)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: End received bad pointer, 0x%lx", (uint32_t)_pif);
      return CAPI_EBADPARAM;
   }

   capi_decimate_t *me_ptr = (capi_decimate_t *)(_pif);

   capi_decimate_release_memory(me_ptr);

   me_ptr->vtbl.vtbl_ptr = NULL;

   AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: End done");
   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: capi_decimate_set_param
  DESCRIPTION: Sets either a parameter value or a parameter structure containing
  multiple parameters. In the event of a failure, the appropriate error code is
  returned.
  -----------------------------------------------------------------------*/
static capi_err_t capi_decimate_set_param(capi_t *                _pif,
                                          uint32_t                param_id,
                                          const capi_port_info_t *port_info_ptr,
                                          capi_buf_t *            params_ptr)

{
   if (NULL == _pif || NULL == params_ptr)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "CAPI DECIMATE: Set param received bad pointer, 0x%lx, 0x%lx",
             (uint32_t)_pif,
             (uint32_t)params_ptr);
      return CAPI_EBADPARAM;
   }

   capi_err_t       capi_result = CAPI_EOK;
   capi_decimate_t *me_ptr      = (capi_decimate_t *)(_pif);

   switch (param_id)
   {

      case PARAM_ID_MODULE_ENABLE:
      {
         if (params_ptr->actual_data_len >= sizeof(param_id_module_enable_t))
         {
            param_id_module_enable_t *cfg_ptr = (param_id_module_enable_t *)(params_ptr->data_ptr);
            // set DECIMATE enable flag
            me_ptr->enable_flag = cfg_ptr->enable;

            /* After enabling/disabling the module, use callback functionality
             * to update the caller service about the process state of module.
             */
            capi_decimate_raise_event(me_ptr);
            AR_MSG(DBG_HIGH_PRIO, "CAPI Decimate : <<set_param>> Enable/Disable %lu ", me_ptr->enable_flag);
         }
         else
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI Decimate : <<set_param>> Bad param size %lu  ", params_ptr->actual_data_len);
            return CAPI_ENEEDMORE;
         }
         break;
      }

      case CAPI_PARAM_ID_UPDATE_DECIMATION_FACTOR:
      {
         if (params_ptr->actual_data_len >= sizeof(decimate_factor_cfg_t))
         {
            decimate_factor_cfg_t *cfg_ptr = (decimate_factor_cfg_t *)(params_ptr->data_ptr);

            if (1 <= cfg_ptr->decimation_factor)
            {
               if (cfg_ptr->decimation_factor != me_ptr->decimation_factor)
               {
                  me_ptr->decimation_factor = cfg_ptr->decimation_factor;
                  AR_MSG(DBG_HIGH_PRIO,
                         "CAPI DECIMATE: <<set_param>> Decimation factor set to %lu",
                         me_ptr->decimation_factor);
                  if (me_ptr->is_mf_received)
                  {
                     // we need to raise output media format because it depends on decimation factor as well, and we
                     // need to update output media format everytime we get a new deciamtion factor
                     capi_decimate_raise_output_media_format_event(me_ptr);
                     capi_decimate_raise_event(me_ptr);
                  }
               }
               else
               {
                  AR_MSG(DBG_HIGH_PRIO,
                         "CAPI DECIMATE: Same decimation factor received.Decimation factor = %lu",
                         me_ptr->decimation_factor);
                  return CAPI_EOK;
               }
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: <<set_param>> "
                      "Decimation factor should be greater than 1 ");
               return CAPI_EUNSUPPORTED;
            }
         }
         else
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: <<set_param>> Bad param size %lu", params_ptr->actual_data_len);
            return CAPI_ENEEDMORE;
         }
         break;
      }
      
      case CAPI_PARAM_ID_UPDATE_DECIMATION_COEFFS:
      {
         if (params_ptr->actual_data_len >= sizeof(control_tx_coeff_arr_t))
         {
            control_tx_coeff_arr_t *cfg_ptr = (control_tx_coeff_arr_t *)(params_ptr->data_ptr);
            memcpy(me_ptr->coeff_val, cfg_ptr->coeff_val, sizeof(cfg_ptr->coeff_val));   
         }
         else
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: <<set_param>> Bad param size %lu", params_ptr->actual_data_len);
            return CAPI_ENEEDMORE;
         }
         break;
      }
      case INTF_EXTN_PARAM_ID_IMCL_PORT_OPERATION:
      {
	 AR_MSG(DBG_ERROR_PRIO,"IMC port operation handler here\n");

         uint32_t supported_intent[1] = {INTENT_ID_DECIMATE_CONTROL};
         capi_result = capi_cmn_ctrl_port_operation_handler(
            &me_ptr->ctrl_port_info, params_ptr, 
            (POSAL_HEAP_ID)me_ptr->heap_info.heap_id, 0, 1, supported_intent);
         break;
      }
      case INTF_EXTN_PARAM_ID_IMCL_INCOMING_DATA:
      {
          #ifdef US_GEN_DEBUG_LOG
	   AR_MSG(DBG_LOW_PRIO,"Inside INTF_EXTN_PARAM_ID_IMCL_INCOMING_DATA.");
	   #endif
	    //this function will receive the sampling frequency sent over imc by us detect
	    capi_result = DecimateRx_imc_set_param_handler(me_ptr, params_ptr);
            if (CAPI_EOK != capi_result)
	    {
		AR_MSG(DBG_ERROR_PRIO,"IMC set param handler failed 0x%x\n",param_id);
	    }
	    break;
	}
      default:
         AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Set unsupported param ID 0x%x", (int)param_id);
         CAPI_SET_ERROR(capi_result, CAPI_EBADPARAM);
   }

   AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Set param done");
   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: capi_decimate_get_param
  DESCRIPTION: Gets either a parameter value or a parameter structure
  containing multiple parameters. In the event of a failure, the appropriate
  error code is returned.
 * -----------------------------------------------------------------------*/
static capi_err_t capi_decimate_get_param(capi_t *                _pif,
                                          uint32_t                param_id,
                                          const capi_port_info_t *port_info_ptr,
                                          capi_buf_t *            params_ptr)
{
   if (NULL == _pif || NULL == params_ptr)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "CAPI DECIMATE: Get param received bad pointer, 0x%lx, 0x%lx",
             (uint32_t)_pif,
             (uint32_t)params_ptr);
      return CAPI_EBADPARAM;
   }

   capi_err_t       capi_result = CAPI_EOK;
   capi_decimate_t *me_ptr      = (capi_decimate_t *)(_pif);
   // void *param_payload_ptr  = (void *)(params_ptr->data_ptr);

   switch (param_id)
   {

      case PARAM_ID_MODULE_ENABLE:
      {
         if (params_ptr->max_data_len >= sizeof(param_id_module_enable_t))
         {
            param_id_module_enable_t *cfg_ptr = (param_id_module_enable_t *)(params_ptr->data_ptr);
            cfg_ptr->enable                   = me_ptr->enable_flag;
            params_ptr->actual_data_len       = sizeof(param_id_module_enable_t);
            AR_MSG(DBG_HIGH_PRIO,
                   "CAPI DECIMATE: <<get_param>> Enable/Disable %lu                                    ",
                   cfg_ptr->enable);
         }
         else
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI DECIMATE: <<get_param>> Bad param size %lu                                    ",
                   params_ptr->max_data_len);
            return CAPI_ENEEDMORE;
         }
         break;
      }

      case CAPI_PARAM_ID_UPDATE_DECIMATION_FACTOR:
      {
         if (params_ptr->max_data_len >= sizeof(decimate_factor_cfg_t))
         {
            decimate_factor_cfg_t *cfg_ptr = (decimate_factor_cfg_t *)(params_ptr->data_ptr);
            cfg_ptr->decimation_factor     = me_ptr->decimation_factor;
            params_ptr->actual_data_len    = sizeof(decimate_factor_cfg_t);
            AR_MSG(DBG_HIGH_PRIO,
                   "CAPI DECIMATE: <<get_param>> Decimation factor = %lu                                              ",
                   cfg_ptr->decimation_factor);
         }
         else
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI DECIMATE: <<get_param>> Bad param size %lu  Param id = %lu",
                   params_ptr->max_data_len,
                   param_id);
            return CAPI_ENEEDMORE;
         }
         break;
      }

      case CAPI_PARAM_ID_UPDATE_DECIMATION_COEFFS:
      {
         if (params_ptr->max_data_len >= sizeof(control_tx_coeff_arr_t))
         {
            control_tx_coeff_arr_t *cfg_ptr = (control_tx_coeff_arr_t *)(params_ptr->data_ptr);
            memcpy(cfg_ptr->coeff_val, me_ptr->coeff_val, sizeof(me_ptr->coeff_val));
            params_ptr->actual_data_len    = sizeof(control_tx_coeff_arr_t);
         }
         else
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI DECIMATE: <<get_param>> Bad param size %lu  Param id = %lu",
                   params_ptr->max_data_len,
                   param_id);
            return CAPI_ENEEDMORE;
         }
         break;
      }

      default:
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Get unsupported param ID 0x%x", (int)param_id);
         CAPI_SET_ERROR(capi_result, CAPI_EBADPARAM);
      }
   }

   AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Get param done");
   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: capi_decimate_set_properties
  DESCRIPTION: Function to set the properties for the Decimate module
 * -----------------------------------------------------------------------*/
static capi_err_t capi_decimate_set_properties(capi_t *_pif, capi_proplist_t *props_ptr)
{
   capi_err_t capi_result = CAPI_EOK;
   if (NULL == _pif)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Set properties received bad pointer, 0x%lx", (uint32_t)_pif);
      return CAPI_EBADPARAM;
   }
   capi_decimate_t *me_ptr = (capi_decimate_t *)_pif;
   capi_result             = capi_decimate_process_set_properties(me_ptr, props_ptr);
   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: capi_decimate_get_properties
  DESCRIPTION: Function to get the properties for the Decimate module
 * -----------------------------------------------------------------------*/
static capi_err_t capi_decimate_get_properties(capi_t *_pif, capi_proplist_t *props_ptr)
{
   capi_err_t capi_result = CAPI_EOK;
   if (NULL == _pif)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Get properties received bad pointer, 0x%lx", (uint32_t)_pif);
      return CAPI_EBADPARAM;
   }
   capi_decimate_t *me_ptr = (capi_decimate_t *)_pif;
   capi_result             = capi_decimate_process_get_properties(me_ptr, props_ptr);
   return capi_result;
}

/*===========================================================================
  Function    : capi_us_gen_imc_set_param_handler
  DESCRIPTION : Function to handle received payload over control link
===========================================================================*/

capi_err_t DecimateRx_imc_set_param_handler (capi_decimate_t * me_ptr, capi_buf_t * intent_buf_ptr)
{
   capi_err_t result = CAPI_EOK;
   if (NULL == intent_buf_ptr->data_ptr)
   {
      AR_MSG(DBG_ERROR_PRIO,"IMC set param handler received null buffer");
      result |= CAPI_EBADPARAM;
      return result;
   }

   // Level 1 check
   if(intent_buf_ptr->actual_data_len < MIN_INCOMING_IMCL_PARAM_SIZE_DECIMATE)
   {
      AR_MSG(DBG_ERROR_PRIO,"Invalid payload size for incoming data %d", intent_buf_ptr->actual_data_len);
      return CAPI_ENEEDMORE;
   }

   /* accessing the wrong payload. need to do + sizeof(incoming payload struct to access the actual data) */
   int8_t *payload_ptr = intent_buf_ptr->data_ptr + sizeof(intf_extn_param_id_imcl_incoming_data_t);
   uint32_t payload_size = intent_buf_ptr->actual_data_len - sizeof(intf_extn_param_id_imcl_incoming_data_t);

   while (payload_size >= sizeof(decimate_imcl_header_t))
   {
      decimate_imcl_header_t *header_ptr = (decimate_imcl_header_t*)payload_ptr;
      payload_ptr += sizeof(decimate_imcl_header_t);
      payload_size -= sizeof(decimate_imcl_header_t);

      switch(header_ptr->opcode)
      {
         // param id defined in the imcl_sp_api.
         case PARAM_ID_DECIMATE_CONTROL_IMC_PAYLOAD:
         {
            if (header_ptr->actual_data_len < sizeof(capi_decimate_control_data_payload_t))
            {
               AR_MSG(DBG_ERROR_PRIO,"IMC Param id 0x%lx Invalid payload size for incoming data %d",header_ptr->opcode, header_ptr->actual_data_len);
               return CAPI_ENEEDMORE;
            }

            capi_decimate_control_data_payload_t *cfg_ptr = (capi_decimate_control_data_payload_t *) payload_ptr;

            // Set decimation factor on dest side and
            // raise media event to update decimation factor
            if (cfg_ptr->decimation_factor >= 1)
            {
               if (cfg_ptr->decimation_factor != me_ptr->decimation_factor)
               {
                  me_ptr->decimation_factor = cfg_ptr->decimation_factor;
                  AR_MSG(DBG_HIGH_PRIO,
                         "CAPI DECIMATE: <<set_param>> Decimation factor set to %lu",
                         me_ptr->decimation_factor);

                  if (me_ptr->is_mf_received)
                  {
                     // we need to raise output media format because it depends on decimation factor as well, and we
                     // need to update output media format everytime we get a new deciamtion factor
                     capi_decimate_raise_output_media_format_event(me_ptr);
                     capi_decimate_raise_event(me_ptr);
                  }
               }
               else
               {
                  AR_MSG(DBG_HIGH_PRIO,
                         "CAPI DECIMATE: Same decimation factor received.Decimation factor = %lu",
                         me_ptr->decimation_factor);
                  return CAPI_EOK;
               }
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: <<set_param>> "
                      "Decimation factor should be greater than 1 ");
               return CAPI_EUNSUPPORTED;
            }
            break;
         }

         case PARAM_ID_DECIMATE_COEFF_IMC_PAYLOAD:
         {
            if (header_ptr->actual_data_len < sizeof(capi_decimate_coeff_arr_payload_t))
            {
               AR_MSG(DBG_ERROR_PRIO,"IMC Param id 0x%lx Invalid payload size for incoming data %d",header_ptr->opcode, header_ptr->actual_data_len);
               return CAPI_ENEEDMORE;
            }

            capi_decimate_coeff_arr_payload_t *cfg_ptr = (capi_decimate_coeff_arr_payload_t *) payload_ptr;
            memcpy(me_ptr->coeff_val, cfg_ptr->coeff_val, sizeof(cfg_ptr->coeff_val));

            break;
         }

         default:
         {
            AR_MSG(DBG_ERROR_PRIO,"Unsupported opcode for incoming data over IMCL %d", header_ptr->opcode);
            return CAPI_EUNSUPPORTED;
         }

         AR_MSG(DBG_HIGH_PRIO,"IMC Set param 0x%x done. payload size = %lu", header_ptr->opcode, header_ptr->actual_data_len);
      }

      payload_ptr += header_ptr->actual_data_len;
      payload_size -= header_ptr->actual_data_len;
   }
   return result;
}
