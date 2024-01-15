#ifndef ELITE_INTF_EXTNS_GAPLESS_H
#define ELITE_INTF_EXTNS_GAPLESS_H

/* ======================================================================== */
/**
@file Elite_intf_extns_gapless.h

@brief Interface extension header for gapless decoders

  This file defines the parameters, events and other behaviors associated
  with gapless decoders
*/

/* =========================================================================
   Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
  ========================================================================== */

/* =========================================================================
                             Edit History

   when       who     what, where, why
   --------   ---     ------------------------------------------------------

   ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


#include "capi_types.h"

/** Unique identifier of the custom interface extension for gapless decoders. */
#define INTF_EXTN_GAPLESS           (0x0A001002)

/*------------------------------------------------------------------------------
 * Parameter IDs
 *----------------------------------------------------------------------------*/

/** ID of the parameter that let's container inform the CAPI that the subsequent buffers until EOS (and including EOS)
    are from the last buffer. 
    This param helps CAPI module detect when trailing zeros of a gapless stream must be removed.
    
 */
#define PARAM_ID_GAPLESS_LAST_BUFFER           (0x0A001011)


#ifdef __cplusplus
}
#endif //__cplusplus
#endif
