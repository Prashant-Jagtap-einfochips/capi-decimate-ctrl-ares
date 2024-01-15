#ifndef _CAPI_INTF_EXTN_MIMO_MODULE_PROCESS_STATE_H_
#define _CAPI_INTF_EXTN_MIMO_MODULE_PROCESS_STATE_H_

/**
 *   \file capi_intf_extn_mimo_module_process_state.h
 *   \brief
 *        This file contains Interface Extension Definitions for disabling MIMO modules.
 *
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#include "capi_types.h"

/* Following discussion is directly pulled into the PDF */
/** @addtogroup capi_intf_extn_mimo_module_process_state

MIMO modules which can sometimes work in SISO mode and doesn't need any processing from input to output can be disabled
using this extension.

If MIMO modules use the generic event (#CAPI_EVENT_PROCESS_STATE) to disable then data flow blocks at their input which
may not be desirable so they can use this extension to inform the framework that they are disabled. Framework tries to
honor the modules request by evaluating certain conditions which are as follows:-
1. Module must be operating in SISO mode. Only one active input and one active output port.
2. Module must have zero algorithm/buffer delay. It should not be maintain any delay buffer during disable state.
3. Module must have valid and same media format on input and output port.

Framework can enable the module at any time without informing it to the module, usually when
1. a new port opens and module is not operating in SISO mode anymore
2. framework detects non-zero algo delay for the module
3. framework detects the different media format on input and output port

Module can also be enabled temporarily to propagate certain metadata. This is why module must not have any algo delay
because during disable state its process can be call discontinuously and if module maintains any delay buffer then it
can get discontinuous data.

*/

/** @addtogroup capi_intf_extn_mimo_module_process_state
@{ */

/*==============================================================================
   Constants
==============================================================================*/

/** Unique identifier of the interface extension that MIMO (multiple input and/or multiple output) module uses to
   enable/disable itself.

   Only MIMO module which can't use the #CAPI_EVENT_PROCESS_STATE to update their enable/disable state should use this
   extension.

    This extension supports the following event:
    - #INTF_EXTN_EVENT_ID_MIMO_MODULE_PROCESS_STATE @newpage
*/
#define INTF_EXTN_MIMO_MODULE_PROCESS_STATE 0x0A00101C

/** ID of the custom event raised by the module to enable/disable itself.

    When module disables itself then framework evaluates whether module can be removed from the processing chain or not. If
   it can be removed then module's process will not be called and if it can't be removed then module is considered
   enable.

   Even if module is considered disabled, framework can still call the module's process sometimes which is usually to propagate certain metadata.

    @msgpayload{intf_extn_event_id_mimo_module_process_state_t}
    @table{weak__intf__extn__event__id__mimo__module__process__state__t}
 */
#define INTF_EXTN_EVENT_ID_MIMO_MODULE_PROCESS_STATE 0x0A001059

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure defined for above Property  */
typedef struct intf_extn_event_id_mimo_module_process_state_t intf_extn_event_id_mimo_module_process_state_t;

/** @weakgroup weak_intf_extn_event_id_mimo_module_process_state_t
@{ */
struct intf_extn_event_id_mimo_module_process_state_t
{
   bool_t is_disabled;
   /**< Indicates whether module is disabled or enabled.

        @valuesbul
        - 0 -- Enabled
        - 1 -- Disabled @tablebulletend */
};
/** @} */ /* end_weakgroup weak_intf_extn_event_id_mimo_module_process_state_t */


#endif /* _CAPI_INTF_EXTN_MIMO_MODULE_PROCESS_STATE_H_ */

