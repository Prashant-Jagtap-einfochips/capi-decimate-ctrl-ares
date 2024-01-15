/*===============================================================================================
 * FILE:        libstd_std_scanul.c
 *
 * DESCRIPTION: Implementation of a secure API memscpy - Size bounded memory copy. 
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
size_t memscpy(void *dst, size_t dst_size, const void *src, size_t src_size)
{
  size_t  copy_size = (dst_size <= src_size)? dst_size : src_size;

  memcpy(dst, src, copy_size);

  return copy_size;
}
#ifdef __cplusplus
    } //extern "C"
} //namespace std
#endif //__cplusplus
