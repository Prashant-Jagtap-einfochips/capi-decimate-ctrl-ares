/**
 * \file spf_begin_pack.h
 * \brief 
 *  	 This file defines pack attributes for different compilers to be used to pack spf API data structures
 * 
 * \copyright
 *    Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*

*/
// clang-format on

/*===========================================================================
NOTE: The @brief description and any detailed descriptions above do not appear 
      in the PDF. 

      The HexagonMM_mainpage.dox file contains all file/group descriptions that
      are in the output PDF generated using Doxygen and Latex. To edit or 
      update any of the file/group text in the PDF, edit the 
      HexagonMM_mainpage.dox file or contact Tech Pubs.

      The above description for this file is part of the adsp_api_pack group 
      description in the HexagonMM_mainpage.dox file. 
===========================================================================*/
 
#if defined( __qdsp6__ )
/* No packing atrributes for Q6 compiler; all structs manually packed */
#elif defined( __GNUC__ )
#elif defined( __arm__ )
  __packed
#elif defined( _MSC_VER )
  #pragma warning( disable:4103 )  /* Another header changing "pack". */
  #pragma pack( push, 1 )
#elif defined( __H2XML__)
#else
  #error "Unsupported compiler."
#endif /* __GNUC__ */
