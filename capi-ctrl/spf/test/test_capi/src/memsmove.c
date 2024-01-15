/*===============================================================================================
 * FILE:        libstd_std_scanul.c
 *
 * DESCRIPTION: Implementation of a secure API memsmove - Size bounded memory move.
 *
 * Copyright (c) 2012, 2013, 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *===============================================================================================*/
 
/*===============================================================================================
 *
 *                            Edit History
 *
 *===============================================================================================*/
 
#include "stringl.h"

#ifdef __cplusplus
namespace std
{
extern "C" {
#endif //__cplusplus

size_t memsmove(void *dst, size_t dst_size, const void *src, size_t src_size)
{
  size_t  copy_size = (dst_size <= src_size)? dst_size : src_size;

  memmove(dst, src, copy_size);

  return copy_size;
}
#ifdef __cplusplus
    } //extern "C"
} //namespace std
#endif //__cplusplus
