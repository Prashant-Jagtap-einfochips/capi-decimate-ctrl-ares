/**
 *   \file posal_mutex_island.c
 * \brief
 *  This file contains utilities for using mutex functionalities.
 *
 *
 * \copyright
 *    Copyright (c) 2020, 2021 Qualcomm Technologies, Inc.
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
#include "posal_internal_inline.h"
#include "qurt.h"
#include "qurt_pimutex.h"

/*--------------------------------------------------------------*/
/* Global variables definitions                                            */
/* -------------------------------------------------------------*/

/**
  Locks a mutex. Recursive mutexes are always used.

  @datatypes
  #posal_mutex_t

  @param[in] pposal_mutex   Pointer to the mutex to lock.

  @return
  None.

  @dependencies
  The object must have been created and initialized before calling this
  function.
 */
void posal_mutex_lock(posal_mutex_t posal_mutex)
{
   posal_mutex_lock_inline(posal_mutex);
}

/**
  Unlocks a mutex. Recursive mutexes are always used.

  @datatypes
  #posal_mutex_t

  @param[in] pposal_mutex   Pointer to the mutex to unlock.

  @return
  None.

  @dependencies
  The object must have been created and initialized before calling this
  function.
 */
void posal_mutex_unlock(posal_mutex_t posal_mutex)
{
   posal_mutex_unlock_inline(posal_mutex);
}
