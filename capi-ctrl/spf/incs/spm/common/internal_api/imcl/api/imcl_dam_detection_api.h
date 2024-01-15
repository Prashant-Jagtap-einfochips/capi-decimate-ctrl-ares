#ifndef IMCL_DAM_DETECTION_API_H
#define IMCL_DAM_DETECTION_API_H

/**
  @file imcl_dam_detection_api.h

  @brief defines the Intent IDs for communication over Inter-Module Control
  Links (IMCL) betweeen Detection Modules (like Voice Wakeup) and DAM module
*/
/*==========================================================================
 * Copyright (c) 2019-2021 Qualcomm Technologies Incorporated
 * All rights reserved
 * Qualcomm Technologies. Proprietary and Confidential
 * =========================================================================*/
/*=============================================================================
  Edit History

  when        who      what, where, why
  --------    ---      -------------------------------------------------------

=============================================================================*/

#include "imcl_spm_intent_api.h"

#ifdef INTENT_ID_AUDIO_DAM_DETECTION_ENGINE_CTRL

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*==============================================================================
  Intent ID -  INTENT_ID_AUDIO_DAM_DETECTION_ENGINE_CTRL
==============================================================================*/
/**< Intent defines the payload structure of the IMCL message.
SVA and Dam modules supports the following functionalities –
1. Channel resizing – This as an input intent ID exposed by Dam module, which allows resizing the channel buffers based
on SVA module’s buffering requirement.
2. Output port data flow control - This as an input intent ID exposed by Dam module. This intent is used to open/close
the Dam output ports gates by SVA module.
3. Best channel output – This intent allows detection engines to send the best channel indices to Dam module. Upon
receiving the best channel indices, Dam module outputs only the best channels from the given output port.
4. FTRT data availably info - This as an output intent ID exposed by Dam module. This intent is used by Dam module to
send the unread data length [FTRT data] length present in the channel buffers to the SVA module. */

/* Parameter used by the  IMC client[Detection engines] to resize the buffers associated
 *  with an output port stream.*/
#define PARAM_ID_AUDIO_DAM_RESIZE 0x0800105C

/* Register as a resize client */
#define AUDIO_DAM_BUFFER_REGISTER_RESIZE 1

/* De-Register an existing resize client */
#define AUDIO_DAM_BUFFER_DEREGISTER_RESIZE 0

/*==============================================================================
   Constants
==============================================================================*/
#define MIN_INCOMING_IMCL_PARAM_SIZE_SVA_DAM                                                                           \
   (sizeof(vw_imcl_header_t) + sizeof(intf_extn_param_id_imcl_incoming_data_t))

/**< Header - Any IMCL message going out of / coming in to the
      Voice Wakeup Module (SVA) will have the
      following header followed by the actual payload.
      The peers have to parse the header accordingly*/
typedef struct vw_imcl_header_t
{
   // specific purpose understandable to the IMCL peers only
   uint32_t opcode;

   // Size (in bytes) for the payload specific to the intent.
   uint32_t actual_data_len;
} vw_imcl_header_t;

/*==============================================================================
  Intent ID -  INTENT_ID_AUDIO_DAM_DETECTION_ENGINE_CTRL
==============================================================================*/
/**< Intent defines the payload structure of the IMCL message.
SVA and Dam modules supports the following functionalities –
1. Channel resizing – This as an input intent ID exposed by Dam module, which allows resizing the channel buffers based
on SVA module’s buffering requirement.
2. Output port data flow control - This as an input intent ID exposed by Dam module. This intent is used to open/close
the Dam output ports gates by SVA module.
3. Best channel output – This intent allows detection engines to send the best channel indices to Dam module. Upon
receiving the best channel indices, Dam module outputs only the best channels from the given output port.
4. FTRT data availably info - This as an output intent ID exposed by Dam module. This intent is used by Dam module to
send the unread data length [FTRT data] length present in the channel buffers to the SVA module. */

/* De-Register an existing resize client */
//#define AUDIO_DAM_BUFFER_DEREGISTER_RESIZE 0

/*==============================================================================
   Constants
==============================================================================*/

/* This param is sent to IMC client[Detection engines] by Dam buffer as a response to Read adjust.
   Un read data length allows the HLOS client to create the first read buffer to read the history
   data in FTRT mode.*/
#define PARAM_ID_AUDIO_DAM_UNREAD_DATA_LENGTH 0x0800105D

/*==============================================================================
   Type Definitions
==============================================================================*/

typedef struct param_id_audio_dam_unread_bytes_t param_id_audio_dam_unread_bytes_t;

/** @h2xmlp_parameter    {"PARAM_ID_AUDIO_DAM_UNREAD_DATA_LENGTH",
                           PARAM_ID_AUDIO_DAM_UNREAD_DATA_LENGTH}
    @h2xmlp_description  {get unread length which are buffered in the history buffer.}
   @h2xmlp_toolPolicy    {Calibration} */

#include "spf_begin_pack.h"
struct param_id_audio_dam_unread_bytes_t
{
   uint32_t unread_in_us;
   /**< @h2xmle_description {get unread data length(in micro secs)from circular buffer.}
         @h2xmle_range      {0..4294967295}
         @h2xmle_default    {0} */
}
#include "spf_end_pack.h"
;
/*==============================================================================
 *    Type Definitions
 * =============================================================================*/
/* Structure definition for Parameter */
typedef struct param_id_audio_dam_buffer_resize_t param_id_audio_dam_buffer_resize_t;

/** @h2xmlp_parameter   {"PARAM_ID_AUDIO_DAM_RESIZE",
                         PARAM_ID_AUDIO_DAM_RESIZE}
    @h2xmlp_description {Resizes a dam circular buffers based on the requirements of a
                         client module[detection engines].}
    @h2xmlp_toolPolicy  {Calibration} */

#include "spf_begin_pack.h"
struct param_id_audio_dam_buffer_resize_t
{
   uint32_t resize_in_us;
   /**< @h2xmle_description {Increase/decrease the circular buffer by given length in micro seconds.}
        @h2xmle_range       {0..4294967295}
        @h2xmle_default     {0} */
}
#include "spf_end_pack.h"
;
/*==============================================================================
   Constants
==============================================================================*/

/* This is a Set param ID to control the data flow at the output port of Dam module.
 * This paramter is set by the detection module to open/close the Dam gate. During open,
   this parameter allows to adjust the read pointer and also provide best channel index
   to the dam module. */
#define PARAM_ID_AUDIO_DAM_DATA_FLOW_CTRL 0x0800105E

/*==============================================================================
   Type Definitions
==============================================================================*/

typedef struct param_id_audio_dam_data_flow_ctrl_t param_id_audio_dam_data_flow_ctrl_t;

/** @h2xmlp_parameter    {"PARAM_ID_AUDIO_DAM_DATA_FLOW_CTRL",
                           PARAM_ID_AUDIO_DAM_DATA_FLOW_CTRL}
    @h2xmlp_description  {Controls the data flow from a output port.}
   @h2xmlp_toolPolicy    {Calibration} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_audio_dam_data_flow_ctrl_t
{
   uint32_t is_gate_open;
   /**< @h2xmle_description { If greater than '0' then the gate is opened, if set to '0' then the
                              gate is closed and stops draining the data.}
         @h2xmle_range       {0..4294967295}
         @h2xmle_default     {0} */

   uint32_t read_offset_in_us;
   /**< @h2xmle_description { Relative offset of the read pointer (in micro secs) before the current write pointer.
                              This is valid only if is_gate_open = TRUE }
        @h2xmle_default     {0} */

   uint32_t num_best_channels;
   /**< @h2xmle_description { number of best channels detected. }
         @h2xmle_range       {0..4294967295}
         @h2xmle_default     {0} */

   uint32_t best_ch_ids[0];
   /**< @h2xmle_description { List of best channel IDs. }
        @h2xmle_default     {0}
        @h2xmle_variableArraySize  { "num_channels" }*/
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* This is a Set param ID to set the best channel indices to the Dam module*/
#define PARAM_ID_AUDIO_DAM_OUTPUT_CH_CFG 0x08001067

/*==============================================================================
   Type Definitions
==============================================================================*/

typedef struct param_id_audio_dam_output_ch_cfg_t param_id_audio_dam_output_ch_cfg_t;

/** @h2xmlp_parameter    {"PARAM_ID_AUDIO_DAM_OUTPUT_CH_CFG",
                           PARAM_ID_AUDIO_DAM_OUTPUT_CH_CFG}
    @h2xmlp_description  { Channel map configuration for the Dam output port associated with this control port. }
   @h2xmlp_toolPolicy    {Calibration} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_audio_dam_output_ch_cfg_t
{
   uint32_t num_channels;
   /**< @h2xmle_description { number of channels of associated output put.}
         @h2xmle_range       {0..4294967295}
         @h2xmle_default     {0} */

   uint32_t channels_ids[0];
   /**< @h2xmle_description { List of channel IDs. }
        @h2xmle_default     {0}
        @h2xmle_variableArraySize  { "num_channels" } */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif // ifdef INTENT_ID_AUDIO_DAM_DETECTION_ENGINE_CTRL

#endif /* #ifndef IMCL_DAM_DETECTION_API_H*/
