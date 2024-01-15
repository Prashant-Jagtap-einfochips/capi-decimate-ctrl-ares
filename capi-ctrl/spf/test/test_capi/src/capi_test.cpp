/*===========================================================================

                  C A P I V 2  T E S T   U T I L I T I E S

  Common definitions, types and functions for CAPI Tests
===========================================================================*/
/* =========================================================================
  Copyright (c) 2018-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ========================================================================== */

/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header:

$

when            who       what, where, why
--------        ---       ----------------------------------------------------------

===========================================================================*/
#include "capi_test.h"

#include <stdio.h>
#include <ctype.h>
#include "ar_error_codes.h"
#include "hexagon_sim_timer.h"
#include "stringl.h"
#include "imcl_spm_intent_api.h"
#include "ecns_metadata_api.h"

#define false 0
#define true 1
#define NUM_CHANNELS 2
#ifdef FRAME_SIZE
#define CAPI_INPUT_LENGTH_MS FRAME_SIZE
#define CAPI_OUTPUT_LENGTH_MS FRAME_SIZE
#else
#define CAPI_INPUT_LENGTH_MS 1  // amith: was 10 in 2.9
#define CAPI_OUTPUT_LENGTH_MS 1 // amith: was 10 in 2.9
#endif

#define MAX_NUM_MODULES_PER_TEST 2

#if (__QDSP6_ARCH__ == 3)
uint32_t num_hw_threads = 6;
#else
uint32_t num_hw_threads = 2;
#endif

// This indicates the iteration in the processing
// at which steady state may have started
static const uint32_t begin_steady_state_iteration = 3;

static const uint32_t MAX_PERSISTENT_PARAMS_PER_MODULE = 20;

typedef struct persistent_param_payload_t
{
   uint32_t is_entry_used;
   uint32_t param_id;
   uint8_t *data_payload;
} persistent_param_payload_t;

static persistent_param_payload_t persistent_param_payload_array[MAX_PERSISTENT_PARAMS_PER_MODULE] = { { 0 },
                                                                                                       { 0 },
                                                                                                       { NULL } };

struct tst_standard_media_format
{
   capi_data_format_header_t   h;
   capi_standard_data_format_t f;
};

struct tst_standard_media_format_v2
{
   capi_data_format_header_t      h;
   capi_standard_data_format_v2_t f;
   capi_channel_type_t            channel_type[CAPI_MAX_CHANNELS_V2];
};

/*===========================================================================
    FUNCTION : copy_media_fmt_v2_to_v1
    DESCRIPTION: Function to copy v2 version of media format to v1 structure.
===========================================================================*/
static void copy_media_fmt_v2_to_v1(tst_standard_media_format *               media_fmt_v1_dst,
                                    const tst_standard_media_format_v2 *const media_fmt_v2_src)
{

   media_fmt_v1_dst->h                   = media_fmt_v2_src->h;
   media_fmt_v1_dst->f.bitstream_format  = media_fmt_v2_src->f.bitstream_format;
   media_fmt_v1_dst->f.num_channels      = media_fmt_v2_src->f.num_channels;
   media_fmt_v1_dst->f.bits_per_sample   = media_fmt_v2_src->f.bits_per_sample;
   media_fmt_v1_dst->f.q_factor          = media_fmt_v2_src->f.q_factor;
   media_fmt_v1_dst->f.sampling_rate     = media_fmt_v2_src->f.sampling_rate;
   media_fmt_v1_dst->f.data_is_signed    = media_fmt_v2_src->f.data_is_signed;
   media_fmt_v1_dst->f.data_interleaving = media_fmt_v2_src->f.data_interleaving;

   memscpy(media_fmt_v1_dst->f.channel_type,
           sizeof(media_fmt_v1_dst->f.channel_type),
           media_fmt_v2_src->channel_type,
           media_fmt_v2_src->f.num_channels * sizeof(media_fmt_v2_src->channel_type[0]));

   return;
}

/*===========================================================================
    FUNCTION : copy_media_fmt_v1_to_v2
    DESCRIPTION: Function to copy v2 version of media format to another v2 structure.
===========================================================================*/
static void copy_media_fmt_v1_to_v2(tst_standard_media_format_v2 *         media_fmt_v2_dst,
                                    const tst_standard_media_format *const media_fmt_v1_src)
{
   media_fmt_v2_dst->h                   = media_fmt_v1_src->h;
   media_fmt_v2_dst->f.minor_version     = CAPI_MEDIA_FORMAT_MINOR_VERSION;
   media_fmt_v2_dst->f.bits_per_sample   = media_fmt_v1_src->f.bits_per_sample;
   media_fmt_v2_dst->f.bitstream_format  = media_fmt_v1_src->f.bitstream_format;
   media_fmt_v2_dst->f.data_interleaving = media_fmt_v1_src->f.data_interleaving;
   media_fmt_v2_dst->f.data_is_signed    = media_fmt_v1_src->f.data_is_signed;
   media_fmt_v2_dst->f.num_channels      = media_fmt_v1_src->f.num_channels;
   media_fmt_v2_dst->f.q_factor          = media_fmt_v1_src->f.q_factor;
   media_fmt_v2_dst->f.sampling_rate     = media_fmt_v1_src->f.sampling_rate;

   memscpy(media_fmt_v2_dst->channel_type,
           sizeof(media_fmt_v2_dst->channel_type),
           media_fmt_v1_src->f.channel_type,
           media_fmt_v1_src->f.num_channels * sizeof(media_fmt_v1_src->f.channel_type[0]));

   return;
}

static inline uint32_t bits_to_bytes(uint32_t bits_per_sample, uint32_t q_factor)
{

   if (27 == q_factor && 24 == bits_per_sample)
   {
      return 4;
   }
   else
   {
      return (bits_per_sample >> 3);
   }
}

capi_err_t ProcessData(module_info_t *module);
capi_err_t SetParamInband(module_info_t *module);
capi_err_t RegisterPersistentParam(module_info_t *module);
capi_err_t RegisterPersistFileParam(module_info_t *module);
capi_err_t DeRegisterPersistentParam(module_info_t *module);
capi_err_t GetParamInband(module_info_t *module);
capi_err_t SetMediaFormat(module_info_t *module);
// capi_err_t SetIntfExtn(module_info_t *module);
capi_err_t SetMaxNumPorts(module_info_t *module);
capi_err_t SetDataPortOperation(module_info_t *module);
capi_err_t SetControlPortOperation(module_info_t *module);
capi_err_t RegisterToEvent(module_info_t *module);
capi_err_t SetParamFromFile(module_info_t *module);

const testCommand steadyStateCommandSet[] = { { "ProcessData", &ProcessData },
                                              { "SetParamInband", &SetParamInband },
                                              { "RegisterPersistentParam", &RegisterPersistentParam },
                                              { "RegisterPersistFileParam", &RegisterPersistFileParam },
                                              { "DeRegisterPersistentParam", &DeRegisterPersistentParam },
                                              { "GetParamInband", &GetParamInband },
                                              { "SetMediaFormat", &SetMediaFormat },
                                              { "SetMaxNumPorts", &SetMaxNumPorts },
                                              { "SetDataPortOperation", &SetDataPortOperation },
                                              { "SetControlPortOperation", &SetControlPortOperation },
                                              { "RegisterToEvent", &RegisterToEvent },
                                              { "SetParamFromFile", &SetParamFromFile} };

static bool_t IsCompressedData(const capi_buf_t *format_ptr)
{
   uint32_t data_format;

   if ((NULL == format_ptr) || (NULL == format_ptr->data_ptr))
   {
      data_format = 0;
   }
   else
   {
      POSAL_ASSERT(format_ptr->max_data_len >= sizeof(capi_data_format_header_t));
      capi_data_format_header_t *header = reinterpret_cast<capi_data_format_header_t *>(format_ptr->data_ptr);
      data_format                       = header->data_format;
   }
   switch (data_format)
   {
      case CAPI_FIXED_POINT:
      case CAPI_FLOATING_POINT:
         return FALSE;
      case CAPI_RAW_COMPRESSED:
      case CAPI_IEC61937_PACKETIZED:
      case CAPI_DSD_DOP_PACKETIZED:
      case CAPI_COMPR_OVER_PCM_PACKETIZED:
      case CAPI_GENERIC_COMPRESSED:
      case CAPI_IEC60958_PACKETIZED:
      case CAPI_IEC60958_PACKETIZED_NON_LINEAR:
         return TRUE;
      case CAPI_MAX_FORMAT_TYPE:
         POSAL_ASSERT(0);
         return FALSE;
   }

   return FALSE;
}

static uint32_t calculateNumBuffers(const capi_buf_t *format)
{
   if (IsCompressedData(format))
   {
      return 1;
   }

   POSAL_ASSERT(format->actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);
   tst_standard_media_format_v2 *tst_format = reinterpret_cast<tst_standard_media_format_v2 *>(format->data_ptr);

   if (tst_format && (CAPI_DEINTERLEAVED_UNPACKED == tst_format->f.data_interleaving))
   {
      return tst_format->f.num_channels;
   }
   else
   {
      return 1;
   }
}

static void update_output_media_format(module_info_t *   module,
                                       const capi_buf_t *out_format,
                                       capi_event_id_t   id,
                                       uint32_t          port_index)
{

   switch (id)
   {
      case CAPI_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED:
      {
         POSAL_ASSERT(out_format->actual_data_len <= sizeof(module->out_format_buf[port_index]));

         tst_standard_media_format *out_format_buf_v1 = (tst_standard_media_format *)out_format->data_ptr;

         uint32_t channel_map_size = out_format_buf_v1->f.num_channels * sizeof(out_format_buf_v1->f.channel_type[0]);
         channel_map_size          = SPF_TEST_ALIGN_BY_4(channel_map_size);

         // Adjust output buffer sizes for new media formats
         module->out_format[port_index].actual_data_len = TST_STD_MEDIA_FMT_V2_MIN_SIZE + channel_map_size;
         module->out_format[port_index].max_data_len    = sizeof(module->out_format_buf[port_index]);
         module->out_format[port_index].data_ptr        = module->out_format_buf[port_index];

         copy_media_fmt_v1_to_v2((tst_standard_media_format_v2 *)module->out_format[port_index].data_ptr,
                                 out_format_buf_v1);

         POSAL_ASSERT(out_format->actual_data_len >= sizeof(capi_data_format_header_t));

         if (IsCompressedData(out_format))
         {
            module->out_buffer_len[port_index] = module->in_buffer_len[port_index];
         }
         else
         {
            POSAL_ASSERT(out_format->actual_data_len >= sizeof(tst_standard_media_format));
            tst_standard_media_format *format = reinterpret_cast<tst_standard_media_format *>(out_format->data_ptr);
            if (format->f.data_interleaving == CAPI_DEINTERLEAVED_UNPACKED)
            {
               module->out_buffer_len[port_index] = (format->f.sampling_rate * CAPI_OUTPUT_LENGTH_MS / 1000) *
                                                    bits_to_bytes(format->f.bits_per_sample, format->f.q_factor);
            }
            else
            {
               module->out_buffer_len[port_index] = (format->f.sampling_rate * CAPI_OUTPUT_LENGTH_MS / 1000) *
                                                    bits_to_bytes(format->f.bits_per_sample, format->f.q_factor) *
                                                    format->f.num_channels;
            }
         }
         break;
      }
      case CAPI_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED_V2:
      {
         POSAL_ASSERT(out_format->actual_data_len <= sizeof(module->out_format_buf[port_index]));

         // Adjust output buffer sizes for new media formats
         module->out_format[port_index].actual_data_len = out_format->actual_data_len;
         module->out_format[port_index].max_data_len    = sizeof(module->out_format_buf[port_index]);
         module->out_format[port_index].data_ptr        = module->out_format_buf[port_index];
         memscpy(module->out_format[port_index].data_ptr,
                 module->out_format[port_index].max_data_len,
                 out_format->data_ptr,
                 out_format->actual_data_len);

         POSAL_ASSERT(out_format->actual_data_len >= sizeof(capi_data_format_header_t));

         if (IsCompressedData(out_format))
         {
            module->out_buffer_len[port_index] = module->in_buffer_len[port_index];
         }
         else
         {
            POSAL_ASSERT(out_format->actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);
            tst_standard_media_format_v2 *format =
               reinterpret_cast<tst_standard_media_format_v2 *>(out_format->data_ptr);
            if (format->f.data_interleaving == CAPI_DEINTERLEAVED_UNPACKED)
            {
               module->out_buffer_len[port_index] = (format->f.sampling_rate * CAPI_OUTPUT_LENGTH_MS / 1000) *
                                                    bits_to_bytes(format->f.bits_per_sample, format->f.q_factor);
            }
            else
            {
               module->out_buffer_len[port_index] = (format->f.sampling_rate * CAPI_OUTPUT_LENGTH_MS / 1000) *
                                                    bits_to_bytes(format->f.bits_per_sample, format->f.q_factor) *
                                                    format->f.num_channels;
            }
         }
         break;
      }
      default:
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI TEST: Error in updating output media format. Unsupported event id : %lu.",
                static_cast<uint32_t>(id));
         return;
      }
   }
}

void FillInputBuffer(capi_stream_data_v2_t *input_bufs_ptr,
                     module_info_t *        module,
                     void *                 temp_in_buffer,
                     uint32_t               port_index)
{
   FILE *in_file = module->finp[port_index];

   uint32_t num_bytes_to_read = input_bufs_ptr->bufs_num * input_bufs_ptr->buf_ptr[0].max_data_len;

   uint32_t num_bytes_read = fread(temp_in_buffer, sizeof(int8_t), num_bytes_to_read, in_file);
   if (num_bytes_read < num_bytes_to_read)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Input file seems to be complete");
      input_bufs_ptr->flags.marker_eos = TRUE;
   }

   // ------------------------
   // Deinterleave data if needed
   // ------------------------
   if (IsCompressedData(&module->in_format[port_index]))
   {
      memcpy(input_bufs_ptr->buf_ptr[0].data_ptr, temp_in_buffer, num_bytes_read);
      input_bufs_ptr->buf_ptr[0].actual_data_len = num_bytes_read;
   }
   else
   {

      POSAL_ASSERT(module->in_format[port_index].actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);
      tst_standard_media_format_v2 *format =
         reinterpret_cast<tst_standard_media_format_v2 *>(module->in_format[port_index].data_ptr);

      switch (format->f.data_interleaving)
      {
         case CAPI_DEINTERLEAVED_PACKED:
         {
            const uint32_t num_channels    = format->f.num_channels;
            const uint32_t bits_per_sample = format->f.bits_per_sample;
            capi_buf_t *   buf_ptr         = &(input_bufs_ptr->buf_ptr[0]);
            if (bits_per_sample == 16)
            {
               int16_t *temp_in_buffer_16       = (int16_t *)temp_in_buffer;
               uint32_t num_samples_per_channel = num_bytes_read / sizeof(int16_t) / num_channels;
               int16_t *data_ptr                = (int16_t *)(buf_ptr->data_ptr);

               for (uint32_t ch = 0; ch < num_channels; ch++)
               {
                  for (uint32_t j = 0; j < num_samples_per_channel; j++)
                  {
                     data_ptr[ch * num_samples_per_channel + j] = temp_in_buffer_16[j * num_channels + ch];
                  }
                  buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int16_t) * num_channels;
               }
            }
            else if (24 == bits_per_sample)
            {
               int8_t * dst_ptr                 = (int8_t *)buf_ptr->data_ptr;
               int8_t * src_ptr                 = (int8_t *)temp_in_buffer;
               uint32_t bytes_per_samp          = bits_per_sample >> 3;
               uint32_t num_samples_per_channel = num_bytes_read / bits_per_sample / num_channels;

               for (uint32_t i = 0; i < num_samples_per_channel; i++)
               {
                  for (uint32_t j = 0; j < num_channels; j++)
                  {
                     for (uint32_t k = 0; k < bytes_per_samp; k++)
                     {
                        dst_ptr[i + j * num_samples_per_channel * bytes_per_samp + k] = *src_ptr++;
                     }
                  }
               }
               buf_ptr->actual_data_len = num_samples_per_channel * bytes_per_samp * num_channels;
            }
            else if (bits_per_sample == 32)
            {
               int32_t *temp_in_buffer_32       = (int32_t *)temp_in_buffer;
               uint32_t num_samples_per_channel = num_bytes_read / sizeof(int32_t) / num_channels;
               int32_t *data_ptr                = (int32_t *)(buf_ptr->data_ptr);

               for (uint32_t ch = 0; ch < num_channels; ch++)
               {

                  for (uint32_t j = 0; j < num_samples_per_channel; j++)
                  {
                     data_ptr[ch * num_samples_per_channel + j] = temp_in_buffer_32[j * num_channels + ch];
                  }
                  buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int32_t) * num_channels;
               }
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: bit-width not supported");
            }
            break;
         }
         case CAPI_DEINTERLEAVED_UNPACKED:
         {

            const uint32_t num_channels    = format->f.num_channels;
            const uint32_t bits_per_sample = format->f.bits_per_sample;
            // const uint32_t q_factor        = format->f.q_factor;
            if (bits_per_sample == 16)
            {
               int16_t *temp_in_buffer_16       = (int16_t *)temp_in_buffer;
               uint32_t num_samples_per_channel = num_bytes_read / sizeof(int16_t) / num_channels;

               for (uint32_t ch = 0; ch < num_channels; ch++)
               {
                  capi_buf_t *buf_ptr  = &(input_bufs_ptr->buf_ptr[ch]);
                  int16_t *   data_ptr = (int16_t *)(buf_ptr->data_ptr);
                  for (uint32_t j = 0; j < num_samples_per_channel; j++)
                  {
                     data_ptr[j] = temp_in_buffer_16[j * num_channels + ch];
                  }
                  buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int16_t);
               }
            }
            else if (24 == bits_per_sample)
            {
               int8_t * src_ptr        = (int8_t *)temp_in_buffer;
               int8_t * dst_ptr        = (int8_t *)input_bufs_ptr->buf_ptr[0].data_ptr;
               uint32_t bytes_per_samp = bits_per_sample >> 3;

               uint32_t num_samples_per_channel = num_bytes_read / bytes_per_samp / num_channels;
               for (uint32_t i = 0; i < num_samples_per_channel; i++)
               {
                  for (uint32_t j = 0; j < num_channels; j++)
                  {
                     for (uint32_t k = 0; k < bytes_per_samp; k++)
                     {
                        dst_ptr                         = (int8_t *)input_bufs_ptr->buf_ptr[j].data_ptr;
                        dst_ptr[i * bytes_per_samp + k] = *src_ptr++;
                     }
                  }
               }

               for (uint32_t i = 0; i < num_channels; i++)
               {
                  input_bufs_ptr->buf_ptr[i].actual_data_len = num_samples_per_channel * bytes_per_samp;
               }
            }
            else if (bits_per_sample == 32)
            {
               int32_t *temp_in_buffer_32       = (int32_t *)temp_in_buffer;
               uint32_t num_samples_per_channel = num_bytes_read / sizeof(int32_t) / num_channels;

               for (uint32_t ch = 0; ch < num_channels; ch++)
               {
                  capi_buf_t *buf_ptr  = &(input_bufs_ptr->buf_ptr[ch]);
                  int32_t *   data_ptr = (int32_t *)(buf_ptr->data_ptr);

                  for (uint32_t j = 0; j < num_samples_per_channel; j++)
                  {
                     data_ptr[j] = temp_in_buffer_32[j * num_channels + ch];
                  }
                  buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int32_t);
               }
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: bit-width not supported");
            }

            break;
         }
         case CAPI_INTERLEAVED:
         case CAPI_INVALID_INTERLEAVING:
         {
            memcpy(input_bufs_ptr->buf_ptr[0].data_ptr, temp_in_buffer, num_bytes_read);
            input_bufs_ptr->buf_ptr[0].actual_data_len = num_bytes_read;
            break;
         }
      }
   }
}

capi_err_t CheckInputOutputSizes(capi_stream_data_v2_t *input[],
                                 uint32_t               input_bytes_given[],
                                 capi_stream_data_v2_t *output[],
                                 bool_t                 requires_data_buffering,
                                 uint32_t               num_input_ports,
                                 uint32_t               num_output_ports)
{
   if (requires_data_buffering)
   {
      // Check if any input is empty
      for (uint32_t i = 0; i < num_input_ports; i++)
      {
         bool_t is_empty = TRUE;
         for (uint32_t j = 0; j < input[i]->bufs_num; j++)
         {
            if (input[i]->buf_ptr[j].actual_data_len != input_bytes_given[i])
            {
               is_empty = FALSE;
               break;
            }
         }

         if (is_empty)
         {
            return CAPI_EOK;
         }
      }

      // If none of the inputs are empty, at least one output must be full
      for (uint32_t i = 0; i < num_output_ports; i++)
      {
         bool_t is_full = TRUE;
         for (uint32_t j = 0; j < output[i]->bufs_num; j++)
         {
            if (output[i]->buf_ptr[j].actual_data_len != output[i]->buf_ptr[j].max_data_len)
            {
               is_full = FALSE;
               break;
            }
         }

         if (is_full)
         {
            return CAPI_EOK;
         }
      }

      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Process did not empty any input or fill any output completely.");
      return CAPI_EFAILED;
   }
   else
   {
      // All inputs should be empty
      for (uint32_t i = 0; i < num_input_ports; i++)
      {
         bool_t is_empty = TRUE;
         for (uint32_t j = 0; j < input[i]->bufs_num; j++)
         {
            if (input[i]->buf_ptr[j].actual_data_len != input_bytes_given[i])
            {
               is_empty = FALSE;
               break;
            }
         }

         if (!is_empty)
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Process did not empty input %lu", i);
            return CAPI_EFAILED;
         }
      }

      // All outputs must be filled with the same amount of data as the input
      for (uint32_t i = 0; i < num_output_ports; i++)
      {
         bool_t is_full = TRUE;
         for (uint32_t j = 0; j < output[i]->bufs_num; j++)
         {
            if (output[i]->buf_ptr[j].actual_data_len != input_bytes_given[i])
            {
               is_full = FALSE;
               break;
            }
         }

         if (!is_full)
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Process did not fill output %lu", i);
            return CAPI_EFAILED;
         }
      }

      return CAPI_EOK;
   }
}

capi_err_t DumpOutputToFile(capi_stream_data_v2_t *output_bufs_ptr,
                            module_info_t *        module,
                            void *                 temp_out_buffer,
                            uint32_t               output_port_index)
{
   FILE *   out_file           = module->fout[output_port_index];
   uint32_t num_bytes_to_write = output_bufs_ptr->bufs_num * output_bufs_ptr->buf_ptr[0].actual_data_len;

   int8_t *out_buf = NULL;

   if (NULL == module->out_format[output_port_index].data_ptr) {
      // seems like sink module, ignore
      return CAPI_EOK;
   }
   // ------------------------
   // Interleave data if not already so.
   // ------------------------
   if (IsCompressedData(&module->out_format[output_port_index])) // Todo: Analyse Ajits logs for if not being hit.
   {
      out_buf = (int8_t *)output_bufs_ptr->buf_ptr[0].data_ptr;
   }
   else
   {
      POSAL_ASSERT(module->in_format[output_port_index].actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);
      tst_standard_media_format_v2 *format =
         reinterpret_cast<tst_standard_media_format_v2 *>(module->out_format[output_port_index].data_ptr);

      switch (format->f.data_interleaving)
      {
         case CAPI_DEINTERLEAVED_PACKED:
         {
            const uint32_t num_channels    = format->f.num_channels;
            const uint32_t bits_per_sample = format->f.bits_per_sample;
            out_buf                        = (int8_t *)temp_out_buffer;

            capi_buf_t *buf_ptr = &(output_bufs_ptr->buf_ptr[0]);
            if (bits_per_sample == 16)
            {
               int16_t *data_ptr                = (int16_t *)(buf_ptr->data_ptr);
               int16_t *temp_out_buffer_16      = (int16_t *)temp_out_buffer;
               uint32_t num_samples_per_channel = buf_ptr->actual_data_len / sizeof(int16_t);
               for (uint32_t ch = 0; ch < num_channels; ch++)
               {
                  for (uint32_t j = 0; j < num_samples_per_channel; j++)
                  {
                     temp_out_buffer_16[j * num_channels + ch] = data_ptr[ch * num_samples_per_channel + j];
                  }
                  buf_ptr->actual_data_len = 0;
               }
            }
            else if (bits_per_sample == 24)
            {
               int8_t * src_ptr                 = (int8_t *)buf_ptr->data_ptr;
               int8_t * dst_ptr                 = (int8_t *)temp_out_buffer;
               uint32_t bytes_per_samp          = bits_per_sample >> 3;
               uint32_t num_samples_per_channel = buf_ptr->actual_data_len / bytes_per_samp;

               for (uint32_t i = 1; i <= num_samples_per_channel; i++)
               {
                  for (uint32_t j = 0; j < num_channels; j++)
                  {
                     for (uint32_t k = 0; k < bytes_per_samp; k++)
                     {
                        *dst_ptr++ = src_ptr[i + j * num_samples_per_channel * bytes_per_samp + k];
                     }
                  }
               }
            }
            else if (bits_per_sample == 32)
            {
               int32_t *temp_out_buffer_32      = (int32_t *)temp_out_buffer;
               int32_t *data_ptr                = (int32_t *)(buf_ptr->data_ptr);
               uint32_t num_samples_per_channel = buf_ptr->actual_data_len / sizeof(int32_t);
               for (uint32_t ch = 0; ch < num_channels; ch++)
               {
                  for (uint32_t j = 0; j < num_samples_per_channel; j++)
                  {
                     temp_out_buffer_32[j * num_channels + ch] = data_ptr[ch * num_samples_per_channel + j];
                  }
                  buf_ptr->actual_data_len = 0;
               }
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: bit-width not supported");
               return CAPI_EUNSUPPORTED;
            }
            break;
         }
         case CAPI_DEINTERLEAVED_UNPACKED:
         {
            const uint32_t num_channels    = format->f.num_channels;
            const uint32_t bits_per_sample = format->f.bits_per_sample;
            const uint32_t q_factor        = format->f.q_factor;
            out_buf                        = (int8_t *)temp_out_buffer;

            if (bits_per_sample == 16)
            {
               int16_t *temp_out_buffer_16 = (int16_t *)temp_out_buffer;
               for (uint32_t ch = 0; ch < num_channels; ch++)
               {
                  capi_buf_t *buf_ptr                 = &(output_bufs_ptr->buf_ptr[ch]);
                  int16_t *   data_ptr                = (int16_t *)(buf_ptr->data_ptr);
                  uint32_t    num_samples_per_channel = buf_ptr->actual_data_len / sizeof(int16_t);

                  for (uint32_t j = 0; j < num_samples_per_channel; j++)
                  {
                     temp_out_buffer_16[j * num_channels + ch] = data_ptr[j];
                  }
                  buf_ptr->actual_data_len = 0;
               }
            }
            else if (bits_per_sample == 24)
            {
               int8_t * src_ptr                 = (int8_t *)output_bufs_ptr->buf_ptr[0].data_ptr;
               int8_t * dst_ptr                 = (int8_t *)temp_out_buffer;
               uint32_t bytes_per_samp          = bits_per_sample >> 3;
               uint32_t num_samples_per_channel = output_bufs_ptr->buf_ptr[0].actual_data_len / bytes_per_samp;
               for (uint32_t i = 0; i < num_samples_per_channel; i++)
               {
                  for (uint32_t j = 0; j < num_channels; j++)
                  {
                     for (uint32_t k = 0; k < bytes_per_samp; k++)
                     {
                        src_ptr    = (int8_t *)output_bufs_ptr->buf_ptr[j].data_ptr;
                        *dst_ptr++ = src_ptr[i * bytes_per_samp + k];
                     }
                  }
               }
            }
            else if (bits_per_sample == 32 || (27 == q_factor && 24 == bits_per_sample))
            {
               int32_t *temp_out_buffer_32 = (int32_t *)temp_out_buffer;
               for (uint32_t ch = 0; ch < num_channels; ch++)
               {
                  capi_buf_t *buf_ptr                 = &(output_bufs_ptr->buf_ptr[ch]);
                  int32_t *   data_ptr                = (int32_t *)(buf_ptr->data_ptr);
                  uint32_t    num_samples_per_channel = buf_ptr->actual_data_len / sizeof(int32_t);

                  for (uint32_t j = 0; j < num_samples_per_channel; j++)
                  {
                     temp_out_buffer_32[j * num_channels + ch] = data_ptr[j];
                  }
                  buf_ptr->actual_data_len = 0;
               }
            }
            else
            {
               AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: bit-width not supported");
               return CAPI_EUNSUPPORTED;
            }
            break;
         }
         //         case CAPI_INTERLEAVED:
         //         case CAPI_INVALID_INTERLEAVING:
         default:
            out_buf = (int8_t *)output_bufs_ptr->buf_ptr[0].data_ptr;
            break;
      }
   }

   // ------------------------
   // Write output
   // ------------------------
   uint32_t num_bytes_written = fwrite(out_buf, sizeof(int8_t), num_bytes_to_write, out_file);
   if (num_bytes_written != num_bytes_to_write)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Failed to write output");
      return CAPI_EFAILED;
   }

   return CAPI_EOK;
}

capi_err_t ProcessData(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing Process Data command.");

   capi_err_t result = CAPI_EOK;
   uint32_t   i;

   uint32_t numBuffers = 0;

   for (uint32_t i = 0; i < module->num_input_port; i++)
   {
      if (module->is_media_format_received[i] != TRUE)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Input Media format is not received for port %lu", i);
         return CAPI_EOK;
      }
   }

// ------------------------
// Profiling attributes
// ------------------------
#if ((defined __hexagon__) || (defined __qdsp6__))
   // Note floats being used only for profiling information
   uint64_t prof_cycles       = 0;
   uint64_t prof_max_iter     = 0;
   uint64_t prof_total_cycles = 0;
   uint64_t prof_num_samples  = 0;
   float    prof_peak_mips    = 0.0;
   float    prof_mips         = 0.0;
#endif // __qdsp6___

   // ------------------------
   // Buffer pointers
   // ------------------------
   int8_t *input_buffer[CAPI_MAX_INPUT_PORTS]     = { NULL };
   int8_t *temp_in_buffer[CAPI_MAX_INPUT_PORTS]   = { NULL };
   int8_t *output_buffer[CAPI_MAX_OUTPUT_PORTS]   = { NULL };
   int8_t *temp_out_buffer[CAPI_MAX_OUTPUT_PORTS] = { NULL };

   // ------------------------
   // Buffer setup
   // ------------------------
   capi_stream_data_v2_t  input_str_data[CAPI_MAX_INPUT_PORTS], output_str_data[CAPI_MAX_OUTPUT_PORTS];
   capi_stream_data_v2_t *input[CAPI_MAX_INPUT_PORTS];
   capi_stream_data_v2_t *output[CAPI_MAX_OUTPUT_PORTS];
   capi_buf_t             in[CAPI_MAX_INPUT_PORTS][CAPI_MAX_CHANNELS_V2];
   capi_buf_t             out[CAPI_MAX_OUTPUT_PORTS][CAPI_MAX_CHANNELS_V2];

   memset(&input_str_data, 0, sizeof(capi_stream_data_v2_t));
   memset(in, 0, CAPI_MAX_INPUT_PORTS * CAPI_MAX_CHANNELS_V2 * sizeof(capi_buf_t));
   memset(out, 0, CAPI_MAX_OUTPUT_PORTS * CAPI_MAX_CHANNELS_V2 * sizeof(capi_buf_t));

   uint32_t in_buf_size[CAPI_MAX_INPUT_PORTS]   = { 0 };
   uint32_t out_buf_size[CAPI_MAX_OUTPUT_PORTS] = { 0 };

   for (uint32_t i = 0; i < CAPI_MAX_INPUT_PORTS; i++)
   {
      input_str_data[i].buf_ptr = &in[i][0];
      input[i]                  = &input_str_data[i];
   }

//   for (uint32_t i = 0; i < CAPI_MAX_INPUT_PORTS; i++)
//   {
//      input[i]->metadata_list_ptr = NULL;
//      output[i]->metadata_list_ptr = NULL;
//   }


   for (uint32_t i = 0; i < CAPI_MAX_OUTPUT_PORTS; i++)
   {
      output_str_data[i].buf_ptr = &out[i][0];
      output[i]                  = &output_str_data[i];
   }

   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: input_port %lu output_port %lu", module->num_input_port, module->num_output_port);
   for (uint32_t i = 0; i < module->num_input_port; i++)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "CAPI TEST: max data length %lu, actual data lenth %lu",
             module->in_format[i].max_data_len,
             module->in_format[i].actual_data_len);
      input_str_data[i].bufs_num = calculateNumBuffers(&module->in_format[i]);
      in_buf_size[i]             = module->in_buffer_len[i] * input_str_data[i].bufs_num;
      module->in_format[i].actual_data_len = in_buf_size[i];
      module->in_format[i].max_data_len = in_buf_size[i];

      //ms13: if threshold not raised than update max data len
      if(!module->is_out_threshold_raised)
      {
    	  module->out_format[i].max_data_len = in_buf_size[i];
      }
   }

   for (uint32_t i = 0; i < module->num_output_port; i++)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "CAPI TEST : max data length %lu, actual data lenth %lu",
             module->out_format[i].max_data_len,
             module->out_format[i].actual_data_len);
      output_str_data[i].bufs_num = calculateNumBuffers(&module->out_format[i]);

      //if ms13: if threshold not raised than update this below
      if(!module->is_out_threshold_raised)
      {
    	  module->out_buffer_len[i] = module->out_format[i].max_data_len;
      }

      out_buf_size[i] = module->out_buffer_len[i] * output_str_data[i].bufs_num;
   }

   // ------------------------
   // Get number of buffers to
   // process from config file
   // ------------------------
   result = GetUIntParameter(module->fCfg, "NumBuffers", &(numBuffers));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Process Buffers command failed to read NumBuffers.");
      return CAPI_EFAILED;
   }
   // Allocate input buffer

   for (uint32_t i = 0; i < module->num_input_port; i++)
   {
      input_buffer[i] = (int8_t *)posal_memory_malloc(in_buf_size[i], POSAL_HEAP_DEFAULT);
      if (NULL == input_buffer[i])
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Process Buffers command Memory allocation error for input buffer");
         result = CAPI_ENOMEMORY;
         goto done;
      }
      temp_in_buffer[i] = (int8_t *)posal_memory_malloc(in_buf_size[i], POSAL_HEAP_DEFAULT);
      if (NULL == temp_in_buffer[i])
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Process Buffers command Memory allocation error for temp input buffer");
         result = CAPI_ENOMEMORY;
         goto done;
      }
   }

   for (uint32_t i = 0; i < module->num_output_port; i++)
   {
      output_buffer[i] = (int8_t *)posal_memory_malloc(out_buf_size[i], POSAL_HEAP_DEFAULT);
      if (NULL == output_buffer[i])
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI TEST: Process Buffers command Memory allocation error for output_str_data buffer");
         result = CAPI_ENOMEMORY;
         goto done;
      }
      temp_out_buffer[i] = (int8_t *)posal_memory_malloc(out_buf_size[i], POSAL_HEAP_DEFAULT);
      if (NULL == temp_out_buffer[i])
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI TEST: Process Buffers command Memory allocation error for temp output_str_data buffer");
         result = CAPI_ENOMEMORY;
         goto done;
      }
   }

   // ------------------------
   // Set input and output_str_data buffers
   // ------------------------
   {
      int8_t * ptr       = NULL;
      uint32_t increment = 0;

      for (uint32_t i = 0; i < module->num_input_port; i++)
      {
         ptr       = input_buffer[i];
         increment = 0;
         for (uint32_t ch = 0; ch < input_str_data[i].bufs_num; ch++)
         {
            in[i][ch].data_ptr        = ptr + increment;
            in[i][ch].actual_data_len = 0;
            in[i][ch].max_data_len    = module->in_buffer_len[i];
            increment += module->in_buffer_len[i];
         }
      }

      for (uint32_t i = 0; i < module->num_output_port; i++)
      {
         ptr       = output_buffer[i];
         increment = 0;
         for (uint32_t ch = 0; ch < output_str_data[i].bufs_num; ch++)
         {
            out[i][ch].data_ptr        = ptr + increment;
            out[i][ch].actual_data_len = 0;
            out[i][ch].max_data_len    = module->out_buffer_len[i];

            //ms13: if threshold not raised update here:
            if(!module->is_out_threshold_raised)
            {
            	out[i][ch].max_data_len    = module->out_format[i].max_data_len;
            }

            increment += module->out_buffer_len[i];
         }
      }
   }

   // ------------------------
   // Process buffers
   // ------------------------
   for (i = 0; i < numBuffers; i++)
   {
      // Fill input buffer
      // while (input buffer not consumed)
      // {
      //    process()
      //    dumpOutput()
      // }

      // ------------------------
      // Read input
      // ------------------------
      bool_t is_file_over = FALSE;

      if ((i + 1) == numBuffers)
      {
         input_str_data->flags.marker_eos = TRUE;
      }

      for (uint32_t j = 0; j < module->num_input_port; j++)
      {
         //         if (!input_str_data->flags.marker_eos)
         //         {
         FillInputBuffer(&input_str_data[j], module, temp_in_buffer[j], j);
         //         }
         // check is file over
         if (input_str_data[j].buf_ptr[0].actual_data_len < input_str_data[j].buf_ptr[0].max_data_len)
         {
            input_str_data[j].flags.marker_eos = TRUE;
            if (0 == input_str_data[j].buf_ptr[0].actual_data_len)
            {
               is_file_over = (is_file_over | TRUE);
               break;
            }
         }
      }

      if (is_file_over)
      {
         break;
      }

      module_cmn_md_list_t *md_ptr1 = NULL, *md_ptr2 = NULL, *md_ptr3 = NULL;

      md_ptr1 =
         (module_cmn_md_list_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_list_t), 8, POSAL_HEAP_DEFAULT);
      md_ptr2 =
         (module_cmn_md_list_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_list_t), 8, POSAL_HEAP_DEFAULT);
      md_ptr3 =
         (module_cmn_md_list_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_list_t), 8, POSAL_HEAP_DEFAULT);

      memset(md_ptr1, 0, sizeof(module_cmn_md_list_t));
      memset(md_ptr2, 0, sizeof(module_cmn_md_list_t));
      memset(md_ptr3, 0, sizeof(module_cmn_md_list_t));

      md_ptr1->obj_ptr = (module_cmn_md_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_t), 8, POSAL_HEAP_DEFAULT);
      md_ptr2->obj_ptr = (module_cmn_md_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_t), 8, POSAL_HEAP_DEFAULT);
      md_ptr3->obj_ptr = (module_cmn_md_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_t), 8, POSAL_HEAP_DEFAULT);

      md_ptr1->obj_ptr->metadata_id = DUMMY_METADATA;
      md_ptr2->obj_ptr->metadata_id = 0;
      md_ptr3->obj_ptr->metadata_id = 0;

      md_ptr1->next_ptr = md_ptr2;
      md_ptr1->prev_ptr = NULL;
      md_ptr2->next_ptr = md_ptr3;
      md_ptr2->prev_ptr = md_ptr1;
      md_ptr3->next_ptr = NULL;
      md_ptr3->prev_ptr = md_ptr2;

      module_cmn_md_list_t *out_md_ptr1 = NULL, *out_md_ptr2 = NULL, *out_md_ptr3 = NULL;

      out_md_ptr1 =
         (module_cmn_md_list_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_list_t), 8, POSAL_HEAP_DEFAULT);
      out_md_ptr2 =
         (module_cmn_md_list_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_list_t), 8, POSAL_HEAP_DEFAULT);
      out_md_ptr3 =
         (module_cmn_md_list_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_list_t), 8, POSAL_HEAP_DEFAULT);

      memset(out_md_ptr1, 0, sizeof(module_cmn_md_list_t));
      memset(out_md_ptr2, 0, sizeof(module_cmn_md_list_t));
      memset(out_md_ptr3, 0, sizeof(module_cmn_md_list_t));

      out_md_ptr1->obj_ptr =
         (module_cmn_md_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_t), 8, POSAL_HEAP_DEFAULT);
      out_md_ptr2->obj_ptr =
         (module_cmn_md_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_t), 8, POSAL_HEAP_DEFAULT);
      out_md_ptr3->obj_ptr =
         (module_cmn_md_t *)posal_memory_aligned_malloc(sizeof(module_cmn_md_t), 8, POSAL_HEAP_DEFAULT);

      memset(out_md_ptr1->obj_ptr, 0, sizeof(module_cmn_md_t));
      memset(out_md_ptr2->obj_ptr, 0, sizeof(module_cmn_md_t));
      memset(out_md_ptr3->obj_ptr, 0, sizeof(module_cmn_md_t));

      out_md_ptr1->obj_ptr->metadata_id = 0;
      out_md_ptr2->obj_ptr->metadata_id = 0;
      out_md_ptr3->obj_ptr->metadata_id = 0;

      out_md_ptr1->next_ptr = out_md_ptr2;
      out_md_ptr1->prev_ptr = NULL;
      out_md_ptr2->next_ptr = out_md_ptr3;
      out_md_ptr2->prev_ptr = out_md_ptr1;
      out_md_ptr3->next_ptr = NULL;
      out_md_ptr3->prev_ptr = out_md_ptr2;

      if (TRUE == input_str_data->flags.marker_eos)
      {
         md_ptr2->obj_ptr->metadata_id = MODULE_CMN_MD_ID_EOS;
      }

      dummy_metadata_t *md_payload_ptr = (dummy_metadata_t *)md_ptr1->obj_ptr->metadata_buf;

      md_payload_ptr->payload_1 = TRUE;

      module_cmn_md_t *md_ptr = (module_cmn_md_t *)md_ptr1->obj_ptr;

      md_ptr->metadata_flag.is_out_of_band = FALSE;

      input_str_data[0].metadata_list_ptr         = md_ptr1;
      output_str_data[0].metadata_list_ptr         = out_md_ptr1;

      input_str_data[1].metadata_list_ptr         = NULL;
      output_str_data[1].metadata_list_ptr         = NULL;

      input_str_data[2].metadata_list_ptr         = NULL;
      output_str_data[2].metadata_list_ptr         = NULL;

      bool_t is_actual_data_len_nonzero = TRUE;
      while (is_actual_data_len_nonzero)
      {

         for (uint32_t j = 0; j < module->num_input_port; j++)
         {

            if (input_str_data[j].buf_ptr[0].actual_data_len == 0)
            {

               is_actual_data_len_nonzero &= FALSE;
            }
         }

         if (!is_actual_data_len_nonzero)
         {
            break;
         }

         result = CAPI_EOK;
         uint32_t input_bytes_given[CAPI_MAX_INPUT_PORTS];
         for (uint32_t j = 0; j < module->num_input_port; j++)
         {

            input_bytes_given[j] = input_str_data[j].buf_ptr[0].actual_data_len;
         }
         // ------------------------
         // Begin profiling
         // ------------------------
         prof_cycles = hexagon_sim_read_pcycles();
         // ------------------------
         // Call Processing function
         // ------------------------
         if (module->is_enabled)
         {
            result = module->module_ptr->vtbl_ptr->process(module->module_ptr,
                                                           (capi_stream_data_t **)input,
                                                           (capi_stream_data_t **)output);
         }
         else
         {

            uint32_t count =
               (module->num_input_port > module->num_output_port) ? module->num_output_port : module->num_input_port;
            for (uint32_t i = 0; i < count; i++)
            {
               for (uint32_t ch = 0; ch < input_str_data[i].bufs_num; ch++)
               {
                  memcpy(output_str_data[i].buf_ptr[ch].data_ptr,
                         input_str_data[i].buf_ptr[ch].data_ptr,
                         input_str_data[i].buf_ptr[ch].actual_data_len);
                  output_str_data[i].buf_ptr[ch].actual_data_len = input_str_data[i].buf_ptr[ch].actual_data_len;
               }
            }
            result = CAPI_EOK;
         }

         // ------------------------
         // Complete profiling
         // ------------------------
         // Diff Current Cycle State against previously acquired to
         // check Cycles elapsed

         prof_cycles = hexagon_sim_read_pcycles() - prof_cycles;
         // Number of samples produced per channel
         if (!IsCompressedData(&module->out_format[0]))
         {
            POSAL_ASSERT(module->out_format[0].actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);
            tst_standard_media_format_v2 *tst_format =
               reinterpret_cast<tst_standard_media_format_v2 *>(module->out_format[0].data_ptr);
            uint32_t SampleCnt = 0;

            if (tst_format) {
               SampleCnt = (output_str_data[0].buf_ptr[0].actual_data_len * output[0]->bufs_num /
                     (bits_to_bytes(tst_format->f.bits_per_sample, tst_format->f.q_factor) * tst_format->f.num_channels));
            } else {
               /* Output foramt is null, try with input format */
               tst_format = reinterpret_cast<tst_standard_media_format_v2 *>(module->in_format[0].data_ptr);
               SampleCnt = (input_str_data[0].buf_ptr[0].actual_data_len * input[0]->bufs_num /
                      (bits_to_bytes(tst_format->f.bits_per_sample, tst_format->f.q_factor) * tst_format->f.num_channels));
            }

            // Begin recording after a few iterations, for steady state

            if (i > begin_steady_state_iteration)
            {
               prof_num_samples += SampleCnt;
               prof_total_cycles += prof_cycles;
               if (SampleCnt > 0)
               {
                  prof_mips = ((float)(((0.000001 * tst_format->f.sampling_rate))) / SampleCnt) * prof_cycles;
                  prof_mips = prof_mips / num_hw_threads;

                  if (prof_mips > prof_peak_mips)
                  {
                     prof_peak_mips = prof_mips;
                     prof_max_iter  = i;
                  }
               }
            }
         }

         // Now check if the process function went through fine.
         if (CAPI_FAILED(result)) // TODO:MS13: Need to handle ENEEDMORE returned from module process
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Process failed with error %d.", (int)result);
            break;
         }

         result = CheckInputOutputSizes(input,
                                        &input_bytes_given[0],
                                        output,
                                        module->requires_data_buffering,
                                        module->num_input_port,
                                        module->num_output_port);

         if (CAPI_FAILED(result))
         {
            break;
         }

         for (uint32_t i = 0; i < module->num_output_port; i++)
         {
            result |= DumpOutputToFile(output[i], module, temp_out_buffer[i], i);
         }
         if (CAPI_FAILED(result))
         {
            break;
         }

         // Adjust the input buffers if they are not completely consumed.
         for (uint32_t j = 0; j < module->num_input_port; j++)
         {
            for (uint32_t ch = 0; ch < input_str_data[j].bufs_num; ch++)
            {
               capi_buf_t *buf_ptr            = &(input_str_data[j].buf_ptr[ch]);
               uint32_t    num_bytes_consumed = buf_ptr->actual_data_len;
               memsmove(buf_ptr->data_ptr,
                        input_bytes_given[j] - num_bytes_consumed,
                        buf_ptr->data_ptr + num_bytes_consumed,
                        input_bytes_given[j] - num_bytes_consumed);
               buf_ptr->actual_data_len = input_bytes_given[j] - num_bytes_consumed;
            }
         }
      }
      if (CAPI_FAILED(result))
      {
         break;
      }

      posal_memory_aligned_free(md_ptr1->obj_ptr);
      posal_memory_aligned_free(md_ptr2->obj_ptr);
      posal_memory_aligned_free(md_ptr3->obj_ptr);

      posal_memory_aligned_free(md_ptr1);
      posal_memory_aligned_free(md_ptr2);
      posal_memory_aligned_free(md_ptr3);
   }
// ------------------------
// Display profiling information
// ------------------------
#if ((defined __hexagon__) || (defined __qdsp6__))
   if (!IsCompressedData(&module->out_format[0]))
   {
      POSAL_ASSERT(module->out_format[0].actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);
      tst_standard_media_format_v2 *tst_format =
         reinterpret_cast<tst_standard_media_format_v2 *>(module->out_format[0].data_ptr);
      if (!tst_format) {
         /* Output format not found, try with intput format */
         tst_format = reinterpret_cast<tst_standard_media_format_v2 *>(module->in_format[0].data_ptr);
      }
      AR_MSG(DBG_HIGH_PRIO, "-------------------------------");
      AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Profiling information");
      AR_MSG(DBG_HIGH_PRIO,
             "CAPI TEST: Average MIPS: %.3f",
             (float)((((0.000001 * tst_format->f.sampling_rate) / prof_num_samples) * prof_total_cycles) /
                     num_hw_threads));
      AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Peak MIPS: %.3f occurred at iteration# %lld", prof_peak_mips, prof_max_iter);
      AR_MSG(DBG_HIGH_PRIO, "-------------------------------");
   }
   else
   {
      AR_MSG(DBG_HIGH_PRIO, "-------------------------------");
      AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Profiling information");
      AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Output format is compressed. Profiling not supported.");
      AR_MSG(DBG_HIGH_PRIO, "-------------------------------");
   }
#endif // __qdsp6__

// ------------------------
// Clear processing buffers
// ------------------------
done:
   for (uint32_t i = 0; i < module->num_input_port; i++)
   {
      if (NULL != input_buffer[i])
      {
         posal_memory_free(input_buffer[i]);
         input_buffer[i] = NULL;
      }
      if (NULL != temp_in_buffer[i])
      {
         posal_memory_free(temp_in_buffer[i]);
         temp_in_buffer[i] = NULL;
      }
   }
   for (uint32_t i = 0; i < module->num_output_port; i++)
   {
      if (NULL != output_buffer[i])
      {
         posal_memory_free(output_buffer[i]);
         output_buffer[i] = NULL;
      }
      if (NULL != temp_out_buffer[i])
      {
         posal_memory_free(temp_out_buffer[i]);
         temp_out_buffer[i] = NULL;
      }
   }

   return result;
}

capi_err_t SetParamInband(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing Set Params command.");

   capi_err_t result = CAPI_EOK;

   uint8_t *                pPacket = NULL;
   int8_t *                 ptr;
   uint32_t                 size;
   uint32_t                 packetSize = 0;
   apm_module_param_data_t *pHeader;

   result = GetUIntParameter(module->fCfg, "PayloadSizeInBytes", &(packetSize));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: SetParam command failed to read PayloadSizeInBytes.");
      goto done;
   }

   pPacket = (uint8_t *)(posal_memory_malloc(packetSize, POSAL_HEAP_DEFAULT));
   if (NULL == pPacket)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: SetParam command failed to allocate packet.");
      result = CAPI_ENOMEMORY;
      goto done;
   }

   result = ReadBufferContents(module->fCfg, packetSize, pPacket);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: SetParam command failed to read packet contents.");
      goto done;
   }

   ptr  = reinterpret_cast<int8_t *>(pPacket);
   size = packetSize;

   // Skip the apm set param packet header
   ptr += sizeof(apm_cmd_header_t);
   size -= sizeof(apm_cmd_header_t);

   pHeader = (apm_module_param_data_t *)(ptr);
   ptr += sizeof(apm_module_param_data_t);
   size -= sizeof(apm_module_param_data_t);

   {
      capi_port_info_t port_info;
      port_info.is_valid = false;
      // Send set params to module
      capi_buf_t param_buf;
      param_buf.data_ptr        = ptr;
      param_buf.actual_data_len = pHeader->param_size;
      param_buf.max_data_len    = size;
      result = module->module_ptr->vtbl_ptr->set_param(module->module_ptr, pHeader->param_id, &port_info, &param_buf);

      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set params, result=%d", (int)result);
         goto done;
      }
   }
done:

   if (NULL != pPacket)
   {
      posal_memory_free(pPacket);
      pPacket = NULL;
   }

   return result;
}

/* call setParam with data read from a file
 *
 *   Form of SetParamFromFile config file command is:
 *        4-byte Module Id
 *        4-byte Param Id
 *        n-byte string containing file path
 *   E.g.
 *   SetParamFromFile
 *        02 2D 01 00                # MODULE_ID 0x00012D02
 *        14 2C 01 00                # param_id to use for setParam call
 *        ..\..\data\sound_model.bin # name of file to be copied into mem
 *
 *   This function
 *       parses module and param ids
 *       parses file name
 *       gets size file
 *       allocates memory
 *       copies data from file into memory
 *       calls setParam
 */
capi_err_t SetParamFromFile(module_info_t* module)
{
	char filename_param_payload[128] = { 0 };
	capi_err_t result = CAPI_EOK;
    uint32_t payload_bytes_to_read = 0;
	uint32_t num_bytes_read = 0;
	int8_t* p_param_data;
	FILE* file_handle;
	uint8_t module_id_u8_array[4];
    //uint32_t *module_id_u32;
	uint8_t param_id_u8_array[4];
    uint32_t *param_id_u32;
	uint8_t* pPacket = NULL;
	int pathIndx = 0;

	// get module id
	result = ReadBufferContents(module->fCfg, 4, module_id_u8_array);
	if (CAPI_EOK != result) {
	    FARF(ERROR, "CAPI TEST: SetParamFromFile failed to read module id.      ");
	    goto done;
	}

//    module_id_u32 = (uint32_t *)module_id_u8_array;
	FARF(HIGH, "CAPI TEST: SetParamFromFile moduleId = 0x%x              ",
			 *module_id_u32);
	// get parameter id
	result = ReadBufferContents(module->fCfg, 4, param_id_u8_array);
	if (CAPI_EOK != result) {
	    FARF(ERROR, "CAPI TEST: SetParamFromFile failed to read param id.      ");
	    goto done;
	}
    param_id_u32 = (uint32_t *)param_id_u8_array;
	FARF(HIGH, "CAPI TEST: SetParamFromFile paramId = 0x%x              ",
			 *param_id_u32);

    // get file path string
#ifdef ENABLE_COMMAND_LINE_PARAMS
	strncpy(filename_param_payload, "..\\", 3);
	pathIndx = 3;
#else
	pathIndx = 0;
#endif
	result = GetWord(module->fCfg, &filename_param_payload[pathIndx]);
	if (CAPI_EOK != result) {
		FARF(ERROR, "CAPI TEST: SetParamFromFile failed for find file name.    ");
		return CAPI_EBADPARAM;
	}

	FARF(HIGH, "CAPI TEST: SetParamFromFile name = %s                          ",
			 filename_param_payload);

	if ((file_handle = fopen(filename_param_payload, "rb")) == NULL) {
	    FARF(ERROR, "Cannot open param payload file                             ");
	    return CAPI_EFAILED;
	}

	// get file size
	if (fseek(file_handle, 0 , SEEK_END) != 0) {
		FARF(ERROR, "Param payload file seek to end failed                             ");
		return CAPI_EFAILED;
	}

	payload_bytes_to_read = ftell(file_handle);
	if (payload_bytes_to_read == -1) {
		FARF(ERROR, "Param payload file tell failed                             ");
		return CAPI_EFAILED;
	}
	FARF(HIGH, "Param payload file size, number of bytes to read = %d          ",
			  payload_bytes_to_read);
	if (fseek(file_handle, 0 , SEEK_SET) != 0) {
		FARF(ERROR, "Param payload file rewind to beginning failed                  ");
		return CAPI_EFAILED;
	}
	pPacket = (uint8_t *)(posal_memory_malloc(payload_bytes_to_read, POSAL_HEAP_DEFAULT));

	if (NULL == pPacket) {
	    FARF(ERROR, "Memory allocation error for payload from file");
	    return CAPI_ENOMEMORY;
	}
	p_param_data = (int8_t*)pPacket;
	FARF(HIGH, "ptr to Param payload data = %p                                ",
			  p_param_data);
	num_bytes_read = fread(p_param_data, sizeof(int8_t), payload_bytes_to_read, file_handle);
	if (num_bytes_read != payload_bytes_to_read) {
		FARF(ERROR, "only %d bytes read from Param payload file                  ", num_bytes_read);
		result = CAPI_EFAILED;
		goto done;
	}

   {
      capi_port_info_t port_info;
      port_info.is_valid = false;
      // Send set params to module
      capi_buf_t param_buf;
      param_buf.data_ptr        = p_param_data;
      param_buf.actual_data_len = num_bytes_read;
      param_buf.max_data_len    = payload_bytes_to_read;
      result = module->module_ptr->vtbl_ptr->set_param(module->module_ptr, *param_id_u32, &port_info, &param_buf);

      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set params from file, result=%d", (int)result);
         goto done;
      }
   }

  done:
	if (NULL != pPacket) {
      posal_memory_free(pPacket);
      pPacket = NULL;
	}
  return result;
}

/* call RegisterPersistentParam with data read from a file
 *
 *   Form of RegisterPersistFileParam config file command is:
 *        4-byte Persistence Type
 *        4-byte Module Id
 *        4-byte Param Id
 *        n-byte string containing file path
 *   E.g.
 *   RegisterPersistFileParam
 *        00 00 00 00                # Persistence type 0 : Shared 1 : Golbal shared
 *        02 2D 01 00                # MODULE_ID 0x00012D02
 *        14 2C 01 00                # param_id to use for setParam call
 *        ..\..\data\sound_model.bin # name of file to be copied into mem
 *
 *   This function
 *       parses module and param ids
 *       parses file name
 *       gets size file
 *       allocates memory
 *       copies data from file into memory
 *       calls RegisterPersistentParam
 */
capi_err_t RegisterPersistFileParam(module_info_t* module)
{
	char filename_param_payload[128] = { 0 };
	capi_err_t result = CAPI_EOK;
    uint32_t payload_bytes_to_read = 0;
	uint32_t num_bytes_read = 0;
	int8_t* p_param_data;
	FILE* file_handle;
	uint8_t persistence_u8_array[4];
    uint32_t *persistence_info_u32;
	uint8_t module_id_u8_array[4];
    //uint32_t *module_id_u32;
	uint8_t param_id_u8_array[4];
    uint32_t *param_id_u32;
	uint8_t* pPacket = NULL;
	int pathIndx = 0;
    uint32_t param_index = 0, existing_entry_found = 0;

	// get persistence info
	result = ReadBufferContents(module->fCfg, 4, persistence_u8_array);
	if (CAPI_EOK != result) {
	    FARF(ERROR, "CAPI TEST: SetParamFromFile failed to read persistenceInfo.      ");
	    goto done;
	}
    persistence_info_u32 = (uint32_t *)persistence_u8_array;
    if(0 == *persistence_info_u32 || 1 == *persistence_info_u32)
    {
		FARF(HIGH, "CAPI TEST: SetParamFromFile persistenceInfo = 0x%x              ",
				 *persistence_info_u32);
	}
	else
    {
		FARF(ERROR, "CAPI TEST: SetParamFromFile persistenceInfo = 0x%x Invalid             ",
				 *persistence_info_u32);
	    goto done;
	}

	// get module id
	result = ReadBufferContents(module->fCfg, 4, module_id_u8_array);
	if (CAPI_EOK != result) {
	    FARF(ERROR, "CAPI TEST: SetParamFromFile failed to read module id.      ");
	    goto done;
	}
    //module_id_u32 = (uint32_t *)module_id_u8_array;
	FARF(HIGH, "CAPI TEST: SetParamFromFile moduleId = 0x%x              ",
			 *module_id_u32);
	// get parameter id
	result = ReadBufferContents(module->fCfg, 4, param_id_u8_array);
	if (CAPI_EOK != result) {
	    FARF(ERROR, "CAPI TEST: SetParamFromFile failed to read param id.      ");
	    goto done;
	}
    param_id_u32 = (uint32_t *)param_id_u8_array;
	FARF(HIGH, "CAPI TEST: SetParamFromFile paramId = 0x%x              ",
			 *param_id_u32);

    // get file path string
#ifdef ENABLE_COMMAND_LINE_PARAMS
	strncpy(filename_param_payload, "..\\", 3);
	pathIndx = 3;
#else
	pathIndx = 0;
#endif
	result = GetWord(module->fCfg, &filename_param_payload[pathIndx]);
	if (CAPI_EOK != result) {
		FARF(ERROR, "CAPI TEST: SetParamFromFile failed for find file name.    ");
		return CAPI_EBADPARAM;
	}

	FARF(HIGH, "CAPI TEST: SetParamFromFile name = %s                          ",
			 filename_param_payload);

	if ((file_handle = fopen(filename_param_payload, "rb")) == NULL) {
	    FARF(ERROR, "Cannot open param payload file                             ");
	    return CAPI_EFAILED;
	}

	// get file size
	if (fseek(file_handle, 0 , SEEK_END) != 0) {
		FARF(ERROR, "Param payload file seek to end failed                             ");
		return CAPI_EFAILED;
	}

	payload_bytes_to_read = ftell(file_handle);
	if (payload_bytes_to_read == -1) {
		FARF(ERROR, "Param payload file tell failed                             ");
		return CAPI_EFAILED;
	}
	FARF(HIGH, "Param payload file size, number of bytes to read = %d          ",
			  payload_bytes_to_read);
	if (fseek(file_handle, 0 , SEEK_SET) != 0) {
		FARF(ERROR, "Param payload file rewind to beginning failed                  ");
		return CAPI_EFAILED;
	}
	pPacket = (uint8_t *)(posal_memory_malloc(payload_bytes_to_read, POSAL_HEAP_DEFAULT));

	if (NULL == pPacket) {
	    FARF(ERROR, "Memory allocation error for payload from file");
	    return CAPI_ENOMEMORY;
	}
	p_param_data = (int8_t*)pPacket;

   for (param_index = 0; param_index < MAX_PERSISTENT_PARAMS_PER_MODULE; param_index++)
   {
      if (persistent_param_payload_array[param_index].param_id == *param_id_u32)
      {
         /* Overriding an existing entry of same param id as persistent param */
         posal_memory_free(persistent_param_payload_array[param_index].data_payload);
         persistent_param_payload_array[param_index].data_payload = (uint8_t*)p_param_data;
         existing_entry_found                                     = 1;
         break;
      }
   }

   if (!existing_entry_found)
   {
      /* Loop through persistent params global table for this module to find a free entry to cache */
      for (param_index = 0; param_index < MAX_PERSISTENT_PARAMS_PER_MODULE; param_index++)
      {
         if (!persistent_param_payload_array[param_index].is_entry_used)
         {
            /* Upon finding a free entry, cache the entry */
            persistent_param_payload_array[param_index].data_payload  = (uint8_t*)p_param_data;
            persistent_param_payload_array[param_index].param_id      = *param_id_u32;
            persistent_param_payload_array[param_index].is_entry_used = TRUE;
            break;
         }
      }
      /* Unused entry not found, list full - can't set anymore persistent params */
      if (param_index == MAX_PERSISTENT_PARAMS_PER_MODULE)
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI TEST: RegisterPersistFileParam : exceeded %d persistent params limit, not setting to module.",
                MAX_PERSISTENT_PARAMS_PER_MODULE);
         result = CAPI_EUNSUPPORTED;
         goto done;
      }
   }

   /* STEP 1 : Set persistence param property to the module to register */
   {
      capi_param_persistence_info_t pers_info;
      pers_info.mem_type    = (capi_persistence_type_t) * ((uint32_t *)persistence_info_u32);
      pers_info.is_register = TRUE;
      pers_info.param_id    = *param_id_u32;
      capi_prop_t prop;
      prop.id                      = CAPI_PARAM_PERSISTENCE_INFO;
      prop.payload.data_ptr        = (int8_t *)(&pers_info);
      prop.payload.actual_data_len = sizeof(pers_info);
      prop.payload.max_data_len    = prop.payload.actual_data_len;
      prop.port_info.is_valid      = FALSE;

      capi_proplist_t proplist = { 1, &prop };
      result                   = module->module_ptr->vtbl_ptr->set_properties(module->module_ptr, &proplist);

      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set persistent param property, result=%d", (int)result);
         return result;
      }
   }

   /* STEP 2 : Set actual param to the module */
	FARF(HIGH, "ptr to Param payload data = %p                                ",
			  p_param_data);
	num_bytes_read = fread(p_param_data, sizeof(int8_t), payload_bytes_to_read, file_handle);
	if (num_bytes_read != payload_bytes_to_read) {
		FARF(ERROR, "only %d bytes read from Param payload file                  ", num_bytes_read);
		result = CAPI_EFAILED;
		goto done;
	}

   {
      capi_port_info_t port_info;
      port_info.is_valid = false;
      // Send set params to module
      capi_buf_t param_buf;
      param_buf.data_ptr        = p_param_data;
      param_buf.actual_data_len = num_bytes_read;
      param_buf.max_data_len    = payload_bytes_to_read;
      result = module->module_ptr->vtbl_ptr->set_param(module->module_ptr, *param_id_u32, &port_info, &param_buf);

      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set params from file, result=%d", (int)result);
         goto done;
      }
      // For persist, return from here for success case to avoid freeing persistent param memory.
      return result;
   }

  done:
	if (NULL != pPacket) {
      posal_memory_free(pPacket);
      pPacket = NULL;
	}
  return result;
}

capi_err_t RegisterPersistentParam(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing Register Persistent Params command.");

   capi_err_t result = CAPI_EOK;

   uint8_t *                pPacket = NULL;
   int8_t *                 ptr;
   uint32_t                 size;
   uint32_t                 packetSize = 0, param_index = 0, existing_entry_found = 0;
   capi_persistence_type_t  persistence_type;
   apm_module_param_data_t *pHeader;

   result = GetUIntParameter(module->fCfg, "PayloadSizeInBytes", &(packetSize));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: RegisterPersistentParam command failed to read PayloadSizeInBytes.");
      return result;
   }

   pPacket = (uint8_t *)(posal_memory_malloc(packetSize, POSAL_HEAP_DEFAULT));
   if (NULL == pPacket)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: RegisterPersistentParam command failed to allocate packet.");
      result = CAPI_ENOMEMORY;
      return result;
   }

   result = ReadBufferContents(module->fCfg, packetSize, pPacket);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: RegisterPersistentParam command failed to read packet contents.");
      return result;
   }

   ptr  = reinterpret_cast<int8_t *>(pPacket);
   size = packetSize;

   persistence_type = (capi_persistence_type_t) * ((uint32_t *)ptr);
   ptr += sizeof(uint32_t);
   // Skip the pp set param packet header
   ptr += sizeof(apm_cmd_header_t);
   size -= sizeof(apm_cmd_header_t);

   pHeader = (apm_module_param_data_t *)(ptr);
   ptr += sizeof(apm_module_param_data_t);
   size -= sizeof(apm_module_param_data_t);

   for (param_index = 0; param_index < MAX_PERSISTENT_PARAMS_PER_MODULE; param_index++)
   {
      if (persistent_param_payload_array[param_index].param_id == pHeader->param_id)
      {
         /* Overriding an existing entry of same param id as persistent param */
         posal_memory_free(persistent_param_payload_array[param_index].data_payload);
         persistent_param_payload_array[param_index].data_payload = pPacket;
         existing_entry_found                                     = 1;
         break;
      }
   }

   if (!existing_entry_found)
   {
      /* Loop through persistent params global table for this module to find a free entry to cache */
      for (param_index = 0; param_index < MAX_PERSISTENT_PARAMS_PER_MODULE; param_index++)
      {
         if (!persistent_param_payload_array[param_index].is_entry_used)
         {
            /* Upon finding a free entry, cache the entry */
            persistent_param_payload_array[param_index].data_payload  = pPacket;
            persistent_param_payload_array[param_index].param_id      = pHeader->param_id;
            persistent_param_payload_array[param_index].is_entry_used = TRUE;
            break;
         }
      }
      /* Unused entry not found, list full - can't set anymore persistent params */
      if (param_index == MAX_PERSISTENT_PARAMS_PER_MODULE)
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI TEST: RegisterPersistentParam : exceeded %d persistent params limit, not setting to module.",
                MAX_PERSISTENT_PARAMS_PER_MODULE);
         result = CAPI_EUNSUPPORTED;
         posal_memory_free(pPacket);
         return result;
      }
   }

   /* STEP 1 : Set persistence param property to the module to register */
   {
      capi_param_persistence_info_t pers_info;
      pers_info.mem_type    = persistence_type;
      pers_info.is_register = TRUE;
      pers_info.param_id    = pHeader->param_id;
      capi_prop_t prop;
      prop.id                      = CAPI_PARAM_PERSISTENCE_INFO;
      prop.payload.data_ptr        = (int8_t *)(&pers_info);
      prop.payload.actual_data_len = sizeof(pers_info);
      prop.payload.max_data_len    = prop.payload.actual_data_len;
      prop.port_info.is_valid      = FALSE;

      capi_proplist_t proplist = { 1, &prop };
      result                   = module->module_ptr->vtbl_ptr->set_properties(module->module_ptr, &proplist);

      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set persistent param property, result=%d", (int)result);
         return result;
      }
   }
   /* STEP 2 : Set actual param to the module */
   {
      capi_port_info_t port_info;
      port_info.is_valid = false;
      // Send set params to module
      capi_buf_t param_buf;
      param_buf.data_ptr        = ptr;
      param_buf.actual_data_len = pHeader->param_size;
      param_buf.max_data_len    = size;
      result = module->module_ptr->vtbl_ptr->set_param(module->module_ptr, pHeader->param_id, &port_info, &param_buf);

      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set params, result=%d", (int)result);
         return result;
      }
   }

   return CAPI_EOK;
}

capi_err_t DeRegisterPersistentParam(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing DeRegisterPersistentParam command.");

   capi_err_t result = CAPI_EOK;

   uint8_t                 pPacket[20] = { 0 };
   int8_t *                ptr;
 //  uint32_t                size;
   uint32_t                packetSize = 0, param_index = 0;
   capi_persistence_type_t persistence_type;
   apm_module_param_data_t pHeader;

   result = GetUIntParameter(module->fCfg, "PayloadSizeInBytes", &(packetSize));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: DeRegisterPersistentParam command failed to read PayloadSizeInBytes.");
      return result;
   }

   result = ReadBufferContents(module->fCfg, packetSize, pPacket);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: DeRegisterPersistentParam command failed to read packet contents.");
      return result;
   }

   ptr              = reinterpret_cast<int8_t *>(pPacket);
 //  size             = packetSize;
   persistence_type = (capi_persistence_type_t) * ((uint32_t *)ptr);
   ptr += sizeof(uint32_t);
   pHeader.param_id = *((uint32_t *)ptr);

   /* STEP 1 : Set persistence param property to the module to deregister */
   {
      capi_param_persistence_info_t pers_info;
      pers_info.mem_type    = persistence_type;
      pers_info.is_register = FALSE;
      pers_info.param_id    = pHeader.param_id;
      capi_prop_t prop;
      prop.id                      = CAPI_PARAM_PERSISTENCE_INFO;
      prop.payload.data_ptr        = (int8_t *)(&pers_info);
      prop.payload.actual_data_len = sizeof(pers_info);
      prop.payload.max_data_len    = prop.payload.actual_data_len;
      prop.port_info.is_valid      = FALSE;

      capi_proplist_t proplist = { 1, &prop };
      result                   = module->module_ptr->vtbl_ptr->set_properties(module->module_ptr, &proplist);

      /* Check for valid entry in persistent params list, free memory if found */
      for (param_index = 0; param_index < MAX_PERSISTENT_PARAMS_PER_MODULE; param_index++)
      {
         if (pHeader.param_id == persistent_param_payload_array[param_index].param_id)
         {
            posal_memory_free(persistent_param_payload_array[param_index].data_payload);
            persistent_param_payload_array[param_index].data_payload  = NULL;
            persistent_param_payload_array[param_index].is_entry_used = 0;
            persistent_param_payload_array[param_index].param_id      = 0;
            break;
         }
      }
      /* Unused entry not found, list full - can't set anymore persistent params */
      if (param_index == MAX_PERSISTENT_PARAMS_PER_MODULE)
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI TEST: DeRegisterPersistentParam : param id %x not registered as persistent or already "
                "deregistered.",
                pHeader.param_id);
      }

      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI TEST: Cannot set persistent param property for param %x, result=%d",
                pHeader.param_id,
                (int)result);
         return result;
      }
   }

   return CAPI_EOK;
}
capi_err_t GetParamInband(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing Get Params command.");

   capi_err_t result = CAPI_EOK;
   uint32_t   i;
   capi_buf_t param_buf;

   int8_t *pPayload = NULL;

   uint32_t payloadSize = 0;
   uint32_t param_id    = 0;
   uint32_t param_size  = 0;

   result = GetUIntParameter(module->fCfg, "PayloadSizeInBytes", &payloadSize);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: GetParam command failed to read PayloadSizeInBytes");
      goto done;
   }

   // Read the module id and param id
   typedef struct dummy_t
   {
      apm_cmd_header_t        a;
      apm_module_param_data_t b;
   } dummy_t; // amith TBD
   union
   {
      dummy_t h;
      uint8_t byte_stream[sizeof(dummy_t)];
   } header;

   POSAL_ASSERT(payloadSize == sizeof(header.byte_stream));
   for (i = 0; i < sizeof(header.byte_stream); i++)
   {
      char word[wordSize];
      result = GetWord(module->fCfg, word);
      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: GetParam command failed to read payload contents");
         goto done;
      }

      int value;
      sscanf(word, "%x", &value);

      header.byte_stream[i] = (value & 0xFF);
   }

   param_id   = header.h.b.param_id;
   param_size = header.h.b.param_size;

   pPayload = (int8_t *)posal_memory_malloc(param_size, POSAL_HEAP_DEFAULT);
   if (NULL == pPayload)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: GetParam command failed to allocate packet");
      result = CAPI_ENOMEMORY;
      goto done;
   }

   {
      // Send set params to module
      param_buf.data_ptr        = pPayload;
      param_buf.actual_data_len = 0;
      param_buf.max_data_len    = param_size;

      capi_port_info_t port_info;
      port_info.is_valid = false;

      result = module->module_ptr->vtbl_ptr->get_param(module->module_ptr, param_id, &port_info, &param_buf);

      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot get params, result=%d", (int)result);
         goto done;
      }

      AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Done Get parameters for Param ID: 0x%lx", param_id);
   }

   // Compare the payload with reference
   result = GetUIntParameter(module->fCfg, "RefPayloadSizeInBytes", &payloadSize);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: GetParam command has no reference for comparison");
      result = CAPI_EOK;
      goto done;
   }

   if (param_size != param_buf.actual_data_len)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "CAPI TEST: GetParam return packet size %lu does not match reference size %lu",
             param_buf.actual_data_len,
             param_size);
      result = CAPI_EFAILED;
      goto done;
   }

   {
      for (i = 0; i < payloadSize; i++)
      {
         char word[wordSize];
         result = GetWord(module->fCfg, word);
         if (CAPI_EOK != result)
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: GetParam command failed to read ref payload contents");
            goto done;
         }

         int value;
         sscanf(word, "%x", &value);

         int8_t refVal = (value & 0xFF);
         if (pPayload[i] != refVal)
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: GetParam packet does not match reference in byte %lu", i);
            result = CAPI_EFAILED;
            goto done;
         }
      }
   }

done:
   if (NULL != pPayload)
   {
      posal_memory_free(pPayload);
      pPayload = NULL;
   }

   return result;
}

capi_err_t SetMediaFormat(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing Set Media format with channel mapping command.");
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: CAPI_INPUT_LENGTH_MS = %d", CAPI_INPUT_LENGTH_MS);
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: CAPI_OUTPUT_LENGTH_MS = %d", CAPI_OUTPUT_LENGTH_MS);

   capi_err_t result = CAPI_EOK;

   tst_standard_media_format_v2 in_format;

   in_format.f.minor_version = CAPI_MEDIA_FORMAT_MINOR_VERSION;

   uint32_t port_index = 0;

   result = GetUIntParameter(module->fCfg, "SetBitstreamFormat", &(in_format.f.bitstream_format));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read bitstream format.");
      return CAPI_EBADPARAM;
   }

   {
      uint32_t temp;
      result = GetUIntParameter(module->fCfg, "SetDataFormat", &(temp));
      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read data format.");
         return CAPI_EBADPARAM;
      }
      in_format.h.data_format = static_cast<data_format_t>(temp);
   }

   result = GetUIntParameter(module->fCfg, "SetNumChannelsAndMapping", &(in_format.f.num_channels));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read num in channels.");
      return CAPI_EBADPARAM;
   }

   result = ReadChannelMapping(module->fCfg, in_format.f.num_channels, in_format.channel_type);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read channel mapping.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "SetBitsPerSample", &(in_format.f.bits_per_sample));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read bits per sample.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "QFactor", &(in_format.f.q_factor));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read q factor.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "SetSamplingRate", &(in_format.f.sampling_rate));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read sampling rate.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "SetIsSigned", &(in_format.f.data_is_signed));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read is signed.");
      return CAPI_EBADPARAM;
   }

   {
      uint32_t temp;
      result = GetUIntParameter(module->fCfg, "SetInterleaving", &temp);
      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read interleaving.");
         return CAPI_EBADPARAM;
      }
      in_format.f.data_interleaving = static_cast<capi_interleaving_t>(temp);
   }

   result = GetUIntParameter(module->fCfg, "SetPortIndex", &port_index);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Set Media format command failed to read Port Index. Setting Port index to 0");
      port_index = 0;
      result     = CAPI_EOK;
   }
   if (port_index >= module->num_input_port)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Setting the input media format for invalid port");
      return CAPI_EBADPARAM;
   }

   // Adjust input buffer sizes for new media formats
   uint32_t media_fmt_size =
      SPF_TEST_ALIGN_BY_4(TST_STD_MEDIA_FMT_V2_MIN_SIZE + in_format.f.num_channels * sizeof(in_format.channel_type));
   POSAL_ASSERT(sizeof(in_format) <= sizeof(module->in_format_buf[port_index]));
   module->in_format[port_index].actual_data_len =
      memscpy(module->in_format_buf[port_index], sizeof(module->in_format_buf[port_index]), &in_format, media_fmt_size);
   module->in_format[port_index].max_data_len = sizeof(module->in_format_buf[port_index]);
   module->in_format[port_index].data_ptr     = module->in_format_buf[port_index];

   if (IsCompressedData(&module->in_format[port_index]))
   {
      // amith TBD
      // ar_result_t result = IEC61937_get_buffer_size(in_format.f.bitstream_format,
      // &module->in_buffer_len[port_index]);
      // if (AR_EOK != result)
      // {
      //    AR_MSG(FARF(ERROR, Unable to get compressed buffer size, result=%d",
      //    result);
      //    return ar_result_to_capi_err(result);
      // }
   }
   else
   {
      if (in_format.f.data_interleaving == CAPI_DEINTERLEAVED_UNPACKED)
      {
         module->in_buffer_len[port_index] = (in_format.f.sampling_rate * CAPI_INPUT_LENGTH_MS / 1000) *
                                             bits_to_bytes(in_format.f.bits_per_sample, in_format.f.q_factor);
      }
      else
      {
         module->in_buffer_len[port_index] = (in_format.f.sampling_rate * CAPI_INPUT_LENGTH_MS / 1000) *
                                             bits_to_bytes(in_format.f.bits_per_sample, in_format.f.q_factor) *
                                             in_format.f.num_channels;
      }
   }

   // ------------------------
   // Send media type information
   // ------------------------
   {
      capi_prop_t     prop = { CAPI_INPUT_MEDIA_FORMAT_V2, module->in_format[port_index], { TRUE, TRUE, port_index } };
      capi_proplist_t proplist = { 1, &prop };

      result = module->module_ptr->vtbl_ptr->set_properties(module->module_ptr, &proplist);

      if ((CAPI_EOK != result) && (in_format.f.num_channels <= CAPI_MAX_CHANNELS))
      {

         tst_standard_media_format temp_in_format;
         capi_buf_t                temp_module_in_format;
         copy_media_fmt_v2_to_v1(&temp_in_format, &in_format);

         // Adjust input buffer sizes for new media formats
         POSAL_ASSERT(sizeof(temp_in_format) <= sizeof(module->in_format_buf[port_index]));
         temp_module_in_format.actual_data_len = sizeof(tst_standard_media_format);
         temp_module_in_format.max_data_len    = temp_module_in_format.actual_data_len;
         temp_module_in_format.data_ptr        = (int8_t *)(&temp_in_format);

         prop.id      = CAPI_INPUT_MEDIA_FORMAT;
         prop.payload = temp_module_in_format;

         result = module->module_ptr->vtbl_ptr->set_properties(module->module_ptr, &proplist);
      }

      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set input media format, result=%d", (int)result);
         return result;
      }
      module->is_media_format_received[port_index] = TRUE;
   }

   return result;
}

// use for multi-port modules
capi_err_t RegisterToEvent(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Resgistering to module events");

   capi_err_t result      = CAPI_EOK;
   uint32_t   event_id    = 0;
   uint32_t   token       = 1234;
   uint32_t   is_register = 0;
   uint32_t   dest_addr   = 1234;

   result = GetUIntParameter(module->fCfg, "EventId", &event_id);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: RegisterToEvent command failed to read EventId.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "IsRegister", &is_register);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: RegisterToEvent command failed to read IsRegister.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "Token", &token);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: RegisterToEvent command failed to read Token.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "DestAddr", &dest_addr);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: RegisterToEvent command failed to read DestAddr");
      return CAPI_EBADPARAM;
   }

   capi_register_event_to_dsp_client_v2_t reg_event;
   reg_event.event_id           = event_id;
   reg_event.token              = token;
   reg_event.is_register        = is_register;
   reg_event.dest_address       = dest_addr;
   reg_event.event_cfg.data_ptr = NULL;

   capi_buf_t buf1;
   buf1.data_ptr        = (int8_t *)(&reg_event);
   buf1.actual_data_len = sizeof(capi_register_event_to_dsp_client_v2_t);
   buf1.max_data_len    = sizeof(capi_register_event_to_dsp_client_v2_t);

   capi_port_info_t port_info; // immaterial for setting max ports info
   port_info.is_valid      = FALSE;
   port_info.is_input_port = TRUE;
   port_info.port_index    = 0;

   capi_prop_t     prop     = { CAPI_REGISTER_EVENT_DATA_TO_DSP_CLIENT_V2, buf1, port_info };
   capi_proplist_t proplist = { 1, &prop };

   result = module->module_ptr->vtbl_ptr->set_properties(module->module_ptr, &proplist);

   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "CAPI TEST: Cannot Register/deregister event id 0x%lx with module, result=%d",
             event_id,
             (int)result);
      return result;
   }
   return result;
}

// use for multi-port modules
capi_err_t SetMaxNumPorts(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing Set Num (MAX) port information.");

   capi_err_t result = CAPI_EOK;

   result = GetUIntParameter(module->fCfg, "SetMaxInputPorts", &(module->num_input_port));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: SetMaxInputPorts command failed to read num max input ports.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "SetMaxOutputPorts", &(module->num_output_port));
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: SetMaxOutputPorts command failed to read num max output ports.");
      return CAPI_EBADPARAM;
   }

   capi_port_num_info_t port_num_info_t;
   port_num_info_t.num_input_ports  = module->num_input_port;
   port_num_info_t.num_output_ports = module->num_output_port;
   capi_buf_t buf1;
   buf1.data_ptr        = (int8_t *)(&port_num_info_t);
   buf1.actual_data_len = sizeof(capi_port_num_info_t);
   buf1.max_data_len    = sizeof(capi_port_num_info_t);

   capi_port_info_t port_info; // immaterial for setting max ports info
   port_info.is_valid      = FALSE;
   port_info.is_input_port = TRUE;
   port_info.port_index    = 0;

   capi_prop_t     prop     = { CAPI_PORT_NUM_INFO, buf1, port_info };
   capi_proplist_t proplist = { 1, &prop };

   result = module->module_ptr->vtbl_ptr->set_properties(module->module_ptr, &proplist);

   if ((module->num_input_port > 1) || (module->num_output_port > 1))
   {
      module->is_enabled = TRUE;
      AR_MSG(DBG_HIGH_PRIO, "Multiport modules are always triggered");
   }

   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set Max port info, result=%d", (int)result);
      return result;
   }
   return result;
}

capi_err_t SetDataPortOperation(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing Set PORT Operation.");

   capi_err_t result = CAPI_EOK;
   uint32_t   is_input_port;
   uint32_t   port_opcode; // open or close
   uint32_t   num_ports;

   result = GetUIntParameter(module->fCfg, "IsInputPort", &is_input_port);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST:  Cannot read is_input_port.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "PortOpCode", &port_opcode);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST:  Cannot read PortOpCode.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "NumPortsAndMaps", &num_ports);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST:  Cannot read NumPortsAndMaps.");
      return CAPI_EBADPARAM;
   }

   result = ReadIdIndexMapping(module->fCfg, num_ports, module->port_id_idx_map_arr);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST:NumPortsAndMaps command failed to read port id-idx mapping.");
      return CAPI_EBADPARAM;
   }

   capi_buf_t buf_port_operation;
   buf_port_operation.data_ptr =
      (int8_t *)posal_memory_malloc(sizeof(intf_extn_data_port_operation_t) +
                                       (num_ports * sizeof(intf_extn_data_port_id_idx_map_t)),
                                    POSAL_HEAP_DEFAULT);
   buf_port_operation.actual_data_len =
      sizeof(intf_extn_data_port_operation_t) + (num_ports * sizeof(intf_extn_data_port_id_idx_map_t));
   intf_extn_data_port_operation_t *op = (intf_extn_data_port_operation_t *)buf_port_operation.data_ptr;
   op->is_input_port                   = (bool_t)is_input_port;
   op->opcode                          = (intf_extn_data_port_opcode_t)port_opcode; // OPEN or CLOSE
   op->num_ports                       = num_ports;

   memcpy(buf_port_operation.data_ptr + sizeof(intf_extn_data_port_operation_t),
          module->port_id_idx_map_arr,
          num_ports * sizeof(intf_extn_data_port_id_idx_map_t));

   capi_port_info_t port_info; // immaterial for setting mapping
   port_info.is_valid      = FALSE;
   port_info.is_input_port = (bool_t)is_input_port;
   port_info.port_index    = 0;

   result = module->module_ptr->vtbl_ptr->set_param(module->module_ptr,
                                                    INTF_EXTN_PARAM_ID_DATA_PORT_OPERATION,
                                                    &port_info,
                                                    &buf_port_operation);

   if (CAPI_EOK != result)
   {
      posal_memory_free(buf_port_operation.data_ptr);
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set port Operation, result=%d", (int)result);
      return result;
   }
   posal_memory_free(buf_port_operation.data_ptr);

   return result;
}

capi_err_t SetControlPortOperation(module_info_t *module)
{
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing Set PORT Operation.");

   capi_err_t result        = CAPI_EOK;
   uint32_t   is_input_port = 0;
   uint32_t   port_opcode   = 0;
   uint32_t   port_id;
   //  uint32_t   port_index;
   uint32_t num_ports   = 1;
   uint32_t num_intents = 0;
   uint32_t intent_list[8];
   intent_list[0] = INTENT_ID_DUMMY_SP;

   result = GetUIntParameter(module->fCfg, "PortOpCode", &port_opcode);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST:  Cannot read PortOpCode.");
      return CAPI_EBADPARAM;
   }

   result = GetUIntParameter(module->fCfg, "PortID", &port_id);
   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST:  Cannot read PortID.");
      return CAPI_EBADPARAM;
   }

   uint32_t total_size = sizeof(intf_extn_param_id_imcl_port_operation_t);
   switch (port_opcode)
   {
      case INTF_EXTN_IMCL_PORT_OPEN:
      {

         result = GetUIntParameter(module->fCfg, "NumIntents", &num_intents);
         if (CAPI_EOK != result)
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI TEST:  Cannot read NumIntents.");
            return CAPI_EBADPARAM;
         }

         //         result = ReadUint32List(module->fCfg, num_intents, intent_list);
         if (CAPI_EOK != result)
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: failed to read intent id list");
            return CAPI_EBADPARAM;
         }

         total_size += sizeof(intf_extn_imcl_port_open_t);
         total_size += num_ports * sizeof(intf_extn_imcl_id_intent_map_t);
         total_size += num_intents * sizeof(uint32_t);
         break;
      }
      case INTF_EXTN_IMCL_PORT_CLOSE:
      case INTF_EXTN_IMCL_PORT_PEER_CONNECTED:
      case INTF_EXTN_IMCL_PORT_PEER_DISCONNECTED:
      {
         total_size += sizeof(intf_extn_imcl_port_close_t);
         total_size += num_ports * sizeof(uint32_t);
         break;
      }
      default:
         break;
   }

   capi_buf_t buf_port_operation;
   buf_port_operation.data_ptr        = (int8_t *)posal_memory_malloc(total_size, POSAL_HEAP_DEFAULT);
   buf_port_operation.actual_data_len = total_size;

   intf_extn_param_id_imcl_port_operation_t *op =
      (intf_extn_param_id_imcl_port_operation_t *)buf_port_operation.data_ptr;

   op->opcode                     = (intf_extn_imcl_port_opcode_t)port_opcode;
   op->op_payload.data_ptr        = buf_port_operation.data_ptr + sizeof(intf_extn_param_id_imcl_port_operation_t);
   op->op_payload.actual_data_len = total_size - sizeof(intf_extn_param_id_imcl_port_operation_t);
   op->op_payload.max_data_len    = total_size - sizeof(intf_extn_param_id_imcl_port_operation_t);

   switch (port_opcode)
   {
      case INTF_EXTN_IMCL_PORT_OPEN:
      {
         intf_extn_imcl_port_open_t *op_cfg = (intf_extn_imcl_port_open_t *)op->op_payload.data_ptr;
         op_cfg->num_ports                  = num_ports;
         for (uint32_t idx = 0; idx < num_ports; idx++)
         {
            op_cfg->intent_map[idx].port_id                 = port_id;
            op_cfg->intent_map[idx].peer_module_instance_id = 0;
            op_cfg->intent_map[idx].peer_port_id            = 0;
            op_cfg->intent_map[idx].num_intents             = num_intents;

            // intent id list per port
            memscpy(&op_cfg->intent_map[idx].intent_arr[0],
                    num_intents * sizeof(uint32_t),
                    &intent_list[0],
                    num_intents * sizeof(uint32_t));
         }

         break;
      }
      case INTF_EXTN_IMCL_PORT_CLOSE:
      case INTF_EXTN_IMCL_PORT_PEER_CONNECTED:
      case INTF_EXTN_IMCL_PORT_PEER_DISCONNECTED: // all these operations have same payload
      {
         intf_extn_imcl_port_close_t *op_cfg = (intf_extn_imcl_port_close_t *)op->op_payload.data_ptr;
         op_cfg->num_ports                   = num_ports;
         for (uint32_t idx = 0; idx < num_ports; idx++)
         {
            op_cfg->port_id_arr[idx] = port_id;
         }
         break;
      }
      default:
         break;
   }

   capi_port_info_t port_info; // immaterial for setting mapping
   port_info.is_valid      = FALSE;
   port_info.is_input_port = (bool_t)is_input_port;
   port_info.port_index    = 0;

   result = module->module_ptr->vtbl_ptr->set_param(module->module_ptr,
                                                    INTF_EXTN_PARAM_ID_IMCL_PORT_OPERATION,
                                                    &port_info,
                                                    &buf_port_operation);

   if (CAPI_EOK != result)
   {
      posal_memory_free(buf_port_operation.data_ptr);
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Cannot set control port Operation, result=%d", (int)result);
      return result;
   }
   posal_memory_free(buf_port_operation.data_ptr);

   return result;
}

capi_err_t ExecuteOneCommand(const testCommand *pTable,
                             const uint32_t     tableLength,
                             module_info_t *    module,
                             bool_t *           pEOFReached,
                             uint32_t           num_modules,
                             uint32_t           module_num)
{
   char     word[wordSize];
   bool_t   commandFound = FALSE;
   uint32_t commandIndex = 0;
   *pEOFReached          = FALSE;

   while (!commandFound)
   {
      capi_err_t result;

      result = GetWord(module->fCfg, word);
      if (CAPI_EOK != result)
      {
         AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: No more valid commands in file.");
         *pEOFReached = TRUE;
         return CAPI_EOK;
      }

      // Search for the command
      for (uint32_t i = 0; i < tableLength; i++)
      {
         if (0 == strncmp(word, pTable[i].opCode, wordSize))
         {
            commandFound = TRUE;
            commandIndex = i;
            break;
         }
      }
   }

   return ((pTable[commandIndex].pFunction)(module));
}

int32_t    g_num_modules = 1;
capi_err_t RunTest(module_info_t *    module,
                   uint32_t           num_modules,
                   const testCommand *pExtendedCmdSet,
                   const uint32_t     extendedCmdSetSize)
{
   capi_err_t            result       = CAPI_EOK;
   static const uint32_t MAX_COMMANDS = 50;
   testCommand           completeSet[MAX_COMMANDS];
   uint32_t              numCommands = 0, param_index = 0;

   g_num_modules = num_modules;
   // Create a complete command set
   uint32_t steadyStateSetSize = sizeof(steadyStateCommandSet) / sizeof(steadyStateCommandSet[0]);
   if (numCommands + steadyStateSetSize > MAX_COMMANDS)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Too many commands in steady state.");
      return CAPI_ENOMEMORY;
   }
   memcpy(completeSet + numCommands, steadyStateCommandSet, sizeof(testCommand) * steadyStateSetSize);
   numCommands += steadyStateSetSize;

   if (numCommands + extendedCmdSetSize > MAX_COMMANDS)
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Too many commands in extended set.");
      return CAPI_ENOMEMORY;
   }
   memcpy(completeSet + numCommands, pExtendedCmdSet, sizeof(testCommand) * extendedCmdSetSize);
   numCommands += extendedCmdSetSize;

   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Executing commands now.");

   for (uint32_t module_num = 0; module_num < num_modules; module_num++)
   {
      while ((!feof((&module[module_num])->fCfg)))
      {
         bool_t EOFReached = FALSE;
         result =
            ExecuteOneCommand(completeSet, numCommands, &module[module_num], &EOFReached, num_modules, module_num);
         if (CAPI_EOK != result)
         {
            AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Error in executing command. Exiting test.");
            break;
         }
         if (EOFReached)
         {
            break;
         }
      }
   }

   /* Free module params which are Registered but not Deregistered */
   for (param_index = 0; param_index < MAX_PERSISTENT_PARAMS_PER_MODULE; param_index++)
   {
      if (persistent_param_payload_array[param_index].is_entry_used)
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI TEST ERROR : Freeing param_id %x which is registered but not deregistered",
                persistent_param_payload_array[param_index].param_id);
         posal_memory_free(persistent_param_payload_array[param_index].data_payload);
         persistent_param_payload_array[param_index].data_payload  = NULL;
         persistent_param_payload_array[param_index].param_id      = 0;
         persistent_param_payload_array[param_index].is_entry_used = 0;
      }
   }

   return result;
}

static capi_err_t capi_tst_cb_fn(void *context_ptr, capi_event_id_t id, capi_event_info_t *event_ptr)
{
   module_info_t *module  = reinterpret_cast<module_info_t *>(context_ptr);
   capi_buf_t *   payload = &event_ptr->payload;
   //   capi_err_t     result  = CAPI_EOK;

   if (payload->actual_data_len > payload->max_data_len)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "CAPI TEST: Error in callback function. The actual size %lu is greater than the max size %lu for id "
             "%lu.",
             payload->actual_data_len,
             payload->max_data_len,
             static_cast<uint32_t>(id));
      return CAPI_EBADPARAM;
   }
   AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: id %u", id);

   switch (id)
   {
      case CAPI_EVENT_PROCESS_STATE:
      {
         if (payload->actual_data_len < sizeof(capi_event_process_state_t))
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI TEST: Error in callback function. The actual size %lu is less than the required size %zu "
                   "for id %lu.",
                   payload->actual_data_len,
                   sizeof(capi_event_process_state_t),
                   static_cast<uint32_t>(id));
            return CAPI_ENEEDMORE;
         }
         module->is_enabled = reinterpret_cast<capi_event_process_state_t *>(payload->data_ptr)->is_enabled;
         AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Module process check set to %lu", static_cast<uint32_t>(module->is_enabled));

         return CAPI_EOK;
      }
      case CAPI_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED:
      case CAPI_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED_V2:
      {
         if (!event_ptr->port_info.is_valid)
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI TEST: Error in callback function. Changing media type only "
                   "supported for port 0. Invalid port id passed.");
            return CAPI_EUNSUPPORTED;
         }

         if (event_ptr->port_info.is_input_port)
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI TEST: Error in callback function. Changing media type only "
                   "supported for output ports. Input port id passed.");
            return CAPI_EUNSUPPORTED;
         }

         if ((event_ptr->port_info.port_index) >= CAPI_MAX_OUTPUT_PORTS)
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI TEST: Error in callback function. Changing media type only supported for port 0. Port id "
                   "%lu passed.",
                   event_ptr->port_info.port_index);
            return CAPI_EUNSUPPORTED;
         }

         if (payload->actual_data_len < sizeof(capi_set_get_media_format_t))
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI TEST: Error in callback function. The actual size %lu is less than the required size %zu "
                   "for id %lu.",
                   payload->actual_data_len,
                   sizeof(capi_set_get_media_format_t),
                   static_cast<uint32_t>(id));
            return CAPI_ENEEDMORE;
         }
         update_output_media_format(module, payload, id, event_ptr->port_info.port_index);
         AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: Module output format updated.");

         return CAPI_EOK;
      }
      case CAPI_EVENT_KPPS:
      case CAPI_EVENT_BANDWIDTH:
      case CAPI_EVENT_ALGORITHMIC_DELAY:
      {
         AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: ID %lu not supported.", static_cast<uint32_t>(id));

         if (id == CAPI_EVENT_KPPS)
         {
            capi_event_KPPS_t *kpps_payload = (capi_event_KPPS_t *)(payload->data_ptr);
            AR_MSG(DBG_ERROR_PRIO, "Expected KPPS = %lu", kpps_payload->KPPS);
         }

         break;
      }
      case CAPI_EVENT_PORT_DATA_THRESHOLD_CHANGE:
      {
         if (payload->actual_data_len < sizeof(capi_port_data_threshold_change_t))
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI TEST: Error in callback function. The actual size %lu is less than the required size %zu "
                   "for id %lu.",
                   payload->actual_data_len,
                   sizeof(capi_event_process_state_t),
                   static_cast<uint32_t>(id));
            return CAPI_ENEEDMORE;
         }

         bool_t   is_input_port = event_ptr->port_info.is_input_port;
         uint32_t port_index    = event_ptr->port_info.port_index;

         tst_standard_media_format_v2 *format =
            reinterpret_cast<tst_standard_media_format_v2 *>(module->in_format[port_index].data_ptr);

         uint32_t threshold_in_bytes =
            reinterpret_cast<capi_port_data_threshold_change_t *>(payload->data_ptr)->new_threshold_in_bytes;

         if (is_input_port)
         {
            module->in_buffer_len[port_index] = threshold_in_bytes / format->f.num_channels;
            module->is_in_threshold_raised = TRUE;
         }
         else
         {
            module->out_buffer_len[port_index] = threshold_in_bytes / format->f.num_channels;
            module->is_out_threshold_raised = TRUE;
         }
         AR_MSG(DBG_HIGH_PRIO,
                "CAPI TEST: Module Port Threshold %lu updated for %s-Port index %lu.",
                threshold_in_bytes,
                is_input_port ? "Input" : "Output",
                port_index);

         break;
      }
      case CAPI_EVENT_DATA_TO_DSP_SERVICE:
      {
         capi_err_t     result      = CAPI_EOK;
         module_info_t *dest_module = NULL;
         if (g_num_modules == MAX_NUM_MODULES_PER_TEST)
         {
            dest_module = &(module[1]); // as of now specific to SP module's control link
         }
         else
         {
            AR_MSG(DBG_ERROR_PRIO, "Destination module not present. Ignoring control data");
            break;
         }
         if (!dest_module->module_ptr)
         {
            AR_MSG(DBG_HIGH_PRIO, "Destination module not present. Leaving without sending control data");
            break;
         }
         capi_port_info_t port_info;
         port_info.is_valid = false;
         if (payload->actual_data_len < sizeof(capi_event_data_to_dsp_service_t))
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "Error in callback function. The actual size %lu is less than the required size "
                   "%lu for id %lu.",
                   payload->actual_data_len,
                   sizeof(capi_event_data_to_dsp_service_t),
                   (uint32_t)(id));
            return CAPI_ENEEDMORE;
         }
         capi_event_get_data_from_dsp_service_t *dsp_event_ptr =
            (capi_event_get_data_from_dsp_service_t *)(event_ptr->payload.data_ptr);

         event_id_imcl_outgoing_data_t *data_ptr = (event_id_imcl_outgoing_data_t *)(dsp_event_ptr->payload.data_ptr);
         intf_extn_param_id_imcl_incoming_data_t *payload_ptr =
            (intf_extn_param_id_imcl_incoming_data_t *)(data_ptr->buf.data_ptr -
                                                        sizeof(intf_extn_param_id_imcl_incoming_data_t));
         capi_buf_t buf;
         uint32_t   size     = data_ptr->buf.actual_data_len + sizeof(intf_extn_param_id_imcl_incoming_data_t);
         buf.data_ptr        = (int8_t *)payload_ptr;
         buf.actual_data_len = size;
         buf.max_data_len    = size;
         result              = dest_module->module_ptr->vtbl_ptr->set_param(dest_module->module_ptr,
                                                               INTF_EXTN_PARAM_ID_IMCL_INCOMING_DATA,
                                                               &port_info,
                                                               &buf);
		 if(CAPI_EOK != result)
		 {
			 return result;
		 }
         break;
      }
      case CAPI_EVENT_DATA_TO_DSP_CLIENT:
      {
         break;
      }
      case CAPI_EVENT_DATA_TO_DSP_CLIENT_V2:
      {
         if (payload->actual_data_len < sizeof(capi_event_data_to_dsp_client_v2_t))
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI TEST: Error in callback function. The actual size %lu is less than the required size %zu "
                   "for id %lu.",
                   payload->actual_data_len,
                   sizeof(capi_event_data_to_dsp_client_v2_t),
                   static_cast<uint32_t>(id));
            return CAPI_ENEEDMORE;
         }

         capi_event_data_to_dsp_client_v2_t *payload_ptr =
            reinterpret_cast<capi_event_data_to_dsp_client_v2_t *>(payload->data_ptr);
         switch (payload_ptr->event_id)
         {
            case INTF_EXTN_EVENT_ID_IMCL_OUTGOING_DATA:
            {
               event_id_imcl_outgoing_data_t *event_ptr =
                  (event_id_imcl_outgoing_data_t *)payload_ptr->payload.data_ptr;

               if (NULL == event_ptr->buf.data_ptr)
               {
                  AR_MSG(DBG_ERROR_PRIO,
                         "CAPI TEST: Failed in sending IMC message on control port id %lu, received NULL buffer.",
                         event_ptr->port_id);
                  return CAPI_EFAILED;
               }

               AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: Received IMC message on control port id %lu", event_ptr->port_id);

               /*Currently test framework frees the intent sent by the module, this can be modified as per module
                 testing needs */
               posal_memory_free(event_ptr->buf.data_ptr);

               break;
            }
            case EVENT_ID_DETECTION_ENGINE_GENERIC_INFO:
            {
               AR_MSG(DBG_HIGH_PRIO,
                      "CAPI_EVENT_DATA_TO_DSP_CLIENT_V2, detection param Id= 0x%lx",
                      payload_ptr->event_id);
               capi_event_data_to_dsp_client_v2_t *event_ptr =
                  (capi_event_data_to_dsp_client_v2_t *)payload_ptr->payload.data_ptr;

               event_id_detection_engine_generic_info_t *data_ptr =
                  (event_id_detection_engine_generic_info_t *)event_ptr;
               AR_MSG(DBG_HIGH_PRIO, "CAPI_EVENT_DATA_TO_DSP_CLIENT_V2 Payload");
               AR_MSG(DBG_HIGH_PRIO, "Status = %lu", data_ptr->status);
               break;
            }

            default:
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI TEST: Failed in CAPI_EVENT_GET_DATA_FROM_DSP_SERVICE! unknow param id 0x%lx",
                      payload_ptr->event_id);
               break;
            }
         }
         break;
      }
      case CAPI_EVENT_GET_DATA_FROM_DSP_SERVICE:
      {
         if (payload->actual_data_len < sizeof(capi_event_get_data_from_dsp_service_t))
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI TEST: Error in callback function. The actual size %lu is less than the required size %zu "
                   "for id %lu.",
                   payload->actual_data_len,
                   sizeof(capi_event_get_data_from_dsp_service_t),
                   static_cast<uint32_t>(id));
            return CAPI_ENEEDMORE;
         }

         capi_event_get_data_from_dsp_service_t *payload_ptr =
            reinterpret_cast<capi_event_get_data_from_dsp_service_t *>(payload->data_ptr);
         switch (payload_ptr->param_id)
         {
            case INTF_EXTN_EVENT_ID_IMCL_GET_ONE_TIME_BUF:
            case INTF_EXTN_EVENT_ID_IMCL_GET_RECURRING_BUF:
            {
               event_id_imcl_get_recurring_buf_t *param_ptr =
                  (event_id_imcl_get_recurring_buf_t *)payload_ptr->payload.data_ptr;

               // for one time buffer module must populate a valid actual data length
               // for recurring buffer module would have requested the intent size
               // before process starts.
               if (payload_ptr->param_id == INTF_EXTN_EVENT_ID_IMCL_GET_ONE_TIME_BUF &&
                   param_ptr->buf.actual_data_len == 0)
               {
                  AR_MSG(DBG_ERROR_PRIO,
                         "CAPI TEST: Module requesting invalid size %lu one time buffer. ",
                         payload_ptr->param_id);
                  return CAPI_EFAILED;
               }

               capi_buf_t buf;
               if (NULL == (buf.data_ptr = (int8_t *)posal_memory_malloc(MAX_INTENT_SIZE, POSAL_HEAP_DEFAULT)))
               {
                  return CAPI_EFAILED;
               }
               buf.actual_data_len = MAX_INTENT_SIZE;
               buf.max_data_len    = MAX_INTENT_SIZE;
               param_ptr->buf      = buf;

               break;
            }
            default:
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "CAPI TEST: Failed in CAPI_EVENT_GET_DATA_FROM_DSP_SERVICE! unknow param id 0x%lx",
                      payload_ptr->param_id);
               break;
            }
         }

         break;
      }
      case CAPI_EVENT_GET_LIBRARY_INSTANCE:
      {
         if (payload->actual_data_len < sizeof(capi_event_get_library_instance_t))
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "CAPI TEST: Error in callback function. The actual size %lu is less than the required size %zu "
                   "for id %lu.",
                   payload->actual_data_len,
                   sizeof(capi_event_process_state_t),
                   static_cast<uint32_t>(id));
            return CAPI_ENEEDMORE;
         }

         //         capi_event_get_library_instance_t *payload_ptr =
         //            reinterpret_cast<capi_event_get_library_instance_t *>(payload->data_ptr);

         //         result = capi_library_factory_get_instance(payload_ptr->id, &payload_ptr->ptr);
         //         if (result != CAPI_EOK)
         //         {
         //            AR_MSG(DBG_ERROR_PRIO, "Failed to get an instance of the library to get capi modules.");
         //         }

         break;
      }
      default:
      {
         AR_MSG(DBG_ERROR_PRIO,
                "CAPI TEST: Error in callback function. ID %lu not supported.",
                static_cast<uint32_t>(id));
         return CAPI_EUNSUPPORTED;
      }
   }

   return CAPI_EOK;
}

capi_event_callback_info_t capi_tst_get_cb_info(module_info_t *module)
{
   capi_event_callback_info_t cb_info;

   cb_info.event_cb      = capi_tst_cb_fn;
   cb_info.event_context = module;

   return cb_info;
}
