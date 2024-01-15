/* ======================================================================== */
/**
   @file control_tx.cpp

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
#include "control_tx.h"
#include "control_tx_utils.h"
#include "assert.h"
#include <hexagon_protos.h>

//IMCL
 #include "capi_cmn_imcl_utils.h"
 #include "capi_intf_extn_imcl.h"
 #include "capi_cmn_ctrl_port_list.h"
 #include "capi_cmn.h"
 #include "imcl_decimate_api.h"



	
/*------------------------------------------------------------------------
 * Static declarations
 * -----------------------------------------------------------------------*/

static capi_err_t control_tx_process(capi_t *_pif, capi_stream_data_t *input[], capi_stream_data_t *output[]);

static capi_err_t control_tx_end(capi_t *_pif);

static capi_err_t control_tx_set_param(capi_t *                _pif,
                                          uint32_t                param_id,
                                          const capi_port_info_t *port_info_ptr,
                                          capi_buf_t *            params_ptr);

static capi_err_t control_tx_get_param(capi_t *                _pif,
                                          uint32_t                param_id,
                                          const capi_port_info_t *port_info_ptr,
                                          capi_buf_t *            params_ptr);

static capi_err_t control_tx_set_properties(capi_t *_pif, capi_proplist_t *props_ptr);

static capi_err_t control_tx_get_properties(capi_t *_pif, capi_proplist_t *props_ptr);

/** Virtual table for control_tx
 *  */
static capi_vtbl_t vtbl = { control_tx_process,        control_tx_end,
                            control_tx_set_param,      control_tx_get_param,
                            control_tx_set_properties, control_tx_get_properties };

#if 0                           
static capi_err_t decimate_send_data_over_imc(control_tx_t * me_ptr,  uint32 data_over_imc);                           
                            

static capi_err_t decimate_send_data_over_imc(control_tx_t * me_ptr,  uint32 data_over_imc)
{
	capi_err_t result = CAPI_EOK;
	capi_buf_t buf;
	uint32_t control_port_id = 0;
	imcl_port_state_t port_state = CTRL_PORT_CLOSE;
	ctrl_port_data_t *port_data_ptr = NULL;
	buf.actual_data_len = sizeof(decimate_imcl_header_t) + sizeof(capi_decimate_control_data_payload_t);
	buf.data_ptr = NULL;
	buf.max_data_len = 0;
	imcl_outgoing_data_flag_t flags;
	flags.should_send = TRUE;
	flags.is_trigger = TRUE;
 
	// Get the first control port id for the intentMODULE_INTENT_ID_FOR_IMCL
	capi_cmn_ctrl_port_list_get_next_port_data(&me_ptr->ctrl_port_info, INTENT_ID_DECIMATE_CONTROL,  control_port_id, &me_ptr->ctrl_port_ptr);
	if (port_data_ptr)
	{
		control_port_id = port_data_ptr->port_info.port_id;
		port_state = port_data_ptr->state;
	}
	else
	{
		AR_MSG(DBG_ERROR_PRIO,"Port data ptr doesnt exist");
	}
	if (CTRL_PORT_PEER_CONNECTED == port_state)
	{
		// Get one time buf from the queue
		result |= capi_cmn_imcl_get_one_time_buf(&me_ptr->cb_info, control_port_id, buf.actual_data_len, &buf);
		if (CAPI_FAILED(result) || NULL == buf.data_ptr)
		{
			AR_MSG(DBG_ERROR_PRIO,"Getting one time buffer failed");
			return result;
		}
		decimate_imcl_header_t *out_cfg_ptr = (decimate_imcl_header_t *)buf.data_ptr;
		capi_decimate_control_data_payload_t *data_over_imc_payload = (capi_decimate_control_data_payload_t*)(out_cfg_ptr + 1);
		out_cfg_ptr->opcode = PARAM_ID_DECIMATE_CONTROL_IMC_PAYLOAD;
		out_cfg_ptr->actual_data_len = sizeof(decimate_imcl_header_t);
		data_over_imc_payload->decimation_factor = data_over_imc;
		// send data to peer/destination module
		if (CAPI_SUCCEEDED(capi_cmn_imcl_send_to_peer(&me_ptr->cb_info,  &buf, control_port_id, flags)))
		{
			AR_MSG(DBG_HIGH_PRIO,"Data %d sent to control port 0x%x",data_over_imc_payload->decimation_factor, control_port_id);
		}
	}
	else
	{
		AR_MSG(DBG_ERROR_PRIO,"Control port is not connected");
	}
	return result;
}
#endif
/*------------------------------------------------------------------------
  Function name: control_tx_get_static_properties
  DESCRIPTION: Function to get the static properties of DECIMATE module
  -----------------------------------------------------------------------*/
capi_err_t control_tx_get_static_properties(capi_proplist_t *init_set_properties, capi_proplist_t *static_properties)
{
   capi_err_t capi_result = CAPI_EOK;

   if (NULL != static_properties)
   {
      capi_result = control_tx_process_get_properties((control_tx_t *)NULL, static_properties);
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
  Function name: control_tx_init
  DESCRIPTION: Initialize the CAPI Decimate module and library.
  This function can allocate memory.
  -----------------------------------------------------------------------*/
capi_err_t control_tx_init(capi_t *_pif, capi_proplist_t *init_set_properties)
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
   control_tx_t *me_ptr        = (control_tx_t *)ptr;
   uint32_t         decimate_size = 0;

   // Each block must begin with 8-byte aligned memory.
   decimate_size = decimate_align_to_8_byte(sizeof(control_tx_t)) +
                   decimate_align_to_8_byte(TEMP_BUF_SIZE_IN_SAMPLES * sizeof(int16_t))    // For L-channel
                   + decimate_align_to_8_byte(TEMP_BUF_SIZE_IN_SAMPLES * sizeof(int16_t)); // For R-channel

   // Initialize the memory to 0
   memset((void *)me_ptr, 0, decimate_size);

   /* Shift the memory pointer by size of control_tx_t
      for other members of the structure.
    */
   ptr += decimate_align_to_8_byte(sizeof(control_tx_t));

   // Allocate memory to output samples after decimation
   me_ptr->out_samples = (uint32_t *)ptr;
   memset(me_ptr->out_samples, 0, sizeof(uint32_t));

   // Disable the module by default
   me_ptr->enable_flag                 = 0;
   me_ptr->lib_config.decimatest_frame = 1;

   control_tx_init_media_fmt(me_ptr);
   me_ptr->vtbl.vtbl_ptr = &vtbl;

   me_ptr->decimation_factor = 1;


   //IMCL
   me_ptr->is_ctrl_port_received = DECIMATE_CTRL_PORT_INFO_NOT_RCVD;
   //Initialize the control port list
   //capi_cmn_ctrl_port_list_init(&me_ptr->ctrl_port_info);
	
   // should contain  EVENT_CALLBACK_INFO, PORT_INFO
   capi_result = control_tx_process_set_properties(me_ptr, init_set_properties);

   // Ignoring non-fatal error code.
   capi_result ^= (capi_result & CAPI_EUNSUPPORTED);
   if (CAPI_FAILED(capi_result))
   {
      return capi_result;
   }

   AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Init done!");
   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: control_tx_process
  DESCRIPTION: Processes an input buffer and generates an output buffer.
  -----------------------------------------------------------------------*/
static capi_err_t control_tx_process(capi_t *_pif, capi_stream_data_t *input[], capi_stream_data_t *output[])
{

   capi_err_t       capi_result = CAPI_EOK;
#if 0
   control_tx_t *me_ptr      = (control_tx_t *)(_pif);
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
#endif

   // Copy flags from input to output
   output[0]->flags = input[0]->flags;

   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: control_tx_end
  DESCRIPTION: Returns the library to the uninitialized state and frees the
  memory that was allocated by init(). This function also frees the virtual
  function table.
  -----------------------------------------------------------------------*/
static capi_err_t control_tx_end(capi_t *_pif)
{
   capi_err_t capi_result = CAPI_EOK;
   if (NULL == _pif)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: End received bad pointer, 0x%lx", (uint32_t)_pif);
      return CAPI_EBADPARAM;
   }

   control_tx_t *me_ptr = (control_tx_t *)(_pif);

   control_tx_release_memory(me_ptr);

   me_ptr->vtbl.vtbl_ptr = NULL;

   AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: End done");
   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: control_tx_set_param
  DESCRIPTION: Sets either a parameter value or a parameter structure containing
  multiple parameters. In the event of a failure, the appropriate error code is
  returned.
  -----------------------------------------------------------------------*/
static capi_err_t control_tx_set_param(capi_t *                _pif,
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
   control_tx_t *me_ptr      = (control_tx_t *)(_pif);

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
            control_tx_raise_event(me_ptr);
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
                     control_tx_raise_output_media_format_event(me_ptr);
                     control_tx_raise_event(me_ptr);
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
      
      case INTF_EXTN_PARAM_ID_IMCL_PORT_OPERATION:
      {
         uint32_t supported_intent[1] = {INTENT_ID_DECIMATE_CONTROL};
         capi_result = capi_cmn_ctrl_port_operation_handler(
            &me_ptr->ctrl_port_info, params_ptr, 
            (POSAL_HEAP_ID)me_ptr->heap_info.heap_id, 0, 1, supported_intent);
         capi_err_t result = CAPI_EOK;
         capi_buf_t buf;
         uint32_t control_port_id = 0;
         imcl_port_state_t port_state = CTRL_PORT_CLOSE;
   
         buf.actual_data_len = sizeof(decimate_imcl_header_t) + sizeof(capi_decimate_control_data_payload_t);
         buf.data_ptr = NULL;
         buf.max_data_len = 0;

         imcl_outgoing_data_flag_t flags;
         flags.should_send = TRUE;
         flags.is_trigger = FALSE;
   
         // Get the first control port id for the intent #INTENT_ID_GAIN_CONTROL
         capi_cmn_ctrl_port_list_get_next_port_data(&me_ptr->ctrl_port_info,
                                              INTENT_ID_DECIMATE_CONTROL,
                                              control_port_id, // initially, an invalid port id
                                              &me_ptr->ctrl_port_ptr);

         if (me_ptr->ctrl_port_ptr)
         {
            control_port_id = me_ptr->ctrl_port_ptr->port_info.port_id;
            port_state = me_ptr->ctrl_port_ptr->state;
         }
         else
         {
            AR_MSG_ISLAND(DBG_ERROR_PRIO,"Port data ptr doesnt exist. ctrl port id=0x%x port state = 0x%x",control_port_id, port_state);
         }

         if (0 != control_port_id) {
            me_ptr->is_ctrl_port_received = DECIMATE_CTRL_PORT_INFO_RCVD;
            if (CTRL_PORT_PEER_CONNECTED == port_state)
            {
               // Get one time buf from the queue
               result |= capi_cmn_imcl_get_one_time_buf(&me_ptr->cb_info, control_port_id, buf.actual_data_len, &buf);
      
               AR_MSG_ISLAND(DBG_ERROR_PRIO,"buf.actual_data_len=0x%x", buf.actual_data_len);

               if (CAPI_FAILED(result) || NULL == buf.data_ptr)
               {
                  AR_MSG(DBG_ERROR_PRIO,"Getting one time buffer failed");
                  return result;
               }
               decimate_imcl_header_t *out_cfg_ptr = (decimate_imcl_header_t *)buf.data_ptr;
               capi_decimate_control_data_payload_t *data_over_imc_payload = (capi_decimate_control_data_payload_t*)(out_cfg_ptr + 1);

               out_cfg_ptr->opcode = PARAM_ID_DECIMATE_CONTROL_IMC_PAYLOAD;
               out_cfg_ptr->actual_data_len = sizeof(capi_decimate_control_data_payload_t);
               data_over_imc_payload->decimation_factor = me_ptr->decimation_factor;
               // send data to peer/destination module
               if (CAPI_SUCCEEDED(capi_cmn_imcl_send_to_peer(&me_ptr->cb_info, &buf, control_port_id, flags)))
               {
                  AR_MSG(DBG_HIGH_PRIO,"Enable %d and dec factor %d sent to control port 0x%x", data_over_imc_payload->decimation_factor, control_port_id);
               }
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,"Control port is not connected");
            }
         }
         else {
		AR_MSG(DBG_ERROR_PRIO,"Control port id is not proper");
	 }
	 break;







#if 0
          #ifdef US_DETECT_DEBUG_LOG
		AR_MSG(DBG_LOW_PRIO,"Inside INTF_EXTN_PARAM_ID_IMCL_PORT_OPERATION");
	  #endif


	  uint32_t supported_intent[1] = {INTENT_ID_DECIMATE_CONTROL};
	  capi_result = capi_cmn_ctrl_port_operation_handler(&me_ptr->ctrl_port_info, params_ptr,
                  me_ptr->ctrl_port_ptr, (*(me_ptr->node_list_ptr) + me_ptr->port_counter), &me_ptr->port_counter, 1, supported_intent);
			
	  #ifdef US_DETECT_DEBUG_LOG
		AR_MSG(DBG_LOW_PRIO,"Called control port operation handler with result %d", result);
	  #endif
			
	  uint32_t control_port_id = 0;
	  imcl_port_state_t port_state = CTRL_PORT_CLOSE;
		
	  //Get the first control port id for intent #INTENT_ID_DECIMATE_CONTROL
	  capi_cmn_ctrl_port_list_get_next_port_data(&me_ptr->ctrl_port_info, INTENT_ID_DECIMATE_CONTROL, control_port_id, &me_ptr->ctrl_port_ptr);
	  if (me_ptr->ctrl_port_ptr)
	  {
	 	control_port_id = me_ptr->ctrl_port_ptr->port_info.port_id;
		port_state = me_ptr->ctrl_port_ptr->state;
	  }
			
	  AR_MSG(DBG_LOW_PRIO,"Port state is  %lu, port id is 0x%x", port_state, control_port_id);
			

	  if (0 != control_port_id)
	  {
		me_ptr->is_ctrl_port_received = DECIMATE_CTRL_PORT_INFO_RCVD;
		if (CTRL_PORT_OPEN == port_state)
		{
			// you can add you lib's imc related set param function
			//AR_MSG(DBG_LOW_PRIO,"The function to send sampling frequency over IMC can now be called");
			decimate_send_data_over_imc(me_ptr, (uint32) me_ptr->decimation_factor);
		}
	  }
	  else
	  {
		AR_MSG(DBG_ERROR_PRIO,"Control port id is not proper");
  	  }

	  break;
#endif

      }
      default:
         AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Set unsupported param ID 0x%x", (int)param_id);
         CAPI_SET_ERROR(capi_result, CAPI_EBADPARAM);
   }

   AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Set param done");
   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: control_tx_get_param
  DESCRIPTION: Gets either a parameter value or a parameter structure
  containing multiple parameters. In the event of a failure, the appropriate
  error code is returned.
 * -----------------------------------------------------------------------*/
static capi_err_t control_tx_get_param(capi_t *                _pif,
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
   control_tx_t *me_ptr      = (control_tx_t *)(_pif);
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
  Function name: control_tx_set_properties
  DESCRIPTION: Function to set the properties for the Decimate module
 * -----------------------------------------------------------------------*/
static capi_err_t control_tx_set_properties(capi_t *_pif, capi_proplist_t *props_ptr)
{
   capi_err_t capi_result = CAPI_EOK;
   if (NULL == _pif)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Set properties received bad pointer, 0x%lx", (uint32_t)_pif);
      return CAPI_EBADPARAM;
   }
   control_tx_t *me_ptr = (control_tx_t *)_pif;
   capi_result             = control_tx_process_set_properties(me_ptr, props_ptr);
   return capi_result;
}

/*------------------------------------------------------------------------
  Function name: control_tx_get_properties
  DESCRIPTION: Function to get the properties for the Decimate module
 * -----------------------------------------------------------------------*/
static capi_err_t control_tx_get_properties(capi_t *_pif, capi_proplist_t *props_ptr)
{
   capi_err_t capi_result = CAPI_EOK;
   if (NULL == _pif)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Get properties received bad pointer, 0x%lx", (uint32_t)_pif);
      return CAPI_EBADPARAM;
   }
   control_tx_t *me_ptr = (control_tx_t *)_pif;
   capi_result             = control_tx_process_get_properties(me_ptr, props_ptr);
   return capi_result;
}
