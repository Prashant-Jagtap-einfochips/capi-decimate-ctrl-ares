#ifndef CAPI_ENTRY_FUCNTION_EXTERNS_H
#define CAPI_ENTRY_FUCNTION_EXTERNS_H

/*==============================================================================
  @file capi_entry_function_externs.h
  @brief capi entry function pointer externs
  ==============================================================================*/

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

/*------------------------------------------------------------------------------
 * Include Files
 *----------------------------------------------------------------------------*/
#include "capi.h"

extern capi_err_t (*capi_entry_get_static)(capi_proplist_t *, capi_proplist_t *);
extern capi_err_t (*capi_entry_init)(capi_t *, capi_proplist_t *);

extern capi_err_t (*capi_entry_get_static_1)(capi_proplist_t *, capi_proplist_t *);
extern capi_err_t (*capi_entry_init_1)(capi_t *, capi_proplist_t *);

#endif /* #ifndef CAPI_ENTRY_FUCNTION_EXTERNS_H */
