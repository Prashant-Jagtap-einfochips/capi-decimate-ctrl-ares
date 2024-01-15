#ifndef __CAPI_TEST_H
#define __CAPI_TEST_H
/*===========================================================================

                  C A P I V 2  T E S T   U T I L I T I E S

  Common definitions, types and functions for CAPI.*/

/*===========================================================================*/

/* =========================================================================
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ========================================================================== */

/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------
===========================================================================*/
#include "capi.h"
#include "capi_test_utils.h"
#include "posal_utils.h"
#include "detection_cmn_api.h"
#include "ar_msg.h"
//#include "apm_api.h"
#include "new_capi_test.h"
#include "gen_topo_metadata.h"

#ifdef AR_MSG_USE_AR_DBG_LOG_COSIM
#undef AR_MSG_USE_AR_DBG_LOG_COSIM
#endif

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

capi_event_callback_info_t capi_tst_get_cb_info(module_info_t *module);

capi_err_t RunTest(module_info_t *    module,
                   uint32_t           num_modules,
                   const testCommand *pExtendedCmdSet    = NULL,
                   const uint32_t     extendedCmdSetSize = 0);

#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // __CAPI_TEST_H
