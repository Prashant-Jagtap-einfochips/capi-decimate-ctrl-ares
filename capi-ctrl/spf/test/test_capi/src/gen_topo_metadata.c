/**
 * \file gen_topo_metadata.c
 *
 * \brief
 *
 *     metadata related implementation.
 *
 *
 * \copyright
 *  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
*/
// clang-format on

//#include "gen_topo_i.h"
//#include "spf_ref_counter.h"
#include "../inc/gen_topo_metadata.h"

#include "stringl.h"
#include "spf_ref_counter.h"

static ar_result_t gen_topo_free_eos(gen_topo_t *           topo_ptr,
                                     module_cmn_md_list_t * eos_node_ptr,
                                     gen_topo_eos_cargo_t * cntr_ref_ptr,
                                     module_cmn_md_eos_t *  eos_metadata_ptr,
                                     module_cmn_md_t *      metadata_ptr,
                                     module_cmn_md_list_t **head_pptr);

/**
 * is_eos_rendered TRUE = render, FALSE = drop
 *
 * is_last - should be relevant only when it's known that one EOS is preset in the md_list_pptr
 */
ar_result_t gen_topo_respond_and_free_eos(gen_topo_t *           topo_ptr,
                                          gen_topo_module_t *    module_ptr,
                                          module_cmn_md_list_t * md_list_ptr,
                                          bool_t                 is_eos_rendered,
                                          module_cmn_md_list_t **head_pptr)
{
   ar_result_t result = AR_EOK;

   if (NULL == md_list_ptr)
   {
      return result;
   }

   module_cmn_md_eos_t *eos_metadata_ptr = NULL;
   module_cmn_md_t *    metadata_ptr     = NULL;
   metadata_ptr                          = md_list_ptr->obj_ptr;

   if (MODULE_CMN_MD_ID_EOS == metadata_ptr->metadata_id)
   {
      uint32_t is_out_band = metadata_ptr->metadata_flag.is_out_of_band;
      if (is_out_band)
      {
         eos_metadata_ptr = (module_cmn_md_eos_t *)metadata_ptr->metadata_ptr;
      }
      else
      {
         eos_metadata_ptr = (module_cmn_md_eos_t *)&(metadata_ptr->metadata_buf);
      }

      if (AR_EOK ==
          topo_ptr->topo_to_cntr_vtable_ptr->raise_eos(topo_ptr, 0x00000000, eos_metadata_ptr, is_eos_rendered))
      {
         AR_MSG(DBG_MED_PRIO,
                "MD_DBG: Raise EOS called with is_eos_rendered %u for end point module (0x%lX), gk payload 0x%p",
                is_eos_rendered,
                0x00000000,
                eos_metadata_ptr->core_ptr);
      }
      // Even for non-flushing EOS below function needs to be called (to remove reference to ext in port in case
      // flushing EOS gets converted to non-flushing)
      gen_topo_free_eos(topo_ptr,
                        md_list_ptr,
                        (gen_topo_eos_cargo_t *)eos_metadata_ptr->cntr_ref_ptr,
                        eos_metadata_ptr,
                        metadata_ptr,
                        head_pptr);
   }
   return result;
}

bool_t gen_topo_is_flushing_eos(module_cmn_md_t *md_ptr)
{
   if (NULL == md_ptr)
   {
      return FALSE;
   }
   if (MODULE_CMN_MD_ID_EOS == md_ptr->metadata_id)
   {
      module_cmn_md_eos_t *eos_metadata_ptr = 0;
      uint32_t             is_out_band      = md_ptr->metadata_flag.is_out_of_band;
      if (is_out_band)
      {
         eos_metadata_ptr = (module_cmn_md_eos_t *)md_ptr->metadata_ptr;
      }
      else
      {
         eos_metadata_ptr = (module_cmn_md_eos_t *)&(md_ptr->metadata_buf);
      }

      if (MODULE_CMN_MD_EOS_FLUSHING == eos_metadata_ptr->flags.is_flushing_eos)
      {
         return TRUE;
      }
   }
   return FALSE;
}

/**
 * this is called after MD/EOS comes to output port, hence always free.
 */
ar_result_t gen_topo_metadata_prop_for_sink(gen_topo_module_t *module_ptr, capi_stream_data_v2_t *output_stream_ptr)
{
   ar_result_t result = AR_EOK;

   result = gen_topo_destroy_all_metadata(0x00000000,
                                          (void *)module_ptr,
                                          &output_stream_ptr->metadata_list_ptr,
                                          FALSE /* is_dropped */);

   gen_topo_t *topo_ptr                                 = module_ptr->topo_ptr;
   topo_ptr->proc_context.process_info.anything_changed = TRUE;

   return result;
}

ar_result_t gen_topo_set_pending_zeros(gen_topo_module_t *module_ptr, gen_topo_input_port_t *in_port_ptr)
{
   ar_result_t result = AR_EOK;
   if (!gen_topo_fwk_owns_md_prop(module_ptr))
   {
      return result;
   }

   // only generic PP modules & EP with Single input port (SISO or sink) and pcm/packetized fmt need zero pushing.
   // also
   // need to have a
   // buf. some encoders also have algo delay. to flush it out, only way is to push zeros.otherwise EOS gets stuck.
   // zero flushing only for flush-eos. also zero pushing not necessary if underflow is happening (underflow = zeros)
   // Note that EP modules with 60958 config can also be zero pushed.
   if (/*(AMDB_MODULE_TYPE_GENERIC != module_ptr->gu.module_type) ||*/
       (!SPF_IS_PACKETIZED_OR_PCM(in_port_ptr->common.media_fmt.data_format)))
   {
      return result;
   }

   // if flushing EOS comes back to back, we need to re-init the value of pending_zeros_at_eos so that second EOS has
   // enough zeros to push it through.
   // if (0 == module_ptr->pending_zeros_at_eos)
   {
      // no need to rescale pending_zeros_at_eos as we don't listen to more input with EoS pending.
      // consider algo delay only if module is not disabled.
      if (!module_ptr->flags.disabled)
      {
         module_ptr->pending_zeros_at_eos =
            topo_us_to_bytes_per_ch(module_ptr->algo_delay, &in_port_ptr->common.media_fmt);
         // algo delay is already accounted in metadata prop
      }

      AR_MSG(DBG_LOW_PRIO,
             "MD_DBG: flushing EoS at module,port 0x%lX, 0x%lx, total pending zeros (bytes per ch) %lu",
             0x00000000,
             in_port_ptr->gu.cmn.id,
             module_ptr->pending_zeros_at_eos);
   }

   return result;
}

void gen_topo_free_eos_cargo(gen_topo_t *topo_ptr, module_cmn_md_t *md_ptr, module_cmn_md_eos_t *eos_metadata_ptr)
{
   if (eos_metadata_ptr && eos_metadata_ptr->cntr_ref_ptr)
   {
      gen_topo_eos_cargo_t *cntr_ref_ptr = (gen_topo_eos_cargo_t *)eos_metadata_ptr->cntr_ref_ptr;

      cntr_ref_ptr->ref_count--;
      if (0 == cntr_ref_ptr->ref_count)
      {
         void *temp_ptr = (void *)eos_metadata_ptr->core_ptr;
         // call back container to clear input EoS flag.
         // Even for non-flushing EOS below function needs to be called (to remove reference to ext in port)
         if (topo_ptr->topo_to_cntr_vtable_ptr->clear_eos)
         {
            topo_ptr->topo_to_cntr_vtable_ptr->clear_eos(topo_ptr,
                                                         cntr_ref_ptr->inp_ref,
                                                         cntr_ref_ptr->inp_id,
                                                         eos_metadata_ptr);
            /*(NULL != cntr_ref_ptr->inp_ref) ||
               (cntr_ref_ptr->did_eos_come_from_ext_in)*/
         }

         // eos_metadata_ptr->core_ptr is handled outside.
         MFREE_NULLIFY(eos_metadata_ptr->cntr_ref_ptr);

         AR_MSG(DBG_LOW_PRIO,
                "MD_DBG: Freed EoS (in this container) with metadata ptr 0x%p gk payload 0x%p. offset %lu",
                eos_metadata_ptr,
                temp_ptr,
                md_ptr->offset);

         // the list will move along the chain and be freed when EoS is terminated (at sink module or at container
         // boundary)
      }
      eos_metadata_ptr->cntr_ref_ptr = NULL;
   }
}

/**
 * this is called for EOS transfer between peer-container as well,
 * where there's no need to clear spf_payload ref counter.
 */
static ar_result_t gen_topo_free_eos(gen_topo_t *           topo_ptr,
                                     module_cmn_md_list_t * eos_node_ptr,
                                     gen_topo_eos_cargo_t * cntr_ref_ptr,
                                     module_cmn_md_eos_t *  eos_metadata_ptr,
                                     module_cmn_md_t *      metadata_ptr,
                                     module_cmn_md_list_t **head_pptr)
{
   ar_result_t result = AR_EOK;

#ifdef METADATA_DEBUGGING
   AR_MSG(DBG_LOW_PRIO,
          "MD_DBG: Free: eos_node_ptr 0x%p cntr_ref_ptr 0x%p eos_metadata_ptr 0x%p, 0x%p",
          eos_node_ptr,
          cntr_ref_ptr,
          eos_metadata_ptr,
          metadata_ptr);
#endif

   /** even if EoS splits, we can destroy metadata_ptr, eos_metadata_ptr and eos_node_ptr
    * as these will be unique per path. Only cntr_ref_ptr is ref counted.
    * ext input port lists & flags must be cleared only when cargo ref count reaches zero.

    Even for non-flushing EOS below function needs to be called (to remove reference to ext in port)*/
   gen_topo_free_eos_cargo(topo_ptr, metadata_ptr, eos_metadata_ptr);

   if (metadata_ptr)
   {
      uint32_t is_out_band = metadata_ptr->metadata_flag.is_out_of_band;
      if (eos_metadata_ptr)
      {
         if (is_out_band)
         {
            MFREE_NULLIFY(eos_metadata_ptr);
         }
      }
   }

   if (eos_node_ptr)
   {
      spf_list_delete_node_and_free_obj((spf_list_node_t **)&eos_node_ptr,
                                        (spf_list_node_t **)head_pptr,
                                        TRUE /* pool_used */);
   }

   return result;
}

static ar_result_t gen_topo_create_eos_for_int_ports(uint32_t                     log_id,
                                                     POSAL_HEAP_ID                heap_id,
                                                     module_cmn_md_list_t **      eos_md_list_pptr,
                                                     module_cmn_md_eos_flags_t    eos_flag,
                                                     uint32_t                     offset /*samples per ch : pcm or
                                                     bytes*/,
                                                     gen_topo_eos_cargo_t *       cntr_ref_ptr,
                                                     module_cmn_md_eos_payload_t *eos_core_payload_ptr)
{
   ar_result_t result = AR_EOK;

   // Deepest item first
   module_cmn_md_eos_t *eos_metadata_ptr = NULL;
   module_cmn_md_t *    metadata_ptr     = NULL;

   uint32_t size = MODULE_CMN_MD_INBAND_GET_REQ_SIZE(sizeof(module_cmn_md_eos_t));
   MALLOC_MEMSET(metadata_ptr, module_cmn_md_t, size, heap_id, result);
   eos_metadata_ptr = (module_cmn_md_eos_t *)&metadata_ptr->metadata_buf;

   if (AR_EOK == (result = spf_list_insert_tail((spf_list_node_t **)eos_md_list_pptr, metadata_ptr, heap_id, TRUE)))
   {
      //      TRY(result, spf_list_insert_tail((spf_list_node_t **)eos_md_list_pptr, metadata_ptr, heap_id, TRUE /* use
      //      pool
      //   */));
      /** never use list pool for metadata. capi modules can also create and add metadata to list.
       * those modules may malloc. Hence always use malloc/free for metadata nodes. */

      /** EoS metadata payload  */
      eos_metadata_ptr->cntr_ref_ptr = cntr_ref_ptr;
      eos_metadata_ptr->core_ptr     = eos_core_payload_ptr;
      eos_metadata_ptr->flags        = eos_flag;

      /** metadata payload  */
      metadata_ptr->metadata_id = MODULE_CMN_MD_ID_EOS;
      metadata_ptr->actual_size = sizeof(module_cmn_md_eos_t);
      metadata_ptr->max_size    = sizeof(module_cmn_md_eos_t);
      metadata_ptr->offset      = offset;
      // data is in band
      metadata_ptr->metadata_flag.is_out_of_band = MODULE_CMN_MD_IN_BAND;

#ifdef METADATA_DEBUGGING
      AR_MSG(DBG_LOW_PRIO,
             "MD_DBG: create: cntr_ref_ptr 0x%p, metadata_ptr 0x%p, eos_metadata_ptr 0x%p head node ptr 0x%p, "
             "module_cmn_eos_flag 0x%lX, offset %lu",
             cntr_ref_ptr,
             metadata_ptr,
             eos_metadata_ptr,
             *eos_md_list_pptr,
             eos_flag.word,
             metadata_ptr->offset);
#endif
   }

   if (AR_EOK != result)
   {
//   CATCH(result, AR_MSG_PREFIX, log_id)
//   {
#if 0 // no exit path after spf_list_insert_tail
      spf_list_node_t *tail_node_ptr = NULL;
      spf_list_get_tail_node((spf_list_node_t *)*eos_md_list_pptr, &tail_node_ptr);
      if (tail_node_ptr)
      {
         spf_list_delete_node(&tail_node_ptr, TRUE /*pool_used*/);
         if ((spf_list_node_t *)*eos_md_list_pptr == tail_node_ptr)
         {
            *eos_md_list_pptr = NULL;
         }
      }
#endif
      MFREE_NULLIFY(metadata_ptr);
   }
   //   }

   return result;
}

/**
 * container reference is needed only for flushing EOS.
 */
ar_result_t gen_topo_create_eos_cntr_ref(gen_topo_t *           topo_ptr,
                                         POSAL_HEAP_ID          heap_id,
                                         gen_topo_input_port_t *input_port_ptr,
                                         uint32_t               input_id,
                                         gen_topo_eos_cargo_t **eos_cargo_pptr)
{
   ar_result_t result = AR_EOK;

   gen_topo_eos_cargo_t *cntr_ref_ptr = NULL;

   // Multiple EOS can come together
   MALLOC_MEMSET(cntr_ref_ptr, gen_topo_eos_cargo_t, sizeof(gen_topo_eos_cargo_t), heap_id, result);

   /** Container specific payload : cargo */
   cntr_ref_ptr->inp_ref                  = input_port_ptr;
   cntr_ref_ptr->inp_id                   = input_id;
   cntr_ref_ptr->did_eos_come_from_ext_in = (NULL != input_port_ptr) && (NULL != input_port_ptr->gu.ext_in_port_ptr);
   cntr_ref_ptr->ref_count                = 1;

   *eos_cargo_pptr = cntr_ref_ptr;

   if (AR_EOK != result)
   {
      //      CATCH(result, AR_MSG_PREFIX, log_id)
      //      {
      MFREE_NULLIFY(cntr_ref_ptr);
      //      }
   }

   return result;
}

/**
 * called for dsp client, and peer-SG stop/flush
 */
ar_result_t gen_topo_create_eos_for_cntr(gen_topo_t *                 topo_ptr,
                                         POSAL_HEAP_ID                heap_id,
                                         module_cmn_md_list_t **      eos_md_list_pptr,
                                         gen_topo_input_port_t *      input_port_ptr,
                                         uint32_t                     input_port_id,
                                         module_cmn_md_eos_payload_t *eos_core_payload_ptr,
                                         module_cmn_md_eos_flags_t *  eos_flag_ptr,
                                         uint32_t                     bytes_in_buf,
                                         topo_media_fmt_t *           media_fmt_ptr)
{
   ar_result_t result = AR_EOK;

   uint32_t              offset       = 0;
   gen_topo_eos_cargo_t *cntr_ref_ptr = NULL;

   /* EOS for data-flow-gap: For STM triggered containers, make flushing EOS as not flushing
    * Note that this is not always true. Imagine a multi-SG Tx HW-EP port container
    * If upstream SG containing STM is stopped, then DS needs to use flushing EOS.
    * However, if unstopped SG has STM, then it's ok not to use flushing EOS. For now, ignore such cases.
    * Firstly, if STM is stopped, whole container doesn't trigger anyway.
    * Usually in Rx path, flushing can be made non-flushing.*/
   /*
    * since EOS causes data-flow-state to change to at-gap where we don't underrun
   if (topo_ptr->flags.is_signal_triggered && eos_flag_ptr->is_flushing_eos)
   {
      AR_MSG(0x00000000, DBG_HIGH_PRIO, "MD_DBG: treating EOS as nonflushing due to signal triggered
      mode"); eos_flag_ptr->is_flushing_eos = MODULE_CMN_MD_EOS_NON_FLUSHING;
   }
   */

   // only if input port is given, create cntr ref. for ext out port we don't need contr ref.
   if (eos_flag_ptr->is_flushing_eos && input_port_ptr)
   {
      if (0 != (result = gen_topo_create_eos_cntr_ref(topo_ptr, heap_id, input_port_ptr, input_port_id, &cntr_ref_ptr)))
      {
         if (AR_EOK != result)
         {
            AR_MSG(DBG_ERROR_PRIO, "MD_DBG: Failed create memory for EoS ");
            MFREE_NULLIFY(cntr_ref_ptr);
         }
         return AR_EOK;
      }
   }

   // put the EOS at the offset = last byte
   gen_topo_do_md_offset_math(0x00000000, &offset, bytes_in_buf, media_fmt_ptr, TRUE /* need_to_add */);

   if (0 != (result = gen_topo_create_eos_for_int_ports(0x00000000,
                                                        heap_id,
                                                        eos_md_list_pptr,
                                                        *eos_flag_ptr,
                                                        offset,
                                                        cntr_ref_ptr,
                                                        eos_core_payload_ptr)))
   {
      if (AR_EOK != result)
      {
         AR_MSG(DBG_ERROR_PRIO, "MD_DBG: Failed create memory for EoS ");
         MFREE_NULLIFY(cntr_ref_ptr);
      }
      return AR_EOK;
   }

   if (0 == (result = gen_topo_create_eos_for_int_ports(0x00000000,
                                                        heap_id,
                                                        eos_md_list_pptr,
                                                        *eos_flag_ptr,
                                                        offset,
                                                        cntr_ref_ptr,
                                                        eos_core_payload_ptr)))
   {
      AR_MSG(DBG_LOW_PRIO,
             "MD_DBG: Created EoS with node_ptr 0x%p, cargo ptr 0x%p, gk payload ptr 0x%p, offset = %lu, eos_flag"
             "0x%lx",
             *eos_md_list_pptr,
             cntr_ref_ptr,
             eos_core_payload_ptr,
             offset,
             (*eos_flag_ptr).word);
   }

   if (AR_EOK != result)
   {

      AR_MSG(DBG_ERROR_PRIO, "MD_DBG: Failed create memory for EoS ");
      MFREE_NULLIFY(cntr_ref_ptr);
   }

   return AR_EOK;
}

static ar_result_t gen_topo_clone_eos(gen_topo_t *           topo_ptr,
                                      module_cmn_md_t *      in_metadata_ptr,
                                      module_cmn_md_list_t **out_md_list_pptr,
                                      POSAL_HEAP_ID          heap_id)
{
   ar_result_t result = AR_EOK;

   if (NULL == in_metadata_ptr)
   {
      return result;
   }

   module_cmn_md_eos_t *in_eos_metadata_ptr = NULL;

   if (MODULE_CMN_MD_ID_EOS == in_metadata_ptr->metadata_id)
   {
      uint32_t is_out_band = in_metadata_ptr->metadata_flag.is_out_of_band;
      if (is_out_band)
      {
         in_eos_metadata_ptr = (module_cmn_md_eos_t *)in_metadata_ptr->metadata_ptr;
      }
      else
      {
         in_eos_metadata_ptr = (module_cmn_md_eos_t *)&(in_metadata_ptr->metadata_buf);
      }

      gen_topo_eos_cargo_t *       cargo_ptr = (gen_topo_eos_cargo_t *)in_eos_metadata_ptr->cntr_ref_ptr;
      module_cmn_md_eos_payload_t *core_ptr =
         (in_eos_metadata_ptr->flags.is_internal_eos) ? NULL : (in_eos_metadata_ptr->core_ptr);

      result = gen_topo_create_eos_for_int_ports(0x00000000,
                                                 heap_id,
                                                 out_md_list_pptr,
                                                 in_eos_metadata_ptr->flags,
                                                 in_metadata_ptr->offset,
                                                 cargo_ptr,
                                                 core_ptr);

      /*Internal EOS does not have a core, so we shouldn't be ref-counting it*/
      if (NULL != core_ptr)
      {
         spf_ref_counter_add_ref((void *)in_eos_metadata_ptr->core_ptr);
      }

      if (cargo_ptr)
      {
         cargo_ptr->ref_count++;
      }
   }

   return result;
}

/**
 * Exposed through gen_topo.h. doesn't populate metadata ID
 */
ar_result_t gen_topo_metadata_create(uint32_t               log_id,
                                     module_cmn_md_list_t **md_list_pptr,
                                     uint32_t               size,
                                     POSAL_HEAP_ID          heap_id,
                                     bool_t                 is_out_band,
                                     module_cmn_md_t **     md_pptr)
{
   ar_result_t ar_result = AR_EOK;

   uint32_t md_size = sizeof(module_cmn_md_t);

   if (NULL == md_list_pptr)
   {
      return ar_result;
   }

   if (NULL == md_pptr)
   {
      return CAPI_EBADPARAM;
   }
   *md_pptr = NULL;
   module_cmn_md_t *md_ptr;
   void *           md_payload_ptr = NULL;
   spf_list_node_t *tail_node_ptr  = NULL;

   // for in-band do only one malloc
   if (!is_out_band)
   {
      md_size = MODULE_CMN_MD_INBAND_GET_REQ_SIZE(size);
   }

   md_ptr = (module_cmn_md_t *)posal_memory_malloc(md_size, heap_id);
   //   VERIFY(ar_result, NULL != md_ptr);
   if (ar_result == (NULL != md_ptr))
   {
      if (is_out_band)
      {
         MFREE_NULLIFY(md_payload_ptr);
      }
      MFREE_NULLIFY(md_ptr);

      *md_pptr = md_ptr;
      return ar_result;
   }

   memset(md_ptr, 0, sizeof(module_cmn_md_t)); // memset only top portion as size may be huge

   if (is_out_band)
   {
      if (size)
      {
         md_payload_ptr = posal_memory_malloc(size, heap_id);
         //         VERIFY(ar_result, NULL != md_payload_ptr);

         if (ar_result == (NULL != md_payload_ptr))
         {
            if (is_out_band)
            {
               MFREE_NULLIFY(md_payload_ptr);
            }
            MFREE_NULLIFY(md_ptr);

            *md_pptr = md_ptr;
            return ar_result;
         }
      }

      md_ptr->actual_size  = size;
      md_ptr->max_size     = size;
      md_ptr->metadata_ptr = md_payload_ptr;
   }
   else
   {
      md_ptr->actual_size = size;
      md_ptr->max_size    = size;
      // md_payload_ptr NULL for in-band
   }

   if (0 != (ar_result = spf_list_insert_tail((spf_list_node_t **)md_list_pptr, md_ptr, heap_id, TRUE /* use pool */)))
   {
      if (AR_EOK != ar_result)
      {

         if (is_out_band)
         {
            MFREE_NULLIFY(md_payload_ptr);
         }
         MFREE_NULLIFY(md_ptr);
      }
      *md_pptr = md_ptr;
      return ar_result;
   }

#if defined(METADATA_DEBUGGING)
   spf_list_get_tail_node((spf_list_node_t *)*md_list_pptr, &tail_node_ptr);
   AR_MSG(DBG_MED_PRIO, "MD_DBG: Metadata create: spf_list_node_t 0x%p host md_ptr 0x%p", tail_node_ptr, md_ptr);
#else
   (void)tail_node_ptr;
#endif

   md_ptr->metadata_flag.is_out_of_band = is_out_band;

   if (AR_EOK != ar_result)
   {

      if (is_out_band)
      {
         MFREE_NULLIFY(md_payload_ptr);
      }
      MFREE_NULLIFY(md_ptr);
   }
   *md_pptr = md_ptr;
   return ar_result;
}

/**
 * these functions are exposed to modules through capiv2 interface extensions
 */

/**
 * doesn't populate metadata ID
 */
static capi_err_t gen_topo_capi_metadata_create(void *                 context_ptr,
                                                module_cmn_md_list_t **md_list_pptr,
                                                uint32_t               size,
                                                capi_heap_id_t         c_heap_id,
                                                bool_t                 is_out_band,
                                                module_cmn_md_t **     md_pptr)
{
   capi_err_t result = CAPI_EOK;

   result = ar_result_to_capi_err(gen_topo_metadata_create(0x00000000, // todo:mid
                                                           md_list_pptr,
                                                           size,
                                                           (POSAL_HEAP_ID)c_heap_id.heap_id,
                                                           is_out_band,
                                                           md_pptr));

   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "MD_DBG: Failed to create metadata ");
   }

   return result;
}

static ar_result_t gen_topo_clone_dfg(void *                 context_ptr,
                                      module_cmn_md_t *      in_metadata_ptr,
                                      module_cmn_md_list_t **out_md_list_pptr,
                                      POSAL_HEAP_ID          heap_id)
{
   ar_result_t result = AR_EOK;

   if (NULL == in_metadata_ptr)
   {
      return result;
   }

   if (MODULE_CMN_MD_ID_DFG != in_metadata_ptr->metadata_id)
   {
      return result;
   }

   capi_heap_id_t c_heap_id = { .heap_id = (uint32_t)heap_id };

   module_cmn_md_t *out_md_ptr = NULL;

   bool_t   is_out_band = in_metadata_ptr->metadata_flag.is_out_of_band;
   uint32_t size        = 0; // For now no specific payload for the DFG metadata

   result = gen_topo_capi_metadata_create(context_ptr, out_md_list_pptr, size, c_heap_id, is_out_band, &out_md_ptr);

   if (AR_DID_FAIL(result) || (NULL == out_md_ptr))
   {
      AR_MSG(DBG_ERROR_PRIO, "Failed to clone DFG metadata with result 0x%x", result);
      return result;
   }

   out_md_ptr->metadata_id        = MODULE_CMN_MD_ID_DFG;
   out_md_ptr->offset             = in_metadata_ptr->offset; // RR: Check this
   out_md_ptr->metadata_flag.word = in_metadata_ptr->metadata_flag.word;

   return result;
}

/**
 *
 *
 * default case is for seamlessly copying metadata across CAPI modules. it doesn't have too-deep copies.
 *
 * context_ptr - context pointer
 * in_md_list_ptr  - pointer to the list node which needs to be cloned
 * out_md_list_pptr - pointer to the pointer to the list node. new node will be inserted at the tail-end.
 *
// */
static capi_err_t gen_topo_capi_metadata_clone(void *                 context_ptr,
                                               module_cmn_md_t *      md_ptr,
                                               module_cmn_md_list_t **out_md_list_pptr,
                                               capi_heap_id_t         c_heap_id)
{
   capi_err_t  result    = CAPI_EOK;
   ar_result_t ar_result = AR_EOK;
   //   INIT_EXCEPTION_HANDLING
   if ((NULL == md_ptr) || (NULL == context_ptr))
   {
      return CAPI_EFAILED;
   }
   if (NULL == out_md_list_pptr)
   {
      return CAPI_EFAILED;
   }

   gen_topo_module_t *module_ptr   = (gen_topo_module_t *)context_ptr;
   gen_topo_t *       topo_ptr     = module_ptr->topo_ptr;
   bool_t             handle_error = FALSE;
   POSAL_HEAP_ID      heap_id      = (POSAL_HEAP_ID)c_heap_id.heap_id;

   module_cmn_md_t *new_md_ptr         = NULL;
   void *           new_md_payload_ptr = NULL;
   uint32_t         is_out_band        = FALSE;

#if defined(METADATA_DEBUGGING)
   AR_MSG(DBG_LOW_PRIO, "MD_DBG: clone, metadata_ptr 0x%p", md_ptr);
#endif

   switch (md_ptr->metadata_id)
   {
      case MODULE_CMN_MD_ID_EOS:
      {
         ar_result = gen_topo_clone_eos(topo_ptr, md_ptr, out_md_list_pptr, heap_id);
         if (AR_DID_FAIL(ar_result))
         {
            AR_MSG(DBG_ERROR_PRIO, "MD_DBG: Cloning EoS failed");
         }

         break;
      }
      case MODULE_CMN_MD_ID_DFG:
      {
         ar_result = gen_topo_clone_dfg(context_ptr, md_ptr, out_md_list_pptr, heap_id);
         if (AR_DID_FAIL(ar_result))
         {
            AR_MSG(DBG_ERROR_PRIO, "DFG_DBG: Cloning DFG failed with 0x%x", ar_result);
         }

         break;
      }
      default:
      {
         handle_error = TRUE;
         // generic metadata is assumed to not require deep cloning
         is_out_band = md_ptr->metadata_flag.is_out_of_band;

         void *   md_payload_ptr = &md_ptr->metadata_buf;
         uint32_t new_md_size    = md_ptr->max_size;
         if (is_out_band)
         {
            md_payload_ptr = &md_ptr->metadata_ptr;

            MALLOC_MEMSET(new_md_payload_ptr, void *, md_ptr->max_size, heap_id, ar_result);
         }
         else
         {
            md_payload_ptr = &md_ptr->metadata_buf;
            new_md_size += sizeof(module_cmn_md_t);
         }

         MALLOC_MEMSET(new_md_ptr, module_cmn_md_t, new_md_size, heap_id, ar_result);

         if (!is_out_band)
         {
            new_md_payload_ptr = (void *)&new_md_ptr->metadata_buf;
         }

         ar_result =
            spf_list_insert_tail((spf_list_node_t **)out_md_list_pptr, new_md_ptr, heap_id, TRUE /* use pool */);

         if (AR_EOK != ar_result)
         {
            AR_MSG(DBG_MED_PRIO, "MD_DBG: Unable to add metadata ptr to list");
            return ar_result;
         }

         // Copy
         *new_md_ptr = *md_ptr;
         if (is_out_band)
         {
            new_md_ptr->metadata_ptr = new_md_payload_ptr;
         }
         memscpy(new_md_payload_ptr, md_ptr->max_size, md_payload_ptr, md_ptr->max_size);
#if defined(METADATA_DEBUGGING)
         AR_MSG(DBG_LOW_PRIO, "MD_DBG: cloned, original md_ptr 0x%p, new md_ptr 0x%p", md_ptr, new_md_ptr);
#endif
      }
   }

   if (AR_EOK != ar_result)
   {

      if (handle_error)
      {
         spf_list_node_t *tail_node_ptr = NULL;
         spf_list_get_tail_node((spf_list_node_t *)*out_md_list_pptr, &tail_node_ptr);
         if (tail_node_ptr)
         {
            spf_list_delete_node(&tail_node_ptr, TRUE /*pool_used*/);
            if ((spf_list_node_t *)*out_md_list_pptr == tail_node_ptr)
            {
               *out_md_list_pptr = NULL;
            }
         }
         MFREE_NULLIFY(new_md_ptr);
         if (is_out_band)
         {
            MFREE_NULLIFY(new_md_payload_ptr);
         }
      }
      result = ar_result_to_capi_err(ar_result);
   }
   //   }

   return result;
}

/*
 * frees up metadata related memories and for specific ones, also takes care of ref counting.
 * context_ptr - context pointer
 * in_md_list_pptr  - pointer to the pointer to the list node which needs to be terminated.
 *    if this list belongs to a linked list, then it's neighboring nodes are properly updated upon destroy.
 *
 * is_dropped - some metadata (EOS) may result in different behavior for drop vs. consumption (render eos).
 */
capi_err_t gen_topo_capi_metadata_destroy(void *                 context_ptr,
                                          module_cmn_md_list_t * md_list_ptr,
                                          bool_t                 is_dropped,
                                          module_cmn_md_list_t **head_pptr)
{
   capi_err_t         result     = CAPI_EOK;
   ar_result_t        ar_result  = AR_EOK;
   gen_topo_module_t *module_ptr = (gen_topo_module_t *)context_ptr;
   gen_topo_t *       topo_ptr   = module_ptr->topo_ptr;

   if (NULL == md_list_ptr)
   {
      return result;
   }

   module_cmn_md_t *md_ptr = (module_cmn_md_t *)md_list_ptr->obj_ptr;

   // metadata such as EOS and DFG should not be dropped if dropping is due to a stall.
   // but currently no such scenario. Hence always destroy EOS and DFG even though they carry info
   // about data flow state.

   switch (md_ptr->metadata_id)
   {
      case MODULE_CMN_MD_ID_EOS:
      {
         ar_result = gen_topo_respond_and_free_eos(topo_ptr, module_ptr, md_list_ptr, !is_dropped, head_pptr);
         break;
      }
      default: // For MODULE_CMN_MD_ID_DFG, this is sufficient
      {
#ifdef METADATA_DEBUGGING
         AR_MSG(DBG_LOW_PRIO,
                "MD_DBG: destroy, node_ptr 0x%p, metadata_ptr 0x%p, md_id 0x%08lX",
                md_list_ptr,
                md_ptr,
                md_ptr->metadata_id);
#endif

         // generic metadata is assumed to not require deep cloning
         uint32_t is_out_band = md_ptr->metadata_flag.is_out_of_band;
         if (is_out_band)
         {
            MFREE_NULLIFY(md_ptr->metadata_ptr);
         }

         if (md_list_ptr)
         {
            spf_list_delete_node_and_free_obj((spf_list_node_t **)&md_list_ptr,
                                              (spf_list_node_t **)head_pptr,
                                              TRUE /* pool_used*/);
         }
      }
   }

   result = ar_result_to_capi_err(ar_result);

   return result;
}

#define CAPI_IS_PCM_FORMAT(data_format) ((CAPI_FIXED_POINT == data_format) || (CAPI_FLOATING_POINT == data_format))

#define CAPI_IS_PCM_PACKETIZED(data_format) (CAPI_RAW_COMPRESSED != data_format)

#define PRINT_MD_PROP_DBG(str1, str2, len_per_ch, str3, ...)                                                           \
   AR_MSG(DBG_LOW_PRIO,                                                                                                \
          "MD_DBG: " str1 ". module 0x%lx, node_ptr 0x%p, md_id 0x%08lx, offset %lu, offset_before %lu," str2          \
          "_per_ch %lu, " str3,                                                                                        \
          0x00000000,                                                                                                  \
          node_ptr,                                                                                                    \
          md_ptr->metadata_id,                                                                                         \
          md_ptr->offset,                                                                                              \
          offset_before,                                                                                               \
          len_per_ch,                                                                                                  \
          ##__VA_ARGS__)

/**
 * Moves metadata from src_list_head_pptrmetadata (internal list or input list) to the output list.
 * returns TRUE if md was moved to output list.
 */
static bool_t gen_topo_move_md_to_out_list(gen_topo_module_t *    module_ptr,
                                           module_cmn_md_list_t * node_ptr,
                                           module_cmn_md_list_t **output_md_list_pptr,
                                           module_cmn_md_list_t **src_list_head_pptr,
                                           uint32_t               out_produced_per_ch,
                                           uint32_t               out_initial_len_per_ch,
                                           uint32_t               in_consumed_per_ch,
                                           data_format_t          in_df,
                                           data_format_t          out_df,
                                           bool_t                 flushing_eos)
{
   module_cmn_md_t *md_ptr                 = node_ptr->obj_ptr;
   uint32_t         offset_before          = md_ptr->offset;
   bool_t           pcmpack_in_pcmpack_out = CAPI_IS_PCM_PACKETIZED(out_df) && CAPI_IS_PCM_PACKETIZED(in_df);
   bool_t           move_to_out            = TRUE;

   // squeezing behavior for decoder like module only
   // For WR EP or such modules b4 decoder, doing squeezig behavior prevents EOS from reaching decoder separate from
   //   buf
   //   this causes gapless to fail (where last chunk of data should be received along with EOS in order to remove
   //   trailing silence)
   if ((AMDB_MODULE_TYPE_DECODER == module_ptr->gu.module_type) ||
       (AMDB_MODULE_TYPE_PACKETIZER == module_ptr->gu.module_type) ||
       (AMDB_MODULE_TYPE_DEPACKETIZER == module_ptr->gu.module_type))
   {
      if (flushing_eos)
      {
         move_to_out = (0 == out_produced_per_ch);
      }
   }

   // md_ptr->offset is input scale

   if (move_to_out && (md_ptr->offset <= in_consumed_per_ch))
   {
      if (pcmpack_in_pcmpack_out)
      {
         // scale based on out len produced for a given input len, so that duration modifying modules are covered.
         // For non-DM modules, this works the same as using sample rates in place of lengths
         uint32_t offset_in_out_scale = TOPO_CEIL(md_ptr->offset * out_produced_per_ch, in_consumed_per_ch);

         // Due to some precision issues offset shouldn't go outside.
         md_ptr->offset = MIN(out_produced_per_ch, offset_in_out_scale);
      }
      else
      {
         if (flushing_eos)
         {
            md_ptr->offset = out_produced_per_ch; // Flushing EOS always goes to the end (e.g.
                                                  // raw_compr->decoder->pcm)
         }
         else
         {
            md_ptr->offset = 0; // pcm->enc->raw_compr. we cannot use original offset as PCM offset could be higher
                                // than
            // raw-compr data len.
         }
      }

      // Newly generated output goes after data that was initially in the output, so incorprate initial data length
      // into
      // the
      // md offset.
      md_ptr->offset += out_initial_len_per_ch;

      spf_list_move_node_to_another_list((spf_list_node_t **)output_md_list_pptr,
                                         (spf_list_node_t *)node_ptr,
                                         (spf_list_node_t **)src_list_head_pptr);
#ifdef METADATA_DEBUGGING
      PRINT_MD_PROP_DBG("moving to output list",
                        "in_consumed",
                        in_consumed_per_ch,
                        "out_produced_per_ch %lu, out_initial_len_per_ch %lu",
                        out_produced_per_ch,
                        out_initial_len_per_ch);
#else
      (void)offset_before;
#endif
      if (flushing_eos && gen_topo_fwk_owns_md_prop(module_ptr) &&
          (AMDB_MODULE_TYPE_GENERIC != module_ptr->gu.module_type) &&
          (AMDB_MODULE_TYPE_FRAMEWORK != module_ptr->gu.module_type))
      {
         // in elite, Eos resulted in algo reset for dec/enc/pack/unpack, continue the same behavior for
         // CAPI backward compat
         gen_topo_algo_reset(module_ptr, 0x00000000, FALSE /*is_port_valid*/, FALSE /*is_input*/, 0);
      }
      // moved to output list
      return TRUE;
   }
   else
   {
      // didn't move to output list
      return FALSE;
   }
}

/**
 * Segregates the input metadata into 2 lists: output list and a list internal to the module
 * by comparing the offsets. internal_md_list_pptr can be given in subsequent calls.
 *
 * There are two types of metadata - offset associated/buffer associated.
 *
 * ---------------------------- SAMPLE ASSOCIATED METADATA
 ------------------------------------------------------------
 * Works for all combinations input-pcm & output-pcm, input-non-pcm & output-non-pcm.
 *
 * puts metadata on o/p port only when corresponding data is in the o/p buffer, i.e., offset cannot be
 * outside actual_data_len. If outside, then module needs to hold internally.
 *
 * Example1:
 *    7 ms input, 3ms offset of metadata1, 6ms offset of metadata2, 5ms input threshold. 6 ms algo delay. 5 ms out
 buf.
 *       first process call:
 *          5 ms is consumed, and sent to output. metadata1 is stored in internal list with offset =(3+6)-5=4ms.
 *          metadata2 is left in the input list as it's outside the consumed data.
 *          metadata1 is within input consumed, but module cannot send out as metadata offset
 *          would be outside the buf (9 ms).
 *       next process: input = 2 ms old data + x ms new data. container should've updated the metadata2 offset to 1
 ms.
 *          metadata 1 will go to output with offset 4 ms (bcoz 4 < out buf 5 ms).
 *          metadata 2 will go to internal list with offset (1+6)-5=2.
 *
 * Since input and output sample rates may be different offsets need to be scaled.
 * internal_md_list_pptr objects are at the output scale.
 *
 * For non-pcm, algo delay is zero. non-pcm cases, metadata can go internal_md_list_pptr due to squeezing behavior.
 *
 * For PCM, All channels must have same number of samples.
 *
 * EOS has special behavior.
 *  - EOS offset always goes to last byte on the output buffer.
 *  - Squeezing behavior is required for non-generic modules. Flushing EOS is not propagated until module stops
 * producing data.
 *     For other module types, if this behavior is needed, then they need to copy EoS internally inside CAPIv2, &
 *     set to output once squeezing is over.  We don't need do this squeezing behavior in GEN_CNTR (underflow)
 *     because GEN_CNTR always underruns and pushes zeros.
 *  - input EOS flag is cleared & output EOS flag set only after EOS propagates out of input and internal list to
 output
 * list.
 *     When EOS is stuck inside due to algo delay or squeezing behavior, input-flag marker_eos is not cleared.
 *
 * ---------------------------- BUFFER ASSOCIATED METADATA
 ------------------------------------------------------------
 *   When the metadata is buffer associated, algo delay does not affect the offset.
 *     E.g :- MODULE_CMN_MD_ID_DFG
 */
capi_err_t gen_topo_capi_metadata_propagate(void *                      context_ptr,
                                            capi_stream_data_v2_t *     input_stream_ptr,
                                            capi_stream_data_v2_t *     output_stream_ptr,
                                            module_cmn_md_list_t **     internal_md_list_pptr,
                                            uint32_t                    algo_delay_us,
                                            intf_extn_md_propagation_t *input_md_info_ptr,
                                            intf_extn_md_propagation_t *output_md_info_ptr)
{
   capi_err_t result = CAPI_EOK;

   if ((NULL == input_stream_ptr) || (NULL == context_ptr) || (NULL == output_stream_ptr))
   {
      return CAPI_EBADPARAM;
   }

   gen_topo_module_t *    module_ptr               = (gen_topo_module_t *)context_ptr;
   module_cmn_md_list_t **input_md_list_pptr       = &input_stream_ptr->metadata_list_ptr;
   module_cmn_md_list_t **output_md_list_pptr      = &output_stream_ptr->metadata_list_ptr;
   uint32_t               in_consumed_per_ch       = input_md_info_ptr->len_per_ch_in_bytes;
   uint32_t               out_produced_per_ch      = output_md_info_ptr->len_per_ch_in_bytes;
   uint32_t               out_initial_len_per_ch   = output_md_info_ptr->initial_len_per_ch_in_bytes;
   uint32_t               buf_in_len_per_ch        = input_md_info_ptr->buf_delay_per_ch_in_bytes;
   bool_t                 any_eos_stuck            = FALSE; // any flushing eos stuck is inside module or at input.
   bool_t                 any_eos_moved            = FALSE; // any flushing eos moved to output port
   bool_t                 any_flushing_eos_present = FALSE; // whether flushing eos is present in the lists.
   uint32_t               algo_delay_len_in        = 0;     // Algo delay in input scale
   bool_t                 in_data_present          = (0 != input_md_info_ptr->initial_len_per_ch_in_bytes);

   /**< Indicates whether data is present or not on input/output.
       When len_per_ch_in_bytes is zero, we have an ambiguity about whether data was present
       and it's not consumed (produced) or whether data was not present at all.
       E.g. if there's no data but metadata was present, then offset is zero, len_per_ch_in_bytes and MD
       propagates ahead, but if there's data but no data is consumed, then metadata must not move ahead
       as data is not consumed. */

   if ((NULL == *input_md_list_pptr) && ((NULL == internal_md_list_pptr) || (NULL == *internal_md_list_pptr)))
   {
#ifdef METADATA_DEBUGGING
//      AR_MSG(DBG_LOW_PRIO, "Both input and internal metadata list are NULL. returning.");
#endif
      return CAPI_EOK;
   }

   if (input_md_info_ptr->initial_len_per_ch_in_bytes < in_consumed_per_ch)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "MD_DBG: Module 0x%lX, data consumed %lu is more than initial data %lu",
             0x00000000,
             in_consumed_per_ch,
             input_md_info_ptr->initial_len_per_ch_in_bytes);
   }

   if ((CAPI_MAX_FORMAT_TYPE == input_md_info_ptr->df) || (CAPI_MAX_FORMAT_TYPE == output_md_info_ptr->df))
   {
      AR_MSG(DBG_ERROR_PRIO,
             "MD_DBG: data formats are not valid for module 0x%lX. Input %d, Output %d",
             0x00000000,
             input_md_info_ptr->df,
             output_md_info_ptr->df);
      return CAPI_EBADPARAM;
   }

   if (CAPI_IS_PCM_PACKETIZED(input_md_info_ptr->df))
   {
      if ((0 == input_md_info_ptr->bits_per_sample) || (0 == input_md_info_ptr->sample_rate))
      {
         AR_MSG(DBG_ERROR_PRIO,
                "MD_DBG: input formats are not valid for module 0x%lX. bits per sample %lu, sample rate %lu",
                0x00000000,
                input_md_info_ptr->bits_per_sample,
                input_md_info_ptr->sample_rate);
         return CAPI_EBADPARAM;
      }

      in_consumed_per_ch = (in_consumed_per_ch / (input_md_info_ptr->bits_per_sample >> 3)); // in samples
      algo_delay_len_in  = topo_us_to_samples(algo_delay_us, input_md_info_ptr->sample_rate);
      buf_in_len_per_ch  = (buf_in_len_per_ch / (input_md_info_ptr->bits_per_sample >> 3)); // in samples
   }

   if (CAPI_IS_PCM_PACKETIZED(output_md_info_ptr->df))
   {
      if ((0 == output_md_info_ptr->bits_per_sample) || (0 == output_md_info_ptr->sample_rate))
      {
         AR_MSG(DBG_ERROR_PRIO,
                "MD_DBG: input formats are not valid for module 0x%lX. bits per sample %lu, sample rate %lu",
                0x00000000,
                output_md_info_ptr->bits_per_sample,
                output_md_info_ptr->sample_rate);
         return CAPI_EBADPARAM;
      }

      out_produced_per_ch    = (out_produced_per_ch / (output_md_info_ptr->bits_per_sample >> 3));    // in samples
      out_initial_len_per_ch = (out_initial_len_per_ch / (output_md_info_ptr->bits_per_sample >> 3)); // in samples
   }

   /**
    * Algorithm:
    * first from internal_md_list_pptr to output list if offset is within out's range or has any buffer associated md
    * then go thru input list and copy to output if it exists or to internal if it exists or else,
    * update offset in input list itself.
    */
   if (internal_md_list_pptr)
   {
      module_cmn_md_list_t *node_ptr = (*internal_md_list_pptr);
      module_cmn_md_list_t *next_ptr = NULL;

      while (node_ptr)
      {
         next_ptr = node_ptr->next_ptr;

         module_cmn_md_t *md_ptr = node_ptr->obj_ptr;

         bool_t   flushing_eos  = gen_topo_is_flushing_eos(md_ptr);
         uint32_t offset_before = md_ptr->offset;

         bool_t md_moved = FALSE;

         // Try to move MD from internal list to output list.
         md_moved = gen_topo_move_md_to_out_list(module_ptr,
                                                 node_ptr,
                                                 output_md_list_pptr,
                                                 internal_md_list_pptr,
                                                 out_produced_per_ch,
                                                 out_initial_len_per_ch,
                                                 in_consumed_per_ch,
                                                 input_md_info_ptr->df,
                                                 output_md_info_ptr->df,
                                                 flushing_eos);

         any_flushing_eos_present |= flushing_eos;
         any_eos_moved |= (flushing_eos && md_moved);

         // if not moved, then update offset in the intermediate list.
         if (!md_moved)
         {
            any_eos_stuck |= flushing_eos;
            md_ptr->offset -= in_consumed_per_ch;
#ifdef METADATA_DEBUGGING
            PRINT_MD_PROP_DBG("updating offset on internal list", "in_consumed", in_consumed_per_ch, "");
#else
            (void)offset_before;
#endif
         }

         node_ptr = next_ptr;
      }
   }

   module_cmn_md_list_t *node_ptr = (*input_md_list_pptr);
   module_cmn_md_list_t *next_ptr = NULL;

   while (node_ptr)
   {
      next_ptr = node_ptr->next_ptr;

      module_cmn_md_t *md_ptr        = node_ptr->obj_ptr;
      uint32_t         offset_before = md_ptr->offset;
      bool_t           flushing_eos  = gen_topo_is_flushing_eos(md_ptr);
      bool_t           md_moved      = FALSE;
      uint32_t         is_ba_md = (MODULE_CMN_MD_BUFFER_ASSOCIATED == md_ptr->metadata_flag.buf_sample_association);

      any_flushing_eos_present |= flushing_eos;

      // If offset of metadata belongs to data that's consumed, then move MD also considered consumed by the module
      // If input is not present, we can propagate only if offset is zero.
      // When input consumed is zero, MD is moved only if input was zero to begin with; otherwise, even if offset is
      // zero, MD is not propagated.
      // Propagated MD will suffer (algo delay +) buffering delay before propagating to output.
      // E.g. in TRM, input is present, no data is consumed, MD shouldn't propagate.
      // However, normally when input is not present, we need to still propagate MD when offset is zero.
      if ((in_data_present && (md_ptr->offset <= in_consumed_per_ch) && (0 != in_consumed_per_ch)) ||
          (!in_data_present && (0 == md_ptr->offset)))
      {
         // Add algo delay only for sample associated metadata.
         md_ptr->offset += (is_ba_md ? 0 : algo_delay_len_in);

         // Add buffering delay for all metadata.
         md_ptr->offset += buf_in_len_per_ch;

         md_moved = gen_topo_move_md_to_out_list(module_ptr,
                                                 node_ptr,
                                                 output_md_list_pptr,
                                                 input_md_list_pptr,
                                                 out_produced_per_ch,
                                                 out_initial_len_per_ch,
                                                 in_consumed_per_ch,
                                                 input_md_info_ptr->df,
                                                 output_md_info_ptr->df,
                                                 flushing_eos);

         any_eos_moved |= (flushing_eos && md_moved);

         if (!md_moved)
         {
            any_eos_stuck |= flushing_eos;
            // new offset in the output next time or the next
            md_ptr->offset -= in_consumed_per_ch; // zero for flushing-EOS
            if (NULL == internal_md_list_pptr)
            {
               AR_MSG(DBG_ERROR_PRIO,
                      "intermediate list is NULL. Sending MD to output even "
                      "though it needs to be held in internal list");
               spf_list_move_node_to_another_list((spf_list_node_t **)output_md_list_pptr,
                                                  (spf_list_node_t *)node_ptr,
                                                  (spf_list_node_t **)input_md_list_pptr);
            }
            else
            {
               spf_list_move_node_to_another_list((spf_list_node_t **)internal_md_list_pptr,
                                                  (spf_list_node_t *)node_ptr,
                                                  (spf_list_node_t **)input_md_list_pptr);
#ifdef METADATA_DEBUGGING
               PRINT_MD_PROP_DBG("moving to internal list",
                                 "in_consumed",
                                 in_consumed_per_ch,
                                 ", out_produced_per_ch %lu, algo_delay_len_in %lu",
                                 out_produced_per_ch,
                                 algo_delay_len_in);
#endif
            }
         }
      }
      else
      {
         any_eos_stuck |= flushing_eos;
// keep in input list itself, no need to update offset as container does it in adj_offset_after_process
// md_ptr->offset -= in_consumed_per_ch;

#ifdef METADATA_DEBUGGING
         PRINT_MD_PROP_DBG("keeping in input",
                           "in_consumed",
                           in_consumed_per_ch,
                           "in_data_present %u",
                           in_data_present);
#else
         (void)offset_before;
#endif
      }

      node_ptr = next_ptr;
   } // loop through input_md_list_pptr

   // if any flushing EOS is stuck inside or at input, keep  input marker_eos set, else
   // clear input marker_eos and if any eos is moved to output, set output marker_eos.
   // marker_eos is set when the EOS is at the input or in the internal list. it's removed only after EOS goes to
   //   output
   // list.
   if (any_flushing_eos_present)
   {
      if (any_eos_stuck)
      {
         input_stream_ptr->flags.marker_eos = TRUE;
      }
      else
      {
         input_stream_ptr->flags.marker_eos = FALSE;
      }

      if (any_eos_moved)
      {
         output_stream_ptr->flags.marker_eos = TRUE;
      }
   }
   return result;
}

void gen_topo_populate_metadata_extn_vtable(module_info_t *                        module_ptr,
                                            intf_extn_param_id_metadata_handler_t *handler_ptr)
{
   memset(handler_ptr, 0, sizeof(*handler_ptr));

   handler_ptr->version                            = 1;
   handler_ptr->context_ptr                        = (void *)module_ptr;
   handler_ptr->metadata_create                    = gen_topo_capi_metadata_create;
   handler_ptr->metadata_clone                     = gen_topo_capi_metadata_clone;
   handler_ptr->metadata_destroy                   = gen_topo_capi_metadata_destroy;
   handler_ptr->metadata_propagate                 = gen_topo_capi_metadata_propagate;
   handler_ptr->metadata_modify_at_data_flow_start = gen_topo_capi_metadata_modify_at_data_flow_start;
}

/**
 * To be removed as container never needs to create DFG. Only pause module creates DFG.
 * Container can interpret it to move ports to data flow stop.
 */
ar_result_t gen_topo_create_dfg_metadata(uint32_t               log_id,
                                         module_cmn_md_list_t **metadata_list_pptr,
                                         POSAL_HEAP_ID          heap_id,
                                         module_cmn_md_t **     dfg_md_pptr)
{
   ar_result_t result = AR_EOK;

   if (dfg_md_pptr)
   {
      ar_result_t local_result =
         gen_topo_metadata_create(log_id, metadata_list_pptr, 0, heap_id, FALSE /* is_out_band*/, dfg_md_pptr);

      if (AR_SUCCEEDED(local_result))
      {
         (*dfg_md_pptr)->metadata_id                          = MODULE_CMN_MD_ID_DFG;
         (*dfg_md_pptr)->metadata_flag.buf_sample_association = MODULE_CMN_MD_BUFFER_ASSOCIATED;
#ifdef METADATA_DEBUGGING
         AR_MSG(DBG_LOW_PRIO, "Created DFG metadata");
#endif
      }
      else
      {
         AR_MSG(DBG_ERROR_PRIO, "DFG metadata create failed");
      }
      result |= local_result;
   }

   return result;
}

/**
 * Any buffer associated MD causes EOF. This is to ensure that threshold modules don't hold DFG or such buffer
 * associated MD
 */
bool_t gen_topo_md_list_has_buffer_associated_md(module_cmn_md_list_t *list_ptr)
{
   module_cmn_md_t *md_ptr                       = NULL;
   bool_t           is_buffer_associated_present = FALSE;

   while (list_ptr)
   {
      md_ptr = list_ptr->obj_ptr;

      if (MODULE_CMN_MD_BUFFER_ASSOCIATED == md_ptr->metadata_flag.buf_sample_association)
      {
         return TRUE;
      }
      list_ptr = list_ptr->next_ptr;
   }
   return is_buffer_associated_present;
}

//////TAKEN FROM gen_topo_capi.c
ar_result_t gen_topo_capi_set_param(uint32_t log_id,
                                    capi_t * capi_ptr,
                                    uint32_t param_id,
                                    int8_t * payload,
                                    uint32_t size)
{
   capi_err_t result = AR_EOK;

   capi_buf_t buf;
   buf.actual_data_len = size;
   buf.data_ptr        = payload;
   buf.max_data_len    = size;

   capi_port_info_t port_info;
   port_info.is_valid   = FALSE;
   port_info.port_index = 0;

   //   VERIFY(result, NULL != capi_ptr);
   if (result == (NULL != capi_ptr))
   {
      return capi_err_to_mm_result(result);
   }

   // Assume the Set param value would be a 32-bit integer, as is specified in CAPI document.
   result = capi_ptr->vtbl_ptr->set_param(capi_ptr, param_id, &port_info, &buf);

   if (CAPI_FAILED(result))
   {
      AR_MSG(DBG_ERROR_PRIO, "set param for (param id 0x%lx, size %lu) result %d", param_id, size, result);
   }
   else
   {
      AR_MSG(DBG_LOW_PRIO, "set param for (param id 0x%lx, size %lu) success", param_id, size);
   }

   //   CATCH(result, TOPO_MSG_PREFIX, log_id)
   //   {
   //   }

   return capi_err_to_mm_result(result);
}

/// Taken from gen_topo_metadata_island.c

capi_err_t gen_topo_capi_metadata_modify_at_data_flow_start(void *                 context_ptr,
                                                            module_cmn_md_list_t * md_node_ptr,
                                                            module_cmn_md_list_t **head_pptr)
{
   capi_err_t         result     = CAPI_EOK;
   ar_result_t        ar_result  = AR_EOK;
   gen_topo_module_t *module_ptr = (gen_topo_module_t *)context_ptr;
   gen_topo_t *       topo_ptr   = module_ptr->topo_ptr;

   if (NULL == md_node_ptr)
   {
      return result;
   }

   module_cmn_md_t *md_ptr = (module_cmn_md_t *)md_node_ptr->obj_ptr;

   if (MODULE_CMN_MD_ID_EOS == md_ptr->metadata_id)
   {
      module_cmn_md_eos_t *eos_metadata_ptr = 0;
      uint32_t             is_out_band      = md_ptr->metadata_flag.is_out_of_band;
      if (is_out_band)
      {
         eos_metadata_ptr = (module_cmn_md_eos_t *)md_ptr->metadata_ptr;
      }
      else
      {
         eos_metadata_ptr = (module_cmn_md_eos_t *)&(md_ptr->metadata_buf);
      }

      if (eos_metadata_ptr->flags.is_internal_eos)
      {
         AR_MSG(DBG_LOW_PRIO,
                "MD_DBG: Module 0x%lX, node_ptr 0x%p, md_ptr 0x%p, destroying internal EOS",
                module_ptr->gu.module_instance_id,
                md_node_ptr,
                md_ptr);

         ar_result =
            gen_topo_respond_and_free_eos(topo_ptr, module_ptr, md_node_ptr, FALSE /*is_eos_rendered*/, head_pptr);
      }
      else
      {
         if (MODULE_CMN_MD_EOS_FLUSHING == eos_metadata_ptr->flags.is_flushing_eos)
         {
            eos_metadata_ptr->flags.is_flushing_eos = MODULE_CMN_MD_EOS_NON_FLUSHING;
            AR_MSG(DBG_LOW_PRIO,
                   "MD_DBG: Module 0x%lX, modify_md: node_ptr 0x%p, md_ptr 0x%p"
                   "converted flushing EOS to non-flushing.",
                   module_ptr->gu.module_instance_id,
                   md_node_ptr,
                   md_ptr);
         }
      }
   }
   else if (MODULE_CMN_MD_ID_DFG == md_ptr->metadata_id)
   {
      AR_MSG(DBG_LOW_PRIO,
             "MD_DBG: Module 0x%lX, modify_md: node_ptr 0x%p, md_ptr 0x%p"
             "Destroying DFG.",
             module_ptr->gu.module_instance_id,
             md_node_ptr,
             md_ptr);

      bool_t IS_DROPPED_TRUE = TRUE;
      ar_result = gen_topo_capi_metadata_destroy((void *)module_ptr, md_node_ptr, IS_DROPPED_TRUE, head_pptr);
   }

   result = ar_result_to_capi_err(ar_result);

   return result;
}

// TAKEN from gen_topo_metadata_island.c
ar_result_t gen_topo_destroy_all_metadata(uint32_t               log_id,
                                          void *                 module_ctx_ptr,
                                          module_cmn_md_list_t **md_list_pptr,
                                          bool_t                 is_dropped)
{
   ar_result_t           result     = AR_EOK;
   gen_topo_module_t *   module_ptr = (gen_topo_module_t *)module_ctx_ptr;
   module_cmn_md_list_t *node_ptr   = *md_list_pptr;
   module_cmn_md_list_t *next_ptr;

   if (*md_list_pptr)
   {
      AR_MSG(DBG_LOW_PRIO,
             "MD_DBG: Destroy metadata for module 0x%lX, list ptr 0x%p, is_dropped %u",
             module_ptr->gu.module_instance_id,
             *md_list_pptr,
             is_dropped);
   }

   while (node_ptr)
   {
      next_ptr = node_ptr->next_ptr;
      // stream associated remains in the list and is not destroyed. hence md_list_pptr cannot be made NULL here.
      gen_topo_capi_metadata_destroy((void *)module_ptr, node_ptr, is_dropped, md_list_pptr);
      node_ptr = next_ptr;
   }
   return result;
}

///**
// * need_to_add - TRUE: convert bytes and add to offset
// *               FALSE: convert bytes and subtract from offset
// */
ar_result_t gen_topo_do_md_offset_math(uint32_t          log_id,
                                       uint32_t *        offset_ptr,
                                       uint32_t          bytes,
                                       topo_media_fmt_t *med_fmt_ptr,
                                       bool_t            need_to_add)
{
   uint32_t converted_bytes = bytes;

   if (SPF_IS_PACKETIZED_OR_PCM(med_fmt_ptr->data_format))
   {
      converted_bytes = topo_bytes_to_samples_per_ch(bytes, med_fmt_ptr);
   }

   if (need_to_add)
   {
      *offset_ptr += converted_bytes;
   }
   else
   {
      if (*offset_ptr >= converted_bytes)
      {
         *offset_ptr -= converted_bytes;
      }
      else
      {
         AR_MSG(DBG_ERROR_PRIO, "MD_DBG: offset calculation error. offset becoming negative. setting as zero");
         *offset_ptr = 0;
      }
   }

   return AR_EOK;
}

ar_result_t gen_topo_algo_reset(void *   topo_module_ptr,
                                uint32_t log_id,
                                bool_t   is_port_valid,
                                bool_t   is_input,
                                uint16_t port_index)
{
   gen_topo_module_t *module_ptr = (gen_topo_module_t *)topo_module_ptr;

   if (module_ptr->capi_ptr)
   {
      capi_err_t result = AR_EOK;

      capi_proplist_t props_list;
      capi_prop_t     props[1];

      if (NULL == module_ptr->capi_ptr)
      {
         result = AR_EBADPARAM;
         return capi_err_to_mm_result(result);
      }

      props[0].id                      = CAPI_ALGORITHMIC_RESET;
      props[0].payload.actual_data_len = 0;
      props[0].payload.max_data_len    = props[0].payload.actual_data_len;
      props[0].payload.data_ptr        = NULL;
      props[0].port_info.is_valid      = is_port_valid;
      props[0].port_info.is_input_port = is_input;
      props[0].port_info.port_index    = port_index;

      props_list.props_num = 1;
      props_list.prop_ptr  = props;

      result = module_ptr->capi_ptr->vtbl_ptr->set_properties(module_ptr->capi_ptr, &props_list);
      if (CAPI_FAILED(result) && (result != CAPI_EUNSUPPORTED))
      {
         AR_MSG(DBG_ERROR_PRIO, "capi algorithmic reset failed");
         return result;
      }
      result = CAPI_EOK;

      return capi_err_to_mm_result(result);
   }
   return AR_EOK;
}
