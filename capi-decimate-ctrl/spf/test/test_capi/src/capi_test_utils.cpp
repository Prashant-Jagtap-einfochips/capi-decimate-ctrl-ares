/*===========================================================================

                  C A P I V 2  T E S T   U T I L I T I E S

  Common definitions, types and functions for APPI Examples */

/* =========================================================================
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ========================================================================== */

/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.


when            who      what, where, why
--------        ---      ----------------------------------------------------------

===========================================================================*/

#include "capi_test_utils.h"

#include <stdio.h>
#include <ctype.h>
#include "ar_msg.h"
#include "posal.h"
#include "spf_list_utils.h"
#include "stringl.h"

/*------------------------------------------------------------------------
  Function name: usage
  Description- Prints out usage instructions
  for the executable
 * -----------------------------------------------------------------------*/
void usage(FILE *fp, char *prog_name)
{
   fprintf(fp, "\
         usage:\n\
         %s  <required_args>  [optional_args]\n\
         \n\
         required_args:\n\
         -i <filename>  Input filename.\n\
         -o <filename>  Output filename.\n\
         -c <filename>  Config filename.\n\
		 \n\
         Example: \n\
         For Q6 platforms qdsp6-sim <exename> -- -i in.raw -o out.raw -c test.cfg \n\
         \n", prog_name);
}

/*------------------------------------------------------------------------
  Function name: get_args
  Description- Read input arguments
 * -----------------------------------------------------------------------*/
void get_eargs(int32_t argc, char *argv[], args_t *input_args, uint32_t *num_input_port, uint32_t *num_output_port)
{
   int16_t input_option;

   if (argc < 6)
   {
      usage(stdout, argv[0]);
      fprintf(stderr, "%s: argc < 6.\n", argv[0]);
      exit(0);
   }
   else
   {
      for (uint32_t i = 0; i < CAPI_MAX_INPUT_PORTS; i++)
      {
         input_args->input_filename[i] = NULL;
      }
      for (uint32_t i = 0; i < CAPI_MAX_OUTPUT_PORTS; i++)
      {
         input_args->output_filename[i] = NULL;
      }

      input_args->config_filename = NULL;

      while ((input_option = getopt(argc, argv, "i:o:c")) != EOF)
      {
         switch (input_option)
         {
            case 'i':
            {

               input_args->input_filename[*num_input_port] = argv[optind - 1];

               *num_input_port += 1;
               break;
            }
            case 'o':
            {
               input_args->output_filename[*num_output_port] = argv[optind - 1];
               *num_output_port += 1;
               break;
            }
            case 'c':
               input_args->config_filename = argv[optind];
               break;
         }
      }
   }
}

/*------------------------------------------------------------------------
  Function name: GetWord
  Description- Gets one word from config file
 * -----------------------------------------------------------------------*/
capi_err_t GetWord(FILE *fCfg, char word[])
{
   if (feof(fCfg))
   {
      AR_MSG(DBG_ERROR_PRIO, "CAPI TEST: End of file reached.");
      return CAPI_EFAILED;
   }

   char c;

   do
   {
      c = fgetc(fCfg);
      if ('#' == c)
      {
         // Go to next line
         char tmp[1024];
         fgets(tmp, 1024, fCfg);
      }
   } while ((isspace(c) || ('#' == c)) && !feof(fCfg));

   if (feof(fCfg))
   {
      AR_MSG(DBG_HIGH_PRIO, "CAPI TEST: End of file reached.");
      return CAPI_EFAILED;
   }

   // Read the word now
   fseek(fCfg, -1L, SEEK_CUR); // Since we read one character into c.
   fscanf(fCfg, "%s", word);

   return CAPI_EOK;
}

/*------------------------------------------------------------------------
  Function name: GetUIntParameter
  Description- Gets value associated with parameter
 * -----------------------------------------------------------------------*/
capi_err_t GetUIntParameter(FILE *fCfg, const char parameterName[], uint32_t *pValues)
{
   int32_t    value = 0; // init
   capi_err_t result;

   result = GetIntParameter(fCfg, parameterName, &value);
   if (CAPI_EOK != result)
   {
      return result;
   }

   if (value < 0)
   {
      value = 0;
   }

   *pValues = (uint32_t)value;

   return CAPI_EOK;
}

/*------------------------------------------------------------------------
  Function name: GetIntParameter
  Description- Gets value associated with parameter
 * -----------------------------------------------------------------------*/
capi_err_t GetIntParameter(FILE *fCfg, const char parameterName[], int32_t *pValue)
{
   capi_err_t result;
   char       word[wordSize];
   fpos_t     position;

   fgetpos(fCfg, &position);
   result = GetWord(fCfg, word);
   if (CAPI_EOK != result)
   {
      return result;
   }

   if (0 != strncmp(word, parameterName, wordSize))
   {
      //      MSG_1(MSG_SSID_QDSP6, DBG_ERROR_PRIO,"APPI TEST: Did not find the %s parameter.", parameterName);
      //      fsetpos(fCfg, &position);
      return CAPI_EFAILED;
   }
   result = GetWord(fCfg, word);
   if (CAPI_EOK != result)
   {
      //      MSG_1(MSG_SSID_QDSP6, DBG_ERROR_PRIO,"APPI TEST: Failed to read %s parameter value number.",
      //      parameterName);
      return result;
   }

   int32_t value;
   value = strtol(word, NULL, 0);

   *pValue = value;

   return CAPI_EOK;
}

capi_err_t ReadBufferContents(FILE *fCfg, const uint32_t payloadSize, uint8_t *pPayload)
{
   uint32_t i;

   // Read the payload contents
   for (i = 0; i < payloadSize; i++)
   {
      capi_err_t result;
      char       word[wordSize];
      result = GetWord(fCfg, word);
      if (CAPI_EOK != result)
      {
         //         MSG(MSG_SSID_QDSP6, DBG_ERROR_PRIO, "APPI TEST: Failed to read
         //         payload contents.");
         return result;
      }

      int value;
      sscanf(word, "%x", &value);

      pPayload[i] = (uint8_t)(value & 0xFF);
   }

   return CAPI_EOK;
}

capi_err_t ReadChannelMapping(FILE *fCfg, const uint32_t num_channels, uint16_t channel_mapping[])
{
   uint32_t i;

   for (i = 0; i < num_channels; i++)
   {
      capi_err_t result;
      char       word[wordSize];
      result = GetWord(fCfg, word);
      if (CAPI_EOK != result)
      {
         //         MSG(MSG_SSID_QDSP6, DBG_ERROR_PRIO, "APPI TEST: Failed to read channel mapping.");
         return result;
      }

      int value;
      sscanf(word, "%x", &value);

      channel_mapping[i] = (uint16_t)(value & 0xFFFF);
   }

   return CAPI_EOK;
}

capi_err_t ReadIdIndexMapping(FILE *fCfg, const uint32_t num_ports, intf_extn_data_port_id_idx_map_t id_idx_arr[])
{
   uint32_t i;

   for (i = 0; i < num_ports; i++)
   {
      capi_err_t result;
      char       word[wordSize];
      result = GetWord(fCfg, word);
      if (CAPI_EOK != result)
      {
         //         MSG(MSG_SSID_QDSP6, DBG_ERROR_PRIO, "APPI TEST: Failed to read port mapping.");
         return result;
      }

      int value;
      sscanf(word, "%x", &value);

      id_idx_arr[i].port_id = value;

      char word2[wordSize];
      result = GetWord(fCfg, word2);
      if (CAPI_EOK != result)
      {
         //         MSG(MSG_SSID_QDSP6, DBG_ERROR_PRIO, "APPI TEST: Failed to read port mapping.");
         return result;
      }

      int value2;
      sscanf(word2, "%x", &value2);
      id_idx_arr[i].port_index = i;
      id_idx_arr[i].port_index = value2;
   }

   return CAPI_EOK;
}

capi_err_t ReadUint32List(FILE *fCfg, const uint32_t num_ports, uint32_t ids[])
{
   uint32_t i;

   for (i = 0; i < num_ports; i++)
   {
      capi_err_t result;
      char       word[wordSize];
      result = GetWord(fCfg, word);
      if (CAPI_EOK != result)
      {
         //         MSG(MSG_SSID_QDSP6, DBG_ERROR_PRIO, "APPI TEST: Failed to read port mapping.");
         return result;
      }

      int value;
      sscanf(word, "%x", &value);

      ids[i] = value;
   }

   return CAPI_EOK;
}

// uint32_t capi_tst_get_num_mallocs()
//{
//   return (uint32_t)num_mallocs;
//}
//
// uint32_t capi_tst_get_num_frees()
//{
//   return (uint32_t)num_frees;
//}
//
// void capi_tst_set_malloc_failure(int num_mallocs_to_skip)
//{
//   malloc_fail_count = num_mallocs_to_skip + 1;
//}
//
// void capi_tst_clear_malloc_failure(void)
//{
//   malloc_fail_count = 0;
//}
