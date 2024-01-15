#ifndef AR_OSAL_ERROR_H
#define AR_OSAL_ERROR_H

/**
*=============================================================================
* \file case_osal_error.h
*
* \brief
*     This file contains error codes used by CASA.
* \cond
*  Copyright (c) 2018-2020 by Qualcomm Technologies, Inc.  All Rights Reserved.
* \endcond
*=============================================================================
*/

/* -----------------------------------------------------------------------
** ERROR CODES
** ----------------------------------------------------------------------- */
/** Success. The operation completed with no errors. */
#define AR_EOK                                     (0)
/** General failure. */
#define AR_EFAILED                                 (1)
/** Bad operation parameter. */
#define AR_EBADPARAM                               (2)
/** Unsupported routine or operation. */
#define AR_EUNSUPPORTED                            (3)
/** Unsupported version. */
#define AR_EVERSION                                (4)
/** Unexpected problem encountered. */
#define AR_EUNEXPECTED                             (5)
/** Unhandled problem occurred. */
#define AR_EPANIC                                  (6)
/** Unable to allocate resource. */
#define AR_ENORESOURCE                             (7)
/** Invalid handle. */
#define AR_EHANDLE                                 (8)
/** Operation is already processed. */
#define AR_EALREADY                                (9)
/** Operation is not ready to be processed. */
#define AR_ENOTREADY                               (10)
/** Operation is pending completion. */
#define AR_EPENDING                                (11)
/** Operation cannot be accepted or processed. */
#define AR_EBUSY                                   (12)
/** Operation was aborted due to an error. */
#define AR_EABORTED                                (13)
/** Operation requests an intervention to complete. */
#define AR_ECONTINUE                               (14)
/** Operation requests an immediate intervention to complete. */
#define AR_EIMMEDIATE                              (15)
/** Operation was not implemented. */
#define AR_ENOTIMPL                                (16)
/** Operation needs more data or resources. */
#define AR_ENEEDMORE                               (17)
/** Operation does not have memory. */
#define AR_ENOMEMORY                               (18)
/** Item does not exist. */
#define AR_ENOTEXIST                               (19)
/** Operation is finished. */
#define AR_ETERMINATED                             (20)
/** Operation timeout. */
#define AR_ETIMEOUT                                (21)

/** Duplicate subgraph or connection opened. */
#define AR_EDUPLICATE                              (24)

/** Macro that checks whether a result is a success. @hideinitializer */
#define AR_SUCCEEDED(x)   (AR_EOK == (x) )

/** Macro that checks whether a result is a failure. @hideinitializer */
#define AR_FAILED(x)   (AR_EOK != (x) )

/** @} */ /* end_addtogroup gpr_status_codes */


#endif /* #ifndef AR_OSAL_ERROR_H */
