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

#include <string>
#include "hal_bdd.h"
#include "hal_bvec.h"

using namespace std;


#define ERROR(msg) fail(msg, __FILE__, __LINE__)

static void fail(const string msg, const char* file, int lineNum)
{
  cout << "Error in " << file << "(" << lineNum << "): " << msg << endl;
  exit(1);
}


static void testSupport(void)
{
  bdd even = bdd_ithvar(0) | bdd_ithvar(2) | bdd_ithvar(4);
  bdd odd  = bdd_ithvar(1) | bdd_ithvar(3) | bdd_ithvar(5);

  cout << "Testing support\n";
  
  bdd s1 = bdd_support(even);
  bdd s2 = bdd_support(odd);

  if (s1 != (bdd_ithvar(0) & bdd_ithvar(2) & bdd_ithvar(4)))
    ERROR("Support of 'even' failed\n");
  if (s2 != (bdd_ithvar(1) & bdd_ithvar(3) & bdd_ithvar(5)))
    ERROR("Support of 'odd' failed\n");

  /* Try many time in order check that the internal support ID
   * is set correctly */
  for (int n=0 ; n<500 ; ++n)
  {
    s1 = bdd_support(even);
    s2 = bdd_support(odd);

    if (s1 != (bdd_ithvar(0) & bdd_ithvar(2) & bdd_ithvar(4)))
      ERROR("Support of 'even' failed");
    if (s2 != (bdd_ithvar(1) & bdd_ithvar(3) & bdd_ithvar(5)))
      ERROR("Support of 'odd' failed");
  }
}


void testBvecIte()
{
  cout << "Testing ITE for vector\n";
  
  bdd  a = bdd_ithvar(0);
  bvec b = bvec_var(3, 1, 2);
  bvec c = bvec_var(3, 2, 2);

  bvec res = bvec_ite(a,b,c);

  bdd r0 = bdd_ite( bdd_ithvar(0), bdd_ithvar(1), bdd_ithvar(2) );
  bdd r1 = bdd_ite( bdd_ithvar(0), bdd_ithvar(3), bdd_ithvar(4) );
  bdd r2 = bdd_ite( bdd_ithvar(0), bdd_ithvar(5), bdd_ithvar(6) );

  if (res[0] != r0)
    ERROR("Bit 0 failed.");
  if (res[1] != r1)
    ERROR("Bit 1 failed.");
  if (res[2] != r2)
    ERROR("Bit 2 failed.");
}


int main(int ac, char** av)
{
  bdd_init(1000,1000);

  bdd_setvarnum(10);

  testSupport();
  testBvecIte();

  bdd_done();
  return 0;
}
