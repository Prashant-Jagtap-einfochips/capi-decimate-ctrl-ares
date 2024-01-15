/*========================================================================
Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
========================================================================= */
/**
@file posal_queue.h

@brief This file contains the queue utilities. Queues must be created and added
to a channel before they can be used. Queues are pushed from the back and can be
popped from either front(FIFO) or back(LIFO). Queues must be destroyed when
they are no longer needed.
*/

#ifndef POSAL_QUEUE_H
#define POSAL_QUEUE_H

#include "posal_types.h"
#include "posal_std.h"
#include "posal_channel.h"
#include "posal_mutex.h"
#include "posal_signal.h"
#include "posal_memory.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/** @addtogroup posal_queue
@{ */

/** Maximum number of characters to use in resource names
  (e.g., thread names).
*/
#define POSAL_DEFAULT_NAME_LEN 16

/** Gets the memory size required by a queue, with a specific number of
  items. @newpage
*/
#define POSAL_QUEUE_GET_REQUIRED_BYTES(nItems) \
                (sizeof(posal_queue_t))

typedef struct posal_queue_element_t
{
   void* q_payload_ptr;
   union
   {
      uint32_t opcode;
      void* q_sec_ptr;
   };
} posal_queue_element_t;

typedef struct posal_queue_element_list_t posal_queue_element_list_t;

struct posal_queue_element_list_t
{
   posal_queue_element_t elem;
   posal_queue_element_list_t* next_ptr;
   posal_queue_element_list_t* prev_ptr;
};

/** Queue that transfers messages between threads.
  The queue implementation is signal based and thread safe. Each queue node is
  always 8 bytes. The number of queue nodes is sized up to powers of 2.

  A queue must be associated with a channel.
 */
typedef struct posal_queue_t posal_queue_t;

struct posal_queue_t
{
   posal_mutex_t        queue_mutex;
   /**< Mutex for thread-safe access to the queue. */

   posal_channel_t      channel_ptr;
   /**< Pointer to the associated channel. */

   posal_queue_element_list_t             *head_ptr;
   /**< Pointer to the head of the circular list of all allocated nodes */

   posal_queue_element_list_t             *tail_ptr;
   /**< Pointer to the tail of the circular list of all allocated nodes */

   posal_queue_element_list_t             *active_head_ptr;

   posal_queue_element_list_t             *active_tail_ptr;

   uint32_t             channel_bit;
   /**< Channel bitfield of this queue. */

   int                  active_nodes;
   /**< Number of nodes currently in use */

   int                  num_nodes;
   /**< Number of queue nodes. */

   int                  max_nodes;
   /**< Number of queue nodes. */

   bool_t               disable_flag;
   /**< Specifies whether the queue is disabled.
        @values
        - TRUE -- Disabled
        - FALSE -- Enabled @tablebulletend */
   char                 name[POSAL_DEFAULT_NAME_LEN];
   /**< Name of the queue. */
   POSAL_HEAP_ID        heap_id;
};

/** Structure containing the attributes to be associated with type posal_queue_t
 */
typedef struct posal_queue_init_attr_t
{
   char_t        name[POSAL_DEFAULT_NAME_LEN];
   /**< Name of the queue. */
   int32_t       max_nodes;
   /**< Max number of queue nodes. */
   int32_t       prealloc_nodes;
   /**< Number of preallocated nodes */
   POSAL_HEAP_ID heap_id;
   /**< Heap ID from which nodes are to be allocated */
}posal_queue_init_attr_t;

/*
 * Should be called at boot to create pool of nodes to be used
 * for the queue
 */

ar_result_t posal_queue_pool_setup(POSAL_HEAP_ID, uint32_t num_arrays);

/*
 * Called only for test framework to not cause mem leaks to be detected
 */
void posal_queue_pool_reset(POSAL_HEAP_ID heap_id);

/*
 * Called only for test framework to not cause mem leaks to be detected
 */
void posal_queue_pool_destroy(POSAL_HEAP_ID heap_id);

/**
  Create a queue with specific attributes. 

  @datatypes
  posal_queue_t, posal_queue_init_attr_t

  @param[in]     attr_ptr   Pointer to the attributes of the queue to be initialized
  @param[in,out] q_ptr      Pointer to the initialized queue.

  @return
  0 -- Success
  @par
  Nonzero -- Failure (for information on mm_result_t error codes, refer to
  @xrefcond{Q1,80-NF774-4,80-NA610-4})

  @dependencies
  None. @newpage
*/
ar_result_t posal_queue_create_v1(posal_queue_t** queue_pptr, posal_queue_init_attr_t *attr_ptr);

/**
  Destroys a queue.

  @note1hang This function will be deprecated in a future release, at which
             time posal_queue_deinit() must be used instead.

  @datatypes
  posal_queue_t

  @param[in,out] q_ptr   Pointer to the queue.

  @return
  None.

  @dependencies
  Before calling this function, the object must be created and initialized.
*/
void posal_queue_destroy(posal_queue_t* q_ptr);

/**
  Disables a queue.

  @datatypes
  posal_queue_t

  @param[in] q_ptr   Pointer to the queue.

  @return
  0 -- Success
  @par
  Nonzero -- Failure (for information on mm_result_t error codes, refer to
  @xrefcond{Q1,80-NF774-4,80-NA610-4})

  @dependencies
  Before calling this function, the object must be created and initialized.
  @newpage
*/
ar_result_t posal_queue_disable(posal_queue_t* q_ptr);

/**
  Pushes an item onto a queue.

  @datatypes
  posal_queue_t

  @param[in] q_ptr        Pointer to the queue.
  @param[in] pPayload  Pointer to the address (location) of the item. \n
                       The item is pushed (copied) onto the queue, not the
                       queue address.

  @detdesc
  This function is nonblocking. The user is responsible for not overflowing
  the queue to avoid getting an unwanted assertion.

  @return
  0 -- Success
  @par
  Nonzero -- Failure (for information on mm_result_t error codes, refer to
  @xrefcond{Q1,80-NF774-4,80-NA610-4})

  @dependencies
  Before calling this function, the object must be created and initialized.
  @newpage
*/
ar_result_t posal_queue_push_back(posal_queue_t* q_ptr, posal_queue_element_t* payload_ptr);

/**
  Pops an item from the front of a queue.

  @datatypes
  posal_queue_t

  @param[in] q_ptr        Pointer to the queue.
  @param[in] pPayload  Pointer to the target address (location) for the item
                       that is popped.

  @detdesc
  This function is nonblocking and returns ADSP_ENOMORE if it is empty.
  @par
  Typically, the client calls this function only after waiting for a channel
  and checking whether this queue contains any items.

  @return
  0 -- Success
  @par
  Nonzero -- Failure (for information on mm_result_t error codes, refer to
  @xrefcond{Q1,80-NF774-4,80-NA610-4})

  @dependencies
  Before calling this function, the object must be created and initialized.
  @newpage
*/
ar_result_t posal_queue_pop_front(posal_queue_t* q_ptr,  posal_queue_element_t* payload_ptr);

/**
  Peeks an item from the front of a queue.

  @datatypes
  posal_queue_t

  @param[in] q_ptr        Pointer to the queue.
  @param[in] pPayload  Pointer to the target address (location) for the
                       address of the item that is peeked into.

  @detdesc
  This function is nonblocking and returns ADSP_ENOMORE if it is empty.
  @par
  Typically, the client calls this function only after waiting for a channel
  and checking whether this queue contains any items.

  @return
  0 -- Success
  @par
  Nonzero -- Failure (for information on mm_result_t error codes, refer to
  @xrefcond{Q1,80-NF774-4,80-NA610-4})

  @dependencies
  Before calling this function, the object must be created and initialized.
  @newpage
*/
ar_result_t posal_queue_peek_front(posal_queue_t *queue_ptr, posal_queue_element_t **payload_ptr);

/**
  Pops an item from the back of a queue.

  @datatypes
  posal_queue_t

  @param[in] q_ptr        Pointer to the queue.
  @param[in] pPayload  Pointer to the target address (location) for the item
                       that is popped.

  @detdesc
  This function is for LIFO queues. It is nonblocking and returns ADSP_ENOMORE
  if it is empty.
  @par
  Typically, the client calls this function only after waiting for a channel
  and checking whether this queue contains any items.

  @return
  0 -- Success
  @par
  Nonzero -- Failure (for information on mm_result_t error codes, refer to
  @xrefcond{Q1,80-NF774-4,80-NA610-4})

  @dependencies
  Before calling this function, the object must be created and initialized.
  @newpage
*/
ar_result_t posal_queue_pop_back(posal_queue_t* q_ptr,  posal_queue_element_t* payload_ptr );

/**
  Queries a queue for its channel.

  @datatypes
  posal_queue_t

  @param[in] q_ptr   Pointer to the queue.

  @return
  A handle to the channel containing the queue.

  @dependencies
  Before calling this function, the object must be created and initialized.
*/
posal_channel_t posal_queue_get_channel(posal_queue_t *q_ptr);

/**
  Queries a queue for its channel bitfield.

  @datatypes
  posal_queue_t

  @param[in] q_ptr   Pointer to the queue.

  @detdesc
  This value is a 32-bit value with a single bit=1.
  @par
  Use this function to find the values to be ORed together to form an enable
  bitfield for a combination of queues in a channel.

  @return
  A bitfield with this queue's bit set to 1.

  Before calling this function, the object must be created and initialized.
*/
uint32_t posal_queue_get_channel_bit(posal_queue_t* q_ptr);

/**
  Checks if a trigger is received for the queue.

  @datatypes
  posal_queue_t

  @param[in] q_ptr          Pointer to the queue.
  @detdesc
  This is a non blocking operation which polls/checks if a signal is set on the
  queue indicating that the queue is not empty.

  @return
  Returns a non zero value if the queue signal is set. The value is equal to
  queue's bit mask.

  @dependencies
  Before calling this function, the object must be created and initialized.
  @newpage
*/
uint32_t posal_queue_poll(posal_queue_t* q_ptr);


/****************************************************************************
** Static Inline functions for queues
*****************************************************************************/
/** Setup the default attributes for the queue */
static inline void posal_queue_attr_init(posal_queue_init_attr_t *attr_ptr)
{
   attr_ptr->name[0]        = 0;
   attr_ptr->prealloc_nodes = 0;
   attr_ptr->max_nodes      = 0;
   attr_ptr->heap_id        = POSAL_HEAP_DEFAULT;
}

/** Setup the attribute 'name' for the queue */
static inline void posal_queue_attr_set_name(posal_queue_init_attr_t *attr_ptr, char_t *name_ptr)
{
   posal_strlcpy(attr_ptr->name, name_ptr, POSAL_DEFAULT_NAME_LEN);
}

/** Setup the attribute 'max_nodes' for the queue */
static inline void posal_queue_attr_set_max_nodes(posal_queue_init_attr_t *attr_ptr, int max_nodes)
{
   attr_ptr->max_nodes = max_nodes;
}

/** Setup the attribute 'prealloc_nodes' for the queue */
static inline void posal_queue_attr_set_prealloc_nodes(posal_queue_init_attr_t *attr_ptr, int prealloc_nodes)
{
   attr_ptr->prealloc_nodes = prealloc_nodes;
}

/** Setup the attribute 'heap_id' for the queue */
static inline void posal_queue_attr_set_heap_id(posal_queue_init_attr_t *attr_ptr, POSAL_HEAP_ID heap_id)
{
   attr_ptr->heap_id = heap_id;
}

/** Function to set up all the available queue attributes   */
ar_result_t posal_queue_set_attributes(posal_queue_init_attr_t *q_attr_ptr,
                                       POSAL_HEAP_ID            heap_id,
                                       uint32_t                 num_max_q_elem,
                                       uint32_t                 num_max_prealloc_q_elem,
                                       char_t *                 q_name_ptr);

/** @} */ /* end_addtogroup posal_queue */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // #ifndef POSAL_QUEUE_H
