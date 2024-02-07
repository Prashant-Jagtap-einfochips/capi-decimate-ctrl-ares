/* ======================================================================== */
/**
   @file capi_utils.h

   Header file to implement utility functions for CAPI Interface for
   DECIMATE example
 */

/* =========================================================================
   Copyright (c) 2015-2021 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef CAPI_DECIMATE_UTILS_H
#define CAPI_DECIMATE_UTILS_H

#include "capi.h"
#include "ar_msg.h"
#include "string.h"

#include "capi_decimate_utils.h"
#include "shared_lib_api.h"
//#include "stringl.h"


#include "capi_cmn_imcl_utils.h"
#include "capi_cmn.h"
#include "capi_cmn_ctrl_port_list.h"
#include "capi_intf_extn_imcl.h"
//#include "imcl_decimate_control_api.h"
#include "spf_list_utils.h"

#ifdef __cplusplus
extern "C" {
#endif
#define CAPI_DECIMATE_MAX_IN_PORTS 1
#define CAPI_DECIMATE_MAX_OUT_PORTS 1
#define CAPI_DECIMATE_MAX_CHANNELS 2
#define TEMP_BUF_SIZE_IN_SAMPLES 960
#define DECIMATE_DELAY_US 1000
#define PCM_Q_FACTOR_15 15
#define BYTE_CONV_FACTOR 3
#define FRAME_SIZE 1 // ms
#define MAX_IMCL_PORT_SIZE 50

static const uint32_t CAPI_DECIMATE_STACK_SIZE      = 4096;
static const uint32_t CAPI_DECIMATE_MAX_SAMPLE_RATE = 384000 / 2; // Max Sample rate Supported by the DECIMATE Library

static inline uint32_t decimate_align_to_8_byte(const uint32_t num)
{
   return ((num + 7) & (0xFFFFFFF8));
}

typedef struct io_buffer_ptr
{
   uint32_t input_num_samples;
   uint32_t output_num_samples;
   uint32_t words_per_samples; // 1 for 16 bit, 2 for 32 bit
   void *   inp_ptrs[CAPI_DECIMATE_MAX_CHANNELS];
   void *   out_ptrs[CAPI_DECIMATE_MAX_CHANNELS];
} io_buffer_ptr_t;

typedef struct capi_decimate_media_fmt
{
   capi_set_get_media_format_t    header;
   capi_standard_data_format_v2_t format;
   uint16_t                       channel_type[CAPI_MAX_CHANNELS_V2];
} capi_decimate_media_fmt_t;

typedef struct capi_decimate_events_config
{
   uint32_t enable;
   uint32_t KPPS;
   uint32_t delay;
} capi_decimate_events_config_t;

typedef struct capi_decimate_lib_config
{
   uint32_t decimatest_frame;
   uint32_t frame_process_mode;
   int32_t  numOutPredSamples;
   int32_t  numInPredSamples;
   int32_t  config_params[20];
} capi_decimate_lib_config_t;

typedef struct capi_decimate_memory
{
   int16_t *coeff_ptr;
   int16_t *temp_lin_ptr;
   int16_t *temp_rin_ptr;

} capi_decimate_memory_t;

typedef enum
{
	US_DETECT_CTRL_PORT_INFO_NOT_RCVD = 0,
	US_DETECT_CTRL_PORT_INFO_RCVD
}us_detect_is_ctrl_port_info_rcvd_t;

typedef struct capi_decimate
{
   capi_t                        vtbl;
   capi_event_callback_info_t    cb_info;
   capi_decimate_media_fmt_t     input_media_fmt[CAPI_DECIMATE_MAX_IN_PORTS];
   capi_decimate_media_fmt_t     output_media_fmt[CAPI_DECIMATE_MAX_OUT_PORTS];
   capi_decimate_lib_config_t    lib_config;
   capi_decimate_memory_t        decimate_mem;
   capi_decimate_events_config_t events_config;
   io_buffer_ptr_t               data_ptrs;
   uint32_t                      enable_flag;
   uint32_t                      decimation_factor;
   uint8_t                       coeff_val[4096];
   uint32_t *                    out_samples;
   uint32_t                      threshold_in_bytes;
   bool_t                        is_mf_received; // set this flag when you receive input media format
   
   /* IMCL */
   us_detect_is_ctrl_port_info_rcvd_t   	  is_ctrl_port_received; 
   capi_heap_id_t heap_info;
   ctrl_port_list_handle_t ctrl_port_info; 
   capi_port_num_info_t       ports;
   ctrl_port_data_t *ctrl_port_ptr;

   // List 
   spf_list_node_t *  node_list_ptr;
   uint32_t           port_counter;

} capi_decimate_t;

capi_err_t capi_decimate_process_set_properties(capi_decimate_t *me_ptr, capi_proplist_t *proplist_ptr);
capi_err_t capi_decimate_process_get_properties(capi_decimate_t *me_ptr, capi_proplist_t *proplist_ptr);

void capi_decimate_raise_process_event(capi_decimate_t *me_ptr);
void capi_decimate_init_media_fmt(capi_decimate_t *me_ptr);
void capi_decimate_release_memory(capi_decimate_t *me_ptr);
void capi_decimate_raise_event(capi_decimate_t *me_ptr);
void capi_decimate_raise_output_media_format_event(capi_decimate_t *me_ptr);
void capi_decimate_raise_delay_event(capi_decimate_t *me_ptr);
int  capi_decimate_get_kpps(capi_decimate_t *me_ptr);
void capi_decimate_raise_threshold_event(capi_event_callback_info_t *cb_info_ptr,
                                         uint32_t                    threshold_bytes,
                                         bool_t                      is_input_port,
                                         uint32_t                    port_index);

#ifdef __cplusplus
}
#endif
#endif /* CAPI_DECIMATE_UTILS_H */
