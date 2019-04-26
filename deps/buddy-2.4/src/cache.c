/*========================================================================
               Copyright (C) 1996-2002 by Jorn Lind-Nielsen
                            All rights reserved

    Permission is hereby granted, without written agreement and without
    license or royalty fees, to use, reproduce, prepare derivative
    works, distribute, and display this software and its documentation
    for any purpose, provided that (1) the above copyright notice and
    the following two paragraphs appear in all copies of the source code
    and (2) redistributions, including without limitation binaries,
    reproduce these notices in the supporting documentation. Substantial
    modifications to this software may be copyrighted by their authors
    and need not follow the licensing terms described here, provided
    that the new terms are clearly indicated in all files where they apply.

    IN NO EVENT SHALL JORN LIND-NIELSEN, OR DISTRIBUTORS OF THIS
    SOFTWARE BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
    INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS
    SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE AUTHORS OR ANY OF THE
    ABOVE PARTIES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    JORN LIND-NIELSEN SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
    BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
    FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS
    ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO
    OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
    MODIFICATIONS.
========================================================================*/

/*************************************************************************
  $Header: /cvsroot/buddy/buddy/src/cache.c,v 1.1.1.1 2004/06/25 13:22:34 haimcohen Exp $
  FILE:  cache.c
  DESCR: Cache class for caching apply/exist etc. results in BDD package
  AUTH:  Jorn Lind
  DATE:  (C) june 1997
*************************************************************************/
#include <stdlib.h>
#include "kernel.h"
#include "cache.h"
#include "prime.h"

/*************************************************************************
*************************************************************************/

int BddCache_init(BddCache *cache, int size)
{
   int n;

   size = bdd_prime_gte(size);
   
   if ((cache->table=NEW(BddCacheData,size)) == NULL)
      return bdd_error(BDD_MEMORY);
   
   for (n=0 ; n<size ; n++)
      cache->table[n].a = -1;
   cache->tablesize = size;
   
   return 0;
}


void BddCache_done(BddCache *cache)
{
   free(cache->table);
   cache->table = NULL;
   cache->tablesize = 0;
}


int BddCache_resize(BddCache *cache, int newsize)
{
   int n;

   free(cache->table);

   newsize = bdd_prime_gte(newsize);
   
   if ((cache->table=NEW(BddCacheData,newsize)) == NULL)
      return bdd_error(BDD_MEMORY);
   
   for (n=0 ; n<newsize ; n++)
      cache->table[n].a = -1;
   cache->tablesize = newsize;
   
   return 0;
}


void BddCache_reset(BddCache *cache)
{
   register int n;
   for (n=0 ; n<cache->tablesize ; n++)
      cache->table[n].a = -1;
}


/* EOF */
