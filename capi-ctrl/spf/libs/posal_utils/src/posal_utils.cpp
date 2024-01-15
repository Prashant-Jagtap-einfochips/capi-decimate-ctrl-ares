#include "posal_utils.h"

#include <stdio.h>
#include <ctype.h>
#include "capi_test.h"
#include "capi_test.h"

static unsigned int num_mallocs       = 0;
static unsigned int num_frees         = 0;
static unsigned int malloc_fail_count = 0;

static inline void appi_tst_atomic_add(unsigned int *target, unsigned int v)
{
   unsigned int result;

   __asm__ __volatile__("1:     %0 = memw_locked(%2)\n"
                        "       %0 = add(%0, %3)\n"
                        "       memw_locked(%2, p0) = %0\n"
                        "       if !p0 jump 1b\n"
                        : "=&r"(result), "+m"(*target)
                        : "r"(target), "r"(v)
                        : "p0");
}

/*------------------------------------------------------------------------
  Function name: posal_memory_malloc
  Description- Mimics posal memory malloc utility (also checks for
    memory corruption, if any standalone algorithms are doing so)
 * -----------------------------------------------------------------------*/
void *posal_memory_malloc(uint32_t unBytes, POSAL_HEAP_ID heapId)
{
   int *ptr;
   if (malloc_fail_count > 0)
   {
      malloc_fail_count--;
      if (0 == malloc_fail_count)
      {
         AR_MSG(DBG_HIGH_PRIO, "Simulated malloc failure.");
         return NULL;
      }
   }

   // round up to multiple of 4, and add space for guard words (8 bytes) and size (4 bytes)
   unBytes = (unBytes + 8 + 4 + 3) & 0xFFFFFFFCL;

   // Add space for guard words and size
   ptr = (int *)malloc(unBytes);
   if (!ptr)
   {
      AR_MSG(DBG_ERROR_PRIO, "Malloc failed!! Out of memory, %d bytes !!", unBytes);
      return NULL;
   }

   // end guard word
   *(ptr + (unBytes >> 2) - 1) = 0xb4b4b4b4L;

   // beginning guard word
   *ptr++ = 0xa3a3a3a3L;

   // size of allocation
   *ptr++ = unBytes;

   appi_tst_atomic_add(&num_mallocs, 1);

#if POSAL_MALLOC_FREE_TRACER
   AR_MSG(DBG_LOW_PRIO, "POSAL_MALLOC_FREE_TRACER: Mallocs %ld bytes, gets ptr 0x%p", unBytes, ptr);
#endif

   return ptr;
}

/*------------------------------------------------------------------------
  Function name: posal_memory_free
  Description- Mimics posal_memory_free utility
 * -----------------------------------------------------------------------*/
void posal_memory_free(void *ptr)
{

   // check for NULL.
   if (!ptr)
   {
      AR_MSG(DBG_ERROR_PRIO, "Trying to free null pointer!");
      return;
   }

   int *pPtr = (int *)ptr;

   // verify the health of the magic words and clear them
   if (*(pPtr - 2) != (int)0xa3a3a3a3L)
   {
      AR_MSG(DBG_ERROR_PRIO, "Heap corruption!! begining guard word before ptr 0x%p", pPtr);
   }

   // clear the magic word
   else
   {
      *(pPtr - 2) = 0;
   }

   uint32_t unBytes = *(pPtr - 1);
   if (*(pPtr + (unBytes >> 2) - 3) != (int)0xb4b4b4b4L)
   {
      AR_MSG(DBG_ERROR_PRIO, "Heap corruption!! end guard word after ptr 0x%p", pPtr);
   }

   // clear the magic word
   else
   {
      *(pPtr + (unBytes >> 2) - 3) = 0;
   }

   appi_tst_atomic_add(&num_frees, 1);

#if POSAL_MALLOC_FREE_TRACER
   AR_MSG(DBG_LOW_PRIO, "POSAL_MALLOC_FREE_TRACER: frees %ld bytes, from ptr 0x%p", unBytes, pPtr);
#endif

   free(ptr);
}

//
//
//
//
void *posal_memory_aligned_malloc(uint32_t unBytes, uint32_t unAlignSize, POSAL_HEAP_ID heapId)
{

   if (heapId >= POSAL_HEAP_OUT_OF_RANGE)
      return NULL;

   /* only allow alignment to 4 or more bytes.*/
   if (unAlignSize <= 4)
   {
      unAlignSize = 4;
   }

   /* keep out the crazy stuff.*/
   if (unAlignSize > (1 << 30))
   {
      return NULL;
   }

   char *   ptr, *ptr2, *aligned_ptr;
   uint32_t align_mask = ~(unAlignSize - 1);

   /* allocate enough for requested bytes + alignment wasteage + 1 word for storing offset
    * (which will be just before the aligned ptr) */
   ptr = (char *)posal_memory_malloc(unBytes + unAlignSize + sizeof(int), heapId);
   if (ptr == NULL)
      return (NULL);

   /* allocate enough for requested bytes + alignment wasteage + 1 word for storing offset */
   ptr2        = ptr + sizeof(int);
   aligned_ptr = (char *)((uint32_t)(ptr2 - 1) & align_mask) + unAlignSize;

   /* save offset to raw pointer from aligned pointer */
   ptr2           = aligned_ptr - sizeof(int);
   *((int *)ptr2) = (int)(aligned_ptr - ptr);

   return (aligned_ptr);
}

void posal_memory_aligned_free(void *ptr)
{
   // Check for NULL first before using the ptr to calculate offsets
   if (NULL == ptr)
   {
      return;
   }

   uint32_t *pTemp = (uint32_t *)ptr;
   uint32_t *ptr2  = pTemp - 1;

   /* Get the base pointer address */
   pTemp -= *ptr2 / sizeof(uint32_t);

   /* Free the memory */
   posal_memory_free(pTemp);
}

//
//
//
//

uint32_t capi_tst_get_num_mallocs()
{
   return (uint32_t)num_mallocs;
}

uint32_t capi_tst_get_num_frees()
{
   return (uint32_t)num_frees;
}

void capi_tst_set_malloc_failure(int num_mallocs_to_skip)
{
   malloc_fail_count = num_mallocs_to_skip + 1;
}

void capi_tst_clear_malloc_failure(void)
{
   malloc_fail_count = 0;
}
