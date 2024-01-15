#ifdef CAPI_STANDALONE
#include "hexagon_sim_timer.h"

/* -----------------------------------------------------------------------
** Standard Integer Types and Definitions
** ----------------------------------------------------------------------- */
typedef unsigned long long uint64_t; /* Unsigned 64 bit value */
typedef unsigned long int  uint32_t; /* Unsigned 32 bit value */
typedef unsigned short     uint16_t; /* Unsigned 16 bit value */
typedef unsigned char      uint8_t;  /* Unsigned 8  bit value */

typedef signed long long int64_t; /* Signed 64 bit value */
typedef signed long int  int32_t; /* Signed 32 bit value */
typedef signed short     int16_t; /* Signed 16 bit value */
typedef signed char      int8_t;  /* Signed 8  bit value */

#undef TRUE
#undef FALSE

#define TRUE (1)  /* Boolean true value. */
#define FALSE (0) /* Boolean false value. */

#ifndef NULL
#define NULL (0)
#endif

/*
   POSAL version of assert.
*/
#define POSAL_ASSERT(x) assert(x)
/* ------------------------------------------------------------------------
 ** POSAL Diag Definitions
 ** ------------------------------------------------------------------------ */
#define debug_enable 1               /**< disable or enable debug msg*/
#define lowest_priority DBG_MED_PRIO /**< lowest debug message level to be shown*/

#define DBG_FATAL_PRIO 16
#define DBG_ERROR_PRIO 8
#define DBG_HIGH_PRIO 4
#define DBG_MED_PRIO 2
#define DBG_LOW_PRIO 1

#define MSG_SSID_QDSP6 0

// if __FILENAME macro is not available, just leave that field blank
#ifndef __FILENAME__
#define __FILENAME__ ""
#endif

// assert is for priority == 16 (i.e on FATAL)
// disable warning "conditional expression is constant" for debug_enable
#define DBG_MSG(ssid, priority, message, ...)                                                                          \
   do                                                                                                                  \
   {                                                                                                                   \
      if (debug_enable && (priority >= lowest_priority))                                                               \
      {                                                                                                                \
         uint64_t cycles = hexagon_sim_read_cycles();                                                                  \
         int      tms    = ((cycles >> 10) * 1805) >> 20;                                                              \
         int      tsec   = ((cycles >> 10) * 1805) >> 30;                                                              \
         int      tmin   = ((cycles >> 10) * 30) >> 30;                                                                \
         printf("%dm:%ds:%dms @ %d of %s : " message "\n",                                                             \
                tmin,                                                                                                  \
                tsec - 60 * tmin,                                                                                      \
                tms - 1000 * tsec,                                                                                     \
                __LINE__,                                                                                              \
                __FILENAME__,                                                                                          \
                ##__VA_ARGS__);                                                                                        \
         if (16 == priority)                                                                                           \
            POSAL_ASSERT(0);                                                                                           \
      }                                                                                                                \
   } while (0)

#define MSG(xx_ss_id, xx_ss_mask, xx_fmt) DBG_MSG(0, xx_ss_mask, xx_fmt)
#define MSG_1(xx_ss_id, xx_ss_mask, xx_fmt, xx_arg1) DBG_MSG(0, xx_ss_mask, xx_fmt, xx_arg1)
#define MSG_2(xx_ss_id, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2) DBG_MSG(0, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2)
#define MSG_3(xx_ss_id, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3)                                                 \
   DBG_MSG(0, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3)

#define MSG_4(xx_ss_id, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4)                                        \
   DBG_MSG(0, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4)

#define MSG_5(xx_ss_id, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, xx_arg5)                               \
   DBG_MSG(0, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, xx_arg5)

#define MSG_6(xx_ss_id, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, xx_arg5, xx_arg6)                      \
   DBG_MSG(0, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, xx_arg5, xx_arg6)

#define MSG_7(xx_ss_id, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, xx_arg5, xx_arg6, xx_arg7)             \
   DBG_MSG(0, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, xx_arg5, xx_arg6, xx_arg7)

#define MSG_8(xx_ss_id, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, xx_arg5, xx_arg6, xx_arg7, xx_arg8)    \
   DBG_MSG(0, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, xx_arg5, xx_arg6, xx_arg7, xx_arg8)

#define MSG_9(xx_ss_id,                                                                                                \
              xx_ss_mask,                                                                                              \
              xx_fmt,                                                                                                  \
              xx_arg1,                                                                                                 \
              xx_arg2,                                                                                                 \
              xx_arg3,                                                                                                 \
              xx_arg4,                                                                                                 \
              xx_arg5,                                                                                                 \
              xx_arg6,                                                                                                 \
              xx_arg7,                                                                                                 \
              xx_arg8,                                                                                                 \
              xx_arg9)                                                                                                 \
   DBG_MSG(0, xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, xx_arg5, xx_arg6, xx_arg7, xx_arg8, xx_arg9)

#define MSG_FATAL(x_fmt, a, b, c) MSG_3(MSG_SSID_QDSP6, DBG_FATAL_PRIO, x_fmt, a, b, c)

/* Original macro cut&paste from AudCmnUtil.h, leave it outside SURF for kernel debugging */
/* This macro prints to QURT buffer */
/* Print macro for debugging */
#define PRINT() printf("%s %d\n", __FILENAME__, __LINE__);

/* Utility to get variable argument
 * Slide the variable arguments to get the string '_N' to be appended */
#define AR_VA_NUM_ARGS_IMPL(a, b, c, d, e, f, g, h, i, j, _N, ...) _N
#define AR_VA_NUM_ARGS(...) AR_VA_NUM_ARGS_IMPL(, ##__VA_ARGS__, _9, _8, _7, _6, _5, _4, _3, _2, _1, )
/* Concatenates string x with y*/
#define AR_TOKENPASTE(x, y) x##y

/*Macro parses the text to MSG with _N as suffix, where N = {1, 2, 3, .... ,9} */
#define AR_MSG_x(_N) AR_TOKENPASTE(MSG, _N)

#ifndef MSG_SSID_DFLT
#define MSG_SSID_DFLT MSG_SSID_QDSP6
#endif // MSG_SSID_DFLT

//#ifndef USES_AUDIO_IN_ISLAND
/* This Logging macro supports variable arguments*/
#define AR_MSG(xx_ss_mask, xx_fmt, ...)                                                                                \
   AR_MSG_x(AR_VA_NUM_ARGS(__VA_ARGS__))(MSG_SSID_QDSP6, xx_ss_mask, xx_fmt, ##__VA_ARGS__)

#endif //#ifdef CAPI_STANDALONE
