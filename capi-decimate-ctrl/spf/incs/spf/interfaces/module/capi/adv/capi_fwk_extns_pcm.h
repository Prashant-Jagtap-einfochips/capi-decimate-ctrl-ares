#ifndef ELITE_FWK_EXTNS_PCM_H
#define ELITE_FWK_EXTNS_PCM_H

/**
  @file capi_fwk_extns_pcm.h

  @brief fwk extension to take care of PCM use cases.


*/
/*==============================================================================
Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

/*==============================================================================
  Edit History

  $Header:

$

  when        who       what, where, why
  --------    ---       -------------------------------------------------------

==============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*------------------------------------------------------------------------------
 * Include Files
 *----------------------------------------------------------------------------*/
#include "capi_types.h"

/**

   For certain PCM use cases, the PCM modules such as converter, decoder and encoder
   need the framework to support extended media format, support setting of performance mode.
   FWK_EXTN_PCM covers these requirements.
*/

/**
@{ */

/** Unique identifier of the framework extension.
 */
#define FWK_EXTN_PCM             0x0A001000

/*------------------------------------------------------------------------------
 * Parameter IDs
 *----------------------------------------------------------------------------*/

/** ID of the extension to the input media format param

   For input media format:
      This param is always set before the CAPI_INPUT_MEDIA_FORMAT (V2)
      Information from this and CAPI_INPUT_MEDIA_FORMAT (V2) need to be handled in tandem.

   For output media format:
      This param is always queried right after CAPI_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED (V2) event or
      the CAPI_OUTPUT_MEDIA_FORMAT property query.
      The information is this and CAPI_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED (V2)
      or the CAPI_OUTPUT_MEDIA_FORMAT property query
      are handled in tandem.

 */
#define FWK_EXTN_PCM_PARAM_ID_MEDIA_FORMAT_EXTN       0x0A001001

typedef struct fwk_extn_pcm_param_id_media_fmt_extn_t
{
   uint32_t bit_width;
   /**< CAPIv2 media format has a bits_per_sample, which actually stands for sample word size.
    * the real bit_width is given by this field.
    * E.g. bit width is 24 bits, sample word size is 32, Q format 27.
    *
    * 24 bit bit width data packed in 24 bit has sample word size of 24.
    * 24 bit bit width data placed in 32 bit has sample word size of 32 (unpacked).
    *    this can be done in 2 ways: MSB aligned or LSB aligned.
    *    in MSB aligned Q factor is Q31 (set q_factor to CAPI_DATA_FORMAT_INVALID_VAL).
    *    in LSB aligned Q factor is Q23.
    *    if Q format is Q27, then real_bits_per_sample is 24.
    * 32 bit bit width data can be in Q31 . Word size is always 32. alignment is CAPI_DATA_FORMAT_INVALID_VAL.
    * 16 bit bit width data can be in Q15. Word size is 16 or 32. If 16, alignment is CAPI_DATA_FORMAT_INVALID_VAL.
    *    if 32, alignment can be MSB or LSB aligned.
    * invalid value =  CAPI_DATA_FORMAT_INVALID_VAL*/

   uint32_t alignment;
   /**< Alignment of samples in a word
    * PCM_LSB_ALIGNED
    * PCM_MSB_ALIGNED
    *
    * invalid value =  CAPI_DATA_FORMAT_INVALID_VAL*/

   uint32_t endianness;
   /**< endianness of data
    * PCM_LITTLE_ENDIAN
    * PCM_BIG_ENDIAN
    *
    * invalid value =  CAPI_DATA_FORMAT_INVALID_VAL*/
} fwk_extn_pcm_param_id_media_fmt_extn_t ;


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef ELITE_FWK_EXTNS_PCM_H*/
