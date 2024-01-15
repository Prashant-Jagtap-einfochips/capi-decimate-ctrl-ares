/**
 *   \file posal_mutex.c
 * \brief 
 *  	This file contains utilities for using mutex functionalities.
 * 
 * \copyright
 *    Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*

*/
// clang-format on

/* =======================================================================
INCLUDE FILES FOR MODULE
========================================================================== */
#include "posal.h"
#include "qurt.h"
#include "qurt_pimutex.h"

/*--------------------------------------------------------------*/
/* Global variables definitions                                            */
/* -------------------------------------------------------------*/

/*
 * This definition is necessary because the
 * */
typedef struct
{
   qurt_mutex_t mutex; /**< QuRT mutex type. */
} posal_qurt_mutex_t;

/**
 Create and initializes a mutex. Recursive mutexes are always used.

 @datatypes
 #posal_mutex_t

 @param[in] pposal_mutex   Pointer to the mutex to initialize.

 @return
 Error code.

 @dependencies
 None. @newpage
 */
ar_result_t posal_mutex_create(posal_mutex_t *pposal_mutex, POSAL_HEAP_ID heap_id)
{
#ifdef DEBUG_POSAL_MUTEX
   AR_MSG(DBG_HIGH_PRIO, "posal_mutex_create");
#endif
   if (NULL == pposal_mutex)
   {
      AR_MSG(DBG_FATAL_PRIO, "Invalid input argument");
      return AR_EFAILED;
   }

   void *temp = posal_memory_malloc(sizeof(posal_qurt_mutex_t), heap_id);
   if (NULL == temp)
   {
      *pposal_mutex = NULL;
      AR_MSG(DBG_FATAL_PRIO, "Failed to allocate memory for mutex.");
      return AR_ENOMEMORY;
   }
#ifdef DEBUG_POSAL_MUTEX
   AR_MSG(DBG_HIGH_PRIO, "qurt_pimutex_init");
#endif
   qurt_pimutex_init((qurt_mutex_t *)temp);

   *pposal_mutex = (posal_mutex_t)temp;
   // The desired attributes are the default attributes.

   return AR_EOK;
}

/**
 Detroys a mutex. This function must be called for each corresponding
 posal_mutex_init() function to clean up all resources.

 @datatypes
 #posal_mutex_t

 @param[in] pposal_mutex   Pointer to the mutex to destroy.

 @return
 None.

 @dependencies
 The object must have been created and initialized before calling this
 function.
 */
void posal_mutex_destroy(posal_mutex_t *pposal_mutex)
{
   qurt_pimutex_destroy((qurt_mutex_t *)*pposal_mutex);
   posal_memory_free((void *)*pposal_mutex);
   *pposal_mutex = NULL;
}

/**
  Attempts to lock a mutex. If the lock is already held, a failure is returned.

  @datatypes
  #posal_mutex_t

  @param[in] pposal_mutex   Pointer to the mutex to try locking.

  @return
  An indication of success (0) or failure (nonzero).

  @dependencies
  The object must have been created and initialized before calling this
  function. @newpage
 */
ar_result_t posal_mutex_try_lock(posal_mutex_t posal_mutex)
{
   return qurt_pimutex_try_lock((qurt_mutex_t *)posal_mutex);
}

/** @} */ /* end_addtogroup posal_mutex */
