#ifndef CAPI_CMN_H
#define CAPI_CMN_H
/**
 * \file capi_cmn.h
 * \brief
 *
 *
 * \copyright
 *  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //source/qcom/qct/platform/adsp/proj/addons_audio/spf/incs/spf/interfaces/module/capi_cmn/inc/capi_cmn.h#1 $
*/
// clang-format on

#include "capi.h"
#include "shared_lib_api.h"
//#include "stringl.h"

#include "module_cmn_api.h"
#include "common_enc_dec_api.h"

/*=====================================================================
  Macros
 ======================================================================*/
#define CAPI_MF_V2_MIN_SIZE (sizeof(capi_standard_data_format_v2_t) + sizeof(capi_set_get_media_format_t))
#define CAPI_RAW_COMPRESSED_MF_MIN_SIZE                                                                                \
   (sizeof(capi_raw_compressed_data_format_t) + sizeof(capi_set_get_media_format_t))
#define CAPI_ALIGN_4_BYTE(x) (((x) + 3) & (0xFFFFFFFC))
#define CAPI_CMN_MAX_IN_PORTS 1
#define CAPI_CMN_MAX_OUT_PORTS 1
#define CAPI_ALIGN_8_BYTE(x) ((((uint32_t)(x) + 7) >> 3) << 3)
#define CAPI_CMN_IS_PCM_FORMAT(data_format) ((CAPI_FIXED_POINT == data_format) || (CAPI_FLOATING_POINT == data_format))

#define CAPI_CMN_IS_POW_OF_2(x) (!((x) & ((x)-1)))

#define CAPI_CMN_ISLAND_VOTE_EXIT 1
#define CAPI_CMN_ISLAND_VOTE_ENTRY 0

#define CAPI_CMN_UNDERRUN_TIME_THRESH_US 10000

#define CAPI_CMN_UNDERRUN_INFO_RESET(underrun_info) \
   underrun_info.underrun_counter = 0; \
   underrun_info.prev_time = 0;

#ifdef __cplusplus
extern "C" {
#endif

/*=====================================================================
  Structure definitions
 ======================================================================*/
typedef struct capi_media_fmt_v1_t
{
   capi_set_get_media_format_t header;
   capi_standard_data_format_t format;
} capi_media_fmt_v1_t;

typedef struct capi_media_fmt_v2_t
{
   capi_set_get_media_format_t    header;
   capi_standard_data_format_v2_t format;
   uint16_t                       channel_type[CAPI_MAX_CHANNELS_V2];
} capi_media_fmt_v2_t;

typedef struct capi_cmn_raw_media_fmt_t
{
   capi_set_get_media_format_t       header;
   capi_raw_compressed_data_format_t format;
} capi_cmn_raw_media_fmt_t;

typedef struct capi_basic_prop_t
{
   uint32_t  init_memory_req;
   uint32_t  stack_size;
   uint32_t  num_fwk_extns;
   uint32_t *fwk_extn_ids_arr;
   bool_t    is_inplace;
   bool_t    req_data_buffering;
   uint32_t  max_metadata_size;
} capi_basic_prop_t;

typedef struct capi_cmn_underrun_info_t
{
   uint64_t prev_time;  //Last time stamp when underrun msg was printed
   uint32_t underrun_counter; //This counts the no. of underruns since last it was printed
} capi_cmn_underrun_info_t;

/*=====================================================================
  Utility functions to retrieve certain bits from a flag
 ======================================================================*/
static inline uint32_t capi_get_bits(uint32_t x, uint32_t mask, uint32_t shift)
{
   return (x & mask) >> shift;
}
static inline void capi_set_bits(uint32_t *x_ptr, uint32_t val, uint32_t mask, uint32_t shift)
{
   val    = (val << shift) & mask;
   *x_ptr = (*x_ptr & ~mask) | val;
}
/*=====================================================================
  Utility function defines
 ======================================================================*/

static inline void pick_config_or_input(uint32_t *val, int32_t config_val, uint32_t inp_val)
{
   *val = (PARAM_VAL_NATIVE != config_val) ? config_val : inp_val;
}

static inline void pick_if_not_unset(int16_t *current_value, int16_t config_value)
{
   if (PARAM_VAL_UNSET != config_value)
   {
      *current_value = config_value;
   }
}

static inline void pcm_to_capi_interleaved_with_native(capi_interleaving_t *capi_value,
                                                       int16_t              cfg_value,
                                                       capi_interleaving_t  inp_value)
{
   switch (cfg_value)
   {
      case PCM_INTERLEAVED:
      {
         *capi_value = CAPI_INTERLEAVED;
         break;
      }
      case PCM_DEINTERLEAVED_PACKED:
      {
         *capi_value = CAPI_DEINTERLEAVED_PACKED;
         break;
      }
      case PCM_DEINTERLEAVED_UNPACKED:
      {
         *capi_value = CAPI_DEINTERLEAVED_UNPACKED;
         break;
      }
      case PARAM_VAL_NATIVE:
      {
         *capi_value = inp_value;
         break;
      }
      default:
         break;
   }
}

static inline intf_extn_data_port_state_t intf_extn_data_port_op_to_port_state(intf_extn_data_port_opcode_t opcode)
{
   switch (opcode)
   {
      case INTF_EXTN_DATA_PORT_OPEN:
      {
         return DATA_PORT_STATE_OPENED;
      }
      case INTF_EXTN_DATA_PORT_START:
      {
         return DATA_PORT_STATE_STARTED;
      }
      case INTF_EXTN_DATA_PORT_STOP:
      {
         return DATA_PORT_STATE_STOPPED;
      }
      case INTF_EXTN_DATA_PORT_CLOSE:
      {
         return DATA_PORT_STATE_CLOSED;
      }
      case INTF_EXTN_DATA_PORT_SUSPEND:
      {
         return DATA_PORT_STATE_SUSPENDED;
      }
      default:
      {
         break;
      }
   }
   return DATA_PORT_STATE_INVALID;
}

capi_err_t capi_cmn_set_basic_properties(capi_proplist_t *           proplist_ptr,
                                         capi_heap_id_t *            heap_mem_ptr,
                                         capi_event_callback_info_t *cb_info_ptr,
                                         bool_t                      check_port_info);

capi_err_t capi_cmn_get_basic_properties(capi_proplist_t *proplist_ptr, capi_basic_prop_t *mod_prop_ptr);

capi_err_t capi_cmn_update_algo_delay_event(capi_event_callback_info_t *cb_info_ptr, uint32_t delay_in_us);

capi_err_t capi_cmn_update_kpps_event(capi_event_callback_info_t *cb_info_ptr, uint32_t kpps);

capi_err_t capi_cmn_update_bandwidth_event(capi_event_callback_info_t *cb_info_ptr,
                                           uint32_t                    code_bandwidth,
                                           uint32_t                    data_bandwidth);

capi_err_t capi_cmn_update_hw_acc_proc_delay_event(capi_event_callback_info_t *cb_info_ptr, uint32_t delay_in_us);

capi_err_t capi_cmn_update_process_check_event(capi_event_callback_info_t *cb_info_ptr, uint32_t process_check);

capi_err_t capi_cmn_update_port_data_threshold_event(capi_event_callback_info_t *cb_info_ptr,
                                                     uint32_t                    threshold_bytes,
                                                     bool_t                      is_input_port,
                                                     uint32_t                    port_index);

capi_err_t capi_cmn_handle_get_port_threshold(capi_prop_t *prop_ptr, uint32_t threshold);

capi_err_t capi_cmn_output_media_fmt_event_v1(capi_event_callback_info_t *cb_info_ptr,
                                              capi_media_fmt_v1_t *       out_media_fmt,
                                              bool_t                      is_input_port,
                                              uint32_t                    port_index);

capi_err_t capi_cmn_output_media_fmt_event_v2(capi_event_callback_info_t *cb_info_ptr,
                                              capi_media_fmt_v2_t *       out_media_fmt,
                                              bool_t                      is_input_port,
                                              uint32_t                    port_index);

capi_err_t capi_cmn_raw_output_media_fmt_event(capi_event_callback_info_t *cb_info_ptr,
                                               capi_cmn_raw_media_fmt_t *  out_media_fmt,
                                               bool_t                      is_input_port,
                                               uint32_t                    port_index);

capi_err_t capi_cmn_raise_data_to_dsp_svc_event(capi_event_callback_info_t *cb_info_ptr,
                                                uint32_t                    event_id,
                                                capi_buf_t *                event_buf);

capi_err_t capi_cmn_init_media_fmt_v1(capi_media_fmt_v1_t *media_fmt_ptr);

capi_err_t capi_cmn_init_media_fmt_v2(capi_media_fmt_v2_t *media_fmt_ptr);

capi_err_t capi_cmn_handle_get_output_media_fmt_v1(capi_prop_t *prop_ptr, capi_media_fmt_v1_t *media_fmt_ptr);

capi_err_t capi_cmn_handle_get_output_media_fmt_v2(capi_prop_t *prop_ptr, capi_media_fmt_v2_t *media_fmt_ptr);

capi_err_t capi_cmn_validate_client_pcm_media_format(const payload_media_fmt_pcm_t *pcm_fmt_ptr);
capi_err_t capi_cmn_validate_client_pcm_output_cfg(const payload_pcm_output_format_cfg_t *pcm_cfg_ptr);
bool_t     capi_cmn_media_fmt_equal(capi_media_fmt_v2_t *media_fmt_1_ptr, capi_media_fmt_v2_t *media_fmt_2_ptr);
capi_err_t capi_cmn_data_fmt_map(uint32_t *in_format, capi_media_fmt_v2_t *media_fmt);

capi_err_t capi_cmn_raise_dm_disable_event(capi_event_callback_info_t *cb_info,
                                           uint32_t                    module_log_id,
                                           uint32_t                    disable);
capi_err_t capi_cmn_raise_dynamic_inplace_event(capi_event_callback_info_t *cb_info_ptr, bool_t is_inplace);

capi_err_t capi_cmn_populate_trigger_ts_payload(capi_buf_t *params_ptr, stm_latest_trigger_ts_t *ts_struct_ptr);

capi_err_t capi_cmn_gapless_remove_zeroes(uint32_t *            bytes_to_remove_per_channel_ptr,
                                          capi_media_fmt_v2_t * out_mf_ptr,
                                          capi_stream_data_t *  output[],
                                          bool_t                initial,
                                          module_cmn_md_list_t *metadata_list_ptr);

capi_err_t capi_cmn_dec_handle_metadata(capi_stream_data_v2_t *                in_stream_ptr,
                                        capi_stream_data_v2_t *                out_stream_ptr,
                                        intf_extn_param_id_metadata_handler_t *metadata_handler_ptr,
                                        module_cmn_md_list_t **                internal_md_list_pptr,
                                        uint32_t *                             in_len_before_process,
                                        capi_media_fmt_v2_t *                  out_media_fmt_ptr,
                                        uint32_t                               dec_algo_delay,
                                        capi_err_t                             process_result);

void capi_cmn_dec_update_buffer_end_md(capi_stream_data_v2_t *in_stream_ptr,
                                       capi_stream_data_v2_t *out_stream_ptr,
                                       capi_err_t *           agg_process_result,
                                       bool_t *               error_recovery_done);

capi_err_t capi_cmn_raise_island_vote_event(capi_event_callback_info_t *cb_info_ptr, bool_t island_vote);

/**
 * Returns needed size for capi v2 media format for passed in number of channels. Useful for
 * get properties validation.
 */
static inline uint32_t capi_cmn_media_fmt_v2_required_size(uint32_t num_channels)
{
   return sizeof(capi_media_fmt_v1_t) + (num_channels * sizeof(capi_channel_type_t));
}

#define CAPI_CMN_CEIL(x, y) (((x) + (y)-1) / (y))
#define CAPI_CMN_BITS_TO_BYTES(x) ((x) >> 3)
#define CAPI_CMN_BYTES_TO_BITS(x) ((x) << 3)
static const uint32_t NUM_US_PER_SEC = 1000000L;
static const uint32_t NUM_NS_PER_SEC = 1000000000L;
static const uint32_t NUM_NS_PER_US  = 1000;
#define CAPI_ZERO_IF_NULL(ptr) ((NULL == ptr) ? 0 : *ptr)

static inline uint32_t capi_cmn_us_to_samples(uint64_t time_us, uint64_t sample_rate)
{
   return CAPI_CMN_CEIL((time_us * sample_rate), NUM_US_PER_SEC);
}

static inline uint32_t capi_cmn_us_to_bytes_per_ch(uint64_t time_us, uint32_t sample_rate, uint16_t bits_per_sample)
{
   uint32_t bytes = 0;
   uint32_t b     = CAPI_CMN_BITS_TO_BYTES(bits_per_sample);

   uint32_t samples = capi_cmn_us_to_samples(time_us, sample_rate);

   bytes = samples * b;

   return (uint32_t)bytes;
}

static inline uint32_t capi_cmn_us_to_bytes(uint64_t time_us,
                                            uint32_t sample_rate,
                                            uint16_t bits_per_sample,
                                            uint32_t num_channels)
{
   uint32_t bytes = capi_cmn_us_to_bytes_per_ch(time_us, sample_rate, bits_per_sample);
   bytes          = bytes * num_channels;
   return bytes;
}

static inline uint32_t capi_cmn_bytes_to_samples_per_ch(uint32_t bytes, uint16_t bits_per_sample, uint32_t num_channels)
{
   uint32_t samples = 0;
   if ((num_channels != 0) && (bits_per_sample != 0))
   {
      uint32_t bytes_for_all_ch = ((uint64_t)num_channels * CAPI_CMN_BITS_TO_BYTES(bits_per_sample));

      samples = bytes / bytes_for_all_ch;
   }

   return samples;
}

/**
 * includes previous fractional time, and calculates new fractional time in fract_time_ptr
 * Unit of *fract_time_ptr is ns
 *
 * Must be called only for PCM or packetized
 */
static inline uint64_t capi_cmn_bytes_to_us(uint32_t  bytes,
                                            uint32_t  sample_rate,
                                            uint16_t  bits_per_sample,
                                            uint32_t  num_channels,
                                            uint64_t *fract_time_ptr)
{
   uint64_t time_us = 0;
   if ((num_channels != 0) && (sample_rate != 0) && (bits_per_sample != 0))
   {
      uint64_t total_byte_ns_per_s = ((uint64_t)bytes * NUM_NS_PER_SEC);
      uint64_t bps                 = ((uint64_t)num_channels * sample_rate * CAPI_CMN_BITS_TO_BYTES(bits_per_sample));

      uint64_t time_ns = (total_byte_ns_per_s / bps) + ((NULL == fract_time_ptr) ? 0 : *fract_time_ptr);
      time_us          = time_ns / (NUM_NS_PER_SEC / NUM_US_PER_SEC);

      if (fract_time_ptr)
      {
         *fract_time_ptr = time_ns - (time_us * (NUM_NS_PER_SEC / NUM_US_PER_SEC));
      }
   }

   return time_us;
}

/* Determine if the sampling rate is fractional.*/
static inline bool_t is_sample_rate_fractional(uint32_t sample_rate)
{
   return (sample_rate % 1000) > 0 ? TRUE : FALSE;
}

/**
 * returns p/q where q has high chance of being power of 2
 */
static inline uint32_t capi_cmn_div_num(uint32_t num, uint32_t den)
{
   if (CAPI_CMN_IS_POW_OF_2(den))
   {
      return s32_shr_s32_sat(num, s32_get_lsb_s32(den));
   }
   return (num / den);
}

static inline void capi_cmn_check_print_underrun(capi_cmn_underrun_info_t *underrun_info_ptr, uint32_t iid)
{
   underrun_info_ptr->underrun_counter++;
   uint64_t curr_time = posal_timer_get_time();
   uint64_t diff      = curr_time - underrun_info_ptr->prev_time;
   if ((diff >= CAPI_CMN_UNDERRUN_TIME_THRESH_US) || (0 == underrun_info_ptr->prev_time))
   {
      AR_MSG_ISLAND(DBG_ERROR_PRIO,
                    "MODULE:%08lX, Process Underrun detected No of underun : %ld, time since last underrun "
                    "print : %ld us",
                    iid,
                    underrun_info_ptr->underrun_counter,
                    diff);
      underrun_info_ptr->prev_time         = curr_time;
      underrun_info_ptr->underrun_counter = 0;
   }

   return;
}

#ifdef __cplusplus
}
#endif
#endif // CAPI_CMN_H
