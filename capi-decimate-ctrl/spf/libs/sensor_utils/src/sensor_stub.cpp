/* =========================================================================
  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ========================================================================== */
#include "sensors_api.h"
#include "sensor_events.h"
#include "stringl.h"
void sns_utils_deregister(void *pointer)
{
   pointer = NULL;
}

void sns_utils_destroy(void *pointer)
{
   pointer = NULL;
   return;
}

capi_err_t sns_utils_init(void *ptr, uint32_t iid, const char str[4], POSAL_HEAP_ID id)
{
   return CAPI_EOK;
}

capi_err_t sns_utils_register(void *ptr)
{
   return CAPI_EOK;
}

capi_err_t sns_utils_poll_and_get_requested_events(void *ptr, void **pptr)
{
   return CAPI_EOK;
}

uint32_t sns_utils_get_motion_detection_state(void *ptr)
{
   return 0;
}
