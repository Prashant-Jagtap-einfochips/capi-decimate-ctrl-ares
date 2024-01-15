/* =========================================================================
  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ========================================================================== */
#include "shared_lib_api.h"
#include "posal.h"

// SPF_IMPORT_BEGIN
//#include "posal_pm_wrapper.h"
// SPF_IMPORT_END

uint8_t posal_debugmsg_lowest_prio = 4;

// SPF_IMPORT_BEGIN
/*
ar_result_t posal_pm_wrapper_register(pm_server_register_t register_info,
                                      pm_server_handle_t **pm_handle_pptr,
                                      posal_signal_t      wait_signal_ptr,
                                      uint32_t             log_id)
{
   return AR_EOK;
}
*/
// SPF_IMPORT_END

void posal_mutex_init(posal_mutex_t *pposal_mutex)
{
   return;
}

ar_result_t posal_mutex_create(posal_mutex_t *pposal_mutex)
{
   return AR_EOK;
}

void posal_mutex_destroy(posal_mutex_t *pposal_mutex)
{
   return;
}

void posal_mutex_lock(posal_mutex_t posal_mutex)
{
   return;
}

void posal_mutex_unlock(posal_mutex_t posal_mutex)
{
   return;
}

ar_result_t posal_channel_create(posal_channel_t *pChannel, POSAL_HEAP_ID heap_id)
{
   return AR_EOK;
}

ar_result_t posal_channel_add_signal(posal_channel_t pChannel, posal_signal_t pSignal, uint32_t unBitMask)
{
   return AR_EOK;
}

uint32_t posal_signal_get_channel_bit(posal_signal_t pSignal)
{
   return 0;
}

uint32_t posal_channel_wait(posal_channel_t pChannel, uint32_t unEnableBitfield)
{
   return 0;
}

void posal_signal_clear(posal_signal_t pSignal)
{
   return;
}

void posal_channel_destroy(posal_channel_t *pChannel)
{
   return;
}

void posal_signal_send(posal_signal_t pSignal)
{
   return;
}

ar_result_t posal_queue_create(char *pzQName, int nMaxItems, posal_queue_t **pQ)
{
   return AR_EOK;
}
void posal_queue_destroy(posal_queue_t *pQ)
{
   return;
}

ar_result_t posal_queue_create_v1(posal_queue_t **queue_pptr, posal_queue_init_attr_t *attr_ptr)
{
   return AR_EOK;
}

ar_result_t posal_queue_push_back(posal_queue_t *pQ, posal_queue_element_t *pPayload)
{
   return AR_EOK;
}

ar_result_t posal_queue_pop_front(posal_queue_t *pQ, posal_queue_element_t *pPayload)
{
   return AR_EOK;
}

ar_result_t posal_queue_init_v1(posal_queue_t *queue_ptr, posal_queue_init_attr_t *attr_ptr)
{
   return AR_EOK;
}

void posal_queue_deinit(posal_queue_t *pQ)
{
   return;
}

uint32_t posal_strlcpy(char_t *dst, const char_t *src, uint32_t size)
{
   return 0;
}

ar_result_t posal_channel_addq(posal_channel_t pChannel, posal_queue_t *pQ, uint32_t unBitMask)
{
   return AR_EOK;
}

uint32_t posal_channel_poll(posal_channel_t pChannel, uint32_t unEnableBitfield)
{
   return 0;
}

ar_result_t posal_memorymap_register(uint32_t *client_token_ptr)
{
   return AR_EOK;
}

ar_result_t posal_memorymap_unregister(uint32_t client_token)
{
   return AR_EOK;
}

ar_result_t posal_memorymap_shm_mem_map(uint32_t                      client_token,
                                        posal_memorymap_shm_region_t *shm_mem_reg_ptr,
                                        uint16_t                      num_shm_reg,
                                        bool_t                        is_cached,
                                        bool_t                        is_offset_map,
                                        POSAL_MEMORYPOOLTYPE          pool_id,
                                        uint32_t *                    shm_mem_map_handle_ptr,
                                        POSAL_HEAP_ID                 heap_id)
{
   return AR_EOK;
}

ar_result_t posal_memorymap_virtaddr_mem_map(uint32_t                      client_token,
                                             posal_memorymap_shm_region_t *shm_mem_reg_ptr,
                                             uint16_t                      num_shm_reg,
                                             bool_t                        is_cached,
                                             bool_t                        is_offset_map,
                                             POSAL_MEMORYPOOLTYPE          pool_id,
                                             uint32_t *                    shm_mem_map_handle_ptr,
                                             POSAL_HEAP_ID                 heap_id)
{
   return AR_EOK;
}

ar_result_t posal_memorymap_get_virtual_addr_from_shm_handle(uint32_t  client_token,
                                                             uint32_t  shm_mem_map_handle,
                                                             uint32_t  shm_addr_lsw,
                                                             uint32_t  shm_addr_msw,
                                                             uint32_t  reg_size,
                                                             bool_t    is_ref_counted,
                                                             uint32_t *virt_addr_ptr)
{
   return AR_EOK;
}

ar_result_t posal_memorymap_shm_mem_unmap(uint32_t client_token, uint32_t shm_mem_map_handle)
{
   return AR_EOK;
}

ar_result_t posal_memorymap_get_virtual_addr_ref_counted(uint32_t  client_token,
                                                         uint32_t  shm_mem_map_handle,
                                                         uint32_t  shm_addr_lsw,
                                                         uint32_t  shm_addr_msw,
                                                         bool_t    is_ref_counted,
                                                         uint32_t *virt_addr_ptr)
{
   return AR_EOK;
}

void posal_thread_join(posal_thread_t nTid, ar_result_t *nStatus)
{
}
ar_result_t posal_thread_launch(posal_thread_t *    posal_obj_ptr,
                                char *              threadname,
                                size_t              stack_size,
                                posal_thread_prio_t nPriority,
                                ar_result_t (*pfStartRoutine)(void *),
                                void *        arg,
                                POSAL_HEAP_ID heap_id)
{
   return AR_EOK;
}

void posal_nmutex_init(posal_nmutex_t *pposal_nmutex)
{
}

void posal_nmutex_lock(posal_nmutex_t *posal_nmutex)
{
}

void posal_nmutex_unlock(posal_nmutex_t pposal_nmutex)
{
}

void posal_nmutex_destroy(posal_nmutex_t pposal_nmutex)
{
}

void posal_condvar_create(posal_condvar_t condition_var_ptr)
{
}

void posal_condvar_signal(posal_condvar_t condition_var_ptr)
{
}

void posal_condvar_wait(posal_condvar_t condition_var_ptr, posal_nmutex_t nmutex)
{
}

void posal_condvar_destroy(posal_condvar_t condition_var_ptr)
{
}
// SPF_IMPORT_BEGIN
/*
ar_result_t posal_pm_wrapper_max_out_pm(uint32_t client_id, posal_pm_client_t **client_ptr_ptr, bool_t is_max_out)
{
   return AR_EOK;
}

bool_t posal_pm_wrapper_is_registered(uint32_t client_id)
{
   return 0;
}

ar_result_t posal_pm_wrapper_deregister(uint32_t *client_id_ptr, posal_pm_client_t **client_ptr_ptr)
{
   return AR_EOK;
}
*/
// SPF_IMPORT_END
uint64_t posal_timer_get_time(void)
{
   return 1;
}

uint64_t posal_profile_get_pcycles(void)
{
   return 1;
}

ar_result_t posal_cache_flush(uint32_t virt_addr, uint32_t mem_size)
{
   return AR_EOK;
}

ar_result_t posal_cache_invalidate(uint32_t virt_addr, uint32_t mem_size)
{
   return AR_EOK;
}

ar_result_t posal_interrupt_register(posal_interrupt_ist_t *ist_ptr,
                                     uint16_t               intr_id,
                                     void (*callback_ptr)(void *),
                                     void *   arg_ptr,
                                     char_t * thread_name,
                                     uint32_t stack_size)
{
   return AR_EOK;
}

ar_result_t posal_interrupt_unregister(posal_interrupt_ist_t ist_ptr)
{
   return AR_EOK;
}

// void posal_memory_aligned_free(void *ptr)
//{
//}
//
// void *posal_memory_aligned_malloc(uint32_t unBytes, uint32_t unAlignBits, POSAL_HEAP_ID heapId)
//{
//   return NULL;
//}

posal_thread_prio_t posal_thread_prio_get(void)
{
   return 1;
}

ar_result_t posal_mutex_create(posal_mutex_t *pposal_mutex, POSAL_HEAP_ID heap_id)
{
   return AR_EOK;
}

ar_result_t posal_memorymap_register(uint32_t *client_token_ptr, POSAL_HEAP_ID heap_id)
{
   return AR_EOK;
}

capi_err_t capi_library_factory_get_instance(uint32_t id, void **lib_ptr)
{
   return CAPI_EOK;
}

uint32_t posal_bufpool_pool_create(uint32_t              node_size,
                                   POSAL_HEAP_ID         heap_id,
                                   uint32_t              num_arrays,
                                   posal_bufpool_align_t alignment)
{
   return 0;
}

void *posal_bufpool_get_node(uint32_t pool_handle)
{
   return NULL;
}

void posal_bufpool_return_node(void *node_ptr)
{
   return;
}

void posal_bufpool_pool_destroy(uint32_t pool_handle)
{
   return;
}

void posal_bufpool_pool_reset_to_base(uint32_t pool_handle)
{
   return;
}

bool_t posal_bufpool_is_address_in_bufpool(void *ptr, uint32_t pool_handle)
{
   return FALSE;
}
