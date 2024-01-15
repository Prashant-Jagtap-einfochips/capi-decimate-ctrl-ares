/* ======================================================================== */
/**
   @file capi_decimate_utils.cpp

   C source file to implement the utility functions for
   CAPI for DECIMATE example.
 */

/* =========================================================================
   Copyright (c) 2015-2021 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#include "capi_decimate_utils.h"
#include "capi.h"
#include "shared_lib_api.h"
//#include "stringl.h"

/*===========================================================================
   FUNCTION : capi_decimate_process_get_properties
   DESCRIPTION: Utility function responsible for getting the properties from Decimate
             example.
===========================================================================*/
capi_err_t capi_decimate_process_get_properties(capi_decimate_t *me_ptr, capi_proplist_t *proplist_ptr)
{
   capi_err_t   capi_result = CAPI_EOK;
   capi_prop_t *prop_array  = proplist_ptr->prop_ptr;
   uint32_t     i;
   for (i = 0; i < proplist_ptr->props_num; i++)
   {
      capi_buf_t *payload_ptr = &(prop_array[i].payload);

      switch (prop_array[i].id)
      {
         /*The amount of memory in bytes to be passed into the capi_init function.
             Payload structure: capi_init_memory_requirement_t.
          */
         case CAPI_INIT_MEMORY_REQUIREMENT:
         {
            if (payload_ptr->max_data_len >= sizeof(capi_init_memory_requirement_t))
            {
               uint32_t decimate_size  = 0;
               uint32_t procblock_size = TEMP_BUF_SIZE_IN_SAMPLES;

               uint32_t port_data_size = ALIGN_8_BYTES(sizeof(ctrl_port_data_t) + 1 * sizeof(uint32_t)) +
                             ALIGN_8_BYTES(sizeof(uint32_t) * 8);

               // Each block must begin with 8-byte aligned memory.
               decimate_size = decimate_align_to_8_byte(sizeof(capi_decimate_t)) +
                               decimate_align_to_8_byte(procblock_size * sizeof(int16_t))    // For L-channel
                               + decimate_align_to_8_byte(procblock_size * sizeof(int16_t)) // For R-channel
                               + decimate_align_to_8_byte(port_data_size)  // For port data size
                               + decimate_align_to_8_byte(sizeof(spf_list_node_t) * MAX_IMCL_PORT_SIZE); // Max IMCL ports that can be used by this program 

               capi_init_memory_requirement_t *data_ptr = (capi_init_memory_requirement_t *)(payload_ptr->data_ptr);
               data_ptr->size_in_bytes                  = decimate_size;
               payload_ptr->actual_data_len             = sizeof(capi_init_memory_requirement_t);

               AR_MSG(DBG_HIGH_PRIO,
                      "CAPI DECIMATE: get property for initialization memory "
                      "requirements %lu bytes",
                      data_ptr->size_in_bytes);
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: get property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
            break;
         }

         /**< Indicates whether the module can perform in-place computation. If this value
             is true, the caller may provide the same pointers for input and output (but this
             is not guaranteed). This requires that the input and output data format be the
             same and the requires_data_buffering property be false.
             Payload Structure: capi_is_inplace_t */

         case CAPI_IS_INPLACE:
         {
            if (payload_ptr->max_data_len >= sizeof(capi_is_inplace_t))
            {
               capi_is_inplace_t *data_ptr  = (capi_is_inplace_t *)payload_ptr->data_ptr;
               data_ptr->is_inplace         = FALSE;
               payload_ptr->actual_data_len = sizeof(capi_is_inplace_t);
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: get property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
            break;
         }

         /**< Inform the caller service whether the module needs data buffering or not.
             If this value is false, the module must behave as follows:
             1. The number of output samples should always be the same as the number
             of input samples on all output ports. The caller must ensure that the
             number of input samples is the same on all input ports.
             2. All the input must be consumed. The caller must ensure that the output
             buffers have enough space.
             3. The module should be able to handle any number of samples.

             If this value is true, the module must behave as follows:
             1. The module must define a threshold in terms of number of bytes for each
             input port and each output port.
             2. The module must consume data on its inputs and fill data on its outputs
             till the amount of remaining data on each buffer of at least one input
             port is less than its threshold or the amount of free space on each buffer
             of at least one output port is less than its threshold.

             Note: Setting this value to true adds significant overhead, so it should
             only be used if:
             1. The module performs encoding/decoding of data.
             2. The module performs rate conversion between the input and output.

             Payload Structure: capi_requires_data_buffering_t
          */

         case CAPI_REQUIRES_DATA_BUFFERING:
         {
            if (payload_ptr->max_data_len >= sizeof(capi_requires_data_buffering_t))
            {
               capi_requires_data_buffering_t *data_ptr = (capi_requires_data_buffering_t *)payload_ptr->data_ptr;
               data_ptr->requires_data_buffering        = TRUE;
               payload_ptr->actual_data_len             = sizeof(capi_requires_data_buffering_t);
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: get property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
            break;
         }

         /**< The amount of stack size in bytes needed by this module
             Payload Structure: capi_stack_size_t */
         case CAPI_STACK_SIZE:
         {
            if (payload_ptr->max_data_len >= sizeof(capi_stack_size_t))
            {
               capi_stack_size_t *data_ptr  = (capi_stack_size_t *)payload_ptr->data_ptr;
               data_ptr->size_in_bytes      = CAPI_DECIMATE_STACK_SIZE;
               payload_ptr->actual_data_len = sizeof(capi_stack_size_t);
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: get property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
            break;
         }

         /**< The maximum size of metadata generated by this module after each call
             to process(). If this value is zero, the module does not generate any
             metadata. It includes size of different structures used to pack
             metadata (See property CAPI_METADATA).
             Payload Structure: capi_max_metadata_size_t */
         case CAPI_MAX_METADATA_SIZE:
         {
            if (payload_ptr->max_data_len >= sizeof(capi_max_metadata_size_t))
            {
               capi_max_metadata_size_t *data_ptr = (capi_max_metadata_size_t *)payload_ptr->data_ptr;
               data_ptr->size_in_bytes            = 0;
               payload_ptr->actual_data_len       = sizeof(capi_max_metadata_size_t);
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: get property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
         }
         break;

         /**< The size of the media format payload for a particular output port.
          *   This excludes the size of
          * capi_data_format_header_t.
          * Payload Structure: capi_output_media_format_size_t
          */
         case CAPI_OUTPUT_MEDIA_FORMAT_SIZE:
         {
            if (payload_ptr->max_data_len >= sizeof(capi_output_media_format_size_t))
            {
               capi_output_media_format_size_t *data_ptr = (capi_output_media_format_size_t *)payload_ptr->data_ptr;
               data_ptr->size_in_bytes                   = sizeof(capi_standard_data_format_v2_t);
               payload_ptr->actual_data_len              = sizeof(capi_output_media_format_size_t);
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: get property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
         }
         break;

         /**< Can be used to query the media format for a particular output port.
          * This property can also be used to set the output media format for modules at support control
          * of the output media format. If a module only supports controlling some aspects like say the
          * sample rate only, all other fields can be set to CAPI_DATA_FORMAT_INVALID_VAL.
          * The port id must be set in the payload by the caller.
          */
         case CAPI_OUTPUT_MEDIA_FORMAT_V2:
         {
            if (NULL == me_ptr)
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: Get, Property 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               capi_result |= CAPI_EBADPARAM;
               break;
            }

            /* The size of the payload is the sum of the size of the media format struct and the channel map size
             * which depends on the number of channels. Every channel will have a uint16_t channel type field*/
            uint32_t total_size =
               sizeof(capi_decimate_media_fmt_t) +
               (sizeof(me_ptr->input_media_fmt->format.channel_type[0]) * me_ptr->input_media_fmt->format.num_channels);

            if (payload_ptr->max_data_len >= total_size)
            {
               capi_decimate_media_fmt_t *data_ptr = (capi_decimate_media_fmt_t *)payload_ptr->data_ptr;
               if ((FALSE == prop_array[i].port_info.is_valid) && (TRUE == prop_array[i].port_info.is_input_port))
               {
                  AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Get output media fmt v2 port id not valid or input port");
                  capi_result |= CAPI_EBADPARAM;
               }
               memscpy(data_ptr, total_size, me_ptr->input_media_fmt, total_size);
               payload_ptr->actual_data_len = total_size;
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: Get property_id 0x%lx, Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               payload_ptr->actual_data_len = 0;
               capi_result |= CAPI_ENEEDMORE;
            }
            break;
         }

         /**< The number of framework extensions needed by this module.
             Payload Structure: capi_num_needed_framework_extensions_t */
#if 1
         case CAPI_NUM_NEEDED_FRAMEWORK_EXTENSIONS:
         {
            if (payload_ptr->max_data_len >= sizeof(capi_num_needed_framework_extensions_t))
            {
               capi_num_needed_framework_extensions_t *data_ptr =
                  (capi_num_needed_framework_extensions_t *)payload_ptr->data_ptr;

               data_ptr->num_extensions     = 0;
               payload_ptr->actual_data_len = sizeof(capi_num_needed_framework_extensions_t);
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: get property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
         }
         break;
#endif
         /**< The threshold in bytes of an input or output port.
          * This property is only used for modules that
          * require data buffering. Refer to the comments for
          * the CAPI_REQUIRES_DATA_BUFFERING property for usage.
          * Payload Structure: capi_port_data_threshold_t
          */
         case CAPI_PORT_DATA_THRESHOLD:
         {
            if (payload_ptr->max_data_len >= sizeof(capi_port_data_threshold_t))
            {
               capi_port_data_threshold_t *data_ptr = (capi_port_data_threshold_t *)payload_ptr->data_ptr;
               if (!prop_array[i].port_info.is_valid)
               {
                  AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: get property id not valid");
                  CAPI_SET_ERROR(capi_result, CAPI_EBADPARAM);
                  break;
               }
               if (prop_array[i].port_info.is_input_port)
               {
                  if (0 != prop_array[i].port_info.port_index)
                  {
                     AR_MSG(DBG_ERROR_PRIO,
                            "CAPI DECIMATE: get property id 0x%lx Max Input Port: 1. "
                            "Requesting for index %lu",
                            (uint32_t)prop_array[i].id,
                            prop_array[i].port_info.port_index);
                     CAPI_SET_ERROR(capi_result, CAPI_EBADPARAM);
                  }
                  data_ptr->threshold_in_bytes = 1;
               }
               else
               {
                  if (0 != prop_array[i].port_info.port_index)
                  {
                     AR_MSG(DBG_ERROR_PRIO,
                            "CAPI DECIMATE: get property id 0x%lx Max Output Port: 1. "
                            "Requesting for index %lu",
                            (uint32_t)prop_array[i].id,
                            prop_array[i].port_info.port_index);
                     CAPI_SET_ERROR(capi_result, CAPI_EBADPARAM);
                  }
                  data_ptr->threshold_in_bytes = 1;
               }
               payload_ptr->actual_data_len = sizeof(capi_port_data_threshold_t);
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: get property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->max_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
            break;
         }
#if 1
	case CAPI_INTERFACE_EXTENSIONS:
        {
            capi_buf_t *                 payload_ptr   = &prop_array[i].payload;
            capi_interface_extns_list_t *intf_ext_list = (capi_interface_extns_list_t *)payload_ptr->data_ptr;

            if ((payload_ptr->max_data_len < sizeof(capi_interface_extns_list_t)) ||
                (payload_ptr->max_data_len < (sizeof(capi_interface_extns_list_t) +
                                              (intf_ext_list->num_extensions * sizeof(capi_interface_extn_desc_t)))))
            {
               capi_result |= CAPI_ENEEDMORE;
            }

            if (CAPI_FAILED(capi_result))
            {
               payload_ptr->actual_data_len = 0;
               AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Insufficient get property size.");
               break;
            }

            capi_interface_extn_desc_t *curr_intf_extn_desc_ptr =
               (capi_interface_extn_desc_t *)(payload_ptr->data_ptr + sizeof(capi_interface_extns_list_t));

            for (uint32_t j = 0; j < intf_ext_list->num_extensions; j++)
            {
               switch (curr_intf_extn_desc_ptr->id)
               {
                  case INTF_EXTN_IMCL:
                  {
                     curr_intf_extn_desc_ptr->is_supported = TRUE;
                     break;
                  }
                  default:
                  {
                     curr_intf_extn_desc_ptr->is_supported = FALSE;
                     break;
                  }
               }
               curr_intf_extn_desc_ptr++;
            }
            break;
         }
#endif
         /**< Provides module-specific initialization data. This property is typically only
             set at initialization time.
             Payload Structure: Module-specific */
         case CAPI_CUSTOM_INIT_DATA:
         {
            // Ignore this property.
            break;
         }
         default:
         {
            AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: get property id for 0x%x is not supported.", prop_array[i].id);
            CAPI_SET_ERROR(capi_result, CAPI_EUNSUPPORTED);
         }
         break;
      }

      if (CAPI_FAILED(capi_result))
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI DECIMATE: get property id for 0x%x failed with opcode %lu",
                prop_array[i].id,
                capi_result);
      }
      else
      {

         AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: get property id for 0x%x done", prop_array[i].id);
      }
   }
   return capi_result;
}

/*===========================================================================
   Function name: capi_decimate_init_media_fmt
   DESCRIPTION: Function to set the default media format for decimate
   during the initialization
===========================================================================*/
void capi_decimate_init_media_fmt(capi_decimate_t *me_ptr)
{
   capi_decimate_media_fmt_t *media_fmt_ptr = &(me_ptr->input_media_fmt[0]);

   uint32_t j;

   /**< Set the media format to default state */
   media_fmt_ptr->header.format_header.data_format = CAPI_FIXED_POINT;
   media_fmt_ptr->format.bits_per_sample           = 16;
   media_fmt_ptr->format.bitstream_format          = CAPI_DATA_FORMAT_INVALID_VAL;
   media_fmt_ptr->format.data_interleaving         = CAPI_DEINTERLEAVED_UNPACKED;
   media_fmt_ptr->format.data_is_signed            = 1;
   media_fmt_ptr->format.num_channels              = CAPI_DATA_FORMAT_INVALID_VAL;
   media_fmt_ptr->format.q_factor                  = PCM_Q_FACTOR_15;
   media_fmt_ptr->format.sampling_rate             = CAPI_DATA_FORMAT_INVALID_VAL;

   for (j = 0; (j < CAPI_MAX_CHANNELS); j++)
   {
      media_fmt_ptr->format.channel_type[j] = (uint16_t)CAPI_DATA_FORMAT_INVALID_VAL;
   }
}

/*===========================================================================
   FUNCTION : capi_decimate_process_set_properties
   DESCRIPTION: Function to set properties for decimate example
===========================================================================*/
capi_err_t capi_decimate_process_set_properties(capi_decimate_t *me_ptr, capi_proplist_t *proplist_ptr)
{
   capi_err_t   capi_result = CAPI_EOK;
   capi_prop_t *prop_array  = proplist_ptr->prop_ptr;
   uint8_t      i;
   for (i = 0; i < proplist_ptr->props_num; i++)
   {
      capi_buf_t *payload_ptr = &(prop_array[i].payload);

      switch (prop_array[i].id)
      {
         case CAPI_PORT_NUM_INFO:
         {
            if (payload_ptr->actual_data_len >= sizeof(capi_port_num_info_t))
            {
               capi_port_num_info_t *data_ptr = (capi_port_num_info_t *)payload_ptr->data_ptr;

               if ((data_ptr->num_input_ports > CAPI_DECIMATE_MAX_IN_PORTS) ||
                   (data_ptr->num_output_ports > CAPI_DECIMATE_MAX_OUT_PORTS))
               {
                  AR_MSG(DBG_ERROR_PRIO,
                         "CAPI DECIMATE: Set property id 0x%lx number of input and output ports "
                         "cannot be more than 1",
                         (uint32_t)prop_array[i].id);
                  CAPI_SET_ERROR(capi_result, CAPI_EBADPARAM);
               }
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: Set property id  0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->actual_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
         }
         break;

         case CAPI_HEAP_ID:
         {
            if (payload_ptr->actual_data_len >= sizeof(capi_heap_id_t))
            {
               // capi_heap_id_t *data_ptr = (capi_heap_id_t*)payload->data_ptr;
               AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Set property id for heap is ignored.");
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: Set property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->actual_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
         }
         break;

         case CAPI_EVENT_CALLBACK_INFO:
         {
            if (payload_ptr->actual_data_len >= sizeof(capi_event_callback_info_t))
            {
               capi_event_callback_info_t *data_ptr = (capi_event_callback_info_t *)payload_ptr->data_ptr;
               if (NULL == data_ptr)
               {
                  CAPI_SET_ERROR(capi_result, CAPI_EBADPARAM);
               }
               else
               {
                  me_ptr->cb_info = *data_ptr;
               }
               AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Set property id for Event CallBack done.");
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: Set property id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->actual_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
         }
         break;

         case CAPI_ALGORITHMIC_RESET:
         {
            AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: received RESET");
         }
         break;

         case CAPI_INPUT_MEDIA_FORMAT_V2:
         {
            if (payload_ptr->actual_data_len >= sizeof(capi_decimate_media_fmt_t))
            {
               capi_decimate_media_fmt_t *media_fmt_ptr = (capi_decimate_media_fmt_t *)(payload_ptr->data_ptr);

               /* Number of channels should be between 0 and 32*/
               if ((0 >= media_fmt_ptr->format.num_channels) ||
                   (CAPI_MAX_CHANNELS_V2 < media_fmt_ptr->format.num_channels))
               {
                  AR_MSG(DBG_ERROR_PRIO,
                         "CAPI DECIMATE: Unsupported number of channels %lu",
                         media_fmt_ptr->format.num_channels);
                  return CAPI_EBADPARAM;
               }

               /* Calculate size of channel map after validating number of channels*/
               uint32_t channel_map_size =
                  (media_fmt_ptr->format.num_channels * sizeof(media_fmt_ptr->channel_type[0]));

               uint32_t required_size =
                  sizeof(capi_set_get_media_format_t) + sizeof(capi_standard_data_format_v2_t) + channel_map_size;

               /* Validate the size of payload again including channel map size */
               if (payload_ptr->actual_data_len < required_size)
               {
                  AR_MSG(DBG_ERROR_PRIO,
                         "CAPI DECIMATE: Not valid media format size %lu, required size %lu",
                         payload_ptr->actual_data_len,
                         required_size);
                  return CAPI_ENEEDMORE;
               }

               /* Validate data format*/
               if (CAPI_FIXED_POINT != media_fmt_ptr->header.format_header.data_format)
               {
                  AR_MSG(DBG_ERROR_PRIO,
                         "CAPI DECIMATE: unsupported data format %lu",
                         (uint32_t)media_fmt_ptr->header.format_header.data_format);
                  return CAPI_EBADPARAM;
               }

               /* Validate bits per sample*/
               if (16 != media_fmt_ptr->format.bits_per_sample)
               {
                  AR_MSG(DBG_ERROR_PRIO,
                         "CAPI DECIMATE: only supports 16 bit data. Received %lu.",
                         media_fmt_ptr->format.bits_per_sample);
                  return CAPI_EBADPARAM;
               }

               /* Validate interleaving*/
               if (media_fmt_ptr->format.data_interleaving != CAPI_DEINTERLEAVED_UNPACKED)
               {
                  AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Interleaved data not supported.");
                  return CAPI_EBADPARAM;
               }

               /* Validate data signed/unsigned*/
               if (!media_fmt_ptr->format.data_is_signed)
               {
                  AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Unsigned data not supported.");
                  return CAPI_EBADPARAM;
               }

               /* Validate sample rate*/
               if ((0 >= media_fmt_ptr->format.sampling_rate) || (384000 < media_fmt_ptr->format.sampling_rate))
               {
                  AR_MSG(DBG_ERROR_PRIO,
                         "CAPI DECIMATE: Unsupported sampling rate %lu",
                         media_fmt_ptr->format.sampling_rate);
                  return CAPI_EBADPARAM;
               }

               /* Copy and save the input media format after passing all validation checks */
               memscpy(me_ptr->input_media_fmt, required_size, media_fmt_ptr, payload_ptr->actual_data_len);
               me_ptr->is_mf_received = TRUE;

               /* Raise event for output media format: This event is raised to inform what should be the output media
                * format of the module. Should be raised when you receive input media format (when you know what is the
                * correct output media format) */
               capi_decimate_raise_output_media_format_event(me_ptr);
               AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Trying to raise threshold");
               me_ptr->threshold_in_bytes = (media_fmt_ptr->format.sampling_rate / 1000) *
                                            (media_fmt_ptr->format.bits_per_sample >> BYTE_CONV_FACTOR) *
                                            media_fmt_ptr->format.num_channels * FRAME_SIZE;
               capi_decimate_raise_threshold_event(&me_ptr->cb_info, me_ptr->threshold_in_bytes, TRUE, 0);
               /* Raise events*/
               capi_decimate_raise_event(me_ptr);
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI DECIMATE: Set Param id 0x%lx Bad param size %lu",
                      (uint32_t)prop_array[i].id,
                      payload_ptr->actual_data_len);
               CAPI_SET_ERROR(capi_result, CAPI_ENEEDMORE);
            }
         }
         break;

         case CAPI_OUTPUT_MEDIA_FORMAT_V2:
         {
            break;
         }

         case CAPI_CUSTOM_INIT_DATA:
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Set property id custom Init data unsupported! ");
            break;
         }
         default:
         {
            AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Set property for 0x%x. Not supported.", prop_array[i].id);
            CAPI_SET_ERROR(capi_result, CAPI_EUNSUPPORTED);
         }
         break;
      }

      if (CAPI_FAILED(capi_result))
      {
         AR_MSG(DBG_HIGH_PRIO,
                "CAPI DECIMATE: Set property for 0x%x failed with opcode %lu",
                prop_array[i].id,
                capi_result);
      }
      else
      {
         AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Set property for 0x%x done", prop_array[i].id);
      }
   }
   return capi_result;
}

/*===========================================================================
FUNCTION : capi_decimate_release_memory
DESCRIPTION: Function to release allocated memory
===========================================================================*/
void capi_decimate_release_memory(capi_decimate_t *me_ptr)
{
   memset(&me_ptr->decimate_mem, 0, sizeof(me_ptr->decimate_mem));
}

/*===========================================================================
   FUNCTION : capi_decimate_update_event_states
   DESCRIPTION: Function to update the event states for software decimation
===========================================================================*/
static void capi_decimate_update_event_states(capi_decimate_t *me_ptr)
{
   int16_t input_delay = DECIMATE_DELAY_US;
   // uint64_t delay_in_us_64 = (input_delay * (uint64_t)(1000000)) / me_ptr->lib_config.config_params[1];
   if (1 == me_ptr->enable_flag)
   {
      me_ptr->events_config.enable = 1;
   }
   else
   {
      me_ptr->events_config.enable = 0;
   }

   me_ptr->events_config.KPPS = capi_decimate_get_kpps(me_ptr);

   me_ptr->events_config.delay = input_delay; //(delay_in_us_64 > UINT32_MAX) ? UINT32_MAX : delay_in_us_64;
}

/*===========================================================================
   FUNCTION : capi_decimate_get_kpps
   DESCRIPTION: Function to update KPPS
===========================================================================*/
int capi_decimate_get_kpps(capi_decimate_t *me_ptr)
{
   return 5000; // roughly profiled value
}

/*===========================================================================
   FUNCTION : capi_decimate_raise_event
   DESCRIPTION: Function to raise all the events using the callback function
===========================================================================*/
void capi_decimate_raise_event(capi_decimate_t *me_ptr)
{
   if (NULL == me_ptr->cb_info.event_cb)
   {
      AR_MSG(DBG_HIGH_PRIO,
             "CAPI DECIMATE: Event callback is not set."
             " Unable to raise output media format event!");
      return;
   }

   capi_decimate_update_event_states(me_ptr);
   capi_decimate_raise_process_event(me_ptr);
   capi_decimate_raise_delay_event(me_ptr);
}

/*===========================================================================
   FUNCTION : capi_decimate_output_media_format_event
   DESCRIPTION: Function to send the output media format using the
   callback function
===========================================================================*/
void capi_decimate_raise_output_media_format_event(capi_decimate_t *me_ptr)
{
   capi_err_t capi_result = CAPI_EOK;

   if (me_ptr->decimation_factor > 1)
   {
      me_ptr->input_media_fmt->format.sampling_rate =
         me_ptr->input_media_fmt->format.sampling_rate /
         me_ptr->decimation_factor; // to raise output media format accordingly. As in this module, it produces
                                    // input/decimation_factor number of samples. This results in SR change as now
                                    // output SR becomes (input SR)/decimation_factor
   }

   // Raise an event
   capi_event_info_t event_info;

   event_info.port_info.is_valid      = TRUE;
   event_info.port_info.is_input_port = FALSE;
   event_info.port_info.port_index    = 0;
   event_info.payload.actual_data_len = event_info.payload.max_data_len = sizeof(me_ptr->input_media_fmt);
   event_info.payload.data_ptr = (signed char *)(&me_ptr->input_media_fmt[0]);

   capi_result =
      me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context, CAPI_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED_V2, &event_info);
   if (CAPI_FAILED(capi_result))
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Failed to send output media format updated event with %lu", capi_result);
   }

   if (me_ptr->decimation_factor > 1)
   {
      me_ptr->input_media_fmt->format.sampling_rate =
         me_ptr->input_media_fmt->format.sampling_rate *
         me_ptr->decimation_factor; // to restore the value to actual input SR
   }
}

/*===========================================================================
   FUNCTION : capi_decimate_raise_process_event
   DESCRIPTION: Function to send process check using the callback function
===========================================================================*/
void capi_decimate_raise_process_event(capi_decimate_t *me_ptr)
{
   if (NULL == me_ptr->cb_info.event_cb)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Event callback is not set. Unable to raise process event!");
      return;
   }
   capi_err_t capi_result = CAPI_EOK;

   capi_event_process_state_t event;
   event.is_enabled = (bool_t)((me_ptr->events_config.enable == 0) ? 0 : 1);
   capi_event_info_t event_info;
   event_info.port_info.is_valid      = FALSE;
   event_info.payload.actual_data_len = sizeof(event);
   event_info.payload.max_data_len    = sizeof(event);
   event_info.payload.data_ptr        = (int8_t *)(&event);
   capi_result = me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context, CAPI_EVENT_PROCESS_STATE, &event_info);
   if (CAPI_FAILED(capi_result))
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Failed to send process_check update event with %lu", capi_result);
   }
}

/*===========================================================================
   FUNCTION : capi_decimate_raise_delay_event
   DESCRIPTION: Function to send process check using the callback function
===========================================================================*/
void capi_decimate_raise_delay_event(capi_decimate_t *me_ptr)
{
   capi_err_t capi_result = CAPI_EOK;

   // Raise an event
   capi_event_info_t event_info;

   event_info.port_info.is_valid      = TRUE;
   event_info.port_info.is_input_port = FALSE;
   event_info.port_info.port_index    = 0;
   event_info.payload.actual_data_len = event_info.payload.max_data_len = sizeof(capi_event_algorithmic_delay_t);
   event_info.payload.data_ptr = (signed char *)(&me_ptr->events_config.delay);

   AR_MSG(DBG_HIGH_PRIO, "CAPI DECIMATE: Raising delay");

   capi_result = me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context, CAPI_EVENT_ALGORITHMIC_DELAY, &event_info);
   if (CAPI_FAILED(capi_result))
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Failed to send delay event with %lu", capi_result);
   }
}

/*===========================================================================
   FUNCTION : capi_decimate_raise_threshold_event
   DESCRIPTION: Function to send process check using the callback function
===========================================================================*/

void capi_decimate_raise_threshold_event(capi_event_callback_info_t *cb_info_ptr,
                                         uint32_t                    threshold_bytes,
                                         bool_t                      is_input_port,
                                         uint32_t                    port_index)
{
   capi_err_t result = CAPI_EOK;

   if (NULL == cb_info_ptr->event_cb)
   {
      AR_MSG(DBG_ERROR_PRIO, "capi_cmn : Event callback is not set, Unable to raise threshold event!");
      return;
   }

   capi_port_data_threshold_change_t evnt;
   evnt.new_threshold_in_bytes = threshold_bytes;

   capi_event_info_t event_info;
   event_info.port_info.is_input_port = is_input_port;
   event_info.port_info.is_valid      = TRUE;
   event_info.port_info.port_index    = port_index;
   event_info.payload.actual_data_len = sizeof(capi_port_data_threshold_change_t);
   event_info.payload.data_ptr        = (int8_t *)&evnt;
   event_info.payload.max_data_len    = sizeof(capi_port_data_threshold_change_t);

   result = cb_info_ptr->event_cb(cb_info_ptr->event_context, CAPI_EVENT_PORT_DATA_THRESHOLD_CHANGE, &event_info);

   if (CAPI_FAILED(result))
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Failed to raised threshold with result =k %d", result);
   }
   else
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI DECIMATE: Raised threshold successfully");
   }
   return;
}
