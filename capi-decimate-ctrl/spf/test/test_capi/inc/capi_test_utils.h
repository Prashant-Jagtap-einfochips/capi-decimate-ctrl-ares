#ifndef _CAPI_TEST_UTILS_H
#define _CAPI_TEST_UTILS_H
/*===========================================================================

                  C A P I  T E S T  U T I L I T I E S

  Common definitions, types and functions for CAPI unit Test.

   Copyright (c) 2013, 2017, 2020, 2021 QUALCOMM Technologies, Incorporated.
   All Rights Reserved.
   QUALCOMM Technologies Proprietary.

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------

===========================================================================*/
/* -----------------------------------------------------------------------
** Standard include files
** ----------------------------------------------------------------------- */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "capi.h"
#include "capi_diag.h"
#include "capi_intf_extn_data_port_operation.h"
#include "capi_intf_extn_imcl.h"

#include "ar_error_codes.h"

static const struct ar_to_capi_err_t
{
   ar_result_t ar_err_code;
   capi_err_t  capi_err_code;
} ar_to_capi_err_lut[] = {
   { AR_EOK, CAPI_EOK },
   { AR_EFAILED, CAPI_EFAILED },
   { AR_EBADPARAM, CAPI_EBADPARAM },
   { AR_EUNSUPPORTED, CAPI_EUNSUPPORTED },
   { AR_ENOMEMORY, CAPI_ENOMEMORY },
   { AR_ENEEDMORE, CAPI_ENEEDMORE },
   { AR_ENOTREADY, CAPI_ENOTREADY },
   { AR_EALREADY, CAPI_EALREADY },
};

static inline ar_result_t capi_err_to_mm_result(capi_err_t capi_err)
{
   uint32_t i;

   if (CAPI_SUCCEEDED(capi_err))
   {
      return AR_EOK;
   }

   for (i = 1; i < sizeof(ar_to_capi_err_lut) / sizeof(ar_to_capi_err_lut[0]); ++i)
   {
      if (CAPI_IS_ERROR_CODE_SET(capi_err, ar_to_capi_err_lut[i].capi_err_code))
      {
         return ar_to_capi_err_lut[i].ar_err_code;
      }
   }

   return AR_EFAILED;
}

static inline capi_err_t ar_result_to_capi_err(ar_result_t ar_err)
{
   uint32_t i;
   for (i = 0; i < sizeof(ar_to_capi_err_lut) / sizeof(ar_to_capi_err_lut[0]); ++i)
   {
      if (ar_to_capi_err_lut[i].ar_err_code == ar_err)
      {
         return ar_to_capi_err_lut[i].capi_err_code;
      }
   }

   return CAPI_EFAILED;
}
//const uint32_t wordSize = 30;

#define CAPI_MAX_EVENTS 3       // change if needed
#define CAPI_MAX_INPUT_PORTS 3  // change if needed
#define CAPI_MAX_OUTPUT_PORTS 3 // change if needed
#define MAX_NUM_PORTS 3         // change if needed
#define FORMAT_BUF_SIZE 128
#define wordSize 30
typedef struct _args_t
{
   char *input_filename[CAPI_MAX_INPUT_PORTS];
   char *output_filename[CAPI_MAX_OUTPUT_PORTS];
   char *config_filename;
} args_t;

#define MAX_INTENT_SIZE 512
//static const uint32_t FORMAT_BUF_SIZE = 128;

#define TST_STD_MEDIA_FMT_V2_MIN_SIZE (sizeof(capi_data_format_header_t) + sizeof(capi_standard_data_format_v2_t))
#define SPF_TEST_ALIGN_BY_4(x) ((((uint32_t)(x) + 3) >> 2) << 2)

typedef struct _module_info_t
{
   capi_t *   module_ptr;
   bool_t     is_enabled;
   uint32_t   alg_delay;
   bool_t     is_in_place;
   uint32_t   in_buffer_len[CAPI_MAX_INPUT_PORTS];
   uint32_t   out_buffer_len[CAPI_MAX_OUTPUT_PORTS];
   uint32_t   num_input_port;
   uint32_t   num_output_port;
   FILE *     finp[CAPI_MAX_INPUT_PORTS];
   FILE *     fout[CAPI_MAX_OUTPUT_PORTS];
   FILE *     fCfg;
   bool_t     requires_data_buffering;
   capi_buf_t in_format[CAPI_MAX_INPUT_PORTS];
   capi_buf_t out_format[CAPI_MAX_OUTPUT_PORTS];
   // Preallocating buffers to avoid the need for handling malloc failures.
   int8_t                                 in_format_buf[CAPI_MAX_INPUT_PORTS][FORMAT_BUF_SIZE];
   int8_t                                 out_format_buf[CAPI_MAX_OUTPUT_PORTS][FORMAT_BUF_SIZE];
   bool_t                                 is_media_format_received[CAPI_MAX_INPUT_PORTS];
   uint32_t                               num_reg_events;
   capi_register_event_to_dsp_client_v2_t event_info[CAPI_MAX_EVENTS];
   intf_extn_data_port_id_idx_map_t       port_id_idx_map_arr[MAX_NUM_PORTS];
   bool_t                                 is_in_threshold_raised;
   bool_t                                 is_out_threshold_raised;
} module_info_t;

typedef struct testCommand
{
   char opCode[wordSize];
   capi_err_t (*pFunction)(module_info_t *module);
} testCommand;

void usage(FILE *fp, char *prog_name);

void get_eargs(int32_t argc, char *argv[], args_t *input_args, uint32_t *num_input_port, uint32_t *num_output_port);
capi_err_t GetWord(FILE *fCfg, char word[]);
capi_err_t GetUIntParameter(FILE *fCfg, const char parameterName[], uint32_t *pValues);
capi_err_t GetIntParameter(FILE *fCfg, const char parameterName[], int32_t *pValue);
capi_err_t ReadBufferContents(FILE *fCfg, const uint32_t payloadSize, uint8_t *pPayload);
capi_err_t ReadChannelMapping(FILE *fCfg, const uint32_t num_channels, uint16_t channel_mapping[]);
capi_err_t ReadIdIndexMapping(FILE *fCfg, const uint32_t num_ports, intf_extn_data_port_id_idx_map_t id_idx_arr[]);
capi_err_t ReadUint32List(FILE *fCfg, const uint32_t num_ports, uint32_t ids[]);
uint32_t   capi_tst_get_num_mallocs(void);
uint32_t   capi_tst_get_num_frees(void);
void       capi_tst_set_malloc_failure(int num_mallocs_to_skip);
void       capi_tst_clear_malloc_failure(void);

#endif /* _CAPI_TEST_UTILS_H */
