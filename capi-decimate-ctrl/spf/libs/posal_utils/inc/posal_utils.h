#ifndef _POSAL_UTILS_H
#define _POSAL_UTILS_H
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

/* -----------------------------------------------------------------------
** ADSP and POSAL include files
** ----------------------------------------------------------------------- */
//#include "adsp_audproc_api.h"
//#include "adsp_asm_api.h"
#include "posal.h"
#include "posal_memory.h"
#include "ar_msg.h"
/* ------------------------------------------------------------------------
 ** POSAL Memory Definitions
 ** ------------------------------------------------------------------------ */
#if 0 // posal.h
typedef enum
{
   POSAL_HEAP_DEFAULT=0,         
   POSAL_HEAP_OUT_OF_RANGE      //keep at end
}  POSAL_HEAP_ID;

void* posal_memory_malloc(uint32_t unBytes, POSAL_HEAP_ID heapId);
void posal_memory_free(void *ptr);

#if defined(__cplusplus)
#define posal_memory_placement_new(pObj, pMemory, typeType, ...)                                                       \
   {                                                                                                                   \
      (pObj) = (new (pMemory)(typeType)(__VA_ARGS__));                                                                 \
   \
}

#define posal_memory_placement_delete(pObject, typeType)                                                               \
   {                                                                                                                   \
      if (pObject)                                                                                                     \
      {                                                                                                                \
         (pObject)->~typeType();                                                                                       \
         (pObject) = NULL;                                                                                             \
      }                                                                                                                \
   \
}

#define posal_memory_new(pObject, typeType, heapId, ...)                                                               \
   {                                                                                                                   \
      void *pObj = posal_memory_malloc(sizeof(typeType), heapId);                                                      \
      (pObject)  = (pObj) ? (new (pObj)(typeType)(__VA_ARGS__)) : NULL;                                                \
   \
}

#define posal_memory_delete(pObject, typeType)                                                                         \
   {                                                                                                                   \
      if (pObject)                                                                                                     \
      {                                                                                                                \
         (pObject)->~typeType();                                                                                       \
         posal_memory_free(pObject);                                                                                   \
         (pObject) = NULL;                                                                                             \
      }                                                                                                                \
   \
}

#endif // __cplusplus
#endif // posal.h
#if defined(__cplusplus)
#include <new>
}
#endif // __cplusplus
#endif //_POSAL_UTILS_H
