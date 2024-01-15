#ifndef _CAPI_FWK_EXTNS_SOFT_PAUSE_H_
#define _CAPI_FWK_EXTNS_SOFT_PAUSE_H_

/*==============================================================================
  @file capi_fwk_extns_soft_pause.h
  @brief This file contains CAPI Pause Module Extension Definitions

  Framework extension for soft pause which is used to communicate to the framework
  when the module is paused, so that it can do the necessary data handling

  Copyright (c) 2018-2020 QUALCOMM Technologies, Inc.  All Rights Reserved.
  QUALCOMM Technologies, Inc Proprietary.  Export of this technology or
  software is regulated by the U.S. Government, Diversion contrary to U.S.
  law prohibited.
==============================================================================*/

/*==============================================================================
  Edit History

  when       who        what, where, why
  --------   ---        ------------------------------------------------------

==============================================================================*/

/*=====================================================================
  Includes
 ======================================================================*/
#include "capi_types.h"

/*==============================================================================
   Constants
==============================================================================*/
/* Framework extension ID for Pause Modules */
#define FWK_EXTN_SOFT_PAUSE  0x0A001006

/*==============================================================================
   Constants
==============================================================================*/
/* Custom event id to raise event when module goes into pause state */
#define FWK_EXTN_EVENT_ID_SOFT_PAUSE_COMPLETE_CB  0x0A001007

/*==============================================================================
   Type definitions
==============================================================================*/
/* Structure defined for above Property  */
typedef struct fwk_extn_event_id_soft_pause_complete_t
{
   bool_t is_timer_expiry;
   /**< Flag which indicates whether pause complete is due to timer expiry
    *  or because ramp down completed with data
        @values : {0..1} */
} fwk_extn_event_id_soft_pause_complete_t;


/*==============================================================================
   Constants
==============================================================================*/
/* Custom event id to raise event when module begins to enter resume state */
#define FWK_EXTN_EVENT_ID_SOFT_PAUSE_RESUME       0x0A00100F

#endif /* _CAPI_FWK_EXTNS_SOFT_PAUSE_H_ */
