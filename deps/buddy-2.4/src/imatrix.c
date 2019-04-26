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
  FILE:  imatrix.cc
  DESCR: Interaction matrix
  AUTH:  Jorn Lind
  DATE:  (C) february 2000
*************************************************************************/
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "kernel.h"
#include "imatrix.h"

/*************************************************************************
*************************************************************************/

imatrix* imatrixNew(int size)
{
   imatrix *mtx = NEW(imatrix,1);
   int n,m;
   
   if (!mtx)
      return NULL;

   if ((mtx->rows=NEW(char*,size)) == NULL)
   {
      free(mtx);
      return NULL;
   }

   for (n=0 ; n<size ; n++)
   {
      if ((mtx->rows[n]=NEW(char,size/8+1)) == NULL)
      {
	 for (m=0 ; m<n ; m++)
	    free(mtx->rows[m]);
	 free(mtx);
	 return NULL;
      }

      memset(mtx->rows[n], 0, size/8+1);
   }

   mtx->size = size;

   return mtx;
}


void imatrixDelete(imatrix *mtx)
{
   int n;

   for (n=0 ; n<mtx->size ; n++)
      free(mtx->rows[n]);
   free(mtx->rows);
   free(mtx);
}


/*======================================================================*/

void imatrixFPrint(imatrix *mtx, FILE *ofile)
{
   int x,y;

   fprintf(ofile, "    ");
   for (x=0 ; x<mtx->size ; x++)
      fprintf(ofile, "%c", x < 26 ? (x+'a') : (x-26)+'A');
   fprintf(ofile, "\n");
   
   for (y=0 ; y<mtx->size ; y++)
   {
      fprintf(ofile, "%2d %c", y, y < 26 ? (y+'a') : (y-26)+'A');
      for (x=0 ; x<mtx->size ; x++)
	 fprintf(ofile, "%c", imatrixDepends(mtx,y,x) ? 'x' : ' ');
      fprintf(ofile, "\n");
   }
}


void imatrixPrint(imatrix *mtx)
{
   imatrixFPrint(mtx, stdout);
}


void imatrixSet(imatrix *mtx, int a, int b)
{
   mtx->rows[a][b/8] |= 1<<(b%8);
}


void imatrixClr(imatrix *mtx, int a, int b)
{
   mtx->rows[a][b/8] &= ~(1<<(b%8));
}


int imatrixDepends(imatrix *mtx, int a, int b)
{
   return mtx->rows[a][b/8] & (1<<(b%8));
}


/*======================================================================*/

#if 0
void main(void)
{
   imatrix *m = imatrixNew(16);

   imatrixSet(m,0,2);
   imatrixSet(m,8,8);
   imatrixSet(m,15,15);
   
   imatrixPrint(m);
}
#endif

/* EOF */
