#ifndef CAPI_ENTRY_FUCNTIONS_H
#define CAPI_ENTRY_FUCNTIONS_H

/*==============================================================================
  @file capi_entry_functions.c
  @brief capi entry function pointers
  ==============================================================================*/

/*==============================================================================
  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

/*==============================================================================
  Edit History

  when        who       what, where, why
  --------    ---       -------------------------------------------------------

==============================================================================*/

/*------------------------------------------------------------------------------
 * Include Files
 *----------------------------------------------------------------------------*/
#include "capi_decimate.h"
#include "capi_entry_function_externs.h"

capi_err_t (*capi_entry_get_static)(capi_proplist_t *, capi_proplist_t *) = &capi_decimate_get_static_properties;
capi_err_t (*capi_entry_init)(capi_t *, capi_proplist_t *)                = &capi_decimate_init;

#endif /* #ifndef CAPI_ENTRY_FUCNTIONS_H*/
