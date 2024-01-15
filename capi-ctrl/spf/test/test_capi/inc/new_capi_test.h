#ifndef __NEW_CAPI_TEST_H
#define __NEW_CAPI_TEST_H
/*===========================================================================

                  C A P I V 2  T E S T   U T I L I T I E S

  Common definitions, types and functions for CAPI.*/

/*===========================================================================*/

/* =========================================================================
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ========================================================================== */

/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------

===========================================================================*/

// SPF_IMPORT_BEGIN apm_api.h
/**
    In-band :
         This structure is followed by an in-band payload of type,
         apm_module_param_data_t
    Out-of-band :
         Out of band payload can be extracted from the mem_map_handle,
         payload_address_lsw and payload_address_msw using the
         posal api posal_memorymap_get_virtual_addr_from_shmm_handle.
         Out of band payload of apm_cmd_header_t can be of 2 types.
         1. apm_module_param_data_t
         2. apm_module_param_shared_data_t - This payload should only be used
            for the following apm commands
               a. APM_CMD_REGISTER_SHARED_CFG
               b. APM_CMD_DEREGISTER_SHARED_CFG
 */
typedef struct apm_cmd_header_t
{
   uint32_t payload_address_lsw;
   /**< Lower 32 bits of the payload address. */

   uint32_t payload_address_msw;
   /**< Upper 32 bits of the payload address.

         The 64-bit number formed by payload_address_lsw and
         payload_address_msw must be aligned to a 32-byte boundary and be in
         contiguous memory.

         @values
         - For a 32-bit shared memory address, this field must be set to 0.
         - For a 36-bit shared memory address, bits 31 to 4 of this field must
           be set to 0. @tablebulletend */

   uint32_t mem_map_handle;
   /**< Unique identifier for a shared memory address.

        @values
        - NULL -- The message is in the payload (in-band).
        - Non-NULL -- The parameter data payload begins at the address
          specified by a pointer to the physical address of the payload in
          shared memory (out-of-band).

        @contcell
        The aDSP returns this memory map handle through
        #apm_CMD_SHARED_MEM_MAP_REGIONS.

        An optional field is available if parameter data is in-band:
        %afe_port_param_data_v2_t param_data[...].
        See <b>Parameter data variable payload</b>. */

   uint32_t payload_size;
   /**< Actual size of the variable payload accompanying the message or in
        shared memory. This field is used for parsing both in-band and
        out-of-band data.

        @values > 0 bytes, in multiples of 4 bytes */
} apm_cmd_header_t;

/**
   Payload of the module parameter data structure.
   Immediately following this structure are param_size bytes of
   calibration data. The structure and size depend on the
   module_instace_id/param_id combination.
*/

#include "spf_begin_pack.h"

typedef struct apm_module_param_data_t
{
   uint32_t module_instance_id;
   /**< Valid instance ID of module
        @values  */

   uint32_t param_id;
   /**< Valid ID of the parameter.

        @values See Chapter */

   uint32_t param_size;
   /**< Size of the parameter data based upon the
        module_instance_id/param_id combination.
        @values > 0 bytes, in multiples of
        4 bytes at least */

   uint32_t error_code;
   /**< Error code populated by the entity hosting the  module.
     Applicable only for out-of-band command mode  */
} apm_module_param_data_t;
#include "spf_end_pack.h"
;

/**
   Payload of the shared persistent module parameter data structure.
   Immediately following this structure are param_size bytes of
   calibration data. The structure and size depend on the param_id.
*/

#include "spf_begin_pack.h"

typedef struct apm_module_param_shared_data_t
{
   uint32_t param_id;
   /**< Valid ID of the parameter.

        @values See Chapter */

   uint32_t param_size;
   /**< Size of the parameter data based upon the
        module_instance_id/param_id combination.
        @values > 0 bytes, in multiples of
        4 bytes at least */
} apm_module_param_shared_data_t;
#include "spf_end_pack.h"
;
// SPF_IMPORT_END apm_api.h

#endif // __NEW_CAPI_TEST_H
