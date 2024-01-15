/*==========================================================================
 * Copyright (c) 2018-2022 QUALCOMM Technologies Incorporated.
 * All rights reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *==========================================================================*/

/**
 * @file capi_unit_test.cpp
 *
 * Standalone Setup for CNG module
 */

/*==========================================================================
 * Edit History:
 * when         who         what, where, why
 * --------     ---         ------------------------------------------------
 *==========================================================================*/
// TODO: NO C FILES
//
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
//#include <string>
#include <assert.h>
//#include "test_main.h"
#include "ar_msg.h"
#include "posal_memory.h"

#define TEST_SUCCESS 0
#define TEST_FAILURE 1

/* Maximum number of interface extensions possible in unit test framework */
#define MAX_NUM_INTF_EXTNS 5
/* Current number of interface extensions chosen in unit test framework */
#define NUM_INTF_EXTNS 3

#ifdef EXTEND_ENABLE
#include "module_extensions.h"
#endif

#include "capi_test.h"
#include "capi_entry_function_externs.h"

#include "gen_topo_metadata.h"

/** Lowest debug message level to be shown in ADSP. This is only valid in SIM mode. */
// uint8_t mmlogs_debugmsg_lowest_prio = DBG_LOW_PRIO;

#define CHECK_NEQ(result, expectation)                                                                                 \
   if (result != expectation)                                                                                          \
   {                                                                                                                   \
      EXIT();                                                                                                          \
   }

#define CHECK_EQ(result, expectation)                                                                                  \
   if (result == expectation)                                                                                          \
   {                                                                                                                   \
      EXIT();                                                                                                          \
   }

#define EXIT()                                                                                                         \
   for (uint32_t j = 0; j < module->num_input_port; j++)                                                               \
      fclose(module->finp[j]);                                                                                         \
   for (uint32_t j = 0; j < module->num_output_port; j++)                                                              \
      fclose(module->fout[j]);                                                                                         \
   fclose(module->fCfg);

#define THROW(exception, errno)                                                                                        \
   exception = errno;                                                                                                  \
   goto exception##bail;

#define CATCH(exception) exception##bail : if (exception != TEST_SUCCESS)

#if 0
static int SigVerify_start(const char *         so_name,
                           const unsigned char *pHdrs,
                           int                  cbHdrs,
                           const unsigned char *pHash,
                           int                  cbHash,
                           void **              ppv)
{
   return 0;
}

static int SigVerify_verifyseg(void *pv, int segnum, const unsigned char *pSeg, int cbSeg)
{
   return 0;
}

static int SigVerify_stop(void *pv)
{
   return 0;
}
#endif

static int test_main_start(int argc, char *argv[]);

int main(int a, char **c)
{
   int nErr = 0;
#if 0
   char *  builtin[] = { (char *)"libc.so", (char *)"libstdc++.so", (char *)"libgcc.so" };
   DL_vtbl vtbl      = { sizeof(DL_vtbl), HAP_debug_v2, SigVerify_start, SigVerify_verifyseg, SigVerify_stop };

   (void)dlinitex(3, builtin, &vtbl);
#endif

   FARF(HIGH, "--------------------------------------------------------------------");
   FARF(HIGH, "Calling test_main_start in target module's test file.               ");
   FARF(HIGH, "--------------------------------------------------------------------");

   nErr = test_main_start(a, c);

   FARF(HIGH, "--------------------------------------------------------------------");
   FARF(HIGH, "%s                                                             ", nErr ? "FAILED " : "Success");
   FARF(HIGH, "--------------------------------------------------------------------");

   return nErr;
}

int test_main_start(int argc, char *argv[])
{
   capi_err_t     result      = CAPI_EOK;
   uint32_t       num_modules = 1; // used in case of control links
   module_info_t *module = (module_info_t *)posal_memory_malloc(sizeof(module_info_t), POSAL_HEAP_DEFAULT); // ms13: *2
   AR_MSG(DBG_HIGH_PRIO,
          "Test module info allocated for %lu bytes of memory at location 0x%p.",
          sizeof(module_info_t),
          module);
   memset(module, 0, sizeof(module_info_t));
#if 0
   if (1 == num_modules)
   {
      memset(&module[1], 0, sizeof(module_info_t));
   }
#endif
   /*Initialize to zero*/
   module->num_input_port  = 0;
   module->num_output_port = 0;

   for (uint32_t i = 0; i < CAPI_MAX_INPUT_PORTS; i++)
   {
      module->is_media_format_received[i] = FALSE;
   }

   /* Read input arguments */
   args_t input_args;
   get_eargs(argc, argv, &input_args, &module->num_input_port, &module->num_output_port);

   AR_MSG(DBG_HIGH_PRIO, "num_input_port = %lu, out = %lu", module->num_input_port, module->num_output_port);
   AR_MSG(DBG_HIGH_PRIO, "input_args.input_filename[0]=%s", input_args.input_filename[0]);

   for (uint32_t i = 0; i < module->num_input_port; i++)
   {
      if ((module->finp[i] = fopen(input_args.input_filename[i], "rb")) == NULL)
      {
         fprintf(stderr, "%s: ERROR CODE 2 - Cannot open input file '%s'\n", argv[0], input_args.input_filename[i]);
         for (uint32_t j = 0; j < module->num_input_port - 1; j++)
         {
            fclose(module->finp[j]);
         }
         exit(-1);
      }
      AR_MSG(DBG_HIGH_PRIO, "input_args.input_filename[%lu]=%s", i, input_args.input_filename[i]);
   }

   for (uint32_t i = 0; i < module->num_output_port; i++)
   {
      if ((module->fout[i] = fopen(input_args.output_filename[i], "wb")) == NULL)
      {
         fprintf(stderr, "%s: ERROR CODE - Cannot open output file '%s' \n", argv[0], input_args.output_filename[i]);
         for (uint32_t j = 0; j < module->num_input_port; j++)
         {
            fclose(module->finp[j]);
         }
         for (uint32_t j = 0; j < module->num_output_port - 1; j++)
         {
            fclose(module->fout[j]);
         }
         exit(-1);
      }
      AR_MSG(DBG_HIGH_PRIO, "input_args.output_filename[%lu]=%s", i, input_args.output_filename[i]);
   }

   if ((module->fCfg = fopen(input_args.config_filename, "rb")) == NULL)
   {
      fprintf(stderr, "%s : ERROR CODE 3 Cannot open config file '%s'.\n", argv[0], input_args.config_filename);
      for (uint32_t j = 0; j < module->num_input_port; j++)
      {
         fclose(module->finp[j]);
      }
      for (uint32_t j = 0; j < module->num_output_port; j++)
      {
         fclose(module->fout[j]);
      }
      exit(-1);
   }

   /* STEP 1: Get size requirements of CAPI */
   AR_MSG(DBG_HIGH_PRIO, "MAIN: -------------------");
   AR_MSG(DBG_HIGH_PRIO, "MAIN:  Initialize module ");
   AR_MSG(DBG_HIGH_PRIO, "MAIN: -------------------");

   /* Query for CAPI size */
   capi_proplist_t static_properties;
   static_properties.props_num = 7;
   capi_prop_t prop_ptr[static_properties.props_num];
   static_properties.prop_ptr = prop_ptr;

   // todo: ENHANCE STATIC PROPS AND INIT PROPS
   /* Populate INIT_MEMORY_REQUIREMENT query */
   capi_init_memory_requirement_t         mem_req;
   capi_stack_size_t                      stack_size;
   capi_max_metadata_size_t               max_metadata_size;
   capi_is_inplace_t                      is_inplace;
   capi_requires_data_buffering_t         requires_data_buffering;
   capi_num_needed_framework_extensions_t num_needed_framework_extensions;
   capi_interface_extns_list_t            *interface_extns_list=NULL;

   /* Allocate interface extensions list with capacity to support 5 interface extensions */
   uint8_t *interface_extns_list_ptr = (uint8_t *)posal_memory_malloc((sizeof(capi_interface_extns_list_t)+ MAX_NUM_INTF_EXTNS * sizeof(capi_interface_extn_desc_t)), POSAL_HEAP_DEFAULT);
   CHECK_EQ(interface_extns_list_ptr, NULL);
   interface_extns_list = (capi_interface_extns_list_t*)interface_extns_list_ptr;

   prop_ptr[0].id                   = CAPI_INIT_MEMORY_REQUIREMENT;
   prop_ptr[0].payload.data_ptr     = (int8_t *)&mem_req;
   prop_ptr[0].payload.max_data_len = sizeof(capi_init_memory_requirement_t);
   prop_ptr[0].port_info.is_valid   = FALSE;

   prop_ptr[1].id                   = CAPI_STACK_SIZE;
   prop_ptr[1].payload.data_ptr     = (int8_t *)&stack_size;
   prop_ptr[1].payload.max_data_len = sizeof(stack_size);

   prop_ptr[2].id                      = CAPI_INTERFACE_EXTENSIONS;
   prop_ptr[2].payload.data_ptr        = (int8_t *)interface_extns_list;
   interface_extns_list->num_extensions = NUM_INTF_EXTNS;
   prop_ptr[2].payload.max_data_len =
      sizeof(capi_interface_extns_list_t) +
      interface_extns_list->num_extensions *
         sizeof(capi_interface_extn_desc_t); // "3" is the maximum no of extensions that
                                             // a module can support in testfwk as of
                                             // now. It is not a fixed value, if you have a
                                             // module that requires more just replace
                                             // "NUM_INTF_EXTNS" by your requirement upto MAX_NUM_INTF_EXTNS

   prop_ptr[5].id                   = CAPI_IS_INPLACE;
   prop_ptr[5].payload.data_ptr     = (int8_t *)&is_inplace;
   prop_ptr[5].payload.max_data_len = sizeof(is_inplace);

   prop_ptr[6].id                   = CAPI_REQUIRES_DATA_BUFFERING;
   prop_ptr[6].payload.data_ptr     = (int8_t *)&requires_data_buffering;
   prop_ptr[6].payload.max_data_len = sizeof(requires_data_buffering);

   prop_ptr[4].id                   = CAPI_NUM_NEEDED_FRAMEWORK_EXTENSIONS;
   prop_ptr[4].payload.data_ptr     = (int8_t *)&num_needed_framework_extensions;
   prop_ptr[4].payload.max_data_len = sizeof(num_needed_framework_extensions);

   prop_ptr[3].id                   = CAPI_MAX_METADATA_SIZE;
   prop_ptr[3].payload.data_ptr     = (int8_t *)&max_metadata_size;
   prop_ptr[3].payload.max_data_len = sizeof(max_metadata_size);

   capi_interface_extn_desc_t *curr_intf_extn_desc_ptr =
      (capi_interface_extn_desc_t *)((int8_t *)(interface_extns_list) + (sizeof(capi_interface_extns_list_t)));

   capi_interface_extn_desc_t *intf_extn_desc_ptr = curr_intf_extn_desc_ptr;

   curr_intf_extn_desc_ptr->id           = INTF_EXTN_IMCL;
   curr_intf_extn_desc_ptr->is_supported = FALSE;
   curr_intf_extn_desc_ptr++;

   curr_intf_extn_desc_ptr->id           = INTF_EXTN_DATA_PORT_OPERATION;
   curr_intf_extn_desc_ptr->is_supported = FALSE;
   curr_intf_extn_desc_ptr++;

   curr_intf_extn_desc_ptr->id           = INTF_EXTN_METADATA;
   curr_intf_extn_desc_ptr->is_supported = FALSE;
   curr_intf_extn_desc_ptr++;

#ifdef __V_DYNAMIC__
   void *h = 0;
   // int (*pfn)(void) = 0;
   const char *cpszmodname               = CAPI_SO_NAME;
   const char *cpszget_static_properties = CAPI_STATIC_PROP_FUNCTION;
   const char *cpszinit                  = CAPI_INIT_FUNCTION;

   capi_get_static_properties_f get_static_properties_f = 0;
   capi_init_f                  init_f                  = 0;
   //int                          err                     = TEST_SUCCESS;

   FARF(HIGH, "-- start dll test --                                                ");

   FARF(HIGH, "attempt to load   %s                               ", cpszmodname);
   h = dlopen(cpszmodname, RTLD_NOW);
   if (0 == h)
   {
      FARF(HIGH, "dlopen %s failed %s                           ", cpszmodname, dlerror());
      //     THROW(err, TEST_FAILURE);
      return 1;
   }
   get_static_properties_f = (capi_get_static_properties_f)dlsym(h, cpszget_static_properties);
   if (0 == get_static_properties_f)
   {
      FARF(HIGH, "dlsym %s failed %s                              ", cpszget_static_properties, dlerror());
      // THROW(err, TEST_FAILURE);
      return 1;
   }
   result = get_static_properties_f(NULL, &static_properties);
   result &= ~CAPI_EUNSUPPORTED;
   CHECK_NEQ(result, CAPI_EOK);
#else
   result = (*capi_entry_get_static)(NULL, &static_properties);
   result &= ~CAPI_EUNSUPPORTED;
   CHECK_NEQ(result, CAPI_EOK);
#endif

   /* STEP 2: Allocate memory */
   uint8_t *ptr = (uint8_t *)posal_memory_malloc(mem_req.size_in_bytes, POSAL_HEAP_DEFAULT);

   CHECK_EQ(ptr, NULL);

   memset(ptr, 0, mem_req.size_in_bytes);
   module->module_ptr = (capi_t *)ptr;
   AR_MSG(DBG_HIGH_PRIO, "Module allocated for %lu bytes of memory at location 0x%p.", mem_req.size_in_bytes, ptr);

   /* STEP 3: Initialize module */
   capi_event_callback_info_t cb_info = capi_tst_get_cb_info(module);
   capi_prop_t                cb_prop[2];
   cb_prop[0].id                      = CAPI_EVENT_CALLBACK_INFO;
   cb_prop[0].payload.max_data_len    = sizeof(cb_info);
   cb_prop[0].payload.actual_data_len = sizeof(cb_info);
   cb_prop[0].payload.data_ptr        = reinterpret_cast<int8_t *>(&cb_info);

   // amith: Set port info as the first thing in the config file
   // capi_port_num_info_t port_info;
   // port_info.num_input_ports = module->num_input_port;
   // port_info.num_output_ports = module->num_output_port;
   // cb_prop[1].id = CAPI_PORT_NUM_INFO;
   // cb_prop[1].payload.actual_data_len = sizeof(port_info);
   // cb_prop[1].payload.max_data_len = sizeof(port_info);
   // cb_prop[1].payload.data_ptr = reinterpret_cast<int8_t*>(&port_info);

   capi_proplist_t init_proplist;
   init_proplist.props_num = 1;
   init_proplist.prop_ptr  = cb_prop;

#ifdef __V_DYNAMIC__
   init_f = (capi_init_f)dlsym(h, cpszinit);
   if (0 == init_f)
   {
      FARF(HIGH, "dlsym %s failed %s                              ", cpszinit, dlerror());
      //    THROW(err, TEST_FAILURE);
      return 1;
   }
   result = init_f((capi_t *)ptr, &init_proplist);
   result &= ~CAPI_EUNSUPPORTED;
   CHECK_NEQ(result, CAPI_EOK);
#else
   result = (*capi_entry_init)((capi_t *)ptr, &init_proplist);
   result &= ~CAPI_EUNSUPPORTED;
   CHECK_NEQ(result, CAPI_EOK);
#endif

   /*Handle interface extns*/
   for (uint32_t extns = 0; extns < interface_extns_list->num_extensions; extns++)
   {
      if ((INTF_EXTN_METADATA == intf_extn_desc_ptr->id) && (TRUE == intf_extn_desc_ptr->is_supported))
      {
         AR_MSG(DBG_HIGH_PRIO, "Interface extension for metadata is supported");
         intf_extn_param_id_metadata_handler_t handler;

         gen_topo_populate_metadata_extn_vtable(module, &handler);

         result |= gen_topo_capi_set_param(0x00000000, // here in original fwk code, MID appears
                                           module->module_ptr,
                                           INTF_EXTN_PARAM_ID_METADATA_HANDLER,
                                           (int8_t *)&handler,
                                           sizeof(handler));
      }
      intf_extn_desc_ptr++;
   }

   /* Run config file */
   AR_MSG(DBG_HIGH_PRIO, "MAIN: ----------------");
   AR_MSG(DBG_HIGH_PRIO, "MAIN: Run config file ");
   AR_MSG(DBG_HIGH_PRIO, "MAIN: ----------------");

   uint32_t count =
      (module->num_input_port > module->num_output_port) ? module->num_output_port : module->num_input_port;
   for (uint32_t i = 0; i < count; i++)
   {
      module->out_buffer_len[i] = module->in_buffer_len[i];
   }
   module->requires_data_buffering = TRUE;
   module->is_enabled              = TRUE;
   module->alg_delay               = 0;

#ifdef __V_DYNAMIC__
   result = RunTest(module, num_modules);

   /* Destroy CAPI and free memory */
   module->module_ptr->vtbl_ptr->end(module->module_ptr);
   //       CATCH(err){};
   if (0 != h)
   {
   	  FARF(HIGH, "closing %s                                                 ", cpszmodname);
      dlclose(h);
   }
   FARF(HIGH, "-- end dll test --                                                  ");

   if (CAPI_EOK != result)
   {
      FARF(HIGH, "##################  Module Test FAILED, Error in RunTest  ##################");
   }
   else
   {
      FARF(HIGH, "##################  Module Test PASSED  ##################");
   }
#else
   result = RunTest(module, num_modules);

   /* Destroy CAPI and free memory */
   module->module_ptr->vtbl_ptr->end(module->module_ptr);

   if (CAPI_EOK != result)
   {
      AR_MSG(DBG_ERROR_PRIO, "##################  Module Test FAILED, Error in RunTest  ##################");
   }
   else
   {
      AR_MSG(DBG_HIGH_PRIO, "##################  Module Test PASSED  ##################");
   }
#endif

   posal_memory_free(interface_extns_list_ptr);
   posal_memory_free(ptr);
   posal_memory_free(module);
   EXIT();

   AR_MSG(DBG_HIGH_PRIO, "####################### MAIN: Done #######################");
   return result;
}
