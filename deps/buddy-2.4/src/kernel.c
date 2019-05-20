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
  $Header: /cvsroot/buddy/buddy/src/kernel.c,v 1.2 2004/07/13 21:04:36 haimcohen Exp $
  FILE:  kernel.c
  DESCR: implements the bdd kernel functions.
  AUTH:  Jorn Lind
  DATE:  (C) june 1997

  WARNING: Do not use pointers to nodes across makenode calls,
           as makenode may resize/move the nodetable.

*************************************************************************/
#include "hal_bdd_config.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "kernel.h"
#include "cache.h"
#include "prime.h"

/*************************************************************************
  Various definitions and global variables
*************************************************************************/

/*=== EXTERNAL VARIABLES FOR PACKAGE USERS =============================*/

/*
NAME    {* bddtrue *}
SECTION {* kernel *}
SHORT   {* the constant true bdd *}
PROTO   {* extern const BDD bddtrue; *}
DESCR   {* This bdd holds the constant true value *}
ALSO    {* bddfalse, bdd\_true, bdd\_false *}
*/
const BDD bddtrue=1;                     /* The constant true bdd */

/*
NAME    {* bddfalse*}
SECTION {* kernel *}
SHORT   {* the constant false bdd *}
PROTO   {* extern const BDD bddfalse; *}
DESCR   {* This bdd holds the constant false value *}
ALSO    {* bddtrue, bdd\_true, bdd\_false *}
*/
const BDD bddfalse=0;                    /* The constant false bdd */


/*=== INTERNAL DEFINITIONS =============================================*/

/* Min. number of nodes (%) that has to be left after a garbage collect
   unless a resize should be done. */
static int minfreenodes=20;


/*=== GLOBAL KERNEL VARIABLES ==========================================*/

int          bddrunning;            /* Flag - package initialized */
int          bdderrorcond;          /* Some error condition */
int          bddnodesize;           /* Number of allocated nodes */
int          bddmaxnodesize;        /* Maximum allowed number of nodes */
int          bddmaxnodeincrease;    /* Max. # of nodes used to inc. table */
BddNode*     bddnodes;          /* All of the bdd nodes */
int          bddfreepos;        /* First free node */
int          bddfreenum;        /* Number of free nodes */
long int     bddproduced;       /* Number of new nodes ever produced */
int          bddvarnum;         /* Number of defined BDD variables */
int*         bddrefstack;       /* Internal node reference stack */
int*         bddrefstacktop;    /* Internal node reference stack top */
int*         bddvar2level;      /* Variable -> level table */
int*         bddlevel2var;      /* Level -> variable table */
jmp_buf      bddexception;      /* Long-jump point for interrupting calc. */
int          bddresized;        /* Flag indicating a resize of the nodetable */

bddCacheStat bddcachestats;


/*=== PRIVATE KERNEL VARIABLES =========================================*/

static BDD*     bddvarset;             /* Set of defined BDD variables */
static int      gbcollectnum;          /* Number of garbage collections */
static int      cachesize;             /* Size of the operator caches */
static long int gbcclock;              /* Clock ticks used in GBC */
static int      usednodes_nextreorder; /* When to do reorder next time */
static bddinthandler  err_handler;     /* Error handler */
static bddgbchandler  gbc_handler;     /* Garbage collection handler */
static bdd2inthandler resize_handler;  /* Node-table-resize handler */


   /* Strings for all error mesages */
static char *errorstrings[BDD_ERRNUM] =
{ "Out of memory", "Unknown variable", "Value out of range",
  "Unknown BDD root dereferenced", "bdd_init() called twice",
  "File operation failed", "Incorrect file format",
  "Variables not in ascending order", "User called break",
  "Mismatch in size of variable sets",
  "Cannot allocate fewer nodes than already in use",
  "Unknown operator", "Illegal variable set",
  "Bad variable block operation",
  "Trying to decrease the number of variables",
  "Trying to replace with variables already in the bdd",
  "Number of nodes reached user defined maximum",
  "Unknown BDD - was not in node table",
  "Bad size argument",
  "Mismatch in bitvector size",
  "Illegal shift-left/right parameter",
  "Division by zero" };


/*=== OTHER INTERNAL DEFINITIONS =======================================*/

#define NODEHASH(lvl,l,h) (TRIPLE(lvl,l,h) % bddnodesize)


/*************************************************************************
  BDD misc. user operations
*************************************************************************/

/*
NAME   {* bdd\_init *}
SECTION {* kernel *}
SHORT  {* initializes the BDD package *}
PROTO  {* int bdd_init(int nodesize, int cachesize) *}
DESCR  {* This function initiates the bdd package and {\em must} be called
          before any bdd operations are done. The argument {\tt nodesize}
	  is the initial number of nodes in the nodetable and {\tt cachesize}
	  is the fixed size of the internal caches. Typical values for
	  {\tt nodesize} are 10000 nodes for small test examples and up to
	  1000000 nodes for large examples. A cache size of 10000 seems to
	  work good even for large examples, but lesser values should do it
	  for smaller examples.

	  The number of cache entries can also be set to depend on the size
	  of the nodetable using a call to {\tt bdd\_setcacheratio}.
	  
	  The initial number of nodes is not critical for any bdd operation
	  as the table will be resized whenever there are to few nodes left
	  after a garbage collection. But it does have some impact on the
	  efficency of the operations. *}
RETURN {* If no errors occur then 0 is returned, otherwise
          a negative error code. *}
ALSO   {* bdd\_done, bdd\_resize\_hook *}
*/
int bdd_init(int initnodesize, int cs)
{
   int n, err;
   
   if (bddrunning)
      return bdd_error(BDD_RUNNING);
   
   bddnodesize = bdd_prime_gte(initnodesize);
   
   if ((bddnodes=(BddNode*)malloc(sizeof(BddNode)*bddnodesize)) == NULL)
      return bdd_error(BDD_MEMORY);

   bddresized = 0;
   
   for (n=0 ; n<bddnodesize ; n++)
   {
      bddnodes[n].refcou = 0;
      LOW(n) = -1;
      bddnodes[n].hash = 0;
      LEVEL(n) = 0;
      bddnodes[n].next = n+1;
   }
   bddnodes[bddnodesize-1].next = 0;

   bddnodes[0].refcou = bddnodes[1].refcou = MAXREF;
   LOW(0) = HIGH(0) = 0;
   LOW(1) = HIGH(1) = 1;
   
   if ((err=bdd_operator_init(cs)) < 0)
   {
      bdd_done();
      return err;
   }

   bddfreepos = 2;
   bddfreenum = bddnodesize-2;
   bddrunning = 1;
   bddvarnum = 0;
   gbcollectnum = 0;
   gbcclock = 0;
   cachesize = cs;
   usednodes_nextreorder = bddnodesize;
   bddmaxnodeincrease = DEFAULTMAXNODEINC;

   bdderrorcond = 0;
   
   bddcachestats.uniqueAccess = 0;
   bddcachestats.uniqueChain = 0;
   bddcachestats.uniqueHit = 0;
   bddcachestats.uniqueMiss = 0;
   bddcachestats.opHit = 0;
   bddcachestats.opMiss = 0;
   bddcachestats.swapCount = 0;
 
   bdd_gbc_hook(bdd_default_gbchandler);
   bdd_error_hook(bdd_default_errhandler);
   bdd_resize_hook(NULL);
   bdd_pairs_init();
   bdd_reorder_init();
   bdd_fdd_init();
   
   if (setjmp(bddexception) != 0)
      assert(0);

   return 0;
}


/*
NAME    {* bdd\_done*}
SECTION {* kernel *}
SHORT {* resets the bdd package *}
PROTO {* void bdd_done(void) *}
DESCR {* This function frees all memory used by the bdd package and resets
         the package to it's initial state.*}
ALSO  {* bdd\_init *}
*/
void bdd_done(void)
{
   /*sanitycheck(); FIXME */
   bdd_fdd_done();
   bdd_reorder_done();
   bdd_pairs_done();
   
   free(bddnodes);
   free(bddrefstack);
   free(bddvarset);
   free(bddvar2level);
   free(bddlevel2var);
   
   bddnodes = NULL;
   bddrefstack = NULL;
   bddvarset = NULL;

   bdd_operator_done();

   bddrunning = 0;
   bddnodesize = 0;
   bddmaxnodesize = 0;
   bddvarnum = 0;
   bddproduced = 0;
   
   err_handler = NULL;
   gbc_handler = NULL;
   resize_handler = NULL;
}


/*
NAME    {* bdd\_setvarnum *}
SECTION {* kernel *}
SHORT   {* set the number of used bdd variables *}
PROTO   {* int bdd_setvarnum(int num) *}
DESCR   {* This function is used to define the number of variables used in
           the bdd package. It may be called more than one time, but only
	   to increase the number of variables. The argument
	   {\tt num} is the number of variables to use. *}
RETURN  {* Zero on succes, otherwise a negative error code. *}
ALSO    {* bdd\_ithvar, bdd\_varnum, bdd\_extvarnum *}
*/
int bdd_setvarnum(int num)
{
   int bdv;
   int oldbddvarnum = bddvarnum;

   bdd_disable_reorder();
      
   if (num < 1  ||  num > MAXVAR)
   {
      bdd_error(BDD_RANGE);
      return bddfalse;
   }

   if (num < bddvarnum)
      return bdd_error(BDD_DECVNUM);
   if (num == bddvarnum)
      return 0;

   if (bddvarset == NULL)
   {
      if ((bddvarset=(BDD*)malloc(sizeof(BDD)*num*2)) == NULL)
	 return bdd_error(BDD_MEMORY);
      if ((bddlevel2var=(int*)malloc(sizeof(int)*(num+1))) == NULL)
      {
	 free(bddvarset);
	 return bdd_error(BDD_MEMORY);
      }
      if ((bddvar2level=(int*)malloc(sizeof(int)*(num+1))) == NULL)
      {
	 free(bddvarset);
	 free(bddlevel2var);
	 return bdd_error(BDD_MEMORY);
      }
   }
   else
   {
      if ((bddvarset=(BDD*)realloc(bddvarset,sizeof(BDD)*num*2)) == NULL)
	 return bdd_error(BDD_MEMORY);
      if ((bddlevel2var=(int*)realloc(bddlevel2var,sizeof(int)*(num+1))) == NULL)
      {
	 free(bddvarset);
	 return bdd_error(BDD_MEMORY);
      }
      if ((bddvar2level=(int*)realloc(bddvar2level,sizeof(int)*(num+1))) == NULL)
      {
	 free(bddvarset);
	 free(bddlevel2var);
	 return bdd_error(BDD_MEMORY);
      }
   }

   if (bddrefstack != NULL)
      free(bddrefstack);
   bddrefstack = bddrefstacktop = (int*)malloc(sizeof(int)*(num*2+4));

   for(bdv=bddvarnum ; bddvarnum < num; bddvarnum++)
   {
      bddvarset[bddvarnum*2] = PUSHREF( bdd_makenode(bddvarnum, 0, 1) );
      bddvarset[bddvarnum*2+1] = bdd_makenode(bddvarnum, 1, 0);
      POPREF(1);
      
      if (bdderrorcond)
      {
	 bddvarnum = bdv;
	 return -bdderrorcond;
      }
      
      bddnodes[bddvarset[bddvarnum*2]].refcou = MAXREF;
      bddnodes[bddvarset[bddvarnum*2+1]].refcou = MAXREF;
      bddlevel2var[bddvarnum] = bddvarnum;
      bddvar2level[bddvarnum] = bddvarnum;
   }

   LEVEL(0) = num;
   LEVEL(1) = num;
   bddvar2level[num] = num;
   bddlevel2var[num] = num;
   
   bdd_pairs_resize(oldbddvarnum, bddvarnum);
   bdd_operator_varresize();
   
   bdd_enable_reorder();
   
   return 0;
}


/*
NAME    {* bdd\_extvarnum *}
SECTION {* kernel *}
SHORT   {* add extra BDD variables *}
PROTO   {* int bdd_extvarnum(int num) *}
DESCR   {* Extends the current number of allocated BDD variables with
           {\tt num} extra variables. *}
RETURN  {* The old number of allocated variables or a negative error code. *}
ALSO    {* bdd\_setvarnum, bdd\_ithvar, bdd\_nithvar *}
*/
int bdd_extvarnum(int num)
{
   int start = bddvarnum;
   
   if (num < 0  ||  num > 0x3FFFFFFF)
      return bdd_error(BDD_RANGE);

   bdd_setvarnum(bddvarnum+num);
   return start;
}


/*
NAME  {* bdd\_error\_hook *}
SECTION {* kernel *}
SHORT {* set a handler for error conditions *}
PROTO {* bddinthandler bdd_error_hook(bddinthandler handler) *}
DESCR {* Whenever an error occurs in the bdd package a test is done to
        see if an error handler is supplied by the user and if such exists
	then it will be called
	with an error code in the variable {\tt errcode}. The handler may
	then print any usefull information and return or exit afterwards.

	This function sets the handler to be {\tt handler}. If a {\tt NULL}
	argument is supplied then no calls are made when an error occurs.
	Possible error codes are found in {\tt bdd.h}. The default handler
	is {\tt bdd\_default\_errhandler} which will use {\tt exit()} to
	terminate the program.

	Any handler should be defined like this:
	\begin{verbatim}
void my_error_handler(int errcode)
{
   ...
}
\end{verbatim} *}
RETURN {* The previous handler *}
ALSO  {* bdd\_errstring *}
*/
bddinthandler bdd_error_hook(bddinthandler handler)
{
   bddinthandler tmp = err_handler;
   err_handler = handler;
   return tmp;
}


/*
NAME    {* bdd\_clear\_error *}
SECTION {* kernel *}
SHORT   {* clears an error condition in the kernel *}
PROTO   {* void bdd_clear_error(void) *}
DESCR   {* The BuDDy kernel may at some point run out of new ROBDD nodes if
           a maximum limit is set with {\tt bdd\_setmaxnodenum}. In this case
	   the current error handler is called and an internal error flag
	   is set. Further calls to BuDDy will always return {\tt bddfalse}.
	   From here BuDDy must either be restarted or {\tt bdd\_clear\_error}
	   may be called after action is taken to let BuDDy continue. This may
	   not be especially usefull since the default error handler exits
	   the program - other needs may of course exist.*}
ALSO    {* bdd\_error\_hook, bdd\_setmaxnodenum *}
*/
void bdd_clear_error(void)
{
   bdderrorcond = 0;
   bdd_operator_reset();
}


/*
NAME  {* bdd\_gbc\_hook *}
SECTION {* kernel *}
SHORT {* set a handler for garbage collections *}
PROTO {* bddgbchandler bdd_gbc_hook(bddgbchandler handler) *}
DESCR {* Whenever a garbage collection is required, a test is done to
         see if a handler for this event is supplied by the user and if such
	 exists then it is called, both before and after the garbage collection
	 takes places. This is indicated by an integer flag {\tt pre} passed to
	 the handler, which will be one before garbage collection and zero
	 after garbage collection.

	 This function sets the handler to be {\tt handler}. If a {\tt
	 NULL} argument is supplied then no calls are made when a
	 garbage collection takes place. The argument {\tt pre}
	 indicates pre vs. post garbage collection and the argument
	 {\tt stat} contains information about the garbage
	 collection. The default handler is {\tt bdd\_default\_gbchandler}.

	 Any handler should be defined like this:
	 \begin{verbatim}
void my_gbc_handler(int pre, bddGbcStat *stat)
{
   ...
}
\end{verbatim} *}
RETURN {* The previous handler *}
ALSO {* bdd\_resize\_hook, bdd\_reorder\_hook *} */
bddgbchandler bdd_gbc_hook(bddgbchandler handler)
{
   bddgbchandler tmp = gbc_handler;
   gbc_handler = handler;
   return tmp;
}


/*
NAME  {* bdd\_resize\_hook  *}
SECTION {* kernel *}
SHORT {* set a handler for nodetable resizes *}
PROTO {* bdd2inthandler bdd_resize_hook(bdd2inthandler handler) *}
DESCR {* Whenever it is impossible to get enough free nodes by a garbage
         collection then the node table is resized and a test is done to see
	 if a handler is supllied by the user for this event. If so then
	 it is called with {\tt oldsize} being the old nodetable size and
	 {\tt newsize} being the new nodetable size.

	 This function sets the handler to be {\tt handler}. If a {\tt NULL}
	 argument is supplied then no calls are made when a table resize
	 is done. No default handler is supplied.

	 Any handler should be defined like this:
	 \begin{verbatim}
void my_resize_handler(int oldsize, int newsize)
{
   ...
}
\end{verbatim} *}
RETURN {* The previous handler *}
ALSO  {* bdd\_gbc\_hook, bdd\_reorder\_hook, bdd\_setminfreenodes  *}
*/
bdd2inthandler bdd_resize_hook(bdd2inthandler handler)
{
   bdd2inthandler tmp = handler;
   resize_handler = handler;
   return tmp;
}


/*
NAME    {* bdd\_setmaxincrease *}
SECTION {* kernel *}
SHORT   {* set max. number of nodes used to increase node table *}
PROTO   {* int bdd_setmaxincrease(int size) *}
DESCR   {* The node table is expanded by doubling the size of the table
           when no more free nodes can be found, but a maximum for the
	   number of new nodes added can be set with {\tt bdd\_maxincrease}
	   to {\tt size} nodes. The default is 50000 nodes (1 Mb). *}
RETURN  {* The old threshold on succes, otherwise a negative error code. *}
ALSO    {* bdd\_setmaxnodenum, bdd\_setminfreenodes *}
*/
int bdd_setmaxincrease(int size)
{
   int old = bddmaxnodeincrease;
   
   if (size < 0)
      return bdd_error(BDD_SIZE);

   bddmaxnodeincrease = size;
   return old;
}

/*
NAME    {* bdd\_setmaxnodenum *}
SECTION {* kernel *}
SHORT {* set the maximum available number of bdd nodes *}
PROTO {* int bdd_setmaxnodenum(int size) *}
DESCR {* This function sets the maximal number of bdd nodes the package may
         allocate before it gives up a bdd operation. The
	 argument {\tt size} is the absolute maximal number of nodes there
	 may be allocated for the nodetable. Any attempt to allocate more
	 nodes results in the constant false being returned and the error
	 handler being called until some nodes are deallocated.
	 A value of 0 is interpreted as an unlimited amount.
	 It is {\em not} possible to specify
	 fewer nodes than there has already been allocated. *}
RETURN {* The old threshold on succes, otherwise a negative error code. *}
ALSO   {* bdd\_setmaxincrease, bdd\_setminfreenodes *}
*/
int bdd_setmaxnodenum(int size)
{
   if (size > bddnodesize  ||  size == 0)
   {
      int old = bddmaxnodesize;
      bddmaxnodesize = size;
      return old;
   }

   return bdd_error(BDD_NODES);
}


/*
NAME    {* bdd\_setminfreenodes *}
SECTION {* kernel *}
SHORT   {* set min. no. of nodes to be reclaimed after GBC. *}
PROTO   {* int bdd_setminfreenodes(int n) *}
DESCR   {* Whenever a garbage collection is executed the number of free
           nodes left are checked to see if a resize of the node table is
	   required. If $X = (\mathit{bddfreenum}*100)/\mathit{maxnum}$
	   is less than or
	   equal to {\tt n} then a resize is initiated. The range of
	   {\tt X} is of course $0\ldots 100$ and has some influence
	   on how fast the package is. A low number means harder attempts
	   to avoid resizing and saves space, and a high number reduces
	   the time used in garbage collections. The default value is
	   20. *}
RETURN  {* The old threshold on succes, otherwise a negative error code. *}
ALSO    {* bdd\_setmaxnodenum, bdd\_setmaxincrease *}
*/
int bdd_setminfreenodes(int mf)
{
   int old = minfreenodes;
   
   if (mf<0 || mf>100)
      return bdd_error(BDD_RANGE);

   minfreenodes = mf;
   return old;
}


/*
NAME    {* bdd\_getnodenum *}
SECTION {* kernel *}
SHORT   {* get the number of active nodes in use *}
PROTO   {* int bdd_getnodenum(void) *}
DESCR   {* Returns the number of nodes in the nodetable that are
           currently in use. Note that dead nodes that have not been
	   reclaimed yet
	   by a garbage collection are counted as active. *}
RETURN  {* The number of nodes. *}
ALSO    {* bdd\_getallocnum, bdd\_setmaxnodenum *}
*/
int bdd_getnodenum(void)
{
   return bddnodesize - bddfreenum;
}


/*
NAME    {* bdd\_getallocnum *}
SECTION {* kernel *}
SHORT   {* get the number of allocated nodes *}
PROTO   {* int bdd_getallocnum(void) *}
DESCR   {* Returns the number of nodes currently allocated. This includes
           both dead and active nodes. *}
RETURN  {* The number of nodes. *}
ALSO    {* bdd\_getnodenum, bdd\_setmaxnodenum *}
*/
int bdd_getallocnum(void)
{
   return bddnodesize;
}


/*
NAME    {* bdd\_isrunning *}
SECTION {* kernel *}
SHORT   {* test whether the package is started or not *}
PROTO   {* void bdd_isrunning(void) *}
DESCR   {* This function tests the internal state of the package and returns
          a status. *}
RETURN  {* 1 (true) if the package has been started, otherwise 0. *}
ALSO    {* bdd\_init, bdd\_done *}
*/
int bdd_isrunning(void)
{
   return bddrunning;
}


/*
NAME    {* bdd\_versionstr *}
SECTION {* kernel *}
SHORT   {* returns a text string with version information *}
PROTO   {* char* bdd_versionstr(void) *}
DESCR   {* This function returns a text string with information about the
           version of the bdd package. *}
ALSO    {* bdd\_versionnum *}
*/
char *bdd_versionstr(void)
{
   static char str[] = "BuDDy -  release " PACKAGE_VERSION;
   return str;
}


/*
NAME    {* bdd\_versionnum *}
SECTION {* kernel *}
SHORT   {* returns the version number of the bdd package *}
PROTO   {* int bdd_versionnum(void) *}
DESCR   {* This function returns the version number of the bdd package. The
           number is in the range 10-99 for version 1.0 to 9.9. *}
ALSO    {* bdd\_versionstr *}
*/
int bdd_versionnum(void)
{
   return MAJOR_VERSION * 10 + MINOR_VERSION;
}


/*
NAME    {* bdd\_stats *}
SECTION {* kernel *}
SHORT   {* returns some status information about the bdd package *}
PROTO   {* void bdd_stats(bddStat* stat) *}
DESCR   {* This function acquires information about the internal state of
           the bdd package. The status information is written into the
	   {\tt stat} argument. *}
ALSO    {* bddStat *}
*/
void bdd_stats(bddStat *s)
{
   s->produced = bddproduced;
   s->nodenum = bddnodesize;
   s->maxnodenum = bddmaxnodesize;
   s->freenodes = bddfreenum;
   s->minfreenodes = minfreenodes;
   s->varnum = bddvarnum;
   s->cachesize = cachesize;
   s->gbcnum = gbcollectnum;
}



/*
NAME    {* bdd\_cachestats *}
SECTION {* kernel *}
SHORT   {* Fetch cache access usage *}
PROTO   {* void bdd_cachestats(bddCacheStat *s) *}
DESCR   {* Fetches cache usage information and stores it in {\tt s}. The
           fields of {\tt s} can be found in the documentaion for
	   {\tt bddCacheStat}. This function may or may not be compiled
	   into the BuDDy package - depending on the setup at compile
	   time of BuDDy. *}
ALSO    {* bddCacheStat, bdd\_printstat *}
*/
void bdd_cachestats(bddCacheStat *s)
{
   *s = bddcachestats;
}


/*
NAME    {* bdd\_printstat *}
EXTRA   {* bdd\_fprintstat *}
SECTION {* kernel *}
SHORT   {* print cache statistics *}
PROTO   {* void bdd_printstat(void)
void bdd_fprintstat(FILE *ofile) *}
DESCR   {* Prints information about the cache performance on standard output
           (or the supplied file). The information contains the number of
	   accesses to the unique node table, the number of times a node
	   was (not) found there and how many times a hash chain had to
	   traversed. Hit and miss count is also given for the operator
	   caches. *}
ALSO    {* bddCacheStat, bdd\_cachestats *}
*/
void bdd_fprintstat(FILE *ofile)
{
   bddCacheStat s;
   bdd_cachestats(&s);
   
   fprintf(ofile, "\nCache statistics\n");
   fprintf(ofile, "----------------\n");
   
   fprintf(ofile, "Unique Access:  %ld\n", s.uniqueAccess);
   fprintf(ofile, "Unique Chain:   %ld\n", s.uniqueChain);
   fprintf(ofile, "Unique Hit:     %ld\n", s.uniqueHit);
   fprintf(ofile, "Unique Miss:    %ld\n", s.uniqueMiss);
   fprintf(ofile, "=> Hit rate =   %.2f\n",
	   (s.uniqueHit+s.uniqueMiss > 0) ? 
	   ((float)s.uniqueHit)/((float)s.uniqueHit+s.uniqueMiss) : 0);
   fprintf(ofile, "Operator Hits:  %ld\n", s.opHit);
   fprintf(ofile, "Operator Miss:  %ld\n", s.opMiss);
   fprintf(ofile, "=> Hit rate =   %.2f\n",
	   (s.opHit+s.opMiss > 0) ? 
	   ((float)s.opHit)/((float)s.opHit+s.opMiss) : 0);
   fprintf(ofile, "Swap count =    %ld\n", s.swapCount);
}


void bdd_printstat(void)
{
   bdd_fprintstat(stdout);
}


/*************************************************************************
  Error handler
*************************************************************************/

/*
NAME    {* bdd\_errstring *}
SECTION {* kernel *}
SHORT   {* converts an error code to a string*}
PROTO   {* const char *bdd_errstring(int errorcode) *}
DESCR   {* Converts a negative error code {\tt errorcode} to a descriptive
           string that can be used for error handling. *}
RETURN  {* An error description string if {\tt e} is known, otherwise {\tt NULL}. *}
ALSO    {* bdd\_err\_hook *}
*/
const char *bdd_errstring(int e)
{
   e = abs(e);
   if (e<1 || e>BDD_ERRNUM)
      return NULL;
   return errorstrings[e-1];
}


void bdd_default_errhandler(int e)
{
    fprintf(stderr, "[BuDDy] BDD error: %s\n", bdd_errstring(e));
    /* Fyrbiak: commented out the exit(1) to actually proceed with the application */
    // exit(1);
}


int bdd_error(int e)
{
   if (err_handler != NULL)
      err_handler(e);
   
   return e;
}


/*************************************************************************
  BDD primitives
*************************************************************************/

/*
NAME    {* bdd\_true *}
SECTION {* kernel *}
SHORT   {* returns the constant true bdd *}
PROTO   {* BDD bdd_true(void) *}
DESCR   {* This function returns the constant true bdd and can freely be
           used together with the {\tt bddtrue} and {\tt bddfalse}
	   constants. *}
RETURN  {* The constant true bdd *}
ALSO    {* bdd\_false, bddtrue, bddfalse *}
*/
BDD bdd_true(void)
{
   return 1;
}


/*
NAME    {* bdd\_false *}
SECTION {* kernel *}
SHORT   {* returns the constant false bdd *}
PROTO   {* BDD bdd_false(void) *}
DESCR   {* This function returns the constant false bdd and can freely be
           used together with the {\tt bddtrue} and {\tt bddfalse}
	   constants. *}
RETURN  {* The constant false bdd *}
ALSO    {* bdd\_true, bddtrue, bddfalse *}
*/
BDD bdd_false(void)
{
   return 0;
}


/*
NAME    {* bdd\_ithvar *}
SECTION {* kernel *}
SHORT   {* returns a bdd representing the I'th variable *}
PROTO   {* BDD bdd_ithvar(int var) *}
DESCR   {* This function is used to get a bdd representing the I'th
           variable (one node with the childs true and false). The requested
	   variable must be in the range define by {\tt
	   bdd\_setvarnum} starting with 0 being the first. For ease
	   of use then the bdd returned from {\tt bdd\_ithvar} does
	   not have to be referenced counted with a call to {\tt
	   bdd\_addref}. The initial variable order is defined by the
	   the index {\tt var} that also defines the position in the
	   variable order -- variables with lower indecies are before
	   those with higher indecies. *}
RETURN  {* The I'th variable on succes, otherwise the constant false bdd *}
ALSO {* bdd\_setvarnum, bdd\_nithvar, bddtrue, bddfalse *} */
BDD bdd_ithvar(int var)
{
   if (var < 0  ||  var >= bddvarnum)
   {
      bdd_error(BDD_VAR);
      return bddfalse;
   }

   return bddvarset[var*2];
}


/*
NAME    {* bdd\_nithvar *}
SECTION {* kernel *}
SHORT   {* returns a bdd representing the negation of the I'th variable *}
PROTO   {* BDD bdd_nithvar(int var) *}
DESCR   {* This function is used to get a bdd representing the negation of
           the I'th variable (one node with the childs false and true).
	   The requested variable must be in the range define by
	   {\tt bdd\_setvarnum} starting with 0 being the first. For ease of
	   use then the bdd returned from {\tt bdd\_nithvar} does not have
	   to be referenced counted with a call to {\tt bdd\_addref}. *}
RETURN  {* The negated I'th variable on succes, otherwise the constant false bdd *}	   
ALSO    {* bdd\_setvarnum, bdd\_ithvar, bddtrue, bddfalse *}
*/
BDD bdd_nithvar(int var)
{
   if (var < 0  ||  var >= bddvarnum)
   {
      bdd_error(BDD_VAR);
      return bddfalse;
   }
   
   return bddvarset[var*2+1];
}


/*
NAME    {* bdd\_varnum *}
SECTION {* kernel *}
SHORT   {* returns the number of defined variables *}
PROTO   {* int bdd_varnum(void) *}
DESCR   {* This function returns the number of variables defined by
           a call to {\tt bdd\_setvarnum}.*}
RETURN  {* The number of defined variables *}
ALSO    {* bdd\_setvarnum, bdd\_ithvar *}
*/
int bdd_varnum(void)
{
   return bddvarnum;
}


/*
NAME    {* bdd\_var *}
SECTION {* info *}
SHORT   {* gets the variable labeling the bdd *}
PROTO   {* int bdd_var(BDD r) *}
DESCR   {* Gets the variable labeling the bdd {\tt r}. *}
RETURN  {* The variable number. *}
*/
int bdd_var(BDD root)
{
   CHECK(root);
   if (root < 2)
      return bdd_error(BDD_ILLBDD);

   return (bddlevel2var[LEVEL(root)]);
}


/*
NAME    {* bdd\_low *}
SECTION {* info *}
SHORT   {* gets the false branch of a bdd  *}
PROTO   {* BDD bdd_low(BDD r) *}
DESCR   {* Gets the false branch of the bdd {\tt r}.  *}
RETURN  {* The bdd of the false branch *}
ALSO    {* bdd\_high *}
*/
BDD bdd_low(BDD root)
{
   CHECK(root);
   if (root < 2)
      return bdd_error(BDD_ILLBDD);

   return (LOW(root));
}


/*
NAME    {* bdd\_high *}
SECTION {* info *}
SHORT   {* gets the true branch of a bdd  *}
PROTO   {* BDD bdd_high(BDD r) *}
DESCR   {* Gets the true branch of the bdd {\tt r}.  *}
RETURN  {* The bdd of the true branch *}
ALSO    {* bdd\_low *}
*/
BDD bdd_high(BDD root)
{
   CHECK(root);
   if (root < 2)
      return bdd_error(BDD_ILLBDD);

   return (HIGH(root));
}



/*************************************************************************
  Garbage collection and node referencing
*************************************************************************/

void bdd_default_gbchandler(int pre, bddGbcStat *s)
{
   if (!pre)
   {
      printf("Garbage collection #%d: %d nodes / %d free",
	     s->num, s->nodes, s->freenodes);
      printf(" / %.1fs / %.1fs total\n",
	     (float)s->time/(float)(CLOCKS_PER_SEC),
	     (float)s->sumtime/(float)CLOCKS_PER_SEC);
   }
}


static void bdd_gbc_rehash(void)
{
   int n;

   bddfreepos = 0;
   bddfreenum = 0;

   for (n=bddnodesize-1 ; n>=2 ; n--)
   {
      register BddNode *node = &bddnodes[n];

      if (LOWp(node) != -1)
      {
	 register unsigned int hash;

	 hash = NODEHASH(LEVELp(node), LOWp(node), HIGHp(node));
	 node->next = bddnodes[hash].hash;
	 bddnodes[hash].hash = n;
      }
      else
      {
	 node->next = bddfreepos;
	 bddfreepos = n;
	 bddfreenum++;
      }
   }
}


void bdd_gbc(void)
{
   int *r;
   int n;
   long int c2, c1 = clock();

   if (gbc_handler != NULL)
   {
      bddGbcStat s;
      s.nodes = bddnodesize;
      s.freenodes = bddfreenum;
      s.time = 0;
      s.sumtime = gbcclock;
      s.num = gbcollectnum;
      gbc_handler(1, &s);
   }
   
   for (r=bddrefstack ; r<bddrefstacktop ; r++)
      bdd_mark(*r);

   for (n=0 ; n<bddnodesize ; n++)
   {
      if (bddnodes[n].refcou > 0)
	 bdd_mark(n);
      bddnodes[n].hash = 0;
   }
   
   bddfreepos = 0;
   bddfreenum = 0;

   for (n=bddnodesize-1 ; n>=2 ; n--)
   {
      register BddNode *node = &bddnodes[n];

      if ((LEVELp(node) & MARKON)  &&  LOWp(node) != -1)
      {
	 register unsigned int hash;

	 LEVELp(node) &= MARKOFF;
	 hash = NODEHASH(LEVELp(node), LOWp(node), HIGHp(node));
	 node->next = bddnodes[hash].hash;
	 bddnodes[hash].hash = n;
      }
      else
      {
	 LOWp(node) = -1;
	 node->next = bddfreepos;
	 bddfreepos = n;
	 bddfreenum++;
      }
   }

   bdd_operator_reset();

   c2 = clock();
   gbcclock += c2-c1;
   gbcollectnum++;

   if (gbc_handler != NULL)
   {
      bddGbcStat s;
      s.nodes = bddnodesize;
      s.freenodes = bddfreenum;
      s.time = c2-c1;
      s.sumtime = gbcclock;
      s.num = gbcollectnum;
      gbc_handler(0, &s);
   }
}


/*
NAME    {* bdd\_addref *}
SECTION {* kernel *}
SHORT   {* increases the reference count on a node *}
PROTO   {* BDD bdd_addref(BDD r) *}
DESCR   {* Reference counting is done on externaly referenced nodes only
           and the count for a specific node {\tt r} can and must be
	   increased using this function to avoid loosing the node in the next
	   garbage collection. *}
ALSO    {* bdd\_delref *}
RETURN  {* The BDD node {\tt r}. *}
*/
BDD bdd_addref(BDD root)
{
   if (root < 2  ||  !bddrunning)
      return root;
   if (root >= bddnodesize)
      return bdd_error(BDD_ILLBDD);
   if (LOW(root) == -1)
      return bdd_error(BDD_ILLBDD);

   INCREF(root);
   return root;
}


/*
NAME    {* bdd\_delref *}
SECTION {* kernel *}
SHORT   {* decreases the reference count on a node *}
PROTO   {* BDD bdd_delref(BDD r) *}
DESCR   {* Reference counting is done on externaly referenced nodes only
           and the count for a specific node {\tt r} can and must be
	   decreased using this function to make it possible to reclaim the
	   node in the next garbage collection. *}
ALSO    {* bdd\_addref *}
RETURN  {* The BDD node {\tt r}. *}
*/
BDD bdd_delref(BDD root)
{
   if (root < 2  ||  !bddrunning)
      return root;
   if (root >= bddnodesize)
      return bdd_error(BDD_ILLBDD);
   if (LOW(root) == -1)
      return bdd_error(BDD_ILLBDD);

   /* if the following line is present, fails there much earlier */ 
   if (!HASREF(root)) bdd_error(BDD_BREAK); /* distinctive */
   
   DECREF(root);
   return root;
}


/*=== RECURSIVE MARK / UNMARK ==========================================*/

void bdd_mark(int i)
{
   BddNode *node;
   
   if (i < 2)
      return;

   node = &bddnodes[i];
   if (LEVELp(node) & MARKON  ||  LOWp(node) == -1)
      return;
   
   LEVELp(node) |= MARKON;
   
   bdd_mark(LOWp(node));
   bdd_mark(HIGHp(node));
}


void bdd_mark_upto(int i, int level)
{
   BddNode *node = &bddnodes[i];
   
   if (i < 2)
      return;
   
   if (LEVELp(node) & MARKON  ||  LOWp(node) == -1)
      return;
   
   if (LEVELp(node) > level)
      return;

   LEVELp(node) |= MARKON;

   bdd_mark_upto(LOWp(node), level);
   bdd_mark_upto(HIGHp(node), level);
}


void bdd_markcount(int i, int *cou)
{
   BddNode *node;
   
   if (i < 2)
      return;

   node = &bddnodes[i];
   if (MARKEDp(node)  ||  LOWp(node) == -1)
      return;
   
   SETMARKp(node);
   *cou += 1;
   
   bdd_markcount(LOWp(node), cou);
   bdd_markcount(HIGHp(node), cou);
}


void bdd_unmark(int i)
{
   BddNode *node;
   
   if (i < 2)
      return;

   node = &bddnodes[i];

   if (!MARKEDp(node)  ||  LOWp(node) == -1)
      return;
   UNMARKp(node);
   
   bdd_unmark(LOWp(node));
   bdd_unmark(HIGHp(node));
}


void bdd_unmark_upto(int i, int level)
{
   BddNode *node = &bddnodes[i];

   if (i < 2)
      return;
   
   if (!(LEVELp(node) & MARKON))
      return;
   
   LEVELp(node) &= MARKOFF;
   
   if (LEVELp(node) > level)
      return;

   bdd_unmark_upto(LOWp(node), level);
   bdd_unmark_upto(HIGHp(node), level);
}


/*************************************************************************
  Unique node table functions
*************************************************************************/

int bdd_makenode(unsigned int level, int low, int high)
{
   register BddNode *node;
   register unsigned int hash;
   register int res;

#ifdef CACHESTATS
   bddcachestats.uniqueAccess++;
#endif
   
      /* check whether childs are equal */
   if (low == high)
      return low;

      /* Try to find an existing node of this kind */
   hash = NODEHASH(level, low, high);
   res = bddnodes[hash].hash;

   while(res != 0)
   {
      if (LEVEL(res) == level  &&  LOW(res) == low  &&  HIGH(res) == high)
      {
#ifdef CACHESTATS
	 bddcachestats.uniqueHit++;
#endif
	 return res;
      }

      res = bddnodes[res].next;
#ifdef CACHESTATS
      bddcachestats.uniqueChain++;
#endif
   }
   
      /* No existing node -> build one */
#ifdef CACHESTATS
   bddcachestats.uniqueMiss++;
#endif

      /* Any free nodes to use ? */
   if (bddfreepos == 0)
   {
      if (bdderrorcond)
	 return 0;
      
         /* Try to allocate more nodes */
      bdd_gbc();

      if ((bddnodesize-bddfreenum) >= usednodes_nextreorder  &&
	   bdd_reorder_ready())
      {
	 longjmp(bddexception,1);
      }

      if ((bddfreenum*100) / bddnodesize <= minfreenodes)
      {
	 bdd_noderesize(1);
	 hash = NODEHASH(level, low, high);
      }

         /* Panic if that is not possible */
      if (bddfreepos == 0)
      {
	 bdd_error(BDD_NODENUM);
	 bdderrorcond = abs(BDD_NODENUM);
	 return 0;
      }
   }

      /* Build new node */
   res = bddfreepos;
   bddfreepos = bddnodes[bddfreepos].next;
   bddfreenum--;
   bddproduced++;
   
   node = &bddnodes[res];
   LEVELp(node) = level;
   LOWp(node) = low;
   HIGHp(node) = high;
   
      /* Insert node */
   node->next = bddnodes[hash].hash;
   bddnodes[hash].hash = res;

   return res;
}


int bdd_noderesize(int doRehash)
{
   BddNode *newnodes;
   int oldsize = bddnodesize;
   int n;

   if (bddnodesize >= bddmaxnodesize  &&  bddmaxnodesize > 0)
      return -1;
   
   bddnodesize = bddnodesize << 1;

   if (bddnodesize > oldsize + bddmaxnodeincrease)
      bddnodesize = oldsize + bddmaxnodeincrease;

   if (bddnodesize > bddmaxnodesize  &&  bddmaxnodesize > 0)
      bddnodesize = bddmaxnodesize;

   bddnodesize = bdd_prime_lte(bddnodesize);
   
   if (resize_handler != NULL)
      resize_handler(oldsize, bddnodesize);

   newnodes = (BddNode*)realloc(bddnodes, sizeof(BddNode)*bddnodesize);
   if (newnodes == NULL)
      return bdd_error(BDD_MEMORY);
   bddnodes = newnodes;

   if (doRehash)
      for (n=0 ; n<oldsize ; n++)
	 bddnodes[n].hash = 0;
   
   for (n=oldsize ; n<bddnodesize ; n++)
   {
      bddnodes[n].refcou = 0;
      bddnodes[n].hash = 0;
      LEVEL(n) = 0;
      LOW(n) = -1;
      bddnodes[n].next = n+1;
   }
   bddnodes[bddnodesize-1].next = bddfreepos;
   bddfreepos = oldsize;
   bddfreenum += bddnodesize - oldsize;

   if (doRehash)
      bdd_gbc_rehash();

   bddresized = 1;
   
   return 0;
}


void bdd_checkreorder(void)
{
   bdd_reorder_auto();

      /* Do not reorder before twice as many nodes have been used */
   usednodes_nextreorder = 2 * (bddnodesize - bddfreenum);
   
      /* And if very little was gained this time (< 20%) then wait until
       * even more nodes (upto twice as many again) have been used */
   if (bdd_reorder_gain() < 20)
      usednodes_nextreorder +=
	 (usednodes_nextreorder * (20-bdd_reorder_gain())) / 20;
}


/*************************************************************************
  Variable sets
*************************************************************************/

/*
NAME    {* bdd\_scanset *}
SECTION {* kernel *}
SHORT   {* returns an integer representation of a variable set *}
PROTO   {* int bdd_scanset(BDD r, int **v, int *n) *}
DESCR   {* Scans a variable set {\tt r} and copies the stored variables into
           an integer array of variable numbers. The argument {\tt v} is
	   the address of an integer pointer where the array is stored and
	   {\tt n} is a pointer to an integer where the number of elements
	   are stored. It is the users responsibility to make sure the
	   array is deallocated by a call to {\tt free(v)}. The numbers
	   returned are guaranteed to be in ascending order. *}
ALSO    {* bdd\_makeset *}
RETURN  {* Zero on success, otherwise a negative error code. *}
*/
int bdd_scanset(BDD r, int **varset, int *varnum)
{
   int n, num;

   CHECK(r);
   if (r < 2)
   {
      *varnum = 0;
      *varset = NULL;
      return 0;
   }
   
   for (n=r, num=0 ; n > 1 ; n=HIGH(n))
      num++;

   if (((*varset) = (int *)malloc(sizeof(int)*num)) == NULL)
      return bdd_error(BDD_MEMORY);
   
   for (n=r, num=0 ; n > 1 ; n=HIGH(n))
      (*varset)[num++] = bddlevel2var[LEVEL(n)];

   *varnum = num;

   return 0;
}


/*
NAME    {* bdd\_makeset *}
SECTION {* kernel *}
SHORT   {* builds a BDD variable set from an integer array *}
PROTO   {* BDD bdd_makeset(int *v, int n) *}
DESCR   {* Reads a set of variable numbers from the integer array {\tt v}
           which must hold exactly {\tt n} integers and then builds a BDD
	   representing the variable set.

	   The BDD variable set is represented as the conjunction of
	   all the variables in their positive form and may just as
	   well be made that way by the user. The user should keep a
	   reference to the returned BDD instead of building it every
	   time the set is needed. *}
ALSO    {* bdd\_scanset *}
RETURN {* A BDD variable set. *} */
BDD bdd_makeset(int *varset, int varnum)
{
   int v, res=1;
   
   for (v=varnum-1 ; v>=0 ; v--)
   {
      BDD tmp;
      bdd_addref(res);
      tmp = bdd_apply(res, bdd_ithvar(varset[v]), bddop_and);
      bdd_delref(res);
      res = tmp;
   }

   return res;
}


/* EOF */
