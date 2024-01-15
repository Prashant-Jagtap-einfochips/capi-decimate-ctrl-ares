/*===========================================================================

                  POSAL  U T I L I T I E S

  Common definitions, types and functions for CAPI.

   Copyright (c) 2013, 2017, 2020, 2021 QUALCOMM Technologies, Incorporated.
   All Rights Reserved.
   QUALCOMM Technologies Proprietary.

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------

===========================================================================*/

#include "../../test_capi/inc/capi_diag.h"
//#include "capi_intf_extn_data_port_operation.h"
//#include "capi_intf_extn_imcl.h"

/* -----------------------------------------------------------------------
** Standard include files
** ----------------------------------------------------------------------- */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if defined(__cplusplus)
#include <new>
extern "C" {
#endif // __cplusplus
#include <assert.h>
#include "capi_types.h"

void       sns_utils_deregister(void *pointer);
void       sns_utils_destroy(void *pointer);
capi_err_t sns_utils_init(void *ptr, uint32_t iid, const char str[4], POSAL_HEAP_ID id);
capi_err_t sns_utils_register(void *ptr);
capi_err_t sns_utils_poll_and_get_requested_events(void *ptr, void **pptr);
uint32_t   sns_utils_get_motion_detection_state(void *ptr);

#if defined(__cplusplus)
#include <new>
#endif
}
