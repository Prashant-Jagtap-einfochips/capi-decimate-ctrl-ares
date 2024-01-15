#ifndef _CAPI_FWK_EXTN_ISLAND_H_
#define _CAPI_FWK_EXTN_ISLAND_H_

/**
 *   \file capi_fwk_extns_island.h
 *   \brief
 *        This file contains extension for island handling
 *
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //source/qcom/qct/platform/adsp/proj/addons_audio/spf/incs/spf/interfaces/module/capi/capi_fwk_extns_island.h#1 $
*/
// clang-format on

#include "capi_types.h"

/** @weakgroup weakf_capi_chapter_island
The Island framework extension (#FWK_EXTN_ISLAND) provides utils to exit from island from CAPI.
Note: This framework extension is deprecated. 
*/

/** @addtogroup capi_fw_ext_island
@{ */

/*==============================================================================
   Constants
==============================================================================*/

/** Unique identifier of the framework extension that modules use to exit from
    Island. (For more information, see Section
    @xref{chp:IslandChap}.)
 */
#define FWK_EXTN_ISLAND 0x0A001057

/** ID of the Event a module uses to trigger Exit from island.
*/
#define FWK_EXTN_EVENT_ID_ISLAND_EXIT 0x0A001058

#endif /* _CAPI_FWK_EXTN_ISLAND_H_ */

