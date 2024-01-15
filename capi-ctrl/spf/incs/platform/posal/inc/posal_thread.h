/*========================================================================
Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
========================================================================== */
/**
@file posal_thread.h

@brief This file contains utilities for threads. Threads must be joined to
avoid memory leaks. This file provides functions to create and destroy threads,
and to change thread priorities.
 */

#ifndef POSAL_THREAD_H
#define POSAL_THREAD_H

#include "posal_memory.h"
#include "ar_error_codes.h"
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

   /** @addtogroup posal_thread
@{ */

   /* -----------------------------------------------------------------------
    ** Global definitions/forward declarations
    ** ----------------------------------------------------------------------- */

   /** Handle to a thread. */
typedef void* posal_thread_t;

   /** thread priority number */
   typedef int32_t posal_thread_prio_t;

   /****************************************************************************
    ** Threads
    *****************************************************************************/

   /**
  Creates and launches a thread.

  @datatypes
  #posal_thread_t \n
  #POSAL_HEAP_ID

  @param[out] pTid           Pointer to the thread ID.
  @param[in]  pzThreadName   Pointer to the thread name.
  @param[in]  pStack         Pointer to the location of the pre-allocated stack
                             (NULL causes a new allocation). \n
                             pStack must point to the lowest address in the
                             stack.
  @param[in]  nStackSize     Size of the thread stack.
  @param[in]  nPriority      Thread priority, where 0 is the lowest priority
                             and 255 is the highest priority.
  @param[in]  pfStartRoutine Pointer to the entry function of the thread.
  @param[in]  arg            Pointer to the arguments passed to the entry
                             function. An argument can be to any pointer type.
  @param[in]  heap_id        ID of the heap to which the thread stack is
                             allocated.

  @detdesc
  The thread stack can be passed in as the pStack argument, or pStack=NULL
  indicates that posal allocates the stack internally. If the caller
  provides the stack, the caller is responsible for freeing the stack memory
  after joining the thread.
  @par
  Pre-allocated stacks must be freed after the dying thread is joined. The
  caller must specify the heap in which the thread stack is to be allocated.

  @return
  An indication of success (0) or failure (nonzero).

  @dependencies
  None. @newpage
    */
   ar_result_t posal_thread_launch( posal_thread_t *pTid,
                                   char *pzThreadName,
                                   size_t nStackSize,
                                   posal_thread_prio_t nPriority,
                                   ar_result_t (*pfStartRoutine)(void *),
                                   void* arg,
                                   POSAL_HEAP_ID heap_id);

   /**
  Waits for a specified thread to exit, and collects the exit status.

  @datatypes
  #posal_thread_t

  @param[in]  nTid     Thread ID to wait on.
  @param[out] nStatus  Pointer to the value returned by pfStartRoutine
                       called in posal_thread_launch().

  @return
  None.

  @dependencies
  Before calling this function, the object must be created and initialized.
    */
   void posal_thread_join(posal_thread_t nTid, ar_result_t* nStatus);

   /**
  Queries the thread id of the given thread object.

  @return
  The thread id which is integer value.

  @dependencies

  @newpage
 */
int32_t posal_thread_get_tid(posal_thread_t thread_obj);


/**
  Queries the thread id of the caller.

  @return
  The thread id which is integer value.

  @dependencies

  @newpage
    */
int32_t posal_thread_get_curr_tid(void);

/**
  Get the thread name.

  @return
  None.

  @dependencies
  None.

  @newpage
*/
void posal_thread_get_name(char *name, unsigned char max_len);

/**
  Queries the thread priority of the caller.

  @return
  The thread priority of the caller, where 0 is the lowest priority and 255 is
  the highest priority.

  @dependencies
  Before calling this function, the object must be created and initialized.

  @newpage
*/
posal_thread_prio_t posal_thread_prio_get(void);

/**
  Changes the thread priority of the caller.

  @param[in] nPrio  New priority, where 0 is the lowest priority and 255 is the
                    highest priority.

  @return
  None.

  @dependencies
  Before calling this function, the object must be created and initialized.
*/
void posal_thread_set_prio(posal_thread_prio_t nPrio);

   /** @} */ /* end_addtogroup posal_thread */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // #ifndef POSAL_THREAD_H
