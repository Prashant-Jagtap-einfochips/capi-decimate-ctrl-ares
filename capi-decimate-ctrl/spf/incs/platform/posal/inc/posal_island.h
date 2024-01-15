/*========================================================================
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.      
  Confidential and Proprietary - Qualcomm Technologies, Inc.
========================================================================= */
/**
@file posal_isalnd.h
@brief @brief This file contains island utilities' declarations.
*/

#ifndef POSAL_ISLAND_H
#define POSAL_ISLAND_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* -----------------------------------------------------------------------
** Global definitions/forward declarations
** ----------------------------------------------------------------------- */
extern POSAL_HEAP_ID spf_mem_island_heap_id;

/**
  This function process island exit.

  @return
  Indication of success (0) or failure (nonzero).

  @dependencies
   None.
*/
ar_result_t posal_island_trigger_island_exit(void);

/**
  This function Get island mode status.

  Returns a value indicating whether the underlying system is executing in island mode.

  @return
  0 - Normal mode.
  1 - Island mode.

  @dependencies
  None.
*/
bool_t posal_island_get_island_status(void);

static inline POSAL_HEAP_ID posal_get_island_heap_id(void)
{
   return spf_mem_island_heap_id;
}

#ifdef __cplusplus
}
#endif //__cplusplus
#endif // #ifndef POSAL_ISLAND_H
