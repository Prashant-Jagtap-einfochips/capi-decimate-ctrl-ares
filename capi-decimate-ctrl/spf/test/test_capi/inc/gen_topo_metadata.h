#ifndef GEN_TOPO_H_
#define GEN_TOPO_H_

/**
 * \file gen_topo.h
 *
 * \brief
 *
 *     Basic Topology header file.
 *
 *
 * \copyright
 *  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
*/
// clang-format on

#include "ar_guids.h"

#include "shared_lib_api.h"
#include "../../../incs/spf/utils/list/inc/spf_list_utils.h"

#include "capi_test_utils.h"
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// clang-format off

#ifdef USES_DEBUG_DEV_ENV
/** Debug masks */


#if 0 //frequently needed ones
#define VERBOSE_DEBUGGING
#define TRIGGER_DEBUG
#endif

//#define BUF_MGMT_DEBUG

//#define CTRL_LINK_DEBUG

//#define TRIGGER_DEBUG_DEEP

#endif

#define DATA_FLOW_STATE_DEBUG
#define DEBUG_TOPO_PORT_PROP_TYPE
#define DEBUG_TOPO_BOUNDARY_STATE_PROP
#define METADATA_DEBUGGING
#define PATH_DELAY_DEBUGGING
#define THRESH_PROP_DEBUG

#define UNITY_Q4 0x10

#define  GEN_TOPO_IS_NON_TRIGGERABLE_PORT(nontrigger_policy) (FWK_EXTN_PORT_NON_TRIGGER_INVALID != nontrigger_policy)

#define SPF_IS_PACKETIZED_OR_PCM(data_format)                                                                          \
     ((data_format != SPF_RAW_COMPRESSED) && (data_format != SPF_UNKNOWN_DATA_FORMAT))


typedef struct gen_topo_t                    gen_topo_t;
typedef struct gen_topo_module_t             gen_topo_module_t;
typedef struct gen_topo_input_port_t         gen_topo_input_port_t;
typedef struct gen_topo_output_port_t        gen_topo_output_port_t;
typedef struct gen_topo_process_context_t	 gen_topo_process_context_t;
typedef struct gen_topo_module_bypass_t      gen_topo_module_bypass_t;
typedef struct gen_topo_vtable_t             gen_topo_vtable_t;
typedef struct gu_module_t             gu_module_t;

/**
   * Data format indicating overall format of the data.
   *
   * Note the difference in data format 'data_format_t' exposed by CAPIv2:
   * the CAPI v2 enum has valid 'zero' value (CAPI_FIXED_POINT).
   * But spf_data_format_t starts with valid value of 1. 0 is invalid or unknown value.
   */

typedef enum topo_endianness_t { TOPO_UNKONWN_ENDIAN = 0, TOPO_LITTLE_ENDIAN, TOPO_BIG_ENDIAN } topo_endianness_t;

typedef enum topo_interleaving_t
  {
     TOPO_INTERLEAVING_UNKNOWN = 0,
     TOPO_INTERLEAVED,
     TOPO_DEINTERLEAVED_PACKED,
     TOPO_DEINTERLEAVED_UNPACKED,
  } topo_interleaving_t;

typedef struct topo_pcm_pack_med_fmt_t
  {
     uint32_t                sample_rate;
     uint16_t                bit_width;        /**< 16, 24, 32 (actual bit width independent of the word size)*/
     uint16_t                bits_per_sample;  /**< bits per sample 16 or 32. This is actually word size in bits*/
     uint16_t                q_factor;         /**< 15, 27, 31 */
     uint32_t                num_channels;
     topo_interleaving_t     interleaving;
     topo_endianness_t       endianness;
     uint8_t                 chan_map[CAPI_MAX_CHANNELS_V2];
  } topo_pcm_pack_med_fmt_t;

typedef enum spf_data_format_t
  {
     SPF_UNKNOWN_DATA_FORMAT            = 0,
     SPF_FIXED_POINT                    = 1, /**< Maps to DATA_FORMAT_FIXED_POINT */
     SPF_FLOATING_POINT                 = 2,
     SPF_RAW_COMPRESSED                 = 3, /**< DATA_FORMAT_RAW_COMPRESSED */
     SPF_IEC61937_PACKETIZED            = 4, /**< DATA_FORMAT_IEC61937_PACKETIZED */
     SPF_DSD_DOP_PACKETIZED             = 5,
     SPF_COMPR_OVER_PCM_PACKETIZED      = 6,
     SPF_GENERIC_COMPRESSED             = 7, /**< DATA_FORMAT_GENERIC_COMPRESSED */
     SPF_IEC60958_PACKETIZED            = 8, /**< DATA_FORMAT_IEC60958_PACKETIZED */
     SPF_IEC60958_PACKETIZED_NON_LINEAR = 9, /**< DATA_FORMAT_IEC60958_PACKETIZED_NON_LINEAR */
     SPF_MAX_FORMAT_TYPE                = 0x7FFFFFFF
  } spf_data_format_t;

    /**
     * Common struct for input and output port.
     */
    typedef struct gu_cmn_port_t
    {
       uint32_t
          id; /**< This is the ID of the port. Index of the port (CAPIv2 requirement) is different. Framework sets ID <->
                 Index mapping to the module.*/
       uint32_t     index;      /**< index (0 to num_ports) is determined by number of active ports */
    } gu_cmn_port_t;



    typedef struct gu_peer_handle_t
    {
       uint32_t      module_instance_id; /**< module-instance-id of the peer */
       uint32_t      port_id;            /**< port-id of the peer */
    } gu_peer_handle_t;
    /**
       * External input
       */
      typedef struct gu_ext_in_port_t
      {
//         spf_handle_t     this_handle;     /**< This external port's handle. must be first element */
         gu_peer_handle_t upstream_handle; /**< gu_peer_handle_t::spf_handle = gu_ext_out_port_t */
      } gu_ext_in_port_t;

  /**
     * input ports
     */
    typedef struct gu_input_port_t
    {
       gu_cmn_port_t     cmn;               /**< Must be first element */

       gu_ext_in_port_t *ext_in_port_ptr;   /**< Non-NULL if connected to ports/modules external to this container.
                                               ext_in_port_list_ptr has the primary ref to this pointer. */
       uint32_t depth; /**< Depth is a measure of distance from this port to its upstream connected external input ports. It
                          might be needed to compare
                            depths of ports on different legs of the graph. */
    } gu_input_port_t;

  /**
    * Raw Media Format
    */
   typedef struct topo_raw_med_fmt_t
   {
      uint8_t  *buf_ptr; /**< struct of this mem: capi_set_get_media_format_t, capi_raw_compressed_data_format_t, payload*/
      /*
      - When sending mf between containers, we only send the payload(from above line).
            Note:
            * Encoders will raise this mf with no payload.
              So, buf_ptr will be NULL. In this case, We only populate the topo_media_fmt_t with data_format and format ID and send it.
      - When a container receives raw mf from peer, it'll call the tu_capi_create_raw_compr_med_fmt function with with_header = TRUE
        From then on, buf_ptr will have a value and it'll adhere to the structure: capi_set_get_media_format_t, capi_raw_compressed_data_format_t
      */
      uint32_t  buf_size;
   } topo_raw_med_fmt_t;

typedef struct topo_media_fmt_t
  {
     spf_data_format_t        data_format;
     uint32_t                fmt_id;
     topo_pcm_pack_med_fmt_t pcm; /**< when SPF_IS_PACKETIZED_OR_PCM(data_fmt is true)*/
     topo_raw_med_fmt_t      raw; /**< when SPF_RAW_COMPRESSED == data_format*/
  } topo_media_fmt_t;

/** Malloc and memset */
#define MALLOC_MEMSET(ptr, type, size, heap_id, result)                                                                \
   do                                                                                                                  \
   {                                                                                                                   \
      ptr = (type *)posal_memory_malloc(size, heap_id);                                                                \
      if (NULL == ptr)                                                                                                 \
      {                                                                                                                \
         AR_MSG(DBG_ERROR_PRIO, "Malloc failed. Required Size %lu", size);                                             \
      }                                                                                                                \
      else                                                                                                             \
      {                                                                                                                \
	     memset(ptr, 0, size);                                                                                         \
      }                                                                                                                \
                                                                                                                       \
   } while (0)

/** check, free and assign ptr as NULL */
#define MFREE_NULLIFY(ptr)                                                                                             \
   do                                                                                                                  \
   {                                                                                                                   \
      if (ptr)                                                                                                         \
      {                                                                                                                \
         posal_memory_free(ptr);                                                                                       \
         ptr = NULL;                                                                                                   \
      }                                                                                                                \
   } while (0)

#define CAPI_CMN_CEIL(x, y) (((x) + (y)-1) / (y))
#define TOPO_CEIL(x, y) CAPI_CMN_CEIL(x, y)
#define CAPI_CMN_BITS_TO_BYTES(x) ((x) >> 3)
static const uint32_t NUM_US_PER_SEC = 1000000L;


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

static inline uint32_t topo_bytes_to_samples_per_ch(uint32_t bytes, topo_media_fmt_t *med_fmt_ptr)
{
   return capi_cmn_bytes_to_samples_per_ch(bytes, med_fmt_ptr->pcm.bits_per_sample, med_fmt_ptr->pcm.num_channels);
}

static inline uint32_t capi_cmn_us_to_samples(uint64_t time_us, uint64_t sample_rate)
  {
     return CAPI_CMN_CEIL((time_us * sample_rate), NUM_US_PER_SEC);
  }

static inline uint32_t topo_us_to_samples(uint64_t time_us, uint64_t sample_rate)
  {
     return capi_cmn_us_to_samples(time_us, sample_rate);
  }

static inline uint32_t capi_cmn_us_to_bytes_per_ch(uint64_t time_us, uint32_t sample_rate, uint16_t bits_per_sample)
  {
     uint32_t bytes = 0;
     uint32_t b     = CAPI_CMN_BITS_TO_BYTES(bits_per_sample);
     uint32_t samples = capi_cmn_us_to_samples(time_us, sample_rate);
     bytes = samples * b;
     return (uint32_t)bytes;
  }

static inline uint32_t topo_us_to_bytes_per_ch(uint64_t time_us, topo_media_fmt_t *med_fmt_ptr)
  {
     return capi_cmn_us_to_bytes_per_ch(time_us, med_fmt_ptr->pcm.sample_rate, med_fmt_ptr->pcm.bits_per_sample);
  }


  /**
     * Module
     */
    typedef struct gu_module_t
    {
       uint16_t module_type; /**< AMDB_MODULE_TYPE_DECODER, AMDB_MODULE_TYPE_ENCODER, AMDB_MODULE_TYPE_PACKETIZER,
                                AMDB_MODULE_TYPE_DEPACKETIZER,
                                  AMDB_MODULE_TYPE_CONVERTER, AMDB_MODULE_TYPE_GENERIC, AMDB_MODULE_TYPE_FRAMEWORK */
       uint16_t itype;       /**< interface type of the module: CAPIV2 or STUB */
       uint32_t module_id;   /**< */
       uint32_t module_instance_id;
       void *   amdb_handle;

       uint32_t max_input_ports;
       uint32_t max_output_ports;

       uint32_t num_input_ports;
       uint32_t num_output_ports;
       uint32_t num_ctrl_ports;

       bool_t is_connected;  /* Indicates if a module is fully connected. For modules that support input, should have at
                                least one input port.
                                For modules that support output, should have at least one output port. */
       bool_t ports_updated; /**< Indicates if input or output ports have been updated, either counts or indices or both */

      uint32_t depth; /**< Depth is a measure of distance from this module to its upstream connected external input ports.
                          For a module
                            this is the max of the depth of the input ports. */
    } gu_module_t;

  static inline bool_t gu_is_sink_module(gu_module_t *module_ptr)
    {
       if ((0 == module_ptr->max_output_ports) && (0 != module_ptr->max_input_ports))
       {
          return TRUE;
       }
       return FALSE;
    }

  static inline bool_t gu_is_siso_module(gu_module_t *module_ptr)
    {
       if ((1 == module_ptr->num_output_ports) && (1 == module_ptr->max_output_ports) &&
           (1 == module_ptr->num_input_ports) && (1 == module_ptr->max_input_ports))
       {
          return TRUE;
       }
       return FALSE;
    }

/*
 * Table of functions that can be populated by different topo implementations
 * that use gen_topo as a base
 */
typedef struct gen_topo_vtable_t
{
   ar_result_t (*capi_get_required_fmwk_extensions)(void *topo_ptr, void *module_ptr, void *amdb_handle, capi_proplist_t *init_proplist_ptr);

   ar_result_t (*input_port_is_trigger_present)(void *topo_ptr,
                                                void *topo_in_port_ptr,
                                                bool_t *is_ext_trigger_not_satisfied_ptr,
                                                bool_t *trigger_present_ptr);
   ar_result_t (*output_port_is_trigger_present)(void *topo_ptr,
                                                 void *topo_out_port_ptr,
                                                 bool_t *is_ext_trigger_not_satisfied_ptr,
                                                 bool_t *trigger_present_ptr);

   ar_result_t (*input_port_is_trigger_absent)(void *topo_ptr, void *topo_in_port_ptr, bool_t *trigger_absent_ptr);
   ar_result_t (*output_port_is_trigger_absent)(void *topo_ptr, void *topo_out_port_ptr, bool_t *trigger_absent_ptr);
   ar_result_t (*output_port_is_size_known)(void *topo_ptr, void *topo_out_port_ptr, bool_t *size_is_known_ptr);

} gen_topo_vtable_t;


typedef struct topo_to_cntr_vtable_t
{
   /* Topo callback to containers for clear EoS */
   ar_result_t (*clear_eos)(gen_topo_t *topo_ptr, void *ext_inp_ref, uint32_t ext_inp_id, module_cmn_md_eos_t *eos_metadata_ptr);

   /* Topo callback to containers for raising EOS */
   ar_result_t (*raise_eos)(gen_topo_t *topo_ptr, uint32_t module_instance_id, void *context_ptr, bool_t is_eos_rendered);

   /* Topo callback to containers to handle events to DSP service. */
   ar_result_t (*raise_data_to_dsp_service_event)(gen_topo_module_t *module_context_ptr,
												              capi_event_info_t *event_info_ptr);

   /* Topo callback to containers to handle events from DSP service. */
   ar_result_t (*raise_data_from_dsp_service_event)(gen_topo_module_t *module_context_ptr,
                                                  capi_event_info_t *event_info_ptr);

   ar_result_t (*handle_capi_event)(gen_topo_module_t *module_context_ptr,
                                              capi_event_id_t    id,
                                              capi_event_info_t *event_info_ptr);

   ar_result_t (*algo_delay_change_event)(gen_topo_module_t *module_context_ptr);

   ar_result_t (*handle_frame_done)(gen_topo_t *gen_topo_ptr);

   ar_result_t (*destroy_module)(gen_topo_t *       topo_ptr,
                                 gen_topo_module_t *module_ptr,
                                 bool_t             reset_capi_dependent_dont_destroy);



   /* --- Below are currently only implemented by Topo2 ---  */

   /* Check and send media fmt message downstream. */
   ar_result_t (*check_apply_ext_out_ports_pending_media_fmt)(gen_topo_t *topo_ptr, bool_t is_data_path);

   /* Topo callback to container for handling port data threshold change event */
   ar_result_t (*handle_port_data_thresh_change_event)(gen_topo_t *                          topo_ptr,
                                                       void *                                port_context_ptr,
                                                       capi_port_data_threshold_change_t *data_ptr,
                                                       bool_t                                is_input_port);

   /* Prompts the container to check for fractional resampling use case. Called when handling output media format of source modules,
      in case the new media format has a fractional sample rate. */
   ar_result_t (*check_set_fractional_sampling_rate_use_case)(gen_topo_t *topo_ptr);

   /* Indicate to the container that the maximum sample requirement on an input port has changed. This is primarily
    * needed for handling of modules running in fixed output mode */
   ar_result_t (*update_input_port_max_samples)(gen_topo_t *topo_ptr, gen_topo_input_port_t* inport_ptr);

   /* Check if the container is running a realtime usecase or not. */
   bool_t (*is_usecase_realtime)(gen_topo_t *gen_topo_ptr);

   /* Clears dm state of all external input ports. */
   ar_result_t (*clear_topo_dm_samples)(gen_topo_t *topo_ptr);

   /* Update variable size flag in icb info of ext input/output ports*/
   ar_result_t (*update_icb_info)(gen_topo_t *topo_ptr);
} topo_to_cntr_vtable_t;




/** This enum used for aggregation in the container */
typedef enum gen_topo_data_need_t
{
   GEN_TOPO_DATA_NOT_NEEDED = 0,
   GEN_TOPO_DATA_NEEDED,
   GEN_TOPO_DATA_NEEDED_OPTIONALLY

} gen_topo_data_need_t;

/** This enum used for external output port wait mask aggregation*/
typedef enum gen_topo_port_trigger_need_t
{
   GEN_TOPO_PORT_TRIGGER_NEEDED = 0,
   GEN_TOPO_PORT_TRIGGER_NOT_NEEDED,
   GEN_TOPO_PORT_TRIGGER_NEEDED_OPTIONALLY

} gen_topo_port_trigger_need_t;

typedef struct gu_t
  {
     uint32_t      log_id;        /**< logging id: see topo_utils.h LOG_ID_CAPABILITY_SHIFT etc */
  } gu_t;

#define GEN_TOPO_MAX_NUM_TRIGGERS 2

typedef enum gen_topo_trigger_t
{
   GEN_TOPO_INVALID_TRIGGER = 0,
   /**< when we come out of signal or buf trigger, invalid trigger is set.
    *  This helps if we check curr_trigger in some other context. */
   GEN_TOPO_DATA_TRIGGER=1,
   /**< data/buffer trigger caused current topo process */
   GEN_TOPO_SIGNAL_TRIGGER=2
   /**< signal/interrupt/timer trigger caused current topo process */
} gen_topo_trigger_t;

// check like this to avoid KW error
#define GEN_TOPO_INDEX_OF_TRIGGER(topo_trigger) (( (topo_trigger) == GEN_TOPO_DATA_TRIGGER) ? 0 : 1 )

/**
 *
 */
typedef struct gen_topo_process_info_t
{
   bool_t   *release_out_buf_ptr;       /**< when media fmt changes if old data existed in buf, that must be released even if it
                                             doesn't contain enough frames (same with timestamp discontinuity, when out buf is too small).
                                             input buf might still be present & we might have to continue processing after releasing out buf.
                                             size = gen_topo_process_context_t:num_ext_out_ports */
   bool_t   port_thresh_event;          /**< if port-thresh event is raised during process, then come back to process without reading more input.
                                             output buffer got recreated. Try to process before returning and waiting for input.
                                             Decoder: read first frame, raise threshold. If we wait for next input, it will
                                             overwrite timestamp of first buffer.
                                             before waiting for next input, we need to finish processing first
                                             frame using newly created output buffers.*/
   bool_t   ext_trigger_cond_not_met;   /**< external trigger condition is not met */
   bool_t   anything_changed;           /**< in process calls either inputs should get consumed, or output produced or EOF/metadata propagated.
                                             If not we need to wait for ext trigger like cmd or data or buf or timer*/

} gen_topo_process_info_t;

typedef struct gen_topo_port_scratch_flags_t
{
   uint32_t            data_pending_in_prev : 1;              /**< prev out port data is copied to next in port before process. but if there's any pending data
                                                                   that must be copied after process, this flag indicates so.
                                                                   Calling gen_topo_check_copy_between_modules may cause MD/flags to go out of sync with data */
   uint32_t            prev_marker_eos : 1;                   /**< topo uses this to determine when eos has left the input or output port (prev_eos 1, cur_eos 0).
                                                                     need to remove this once spl_topo migrates to below flag */
   uint32_t            prev_eos_dfg : 1;                      /**< topo uses this to determine whether EOS/DFG left input or output port.
                                                                   Note that marker_eos is still set when EOS is stuck inside algos,
                                                                   but prev_eos_dfg is based on md list (which results in FALSE if EOS inside algo.
                                                                   keeping marker_eos set helps algos do zero pushing. marker_eos is also used here. */
} gen_topo_port_scratch_flags_t;
/**
 * a scratch memory for storing info such as previous actual len etc.
 */
typedef struct gen_topo_port_scratch_data_t
{
   uint32_t                prev_actual_data_len;
   capi_buf_t              bufs[CAPI_MAX_CHANNELS_V2];        /**< Used only for internal ports.
                                                                  for deinterleaved unpacked, multiple buffers are used.
                                                                  these are just pointers to actual buffers gen_topo_common_port_t.buf */
   module_cmn_md_list_t    *md_list_ptr;                      /**< Metadata list ptr. When module is called in loop, only MD generated in a given call must be
                                                                   present in sdata as their offset is based on new data. Any old MD is kept here as old MD have
                                                                   offset based on beginning of the buffer. Used only for output ports*/
   gen_topo_port_scratch_flags_t flags;
   /**
    * Below fields are only needed in spl_topo.
    * To save memory in gen_topo, it's possible to separate these into a spl_topo_only structure. To do so, gen_topo_process_context_t needs
    * to use void *'s which have a different type for gen_topo/adv_topo. gen_topo_create_modules() would have to take sizes in to allocate
    * the correct sizes. Currently postponing this optimization since it becomes a hassle to reference scratch_data fields (needs a type cast).
    */

   int64_t                timestamp;                          /**< spl_topo uses this for storing the initial output port's timestamp before calling topo process.
                                                                   This is compared against output timestamp for checking timestamp discontinuity. */
   bool_t                 is_timestamp_valid;
} gen_topo_port_scratch_data_t;

/**
 * a scratch memory for storing history info such as previous actual len etc.
 */
typedef struct gen_topo_ext_port_scratch_data_t
{
   uint32_t                prev_actual_data_len;
} gen_topo_ext_port_scratch_data_t;

/**
 * this structure stores hist info only inside a process trigger. not across 2 process triggers.
 */
typedef struct gen_topo_process_context_t
{
   uint32_t                           num_in_ports;              /**< max of max input ports across any module */
   gen_topo_port_scratch_data_t       *in_port_scratch_ptr;      /**< input port scratch data. size num_in_ports. Must index with in_port_ptr->gu.index */

   uint32_t                           num_out_ports;             /**< max of max output ports across any module */
   gen_topo_port_scratch_data_t       *out_port_scratch_ptr;     /**< output port scratch data. size num_out_ports. Must index with  out_port_ptr->gu.index */

   uint32_t                           num_ext_in_ports;               /**< max input ports across any module */
   gen_topo_ext_port_scratch_data_t   *ext_in_port_scratch_ptr;       /**< input port history data. size num_ext_in_ports. Must index with 0 .. num_ext_in_ports-1 */

   uint32_t                           num_ext_out_ports;              /**< max output ports across any module */
   gen_topo_ext_port_scratch_data_t   *ext_out_port_scratch_ptr;      /**< output port history data. size num_ext_out_ports.  Must index with 0 .. num_ext_in_ports-1 */

   gen_topo_process_info_t            process_info;           /**< */

   capi_stream_data_v2_t              **in_port_sdata_pptr;   /**< array of ptr to sdata for calling capi process. size num_in_ports. Must index with in_port_ptr->gu.index*/

   capi_stream_data_v2_t              **out_port_sdata_pptr;  /**< array of ptr to sdata for calling capi process. size num_out_ports. Must index with  out_port_ptr->gu.index */

   gen_topo_trigger_t                 curr_trigger;           /**< current trigger that caused process frames */

} gen_topo_process_context_t;


typedef capi_err_t (*topo_capi_callback_f)(void *context_ptr, capi_event_id_t id, capi_event_info_t *event_info_ptr);

typedef struct gen_topo_timestamp_t
{
   int64_t value; // Microseconds.
   bool_t  valid; // TS validity
} gen_topo_timestamp_t;

/**
 * High precision.
 */
typedef struct gen_topo_hp_timestamp_t
{
   int64_t  ivalue;           // Microseconds (integer).
   uint64_t fvalue;           // Fractional, ns. +ve
   bool_t   ts_valid;         // TS validity
} gen_topo_hp_timestamp_t;

/** Event flag is useful in 2  cases
 * 1. when event is not per port
 * 2. when event is per port, but the event handling is done for all ports at once.
 *
 * It's not useful when event is not per port and event cannot be handled at once, because
 * it's not known when to clear the mask*/
typedef union gen_topo_capi_event_flag_t
{
   struct
   {
      uint32_t    kpps: 1;                /**< Set when kpps and kpss scale factor are changed by the module */
      uint32_t    bw : 1;                 /**< Set when code or data BW are changed by the module */
      uint32_t    port_thresh : 1;        /**< at least one threshold event is pending; thresh prop is pending */
      uint32_t    process_state : 1;
      uint32_t    media_fmt_event : 1;    /**< to track any change in output media fmt of a module and if so, to trigger media fmt dependent global
                                                operations such as threshold, kpps/bw. etc */
      uint32_t    algo_delay_event : 1;   /**< algo delay change */
      uint32_t    data_trigger_policy_change : 1;
      uint32_t    port_prop_is_up_strm_rt_change: 1; /**< is real time port property change event */
      uint32_t    port_prop_is_down_strm_rt_change: 1; /**< is real time port property change event */
   };
   uint32_t word;
} gen_topo_capi_event_flag_t;

typedef struct gen_topo_flags_t
{
   uint32_t any_data_trigger_policy: 1;   /**< is there any module which raised data trigger policy extn FWK_EXTN_TRIGGER_POLICY
                                                                  (note: currently if trigger policy extn is removed this is not reset.*/

   uint32_t is_signal_triggered : 1;      /**< whether container is signal triggered */

   /*** temporary flags */
   uint32_t process_us_gap : 1;           /**< flag indicates that EOS needs to be processed for upstream stop or flush*/
   uint32_t state_prop_going_on : 1;      /**< Set when state propagation is going on (to error out in capi event handling) */
} gen_topo_flags_t;

typedef struct gen_topo_t
{
   gu_t                          gu;                        /**< Graph utils. */
   gen_topo_process_context_t    proc_context;              /**< history data required for data processing */
   gen_topo_capi_event_flag_t    capi_event_flag;
   const gen_topo_vtable_t       *gen_topo_vtable_ptr;      /**< vtable for abstracting topo vs. spl_topo */
   const topo_to_cntr_vtable_t   *topo_to_cntr_vtable_ptr;  /**< vtable with topo to container functions*/
   void *                        buf_mgr_ptr;               /**< buffer manager pointer */
   uint8_t                       module_count;              /**< tracks modules created in this container for logging purposes (only increasing count).
                                                                 See LOG_ID_LOG_MODULE_INSTANCES_MASK)
                                                                 Every module create adds one count.
                                                                 This uniquely identifies a module in this container */
   gen_topo_flags_t              flags;
   POSAL_HEAP_ID                 heap_id;                   /*Heap ID used for all memory allocations in the topology*/
} gen_topo_t;

/**
 * A bitfield for module flags
 */

#define AMDB_MODULE_REG_STRUCT_V1                  1
  /** Interface type is Common Audio Processor Interface */
  #define AMDB_INTERFACE_TYPE_CAPI                   2

  /** Interface version is Common Audio Processor Interface version 3. */
  #define AMDB_INTERFACE_VERSION_CAPI_V3             3

  /** Module is generic, such as a preprocessor, post-processor, etc. */
  #define AMDB_MODULE_TYPE_GENERIC                   2

  /** Module is a decoder. */
  #define AMDB_MODULE_TYPE_DECODER                   3

  /** Module is an encoder. */
  #define AMDB_MODULE_TYPE_ENCODER                   4

  /** Module is a converter. */
  #define AMDB_MODULE_TYPE_CONVERTER                 5

  /** Module is a packetizer. */
  #define AMDB_MODULE_TYPE_PACKETIZER                6

  /** Module is a de-packetizer. */
  #define AMDB_MODULE_TYPE_DEPACKETIZER              7

/** Module is of type framework-module. */
  #define AMDB_MODULE_TYPE_FRAMEWORK (1)

typedef union gen_topo_module_flags_t
{
   struct
   {
      uint32_t inplace             : 1;
      uint32_t requires_data_buf   : 1;
      uint32_t disabled            : 1;         /**< by default modules are enabled.*/

      uint32_t need_data_trigger_in_st  : 1;    /**< Flag to indicate if data_trigger is needed in signal triggered container.
                                                          By default data triggers are not handled in signal triggered container unless this flag is set by the event.
                                                          #FWK_EXTN_EVENT_ID_DATA_TRIGGER_IN_ST_CNTR
                                                          This flag is used to evaluate the topo trigger policy of signal trigger container when a subgraph opens/closes.*/
      uint32_t dm_mode                  : 2;    /**< Indicates operating dm mode [value range is enum gen_topo_dm_mode_t] */

      /** Flags which indicate whether framework extension is required for the module */
      uint32_t need_stm_extn            : 1;    /**< FWK_EXTN_STM (SIGNAL TRIGGERED MODULE)*/
      uint32_t need_pcm_extn            : 1;    /**< FWK_EXTN_PCM */
      uint32_t need_mp_buf_extn         : 1;    /**< FWK_EXTN_MULTI_PORT_BUFFERING */
      uint32_t need_trigger_policy_extn : 1;    /**< FWK_EXTN_TRIGGER_POLICY */
      uint32_t need_cntr_frame_dur_extn : 1;    /**< FWK_EXTN_CONTAINER_FRAME_DURATION*/
      uint32_t need_thresh_cfg_extn     : 1;    /**< FWK_EXTN_THRESHOLD_CONFIGURATION*/
      uint32_t need_proc_dur_extn       : 1;    /**< FWK_EXTN_PARAM_ID_CONTAINER_PROC_DELAY */
      uint32_t need_dm_extn             : 1;    /**< FWK_EXTN_DM */

      /** Flags which record the interface extensions supported by the module
       * Other extensions such as INTF_EXTN_IMCL, INTF_EXTN_PATH_DELAY are not stored */
      uint32_t supports_metadata       : 1;       /**< INTF_EXTN_METADATA: this also means module propagates metadata */
      uint32_t supports_data_port_ops  : 1;       /**< INTF_EXTN_DATA_PORT_OPERATION: this also means module requires data port states. */
      uint32_t supports_prop_port_ds_state : 1;   /**< INTF_EXTN_PROP_PORT_DS_STATE */
      uint32_t supports_prop_is_rt_port_prop : 1; /**< INTF_EXTN_PROP_IS_RT_PORT_PROPERTY */

   };
   uint32_t word;
} gen_topo_module_flags_t;

typedef struct gen_topo_cached_event_node_t
{
   capi_register_event_to_dsp_client_v2_t reg_event_payload;
} gen_topo_cached_event_node_t;

typedef struct gen_topo_trigger_policy_t
{
   fwk_extn_port_nontrigger_group_t          nontrigger_policy;   /**< ports belongs to non-trigger category? */

   fwk_extn_port_trigger_policy_t            port_trigger_policy; /**< port trigger policy */
   uint32_t                                  num_trigger_groups;  /**< groups of ports that can trigger */
   fwk_extn_port_trigger_group_t             *trigger_groups_ptr; /**< trigger groups */
} gen_topo_trigger_policy_t;

typedef struct gen_topo_module_t
{
   gu_module_t                               gu;                  /**< Must be the first element. */
   capi_t                                    *capi_ptr;
   gen_topo_t                                *topo_ptr;
   uint32_t                                  kpps;                /**< Total kpps of this capi for all its ports. needed to check for change. */
   uint32_t                                  algo_delay;          /**< us, for single port modules */
   uint32_t                                  code_bw;             /**< BW in Bytes per sec */
   uint32_t                                  data_bw;             /**< BW in Bytes per sec */
   gen_topo_module_flags_t                   flags;
   gen_topo_module_bypass_t *                bypass_ptr;          /**< if module is disabled & can be bypassed, then it will use this mem to park prev media fmt, kpps etc.
                                                                       note: most SISO modules are bypassed when disabled. MIMO modules cannot be bypassed.
                                                                       If bypass is not possible, then module proc will be called and module needs to take care of out media, kpps etc */
   uint32_t                                  num_proc_loops;      /**< in case LCM thresh is used, then num process loop that need to be called per module */

   /** per port properties applicable to SISO modules only (hence stored in module)*/
   uint32_t                                  pending_zeros_at_eos;/**< pending amount of zeros to be pushed at flushing EoS in bytes (per channel)*/
   module_cmn_md_list_t                      *int_md_list_ptr;    /**< internal metadata list. for SISO modules that don't support metadata prop, MD stays here until algo delay elapses. */
   uint8_t                                   serial_num;          /**< serial number assigned to the module, when it's first created (See LOG_ID_LOG_MODULE_INSTANCES_MASK) */
   uint32_t                                  kpps_scale_factor_q4;   /**< Multiply by scale factor for running at higher speeds if module sets this flag, qfactor Q28.4 */
   gen_topo_trigger_policy_t                 *tp_ptr[GEN_TOPO_MAX_NUM_TRIGGERS];            /**< trigger policy for each trigger type*/

} gen_topo_module_t;

/**
 * Every port same path-id belong to same path. hence delay can be added.
 */
typedef struct gen_topo_delay_info_t
{
   uint32_t                path_id;
   //for MIMO modules, per-in-per-out delay is determined by query (query doesn't exist as of May 2019)
} gen_topo_delay_info_t;

/** bit mask is used */
#define GEN_TOPO_BUF_ORIGIN_INVALID 0
/** buffer is from buf manager */
#define GEN_TOPO_BUF_ORIGIN_BUF_MGR 1
/** buffer is from ext-buf used in the internal port corresponding to the ext port */
#define GEN_TOPO_BUF_ORIGIN_EXT_BUF 2
/** buffer is borrowed from ext-port to an inplace nblc port */
#define GEN_TOPO_BUF_ORIGIN_EXT_BUF_BORROWED 4
/** buffer is borrowed from int-port, specifically, end of an inplace nblc */
#define GEN_TOPO_BUF_ORIGIN_BUF_MGR_BORROWED 8

typedef union gen_topo_port_flags_t
{
   struct
   {
      /** Permanent    */
      uint32_t       port_has_threshold : 1;       /**< module has threshold if it returns thresh >= 1. */
      uint32_t       requires_data_buf : 1;        /**< TRUE if CAPI requires framework to do buffering.
                                                        If FALSE and module has threshold, then module will not be called unless input threshold is met
                                                        (encoder, module like EC) & for output, output must have thresh amount of empty space.
                                                        Enc may also use requires_data_buf = TRUE and do thresh check inside. See comment in gen_topo_capi_get_port_thresh*/
      uint32_t       buf_origin: 4;                /**< GEN_TOPO_BUF_ORIGIN_*. this is a bit mask. */
      uint32_t       is_upstream_realtime : 1;     /**< indicates if the upstream port produces RT data. Assigned only through propagation. */
      uint32_t       is_downstream_realtime : 1;   /**< indicates if the downstream is a RT consumer. Assigned only through propagation. */
      uint32_t       is_state_prop_blocked :1;     /**< indicates if the downstream state propagation is blocked on this port. This is only for output ports. */

      /** temporary: cleared after event handling */
      uint32_t       media_fmt_event : 1;
      uint32_t       port_prop_is_rt_change: 1;    /**< is real time port property change event */
      uint32_t       port_prop_state_change: 1;    /**< state port property propagation event */
   };
   uint32_t          word;

} gen_topo_port_flags_t;

typedef struct gen_topo_common_port_t
{
//   topo_port_state_t             state;                     /**< Downgraded state = downgrade of (self state, connected port state)*/
//   topo_data_flow_state_t        data_flow_state;           /**< data flow state. currently no requirement for output ports. applicable for input port only. */
   topo_media_fmt_t              media_fmt;                 /**< Media format from CAPI*/

   capi_buf_t                    buf;                       /**< some ports (ones with thresh or ones with assigned threshold
                                                                  due to thresh-propagation) have buf created here. Some have buf_ptr as NULL.
                                                                  During process, neighboring buffers are assigned and made NULL after module-process is complete.

                                                                  when this buf is deinterleaved, it's always deinterleaved by max_len (unpacked)
                                                                  */
   capi_stream_data_v2_t         sdata;                     /**< buf in this points to a temp buf array, which in turn is made of buf in above line
                                                                sdata.flags:
                                                                 Reasons for EOF are: a) TS discontinuity b) input media fmt. c) EOF set by the DSP client
                                                                 EOF and flushing EOS both have squeeze behavior*/
   uint64_t                      fract_timestamp;           /**< ns, fractional part in the timestamp of sdata.timestamp. should be +ve*///

   gen_topo_port_flags_t         flags;

   /** Events update the below fields temporarily */
   uint32_t                      port_event_new_threshold;  /**< thresh from module is assigned here.
                                                                  thresh propagation also assigned here. 0 means no change in thresh.
                                                                  1 means no buffer needed.
                                                                  cleared after event is handled. flags.create_buf decides if the buf is created.*/
   intf_extn_data_port_opcode_t  last_issued_opcode;        /**< Last issued port operation is stored to avoid consecutively
                                                              setting same operation.*/

   spf_list_node_t                *delay_list_ptr;           /** list of delay objects gen_topo_delay_info_t */
} gen_topo_common_port_t;


typedef union gen_topo_input_port_flags_t
{
   struct
   {
      uint32_t       processing_began   : 1;          /**< indicates if the module processed from this input port at least once */
      uint32_t       need_more_input    : 1;          /**< indicates whether more input data is needed to continue processing. */
      uint32_t       media_fmt_received : 1;          /**< indicates if this input port has received media format at least once */
      uint32_t       disable_ts_disc_check : 1;       /**< If decoder drops input data, then input TS may no longer be used for
                                                            discontinuity check in gen_topo_check_copy_incoming_ts */
      uint32_t       was_eof_set: 1;                  /**< if eof was set on inport in the last process call, and another eof comes in the next process b4 data, we can simply ignore it.
                                                           When TS discont happens in first module, EOF goes thru all, but subsequent modules also detect EOF again, which can be dropped with this. */
   };
   uint32_t          word;
} gen_topo_input_port_flags_t;

#define GEN_TOPO_IS_IN_BUF_FULL(in_port_ptr)                                                                           \
   (in_port_ptr->common.buf.data_ptr && in_port_ptr->common.buf.max_data_len &&                                        \
    (in_port_ptr->common.buf.max_data_len == in_port_ptr->common.buf.actual_data_len))

typedef struct gen_topo_input_port_t
{
   gu_input_port_t               gu;                  /**< Must be the first element */
   gen_topo_common_port_t        common;
   gen_topo_input_port_flags_t   flags;
   gen_topo_input_port_t         *nblc_start_ptr;     /**< beginning of the non-buffering linear chain. similar to end. */
   gen_topo_input_port_t         *nblc_end_ptr;       /**< end port of non-buffering linear chain.
                                                         beginning port as well as the middle ports will have end ptr
                                                         this is required to find out if there's a downstream i/p trigger at every module o/p port.*/
   uint32_t                      bytes_from_prev_buf; /**< when module returns need more this many bytes are pending in input buffer.
                                                           this is decremented as and when module consumes these bytes. needed for syncing to TS coming from prev module or ext in.
                                                           decoder -> raw input => when bytes_from_prev_buf is nonzero, don't sync to input until decoder
                                                           stops returning need-more. pcm/61937 input => timestamp need to be adjusted for this
                                                           before assigning to input timestamp */
   gen_topo_hp_timestamp_t       ts_to_sync;          /**< timestamp that's yet to be synced to common.sdata*/

} gen_topo_input_port_t;

typedef struct gen_topo_output_port_t
{
//   gu_output_port_t              gu;                /**< Must be the first element */
   gen_topo_common_port_t        common;
   gen_topo_output_port_t        *nblc_start_ptr;     /**< beginning of the non-buffering linear chain. similar to end*/
   gen_topo_output_port_t        *nblc_end_ptr;       /**< end port of non-buffering linear chain.
                                                         beginning port as well as the middle ports will have end ptr
                                                         this is required to find out if there's ext o/p trigger at every module port.*/
} gen_topo_output_port_t;

/**
 * Container specific structure for EoS
 */
typedef struct gen_topo_eos_cargo_t
{
   bool_t                        did_eos_come_from_ext_in;  /**< if flushing EOS came from ext-in */
   void *                        inp_ref;                   /**< to be removed after SPL_CNTR gets rid of it */
   uint32_t                      inp_id;                    /**< Unique ID of the external input port. */
   uint32_t                      ref_count;                 /**< every time split happens (inside the container) this is incremented. */
} gen_topo_eos_cargo_t;

ar_result_t gen_topo_algo_reset(void *   topo_module_ptr,
                                uint32_t log_id,
                                bool_t   is_port_valid,
                                bool_t   is_input,
                                uint16_t port_index);

////////////////////////////////////////////// metadata
ar_result_t gen_topo_create_eos_for_cntr(gen_topo_t *                 topo_ptr,
                                         POSAL_HEAP_ID                heap_id,
                                         module_cmn_md_list_t **      eos_md_list_pptr,
                                         gen_topo_input_port_t *      input_port_ptr,
                                         uint32_t                     input_port_id,
                                         module_cmn_md_eos_payload_t *eos_core_payload_ptr,
                                         module_cmn_md_eos_flags_t *  eos_flag_ptr,
                                         uint32_t                     bytes_in_buf,
                                         topo_media_fmt_t *           media_fmt_ptr);
ar_result_t gen_topo_create_eos_cntr_ref(gen_topo_t *           topo_ptr,
                                         POSAL_HEAP_ID          heap_id,
                                         gen_topo_input_port_t *input_port_ptr,
                                         uint32_t               input_id,
                                         gen_topo_eos_cargo_t **eos_cargo_pptr);

ar_result_t gen_topo_destroy_all_metadata(uint32_t               log_id,
                                          void *                 module_ctx_ptr,
                                          module_cmn_md_list_t **md_list_pptr,
                                          bool_t                 is_dropped);

ar_result_t gen_topo_do_md_offset_math(uint32_t          log_id,
                                       uint32_t *        offset_ptr,
                                       uint32_t          bytes,
                                       topo_media_fmt_t *med_fmt_ptr,
                                       bool_t            need_to_add);

ar_result_t gen_topo_respond_and_free_eos(gen_topo_t *           topo_ptr,
                                          gen_topo_module_t *    module_ptr,
                                          module_cmn_md_list_t * md_list_ptr,
                                          bool_t                 is_eos_rendered,
                                          module_cmn_md_list_t **head_pptr);
bool_t      gen_topo_is_flushing_eos(module_cmn_md_t *md_ptr);

void gen_topo_populate_metadata_extn_vtable(module_info_t *                        module_ptr,
                                            intf_extn_param_id_metadata_handler_t *handler_ptr);


ar_result_t gen_topo_create_dfg_metadata(uint32_t               log_id,
                                         module_cmn_md_list_t **metadata_list_pptr,
                                         POSAL_HEAP_ID          heap_id,
                                         module_cmn_md_t **     dfg_md_pptr);

bool_t gen_topo_md_list_has_buffer_associated_md(module_cmn_md_list_t *list_ptr);

ar_result_t gen_topo_metadata_create(uint32_t               log_id,
                                     module_cmn_md_list_t **md_list_pptr,
                                     uint32_t               size,
                                     POSAL_HEAP_ID          heap_id,
                                     bool_t                 is_out_band,
                                     module_cmn_md_t **     md_pptr);

capi_err_t gen_topo_capi_metadata_destroy(void *                 context_ptr,
                                          module_cmn_md_list_t * md_list_ptr,
                                          bool_t                 is_dropped,
                                          module_cmn_md_list_t **head_pptr);
capi_err_t gen_topo_capi_metadata_modify_at_data_flow_start(void *                 context_ptr,
                                                            module_cmn_md_list_t * md_node_pptr,
                                                            module_cmn_md_list_t **head_pptr);

ar_result_t gen_topo_set_pending_zeros(gen_topo_module_t *module_ptr, gen_topo_input_port_t *in_port_ptr);
ar_result_t gen_topo_propagate_metadata(gen_topo_t *       topo_ptr,
                                        gen_topo_module_t *module_ptr,
                                        bool_t             input_has_metadata_or_eos,
                                        uint32_t           in_bytes_before,
                                        uint32_t           in_bytes_consumed,
                                        uint32_t           out_bytes_produced);
//
ar_result_t gen_topo_capi_set_param(uint32_t log_id, // Taken from gen_topo_capi.h
                                    capi_t * capi_ptr,
                                    uint32_t param_id,
                                    int8_t * payload,
                                    uint32_t size);

/** framework owns metadata propagation? */
static inline bool_t gen_topo_fwk_owns_md_prop(gen_topo_module_t *module_ptr)
{
   if (module_ptr->bypass_ptr)
   {
      // Fwk will always propagate metadata for bypassed modules.
      return TRUE;
   }

   if (module_ptr->flags.supports_metadata)
   {
      return FALSE;
   }

   // if source module needs to introduce metadata, then will have  module_ptr->flags.supports_metadata=TRUE
   if (gu_is_siso_module(&module_ptr->gu) ||
       gu_is_sink_module(&module_ptr->gu) /* || gu_is_source_module(&module_ptr->gu) */)
   {
      return TRUE;
   }

   return FALSE;
}

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // #ifndef GEN_TOPO_H_
