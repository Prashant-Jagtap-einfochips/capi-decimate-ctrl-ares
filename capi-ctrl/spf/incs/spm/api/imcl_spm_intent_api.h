/**========================================================================
 @file imcl_spm_intent_api.h

 @brief This file contains all the public intent names
 the intent related structs are defined in internal header files

 Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

 ====================================================================== */

/**========================================================================
 Edit History

 $Header:
$

 when       who     what, where, why
 --------   ---     -------------------------------------------------------

========================================================================== */

#ifndef _IMCL_SPM_INTENT_API_H_
#define _IMCL_SPM_INTENT_API_H_

/*------------------------------------------------------------------------------
 *  Header Includes
 *----------------------------------------------------------------------------*/

#include "ar_defs.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/** -----------------------------------------------------------------------
 ** Type Declarations
 ** ----------------------------------------------------------------------- */

/* clang-format off */

/*==============================================================================
  Intent ID
==============================================================================*/
/**< Intent defines the payload structure of the IMCL message.
SVA and Dam modules supports the following functionalities â€“ */

#define INTENT_ID_AUDIO_DAM_DETECTION_ENGINE_CTRL    0x08001064
#define INTENT_ID_DUMMY_SP 0x08001231
/* clang-format on */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _IMCL_SPM_INTENT_API_H_ */
