/*========================================================================
posal.h

This is the top-level include file for the posal utilities.

This file includes all header files required for using posal functions.
The user of posal should only include this file to call the posal functions.
*/
/*========================================================================
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
====================================================================== */

#ifndef _POSAL_H_
#define _POSAL_H_

/* =======================================================================
INCLUDE FILES FOR MODULE
========================================================================== */
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
//#include "stringl.h"

#include "ar_defs.h"
#include "ar_msg.h"
#include "ar_error_codes.h"

#include "posal_std.h"
#include "posal_types.h"
#include "posal_atomic.h"
#include "posal_intrinsics.h"

#include "posal_memory.h"
#include "posal_memorymap.h"
#include "posal_heapmgr.h"

#include "posal_channel.h"
#include "posal_signal.h"
#include "posal_queue.h"
#include "posal_bufpool.h"

#include "posal_cache.h"
#include "posal_mutex.h"
#include "posal_nmutex.h"
#include "posal_thread.h"
#include "posal_timer.h"
#include "posal_interrupt.h"
#include "posal_condvar.h"

#include "posal_island.h"
#include "posal_data_log.h"
#include "posal_globalstate.h"
void posal_init(void);
void posal_deinit(void);

#endif // #ifndef _POSAL_H_
