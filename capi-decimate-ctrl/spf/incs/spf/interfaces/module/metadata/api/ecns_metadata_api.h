#ifndef ECNS_METADATA_API_H
#define ECNS_METADATA_API_H

/**
 *   \file ecns_metadata_api.h
 *   \brief
 *        This file contains Metadata ID and payloads for dummy ecns module
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

/** @h2xml_title1           {ECNS Metadata API}
    @h2xml_title_agile_rev  {ECNS Metadata API}
    @h2xml_title_date       {August 14, 2019} */

/**************************************** DUMMY ECNS - Begin ***********************************/

/**
    Metadata ID for DUMMY ECNS. module_cmn_md_t structure
    has to set the metadata_id field to this id when the metadata
    is related to DUMMY ECNS.  The module also should check this ID before operating on
    DUMMY ECNS structures.

 */
#define DUMMY_METADATA 0x0A001051

typedef struct dummy_metadata_t dummy_metadata_t;

/** Data structure for the stream's metadata */
struct dummy_metadata_t
{
   bool payload_1;
   /**<Just adding dummy payload. Module developer may add payload relevant to his implementation. This payload can be
    * used by the module to inform upstream modules some relevant info based on which it can process data.
    * Say it can send sampling rate to some module further down the chain. In that way that module knows the sampling
    * rate at which this module received data. Than that module can check if the sampling rate of data that it has
    * received is same or not and in case if it is not same it can internally process to output with required sampling
    * rate (If that is a requirement)  */
};

/**************************************** DUMMY ECNS - End *************************************/

#endif // ECNS_METADATA_API_H
