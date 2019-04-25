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
  $Header: /cvsroot/buddy/buddy/src/fdd.c,v 1.1.1.1 2004/06/25 13:22:40 haimcohen Exp $
  FILE:  fdd.c
  DESCR: Finite domain extensions to BDD package
  AUTH:  Jorn Lind
  DATE:  (C) june 1997

  NOTE: If V1,...,Vn is BDD vars for a FDD, then Vn is the Least Sign. Bit
*************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "kernel.h"
#include "hal_fdd.h"


static void fdd_printset_rec(FILE *, int, int *);

/*======================================================================*/
/* NOTE: ALL FDD operations works with LSB in top of the variable order */
/*       and in index zero of the domain tables                         */
/*======================================================================*/

typedef struct s_Domain
{
   int realsize;   /* The specified domain (0...N-1) */
   int binsize;    /* The number of BDD variables representing the domain */
   int *ivar;      /* Variable indeces for the variable set */
   BDD var;        /* The BDD variable set */
} Domain;


static void Domain_allocate(Domain*, int);
static void Domain_done(Domain*);

static int    firstbddvar;
static int    fdvaralloc;         /* Number of allocated domains */
static int    fdvarnum;           /* Number of defined domains */
static Domain *domain;            /* Table of domain sizes */

static bddfilehandler filehandler;

/*************************************************************************
  Domain definition
*************************************************************************/

void bdd_fdd_init(void)
{
   domain = NULL;
   fdvarnum = fdvaralloc = 0;
   firstbddvar = 0;
}


void bdd_fdd_done(void)
{
   int n;
   
   if (domain != NULL)
   {
      for (n=0 ; n<fdvarnum ; n++)
	 Domain_done(&domain[n]);
      free(domain);
   }

   domain = NULL;
}


/*
NAME    {* fdd\_extdomain *}
SECTION {* fdd *}
SHORT   {* adds another set of finite domain blocks *}
PROTO   {* int fdd_extdomain(int *dom, int num) *}
DESCR   {* Extends the set of finite domain blocks with the {\tt num}
           domains in
           {\tt dom}. Each entry in {\tt dom} defines the size of a new
	   finite domain which later on can be used for finite state machine
	   traversal and other operations on finte domains. Each domain
	   allocates $\log_2(|dom[i]|)$ BDD variables to be used later.
	   The ordering is interleaved for the domains defined in each
	   call to {\tt bdd\_extdomain}. This means that assuming domain
	   $D_0$ needs 2 BDD variables $x_1$ and $x_2$, and another domain
	   $D_1$ needs 4 BDD variables $y_1,y_2,y_3$ and $y_4$, then the
	   order will be $x_1,y_1,x_2,y_2,y_3,y_4$. The index of the first
	   domain in {\tt dom} is returned. The index of the other domains
	   are offset from this index with the same offset as in {\tt dom}.

	   The BDD variables needed to encode the domain are created for the
	   purpose and do not interfere with the BDD variables already in
	   use. *}
RETURN  {* The index of the first domain or a negative error code. *}
ALSO    {* fdd\_ithvar, fdd\_equals, fdd\_overlapdomain *}
*/
int fdd_extdomain(int *dom, int num)
{
   int offset = fdvarnum;
   int binoffset;
   int extravars = 0;
   int n, bn, more;

   if (!bddrunning)
      return bdd_error(BDD_RUNNING);
   
      /* Build domain table */
   if (domain == NULL)  /* First time */
   {
      fdvaralloc = num;
      if ((domain=(Domain*)malloc(sizeof(Domain)*num)) == NULL)
	 return bdd_error(BDD_MEMORY);
   }
   else  /* Allocated before */
   {
      if (fdvarnum + num > fdvaralloc)
      {
         fdvaralloc += (num > fdvaralloc) ? num : fdvaralloc;
	 
	 domain = (Domain*)realloc(domain, sizeof(Domain)*fdvaralloc);
	 if (domain == NULL)
	    return bdd_error(BDD_MEMORY);
      }
   }

      /* Create bdd variable tables */
   for (n=0 ; n<num ; n++)
   {
      Domain_allocate(&domain[n+fdvarnum], dom[n]);
      extravars += domain[n+fdvarnum].binsize;
   }

   binoffset = firstbddvar;
   if (firstbddvar + extravars > bddvarnum)
      bdd_setvarnum(firstbddvar + extravars);

      /* Set correct variable sequence (interleaved) */
   for (bn=0,more=1 ; more ; bn++)
   {
      more = 0;

      for (n=0 ; n<num ; n++)
	 if (bn < domain[n+fdvarnum].binsize)
	 {
	    more = 1;
	    domain[n+fdvarnum].ivar[bn] = binoffset++;
	 }
   }

   for (n=0 ; n<num ; n++)
   {
      domain[n+fdvarnum].var = bdd_makeset(domain[n+fdvarnum].ivar,
					   domain[n+fdvarnum].binsize);
      bdd_addref(domain[n+fdvarnum].var);
   }

   fdvarnum += num;
   firstbddvar += extravars;
   
   return offset;
}


/*
NAME    {* fdd\_overlapdomain *}
SECTION {* fdd *}
SHORT   {* combine two FDD blocks into one *}
PROTO   {* int fdd_overlapdomain(int v1, int v2) *}
DESCR   {* This function takes two FDD blocks and merges them into a new one,
           such that the new one is encoded using both sets of BDD variables.
	   If {\tt v1} is encoded using the BDD variables $a_1, \ldots,
	   a_n$ and has a domain of $[0,N_1]$, and {\tt v2} is encoded using
	   $b_1, \ldots, b_n$ and has a domain of $[0,N_2]$, then the result
	   will be encoded using the BDD variables $a_1, \ldots, a_n, b_1,
	   \ldots, b_n$ and have the domain $[0,N_1*N_2]$. The use of this
	   function may result in some strange output from
	   {\tt fdd\_printset}. *}
RETURN  {* The index of the finite domain block *}
ALSO    {* fdd\_extdomain *}
*/
int fdd_overlapdomain(int v1, int v2)
{
   Domain *d;
   int n;
   
   if (!bddrunning)
      return bdd_error(BDD_RUNNING);
   
   if (v1 < 0  ||  v1 >= fdvarnum  ||  v2 < 0  ||  v2 >= fdvarnum)
      return bdd_error(BDD_VAR);

   if (fdvarnum + 1 > fdvaralloc)
   {
      fdvaralloc += fdvaralloc;
      
      domain = (Domain*)realloc(domain, sizeof(Domain)*fdvaralloc);
      if (domain == NULL)
	 return bdd_error(BDD_MEMORY);
   }

   d = &domain[fdvarnum];
   d->realsize = domain[v1].realsize * domain[v2].realsize;
   d->binsize = domain[v1].binsize + domain[v2].binsize;
   d->ivar = (int *)malloc(sizeof(int)*d->binsize);

   for (n=0 ; n<domain[v1].binsize ; n++)
      d->ivar[n] = domain[v1].ivar[n];
   for (n=0 ; n<domain[v2].binsize ; n++)
      d->ivar[domain[v1].binsize+n] = domain[v2].ivar[n];
	 
   d->var = bdd_makeset(d->ivar, d->binsize);
   bdd_addref(d->var);
   
   return fdvarnum++;
}


/*
NAME    {* fdd\_clearall *}
SECTION {* fdd *}
SHORT   {* clear all allocated FDD blocks *}
PROTO   {* void fdd_clearall(void) *}
DESCR   {* Removes all defined finite domain blocks defined by
           {\tt fdd\_extdomain()} and {\tt fdd\_overlapdomain()} *}
*/
void fdd_clearall(void)
{
   bdd_fdd_done();
   bdd_fdd_init();
}


/*************************************************************************
  FDD helpers
*************************************************************************/

/*
NAME    {* fdd\_domainnum *}
SECTION {* fdd *}
SHORT   {* number of defined finite domain blocks *}
PROTO   {* int fdd_domainnum(void) *}
DESCR   {* Returns the number of finite domain blocks define by calls to
           {\tt bdd\_extdomain}. *}
RETURN  {* The number of defined finite domain blocks
           or a negative error code *}
ALSO    {* fdd\_domainsize, fdd\_extdomain *}
*/
int fdd_domainnum(void)
{
   if (!bddrunning)
      return bdd_error(BDD_RUNNING);
   
   return fdvarnum;
}


/*
NAME    {* fdd\_domainsize *}
SECTION {* fdd *}
SHORT   {* real size of a finite domain block *}
PROTO   {* int fdd_domainsize(int var) *}
DESCR   {* Returns the size of the domain for the finite domain
           block {\tt var}. *}
RETURN  {* The size or a negative error code *}
ALSO    {* fdd\_domainnum *}
*/
int fdd_domainsize(int v)
{
   if (!bddrunning)
      return bdd_error(BDD_RUNNING);
   
   if (v < 0  ||  v >= fdvarnum)
      return bdd_error(BDD_VAR);
   return domain[v].realsize;
}


/*
NAME    {* fdd\_varnum *}
SECTION {* fdd *}
SHORT   {* binary size of a finite domain block *}
PROTO   {* int fdd_varnum(int var) *}
DESCR   {* Returns the number of BDD variables used for the finite domain
           block {\tt var}. *}
RETURN  {* The number of variables or a negative error code *}
ALSO    {* fdd\_vars *}
*/
int fdd_varnum(int v)
{
   if (!bddrunning)
      return bdd_error(BDD_RUNNING);
   
   if (v >= fdvarnum  ||  v < 0)
      return bdd_error(BDD_VAR);
   return domain[v].binsize;
}


/*
NAME    {* fdd\_vars *}
SECTION {* fdd *}
SHORT   {* all BDD variables associated with a finite domain block *}
PROTO   {* int *fdd_vars(int var) *}
DESCR   {* Returns an integer array containing the BDD variables used to
           define the finite domain block {\tt var}. The size of the array
	   is the number of variables used to define the finite domain block.
	   The array will have the Least Significant Bit at pos 0. The
	   array must {\em not} be deallocated. *}
RETURN  {* Integer array contaning the variable numbers or NULL if
           {\tt v} is an unknown block. *}
ALSO    {* fdd\_varnum *}
*/
int *fdd_vars(int v)
{
   if (!bddrunning)
   {
      bdd_error(BDD_RUNNING);
      return NULL;
   }
   
   if (v >= fdvarnum  ||  v < 0)
   {
      bdd_error(BDD_VAR);
      return NULL;
   }

   return domain[v].ivar;
}



/*************************************************************************
  FDD primitives
*************************************************************************/

/*
NAME    {* fdd\_ithvar *}
SECTION {* fdd *}
SHORT   {* the BDD for the i'th FDD set to a specific value *}
PROTO   {* BDD fdd_ithvar(int var, int val) *}
DESCR   {* Returns the BDD that defines the value {\tt val} for the
           finite domain block {\tt var}. The encoding places the
	   Least Significant Bit at the top of the BDD tree
	   (which means they will have the lowest variable index).
	   The returned BDD will be $V_0 \conj V_1 \conj \ldots
	   \conj V_N$ where each $V_i$ will be in positive or negative form
	   depending on the value of {\tt val}. *}
RETURN  {* The correct BDD or the constant false BDD on error. *}
ALSO    {* fdd\_ithset *}
*/
BDD fdd_ithvar(int var, int val)
{
   int n;
   int v=1, tmp;
   
   if (!bddrunning)
   {
      bdd_error(BDD_RUNNING);
      return bddfalse;
   }
   
   if (var < 0  ||  var >= fdvarnum)
   {
      bdd_error(BDD_VAR);
      return bddfalse;
   }

   if (val < 0  ||  val >= domain[var].realsize)
   {
      bdd_error(BDD_RANGE);
      return bddfalse;
   }

   for (n=0 ; n<domain[var].binsize ; n++)
   {
      bdd_addref(v);
      
      if (val & 0x1)
	 tmp = bdd_apply(bdd_ithvar(domain[var].ivar[n]), v, bddop_and);
      else
	 tmp = bdd_apply(bdd_nithvar(domain[var].ivar[n]), v, bddop_and);

      bdd_delref(v);
      v = tmp;
      val >>= 1;
   }

   return v;
}


/*
NAME    {* fdd\_scanvar *}
SECTION {* fdd *}
SHORT   {* Finds one satisfying value of a FDD variable *}
PROTO   {* int fdd_scanvar(BDD r, int var) *}
DESCR   {* Finds one satisfying assignment of the FDD variable {\tt var} in the
           BDD {\tt r} and returns this value. *}
RETURN  {* The value of a satisfying assignment of {\tt var}. If {\tt r} is
           the trivially false BDD, then a negative value is returned. *}
ALSO    {* fdd\_scanallvar *}
*/
int fdd_scanvar(BDD r, int var)
{
   int *allvar;
   int res;

   CHECK(r);
   if (r == bddfalse)
      return -1;
   if (var < 0  ||  var >= fdvarnum)
      return bdd_error(BDD_VAR);

   allvar = fdd_scanallvar(r);
   res = allvar[var];
   free(allvar);

   return res;
}


/*
NAME    {* fdd\_scanallvar *}
SECTION {* fdd *}
SHORT   {* Finds one satisfying value of all FDD variables *}
PROTO   {* int* fdd_scanallvar(BDD r) *}
DESCR   {* Finds one satisfying assignment in {\tt r} of all the defined
           FDD variables. Each value is stored in an array which is
	   returned. The size of this array is exactly the number of
	   FDD variables defined. It is the user's responsibility to
	   free this array using {\tt free()}. *}
RETURN  {* An array with all satisfying values. If {\tt r} is the trivially
           false BDD, then NULL is returned. *}
ALSO    {* fdd\_scanvar *}
*/
int* fdd_scanallvar(BDD r)
{
   int n;
   char *store;
   int *res;
   BDD p = r;
   
   CHECKa(r,NULL);
   if (r == bddfalse)
      return NULL;
   
   store = NEW(char,bddvarnum);
   for (n=0 ; n<bddvarnum ; n++)
      store[n] = 0;

   while (!ISCONST(p))
   {
      if (!ISZERO(LOW(p)))
      {
	 store[bddlevel2var[LEVEL(p)]] = 0;
	 p = LOW(p);
      }
      else
      {
	 store[bddlevel2var[LEVEL(p)]] = 1;
	 p = HIGH(p);
      }
   }

   res = NEW(int, fdvarnum);

   for (n=0 ; n<fdvarnum ; n++)
   {
      int m;
      int val=0;
      
      for (m=domain[n].binsize-1 ; m>=0 ; m--)
	 if ( store[domain[n].ivar[m]] )
	    val = val*2 + 1;
         else
	    val = val*2;
      
      res[n] = val;
   }
   
   free(store);
   
   return res;
}
   
/*
NAME    {* fdd\_ithset *}
SECTION {* fdd *}
SHORT   {* the variable set for the i'th finite domain block *}
PROTO   {* BDD fdd_ithset(int var) *}
DESCR   {* Returns the variable set that contains the variables used to
           define the finite domain block {\tt var}. *}
RETURN  {* The variable set or the constant false BDD on error. *}
ALSO    {* fdd\_ithvar *}
*/
BDD fdd_ithset(int var)
{
   if (!bddrunning)
   {
      bdd_error(BDD_RUNNING);
      return bddfalse;
   }
   
   if (var < 0  ||  var >= fdvarnum)
   {
      bdd_error(BDD_VAR);
      return bddfalse;
   }

   return domain[var].var;
}

/*
NAME    {* fdd\_domain *}
SECTION {* fdd *}
SHORT   {* BDD encoding of the domain of a FDD variable *}
PROTO   {* BDD fdd_domain(int var) *}
DESCR   {* Returns what corresponds to a disjunction of all possible
           values of the variable  {\tt var}.
	   This is more efficient than doing
	   {\tt fdd\_ithvar(var,0) OR fdd\_ithvar(var,1) ...} explicitely
	   for all values in the domain of {\tt var}. *}
RETURN  {* The encoding of the domain*}
*/
BDD fdd_domain(int var)
{
   int n,val;
   Domain *dom;
   BDD d;
      
   if (!bddrunning)
   {
      bdd_error(BDD_RUNNING);
      return bddfalse;
   }
   
   if (var < 0  ||  var >= fdvarnum)
   {
      bdd_error(BDD_VAR);
      return bddfalse;
   }

      /* Encode V<=X-1. V is the variables in 'var' and X is the domain size */
   
   dom = &domain[var];
   val = dom->realsize-1;
   d = bddtrue;
   
   for (n=0 ; n<dom->binsize ; n++)
   {
      BDD tmp;
      
      if (val & 0x1)
	 tmp = bdd_apply( bdd_nithvar(dom->ivar[n]), d, bddop_or );
      else
	 tmp = bdd_apply( bdd_nithvar(dom->ivar[n]), d, bddop_and );

      val >>= 1;

      bdd_addref(tmp);
      bdd_delref(d);
      d = tmp;
   }

   return d;
}


/*
NAME    {* fdd\_equals *}
SECTION {* fdd *}
SHORT   {* returns a BDD setting two FD. blocks equal *}
PROTO   {* BDD fdd_equals(int f, int g) *}
DESCR   {* Builds a BDD which is true for all the possible assignments to
           the variable blocks {\tt f} and {\tt g} that makes the blocks
	   equal. This is more or less just a shorthand for calling
	   {\tt fdd\_equ()}. *}
RETURN  {* The correct BDD or the constant false on errors. *}
*/
BDD fdd_equals(int left, int right)
{
   BDD e = bddtrue, tmp1, tmp2;
   int n;
   
   if (!bddrunning)
   {
      bdd_error(BDD_RUNNING);
      return bddfalse;
   }
   
   if (left < 0  ||  left >= fdvarnum  ||  right < 0  ||  right >= fdvarnum)
   {
      bdd_error(BDD_VAR);
      return bddfalse;
   }
   if (domain[left].realsize != domain[right].realsize)
   {
      bdd_error(BDD_RANGE);
      return bddfalse;
   }
   
   for (n=0 ; n<domain[left].binsize ; n++)
   {
      tmp1 = bdd_addref( bdd_apply(bdd_ithvar(domain[left].ivar[n]),
				   bdd_ithvar(domain[right].ivar[n]),
				   bddop_biimp) );
      
      tmp2 = bdd_addref( bdd_apply(e, tmp1, bddop_and) );
      bdd_delref(tmp1);
      bdd_delref(e);
      e = tmp2;
   }

   bdd_delref(e);
   return e;
}


/*************************************************************************
  File IO
*************************************************************************/

/*
NAME    {* fdd\_file\_hook *}
SECTION {* fdd *}
SHORT   {* Specifies a printing callback handler *}
PROTO   {* bddfilehandler fdd_file_hook(bddfilehandler handler) *}
DESCR   {* A printing callback handler for use with FDDs is used to
           convert the FDD integer identifier into something readable by the
	   end user. Typically the handler will print a string name
	   instead of the identifier. A handler could look like this:
	   \begin{verbatim}
void printhandler(FILE *o, int var)
{
   extern char **names;
   fprintf(o, "%s", names[var]);
}
\end{verbatim}

           \noindent
           The handler can then be passed to BuDDy like this:
	   {\tt fdd\_file\_hook(printhandler)}.

	   No default handler is supplied. The argument {\tt handler} may be
	   NULL if no handler is needed. *}
RETURN  {* The old handler *}
ALSO    {* fdd\_printset, bdd\_file\_hook *}
*/
bddfilehandler fdd_file_hook(bddfilehandler h)
{
   bddfilehandler old = filehandler;
   filehandler = h;
   return old;
}

/*
NAME    {* fdd\_printset *}
EXTRA   {* fdd\_fprintset *}
SECTION {* fdd *}
SHORT   {* prints a BDD for a finite domain block *}
PROTO   {* void fdd_printset(BDD r)
void fdd_fprintset(FILE *ofile, BDD f) *}
DESCR   {* Prints the BDD {\tt f} using a set notation as in
           {\tt bdd\_printset} but with the index of the finite domain blocks
	   included instead of the BDD variables. It is possible to specify
	   a printing callback function with {\tt fdd\_file\_hook} or
	   {\tt fdd\_strm\_hook} which can be used to print the FDD
	   identifier in a readable form. *}
ALSO    {* bdd\_printset, fdd\_file\_hook, fdd\_strm\_hook *}
*/
void fdd_printset(BDD r)
{
   CHECKn(r);
   fdd_fprintset(stdout, r);
}


void fdd_fprintset(FILE *ofile, BDD r)
{
   int *set;
   
   if (!bddrunning)
   {
      bdd_error(BDD_RUNNING);
      return;
   }
   
   if (r < 2)
   {
      fprintf(ofile, "%s", r == 0 ? "F" : "T");
      return;
   }

   set = (int *)malloc(sizeof(int)*bddvarnum);
   if (set == NULL)
   {
      bdd_error(BDD_MEMORY);
      return;
   }
   
   memset(set, 0, sizeof(int) * bddvarnum);
   fdd_printset_rec(ofile, r, set);
   free(set);
}


static void fdd_printset_rec(FILE *ofile, int r, int *set)
{
   int n,m,i;
   int used = 0;
   int *var;
   int *binval;
   int ok, first;
   
   if (r == 0)
      return;
   else
   if (r == 1)
   {
      fprintf(ofile, "<");
      first=1;

      for (n=0 ; n<fdvarnum ; n++)
      {
	 int firstval=1;
	 used = 0;

	 for (m=0 ; m<domain[n].binsize ; m++)
	    if (set[domain[n].ivar[m]] != 0)
	       used = 1;
	 
	 if (used)
	 {
	    if (!first)
	       fprintf(ofile, ", ");
	    first = 0;
	    if (filehandler)
	       filehandler(ofile, n);
	    else
	       fprintf(ofile, "%d", n);
	    printf(":");

	    var = domain[n].ivar;
	    
	    for (m=0 ; m<(1<<domain[n].binsize) ; m++)
	    {
	       binval = fdddec2bin(n, m);
	       ok=1;
	       
	       for (i=0 ; i<domain[n].binsize && ok ; i++)
		  if (set[var[i]] == 1  &&  binval[i] != 0)
		     ok = 0;
		  else
		  if (set[var[i]] == 2  &&  binval[i] != 1)
		     ok = 0;

	       if (ok)
	       {
		  if (firstval)
		     fprintf(ofile, "%d", m);
		  else
		     fprintf(ofile, "/%d", m);
		  firstval = 0;
	       }

	       free(binval);
	    }
	 }
      }

      fprintf(ofile, ">");
   }
   else
   {
      set[bddlevel2var[LEVEL(r)]] = 1;
      fdd_printset_rec(ofile, LOW(r), set);
      
      set[bddlevel2var[LEVEL(r)]] = 2;
      fdd_printset_rec(ofile, HIGH(r), set);
      
      set[bddlevel2var[LEVEL(r)]] = 0;
   }
}


/*======================================================================*/

/*
NAME    {* fdd\_scanset *}
SECTION {* fdd *}
SHORT   {* scans a variable set *}
PROTO   {* int fdd_scanset(BDD r, int **varset, int *varnum) *}
DESCR   {* Scans the BDD {\tt r} to find all occurences of FDD variables
           and then stores these in {\tt varset}. {\tt varset} will be set
	   to point to an array of size {\tt varnum} which will contain
	   the indices of the found FDD variables. It is the users
	   responsibility to free {\tt varset} after use. *}
RETURN  {* Zero on success or a negative error code on error. *}
ALSO    {* fdd\_makeset *}
*/
int fdd_scanset(BDD r, int **varset, int *varnum)
{
   int *fv, fn;
   int num,n,m,i;
      
   if (!bddrunning)
      return bdd_error(BDD_RUNNING);
   
   if ((n=bdd_scanset(r, &fv, &fn)) < 0)
      return n;

   for (n=0,num=0 ; n<fdvarnum ; n++)
   {
      int found=0;
      
      for (m=0 ; m<domain[n].binsize && !found ; m++)
      {
	 for (i=0 ; i<fn && !found ; i++)
	    if (domain[n].ivar[m] == fv[i])
	    {
	       num++;
	       found=1;
	    }
      }
   }

   if ((*varset=(int*)malloc(sizeof(int)*num)) == NULL)
      return bdd_error(BDD_MEMORY);

   for (n=0,num=0 ; n<fdvarnum ; n++)
   {
      int found=0;
      
      for (m=0 ; m<domain[n].binsize && !found ; m++)
      {
	 for (i=0 ; i<fn && !found ; i++)
	    if (domain[n].ivar[m] == fv[i])
	    {
	       (*varset)[num++] = n;
	       found=1;
	    }
      }
   }

   *varnum = num;

   return 0;
}


/*======================================================================*/

/*
NAME    {* fdd\_makeset *}
SECTION {* fdd *}
SHORT   {* creates a variable set for N finite domain blocks *}
PROTO   {* BDD fdd_makeset(int *varset, int varnum) *}
DESCR   {* Returns a BDD defining all the variable sets used to define
           the variable blocks in the array {\tt varset}. The argument
	   {\tt varnum} defines the size of {\tt varset}. *}
RETURN  {* The correct BDD or the constant false on errors. *}
ALSO    {* fdd\_ithset, bdd\_makeset *}
*/
BDD fdd_makeset(int *varset, int varnum)
{
   BDD res=bddtrue, tmp;
   int n;

   if (!bddrunning)
   {
      bdd_error(BDD_RUNNING);
      return bddfalse;
   }
   
   for (n=0 ; n<varnum ; n++)
      if (varset[n] < 0  ||  varset[n] >= fdvarnum)
      {
	 bdd_error(BDD_VAR);
	 return bddfalse;
      }
	  
   for (n=0 ; n<varnum ; n++)
   {
      bdd_addref(res);
      tmp = bdd_apply(domain[varset[n]].var, res, bddop_and);
      bdd_delref(res);
      res = tmp;
   }

   return res;
}


/*
NAME    {* fdd\_intaddvarblock *}
SECTION {* fdd *}
SHORT   {* adds a new variable block for reordering *}
PROTO   {* int fdd_intaddvarblock(int first, int last, int fixed) *}
DESCR   {* Works exactly like {\tt bdd\_addvarblock} except that
           {\tt fdd\_intaddvarblock} takes a range of FDD variables
	   instead of BDD variables. *}
RETURN  {* Zero on success, otherwise a negative error code. *}
ALSO    {* bdd\_addvarblock, bdd\_intaddvarblock, bdd\_reorder *}
*/
int fdd_intaddvarblock(int first, int last, int fixed)
{
   bdd res = bddtrue, tmp;
   int n, err;
   
   if (!bddrunning)
      return bdd_error(BDD_RUNNING);
   
   if (first > last ||  first < 0  ||  last >= fdvarnum)
      return bdd_error(BDD_VARBLK);

   for (n=first ; n<=last ; n++)
   {
      bdd_addref(res);
      tmp = bdd_apply(domain[n].var, res, bddop_and);
      bdd_delref(res);
      res = tmp;
   }

   err = bdd_addvarblock(res, fixed);
   
   bdd_delref(res);
   return err;
}


/*
NAME    {* fdd\_setpair *}
SECTION {* fdd *}
SHORT   {* defines a pair for two finite domain blocks *}
PROTO   {* int fdd_setpair(bddPair *pair, int p1, int p2) *}
DESCR   {* Defines each variable in the finite domain block {\tt p1} to
           be paired with the corresponding variable in {\tt p2}. The result
	   is stored in {\tt pair} which must be allocated using
	   {\tt bdd\_makepair}. *}
RETURN  {* Zero on success or a negative error code on error. *}
ALSO    {* fdd\_setpairs *}
*/
int fdd_setpair(bddPair *pair, int p1, int p2)
{
   int n,e;

   if (!bddrunning)
      return bdd_error(BDD_RUNNING);
   
   if (p1<0 || p1>=fdvarnum || p2<0 || p2>=fdvarnum)
      return bdd_error(BDD_VAR);
   
   if (domain[p1].binsize != domain[p2].binsize)
      return bdd_error(BDD_VARNUM);

   for (n=0 ; n<domain[p1].binsize ; n++)
      if ((e=bdd_setpair(pair, domain[p1].ivar[n], domain[p2].ivar[n])) < 0)
	 return e;

   return 0;
}


/*
NAME    {* fdd\_setpairs *}
SECTION {* fdd *}
SHORT   {* defines N pairs for finite domain blocks *}
PROTO   {* int fdd_setpairs(bddPair *pair, int *p1, int *p2, int size) *}
DESCR   {* Defines each variable in all the finite domain blocks listed in
           the array {\tt p1} to be paired with the corresponding variable
	   in {\tt p2}. The result
	   is stored in {\tt pair} which must be allocated using
	   {\tt bdd\_makeset}.*}
RETURN  {* Zero on success or a negative error code on error. *}
ALSO    {* bdd\_setpair *}
*/
int fdd_setpairs(bddPair *pair, int *p1, int *p2, int size)
{
   int n,e;

   if (!bddrunning)
      return bdd_error(BDD_RUNNING);
   
   for (n=0 ; n<size ; n++)
      if (p1[n]<0 || p1[n]>=fdvarnum || p2[n]<0 || p2[n]>=fdvarnum)
	 return bdd_error(BDD_VAR);
   
   for (n=0 ; n<size ; n++)
      if ((e=fdd_setpair(pair, p1[n], p2[n])) < 0)
	 return e;

   return 0;
}


/*************************************************************************
  Domain storage "class"
*************************************************************************/

static void Domain_done(Domain* d)
{
   free(d->ivar);
   bdd_delref(d->var);
}


static void Domain_allocate(Domain* d, int range)
{
   int calcsize = 2;
   
   if (range <= 0  || range > INT_MAX/2)
   {
      bdd_error(BDD_RANGE);
      return;
   }

   d->realsize = range;
   d->binsize = 1;

   while (calcsize < range)
   {
      d->binsize++;
      calcsize <<= 1;
   }

   d->ivar = (int *)malloc(sizeof(int)*d->binsize);
   d->var = bddtrue;
}


int *fdddec2bin(int var, int val)
{
   int *res;
   int n = 0;

   res = (int *)malloc(sizeof(int)*domain[var].binsize);
   memset(res, 0, sizeof(int)*domain[var].binsize);

   while (val > 0)
   {
      if (val & 0x1)
	 res[n] = 1;
      val >>= 1;
      n++;
   }

   return res;
}


/* EOF */
