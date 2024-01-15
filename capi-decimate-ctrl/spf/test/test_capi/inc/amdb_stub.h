/* =========================================================================
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ========================================================================== */
#ifndef AMDB_STUB_H
#define AMDB_STUB_H
#include "capi.h"
#include "capi_types.h"
#include "posal.h"

typedef struct gk_list_node_t gk_list_node_t;

typedef struct amdb_capi_t amdb_capi_t;
typedef void (*amdb_get_modules_callback_f)(void *callback_context);

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

ar_result_t amdb_init(POSAL_HEAP_ID heap_id);

void amdb_release_module_handles(gk_list_node_t *module_handle_info_list_ptr)
{
}

void amdb_request_module_handles(gk_list_node_t *            module_handle_info_list_ptr,
                                 amdb_get_modules_callback_f callback_function,
                                 void *                      callback_context)
{
}

capi_err_t amdb_capi_get_static_properties_f(void *           handle_ptr,
                                             capi_proplist_t *init_set_properties,
                                             capi_proplist_t *static_properties)
{
   return CAPI_EOK;
}

capi_err_t amdb_capi_init_f(void *handle_ptr, capi_t *_pif, capi_proplist_t *init_set_properties)
{
   return CAPI_EOK;
}

#ifdef __cplusplus
}
#endif //__cplusplus
#endif // AMDB_STUB_H
