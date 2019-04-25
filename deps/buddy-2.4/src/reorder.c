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
  $Header: /cvsroot/buddy/buddy/src/reorder.c,v 1.1.1.1 2004/06/25 13:22:59 haimcohen Exp $
  FILE:  reorder.c
  DESCR: BDD reordering functions
  AUTH:  Jorn Lind
  DATE:  (C) january 1998
*************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "kernel.h"
#include "bddtree.h"
#include "imatrix.h"
#include "prime.h"

/* IMPORTANT:
 * The semantics of the "level" field in the BddNode struct changes during
 * variable reordering in order to make a fast variable swap possible when
 * two variables are independent. Instead of refering to the level of the node
 * it refers to the *variable* !!!
 */

   /* Change macros to reflect the above idea */
#define VAR(n) (bddnodes[n].level)
#define VARp(p) (p->level)

   /* Avoid these - they are misleading! */
#undef LEVEL
#undef LEVELp


#define __USERESIZE /* FIXME */

   /* Current auto reord. method and number of automatic reorderings left */
static int bddreordermethod;
static int bddreordertimes;

   /* Flag for disabling reordering temporarily */
static int reorderdisabled;

   /* Store for the variable relationships */
static BddTree *vartree;
static int blockid;

   /* Store for the ref.cou. of the external roots */
static int *extroots;
static int extrootsize;

/* Level data */
typedef struct _levelData
{
   int start;    /* Start of this sub-table (entry in "bddnodes") */
   int size;     /* Size of this sub-table */
   int maxsize;  /* Max. allowed size of sub-table */
   int nodenum;  /* Number of nodes in this level */
} levelData;

static levelData *levels; /* Indexed by variable! */

   /* Interaction matrix */
static imatrix *iactmtx;

   /* Reordering information for the user */
static int verbose;
static bddinthandler reorder_handler;
static bddfilehandler reorder_filehandler;
static bddsizehandler reorder_nodenum;

   /* Number of live nodes before and after a reordering session */
static int usednum_before;
static int usednum_after;
	    
   /* Kernel variables needed for reordering */
extern int bddfreepos;
extern int bddfreenum;
extern int bddproduced;

   /* Flag telling us when a node table resize is done */
static int resizedInMakenode;

   /* New node hashing function for use with reordering */
#define NODEHASH(var,l,h) ((PAIR((l),(h))%levels[var].size)+levels[var].start)

   /* Reordering prototypes */
static void blockdown(BddTree *);
static void addref_rec(int, char *);
static void reorder_gbc();
static void reorder_setLevellookup(void);
static int  reorder_makenode(int, int, int);
static int  reorder_varup(int);
static int  reorder_vardown(int);
static int  reorder_init(void);
static void reorder_done(void);

#define random(a) (rand() % (a))

   /* For sorting the blocks according to some specific size value */
typedef struct s_sizePair
{
   int val;
   BddTree *block;
} sizePair;


/*************************************************************************
  Initialize and shutdown
*************************************************************************/

void bdd_reorder_init(void)
{
   reorderdisabled = 0;
   vartree = NULL;
   
   bdd_clrvarblocks();
   bdd_reorder_hook(bdd_default_reohandler);
   bdd_reorder_verbose(0);
   bdd_autoreorder_times(BDD_REORDER_NONE, 0);
   reorder_nodenum = bdd_getnodenum;
   usednum_before = usednum_after = 0;
   blockid = 0;
}


void bdd_reorder_done(void)
{
   bddtree_del(vartree);
   bdd_operator_reset();
   vartree = NULL;
}


/*************************************************************************
  Reordering heuristics
*************************************************************************/

/*=== Reorder using a sliding window of size 2 =========================*/

static BddTree *reorder_win2(BddTree *t)
{
   BddTree *this=t, *first=t;

   if (t == NULL)
      return t;

   if (verbose > 1)
      printf("Win2 start: %d nodes\n", reorder_nodenum());
   fflush(stdout);

   while (this->next != NULL)
   {
      int best = reorder_nodenum();
      blockdown(this);
      
      if (best < reorder_nodenum())
      {
	 blockdown(this->prev);
	 this = this->next;
      }
      else
      if (first == this)
	 first = this->prev;

      if (verbose > 1)
      {
	 printf(".");
	 fflush(stdout);
      }
   }
   
   if (verbose > 1)
      printf("\nWin2 end: %d nodes\n", reorder_nodenum());
   fflush(stdout);

   return first;
}


static BddTree *reorder_win2ite(BddTree *t)
{
   BddTree *this, *first=t;
   int lastsize;
   int c=1;
   
   if (t == NULL)
      return t;
   
   if (verbose > 1)
      printf("Win2ite start: %d nodes\n", reorder_nodenum());

   do
   {
      lastsize = reorder_nodenum();

      this = t;
      while (this->next != NULL)
      {
	 int best = reorder_nodenum();

	 blockdown(this);

	 if (best < reorder_nodenum())
	 {
	    blockdown(this->prev);
	    this = this->next;
	 }
	 else
	 if (first == this)
	    first = this->prev;
	 if (verbose > 1)
	 {
	    printf(".");
	    fflush(stdout);
	 }
      }

      if (verbose > 1)
	 printf(" %d nodes\n", reorder_nodenum());
      c++;
   }
   while (reorder_nodenum() != lastsize);

   return first;
}


/*=== Reorder using a sliding window of size 3 =========================*/
#define X(a)

static BddTree *reorder_swapwin3(BddTree *this, BddTree **first)
{
   int setfirst = (this->prev == NULL ? 1 : 0);
   BddTree *next = this;
   int best = reorder_nodenum();
   
   if (this->next->next == NULL) /* Only two blocks left -> win2 swap */
   {
      blockdown(this);
      
      if (best < reorder_nodenum())
      {
	 blockdown(this->prev);
	 next = this->next;
      }
      else
      {
	 next = this;
	 if (setfirst)
	    *first = this->prev;
      }
   }
   else /* Real win3 swap */
   {
      int pos = 0;
      X(printf("%d: ", reorder_nodenum()));
      blockdown(this);  /* B A* C (4) */
      X(printf("A"));
      pos++;
      if (best > reorder_nodenum())
      {
	 X(printf("(%d)", reorder_nodenum()));
	 pos = 0;
	 best = reorder_nodenum();
      }
      
      blockdown(this);  /* B C A* (3) */
      X(printf("B"));
      pos++;
      if (best > reorder_nodenum())
      {
	 X(printf("(%d)", reorder_nodenum()));
	 pos = 0;
	 best = reorder_nodenum();
      }
      
      this = this->prev->prev;
      blockdown(this);  /* C B* A (2) */
      X(printf("C"));
      pos++;
      if (best > reorder_nodenum())
      {
	 X(printf("(%d)", reorder_nodenum()));
	 pos = 0;
	 best = reorder_nodenum();
      }
      
      blockdown(this);  /* C A B* (1) */
      X(printf("D"));
      pos++;
      if (best > reorder_nodenum())
      {
	 X(printf("(%d)", reorder_nodenum()));
	 pos = 0;
	 best = reorder_nodenum();
      }
      
      this = this->prev->prev;
      blockdown(this);  /* A C* B (0)*/
      X(printf("E"));
      pos++;
      if (best > reorder_nodenum())
      {
	 X(printf("(%d)", reorder_nodenum()));
	 pos = 0;
	 best = reorder_nodenum();
      }
      
      X(printf(" -> "));
      
      if (pos >= 1)  /* A C B -> C A* B */
      {
	 this = this->prev;
	 blockdown(this);
	 next = this;
	 if (setfirst)
	    *first = this->prev;
	 X(printf("a(%d)", reorder_nodenum()));
      }
      
      if (pos >= 2)  /* C A B -> C B A* */
      {
	 blockdown(this);
	 next = this->prev;
	 if (setfirst)
	    *first = this->prev->prev;
	 X(printf("b(%d)", reorder_nodenum()));
      }
      
      if (pos >= 3)  /* C B A -> B C* A */
      {
	 this = this->prev->prev;
	 blockdown(this);
	 next = this;
	 if (setfirst)
	    *first = this->prev;
	 X(printf("c(%d)", reorder_nodenum()));
      }
      
      if (pos >= 4)  /* B C A -> B A C* */
      {
	 blockdown(this);
	 next = this->prev;
	 if (setfirst)
	    *first = this->prev->prev;
	 X(printf("d(%d)", reorder_nodenum()));
      }
      
      if (pos >= 5)  /* B A C -> A B* C */
      {
	 this = this->prev->prev;
	 blockdown(this);
	 next = this;
	 if (setfirst)
	    *first = this->prev;
	 X(printf("e(%d)", reorder_nodenum()));
      }
      X(printf("\n"));
   }

   return next;
}


static BddTree *reorder_win3(BddTree *t)
{
   BddTree *this=t, *first=t;

   if (t == NULL)
      return t;

   if (verbose > 1)
      printf("Win3 start: %d nodes\n", reorder_nodenum());
   fflush(stdout);

   while (this->next != NULL)
   {
      this = reorder_swapwin3(this, &first);
      
      if (verbose > 1)
      {
	 printf(".");
	 fflush(stdout);
      }
   }
   
   if (verbose > 1)
      printf("\nWin3 end: %d nodes\n", reorder_nodenum());
   fflush(stdout);

   return first;
}


static BddTree *reorder_win3ite(BddTree *t)
{
   BddTree *this=t, *first=t;
   int lastsize;
   
   if (t == NULL)
      return t;
   
   if (verbose > 1)
      printf("Win3ite start: %d nodes\n", reorder_nodenum());

   do
   {
      lastsize = reorder_nodenum();
      this = first;
      
      while (this->next != NULL  &&  this->next->next != NULL)
      {
	 this = reorder_swapwin3(this, &first);

	 if (verbose > 1)
	 {
	    printf(".");
	    fflush(stdout);
	 }
      }

      if (verbose > 1)
	 printf(" %d nodes\n", reorder_nodenum());
   }
   while (reorder_nodenum() != lastsize);

   if (verbose > 1)
      printf("Win3ite end: %d nodes\n", reorder_nodenum());
   
   return first;
}


/*=== Reorder by sifting =============================================*/

/* Move a specific block up and down in the order and place at last in
   the best position
*/
static void reorder_sift_bestpos(BddTree *blk, int middlePos)
{
   int best = reorder_nodenum();
   int maxAllowed;
   int bestpos = 0;
   int dirIsUp = 1;
   int n;
   
   if (bddmaxnodesize > 0)
      maxAllowed = MIN(best/5+best, bddmaxnodesize-bddmaxnodeincrease-2);
   else
      maxAllowed = best/5+best;

      /* Determine initial direction */
   if (blk->pos > middlePos)
      dirIsUp = 0;

      /* Move block back and forth */
   for (n=0 ; n<2 ; n++)
   {
      int first = 1;
      
      if (dirIsUp)
      {
	 while (blk->prev != NULL  &&
		(reorder_nodenum() <= maxAllowed || first))
	 {
	    first = 0;
	    blockdown(blk->prev);
	    bestpos--;
	    
	    if (verbose > 1)
	    {
	       printf("-");
	       fflush(stdout);
	    }
	    
	    if (reorder_nodenum() < best)
	    {
	       best = reorder_nodenum();
	       bestpos = 0;

	       if (bddmaxnodesize > 0)
		  maxAllowed = MIN(best/5+best,
				   bddmaxnodesize-bddmaxnodeincrease-2);
	       else
		  maxAllowed = best/5+best;
	    }
	 }
      }
      else
      {
	 while (blk->next != NULL  &&
		(reorder_nodenum() <= maxAllowed  ||  first))
	 {
	    first = 0;
	    blockdown(blk);
	    bestpos++;
	    
	    if (verbose > 1)
	    {
	       printf("+");
	       fflush(stdout);
	    }
	    
	    if (reorder_nodenum() < best)
	    {
	       best = reorder_nodenum();
	       bestpos = 0;
	       
	       if (bddmaxnodesize > 0)
		  maxAllowed = MIN(best/5+best,
				   bddmaxnodesize-bddmaxnodeincrease-2);
	       else
		  maxAllowed = best/5+best;
	    }
	 }
      }
	 
      if (reorder_nodenum() > maxAllowed  &&  verbose > 1)
      {
	 printf("!");
	 fflush(stdout);
      }

      dirIsUp = !dirIsUp;
   }

      /* Move to best pos */
   while (bestpos < 0)
   {
      blockdown(blk);
      bestpos++;
   }
   while (bestpos > 0)
   {
      blockdown(blk->prev);
      bestpos--;
   }
}


/* Go through all blocks in a specific sequence and find best
   position for each of them
*/
static BddTree *reorder_sift_seq(BddTree *t, BddTree **seq, int num)
{
   BddTree *this;
   int n;
   
   if (t == NULL)
      return t;

   for (n=0 ; n<num ; n++)
   {
      long c2, c1 = clock();
   
      if (verbose > 1)
      {
	 printf("Sift ");
	 if (reorder_filehandler)
	    reorder_filehandler(stdout, seq[n]->id);
	 else
	    printf("%d", seq[n]->id);
	 printf(": ");
      }

      reorder_sift_bestpos(seq[n], num/2);

      if (verbose > 1)
	 printf("\n> %d nodes", reorder_nodenum());

      c2 = clock();
      if (verbose > 1)
	 printf(" (%.1f sec)\n", (float)(c2-c1)/CLOCKS_PER_SEC);
   }

      /* Find first block */
   for (this=t ; this->prev != NULL ; this=this->prev)
      /* nil */;

   return this;
}


/* Compare function for sorting sifting sequence
 */
static int siftTestCmp(const void *aa, const void *bb)
{
   const sizePair *a = (sizePair*)aa;
   const sizePair *b = (sizePair*)bb;

   if (a->val < b->val)
      return -1;
   if (a->val > b->val)
      return 1;
   return 0;
}


/* Find sifting sequence based on the number of nodes at each level
 */
static BddTree *reorder_sift(BddTree *t)
{
   BddTree *this, **seq;
   sizePair *p;
   int n, num;

   for (this=t,num=0 ; this!=NULL ; this=this->next)
      this->pos = num++;
   
   if ((p=NEW(sizePair,num)) == NULL)
      return t;
   if ((seq=NEW(BddTree*,num)) == NULL)
   {
      free(p);
      return t;
   }

   for (this=t,n=0 ; this!=NULL ; this=this->next,n++)
   {
      int v;

         /* Accumulate number of nodes for each block */
      p[n].val = 0;
      for (v=this->first ; v<=this->last ; v++)
	 p[n].val -= levels[v].nodenum;

      p[n].block = this;
   }

      /* Sort according to the number of nodes at each level */
   qsort(p, num, sizeof(sizePair), siftTestCmp);
   
      /* Create sequence */
   for (n=0 ; n<num ; n++)
      seq[n] = p[n].block;

      /* Do the sifting on this sequence */
   t = reorder_sift_seq(t, seq, num);
   
   free(seq);
   free(p);
   
   return t;
}


/* Do sifting iteratively until no more improvement can be found
 */
static BddTree *reorder_siftite(BddTree *t)
{
   BddTree *first=t;
   int lastsize;
   int c=1;
   
   if (t == NULL)
      return t;
   
   do
   {
      if (verbose > 1)
	 printf("Reorder %d\n", c++);
      
      lastsize = reorder_nodenum();
      first = reorder_sift(first);
   }
   while (reorder_nodenum() != lastsize);

   return first;
}


/*=== Random reordering (mostly for debugging and test ) =============*/

static BddTree *reorder_random(BddTree *t)
{
   BddTree *this;
   BddTree **seq;
   int n, num=0;

   if (t == NULL)
      return t;
   
   for (this=t ; this!=NULL ; this=this->next)
      num++;
   seq = NEW(BddTree*,num);
   for (this=t,num=0 ; this!=NULL ; this=this->next)
      seq[num++] = this;
   
   for (n=0 ; n<4*num ; n++)
   {
      int blk = random(num);
      if (seq[blk]->next != NULL)
	 blockdown(seq[blk]);
   }

      /* Find first block */
   for (this=t ; this->prev != NULL ; this=this->prev)
      /* nil */;

   free(seq);

   if (verbose)
      printf("Random order: %d nodes\n", reorder_nodenum());
   return this;
}


/*************************************************************************
  Swapping adjacent blocks
*************************************************************************/

static void blockdown(BddTree *left)
{
   BddTree *right = left->next;
   int n;
   int leftsize = left->last - left->first;
   int rightsize = right->last - right->first;
   int leftstart = bddvar2level[left->seq[0]];
   int *lseq = left->seq;
   int *rseq = right->seq;

      /* Move left past right */
   while (bddvar2level[lseq[0]] < bddvar2level[rseq[rightsize]])
   {
      for (n=0 ; n<leftsize ; n++)
      {
	 if (bddvar2level[lseq[n]]+1  !=  bddvar2level[lseq[n+1]]
	     && bddvar2level[lseq[n]]  <  bddvar2level[rseq[rightsize]])
	 {
	    reorder_vardown(lseq[n]);
	 }
      }

      if (bddvar2level[lseq[leftsize]] <  bddvar2level[rseq[rightsize]])
      {
	 reorder_vardown(lseq[leftsize]);
      }
   }

      /* Move right to where left started */
   while (bddvar2level[rseq[0]] > leftstart)
   {
      for (n=rightsize ; n>0 ; n--)
      {
	 if (bddvar2level[rseq[n]]-1 != bddvar2level[rseq[n-1]]
	     && bddvar2level[rseq[n]] > leftstart)
	 {
	    reorder_varup(rseq[n]);
	 }
      }

      if (bddvar2level[rseq[0]] > leftstart)
	 reorder_varup(rseq[0]);
   }

      /* Swap left and right data in the order */
   left->next = right->next;
   right->prev = left->prev;
   left->prev = right;
   right->next = left;

   if (right->prev != NULL)
      right->prev->next = right;
   if (left->next != NULL)
      left->next->prev = left;

   n = left->pos;
   left->pos = right->pos;
   right->pos = n;
}


/*************************************************************************
  Kernel reordering routines
*************************************************************************/

/*=== Garbage collection for reordering ================================*/

/* Note: Node may be marked
 */
static void addref_rec(int r, char *dep)
{
   if (r < 2)
      return;
   
   if (bddnodes[r].refcou == 0)
   {
      bddfreenum--;

         /* Detect variable dependencies for the interaction matrix */
      dep[VAR(r) & MARKHIDE] = 1;

         /* Make sure the nodenum field is updated. Used in the initial GBC */
      levels[VAR(r) & MARKHIDE].nodenum++;
      
      addref_rec(LOW(r), dep);
      addref_rec(HIGH(r), dep);
   }
   else
   {
      int n;
      
         /* Update (from previously found) variable dependencies
	  * for the interaction matrix */
      for (n=0 ; n<bddvarnum ; n++)
	 dep[n] |= imatrixDepends(iactmtx, VAR(r) & MARKHIDE, n);
   }
   
   INCREF(r);
}


static void addDependencies(char *dep)
{
   int n,m;

   for (n=0 ; n<bddvarnum ; n++)
   {
      for (m=n ; m<bddvarnum ; m++)
      {
	 if (dep[n]  &&  dep[m])
	 {
	    imatrixSet(iactmtx, n,m);
	    imatrixSet(iactmtx, m,n);
	 }
      }
   }
}


/* Make sure all nodes are recursively reference counted and store info about
   nodes that are refcou. externally. This info is used at last to revert
   to the standard GBC mode.
 */
static int mark_roots(void)
{
   char *dep = NEW(char,bddvarnum);
   int n;

   for (n=2,extrootsize=0 ; n<bddnodesize ; n++)
   {
         /* This is where we go from .level to .var!
	  * - Do NOT use the LEVEL macro here. */
      bddnodes[n].level = bddlevel2var[bddnodes[n].level];
      
      if (bddnodes[n].refcou > 0)
      {
	 SETMARK(n);
	 extrootsize++;
      }
   }
   
   if ((extroots=(int*)(malloc(sizeof(int)*extrootsize))) == NULL)
      return bdd_error(BDD_MEMORY);

   iactmtx = imatrixNew(bddvarnum);

   for (n=2,extrootsize=0 ; n<bddnodesize ; n++)
   {
      BddNode *node = &bddnodes[n];

      if (MARKEDp(node))
      {
	 UNMARKp(node);
	 extroots[extrootsize++] = n;

	 memset(dep,0,bddvarnum);
	 dep[VARp(node)] = 1;
	 levels[VARp(node)].nodenum++;
	 
	 addref_rec(LOWp(node), dep);
	 addref_rec(HIGHp(node), dep);

	 addDependencies(dep);
      }

      /* Make sure the hash field is empty. This saves a loop in the
	 initial GBC */
      node->hash = 0;
   }

   bddnodes[0].hash = 0;
   bddnodes[1].hash = 0;

   free(dep);
   return 0;
}


/* Now that all nodes are recursively reference counted we must make sure
   that the new hashing scheme is used AND that dead nodes are removed.
   This is also a good time to create the interaction matrix.
*/
static void reorder_gbc(void)
{
   int n;

   bddfreepos = 0;
   bddfreenum = 0;

      /* No need to zero all hash fields - this is done in mark_roots */
   
   for (n=bddnodesize-1 ; n>=2 ; n--)
   {
      register BddNode *node = &bddnodes[n];

      if (node->refcou > 0)
      {
	 register unsigned int hash;
	 
	 hash = NODEHASH(VARp(node), LOWp(node), HIGHp(node));
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
}


static void reorder_setLevellookup(void)
{
   int n;

   for (n=0 ; n<bddvarnum ; n++)
   {
#ifdef USERESIZE
      levels[n].maxsize = bddnodesize / bddvarnum;
      levels[n].start = n * levels[n].maxsize;
      levels[n].size = MIN(levels[n].maxsize, (levels[n].nodenum*5)/4);
#else
      levels[n].maxsize = bddnodesize / bddvarnum;
      levels[n].start = n * levels[n].maxsize;
      levels[n].size = levels[n].maxsize;
#endif

      if (levels[n].size >= 4)
	 levels[n].size = bdd_prime_lte(levels[n].size);
      
#if 0
      printf("L%3d: start %d, size %d, nodes %d\n", n, levels[n].start,
	     levels[n].size, levels[n].nodenum);
#endif
   }
}


static void reorder_rehashAll(void)
{
   int n;

   reorder_setLevellookup();
   bddfreepos = 0;

   for (n=bddnodesize-1 ; n>=0 ; n--)
      bddnodes[n].hash = 0;
   
   for (n=bddnodesize-1 ; n>=2 ; n--)
   {
      register BddNode *node = &bddnodes[n];

      if (node->refcou > 0)
      {
	 register unsigned int hash;
	 
	 hash = NODEHASH(VARp(node), LOWp(node), HIGHp(node));
	 node->next = bddnodes[hash].hash;
	 bddnodes[hash].hash = n;
      }
      else
      {
	 node->next = bddfreepos;
	 bddfreepos = n;
      }
   }
}


/*=== Unique table handling for reordering =============================*/

/* Note: rehashing must not take place during a makenode call. It is okay
   to resize the table, but *not* to rehash it.
 */
static int reorder_makenode(int var, int low, int high)
{
   register BddNode *node;
   register unsigned int hash;
   register int res;

#ifdef CACHESTATS
   bddcachestats.uniqueAccess++;
#endif
   
      /* Note: We know that low,high has a refcou greater than zero, so
	 there is no need to add reference *recursively* */
   
      /* check whether childs are equal */
   if (low == high)
   {
      INCREF(low);
      return low;
   }

      /* Try to find an existing node of this kind */
   hash = NODEHASH(var, low, high);
   res = bddnodes[hash].hash;
      
   while(res != 0)
   {
      if (LOW(res) == low  &&  HIGH(res) == high)
      {
#ifdef CACHESTATS
	 bddcachestats.uniqueHit++;
#endif
	 INCREF(res);
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
      
         /* Try to allocate more nodes - call noderesize without
	  * enabling rehashing.
          * Note: if ever rehashing is allowed here, then remember to
	  * update local variable "hash" */
      bdd_noderesize(0);
      resizedInMakenode = 1;

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
   levels[var].nodenum++;
   bddproduced++;
   bddfreenum--;
   
   node = &bddnodes[res];
   VARp(node) = var;
   LOWp(node) = low;
   HIGHp(node) = high;

      /* Insert node in hash chain */
   node->next = bddnodes[hash].hash;
   bddnodes[hash].hash = res;

      /* Make sure it is reference counted */
   node->refcou = 1;
   INCREF(LOWp(node));
   INCREF(HIGHp(node));
   
   return res;
}


/*=== Swapping two adjacent variables ==================================*/

/* Go through var 0 nodes. Move nodes that depends on var 1 to a separate
 * chain (toBeProcessed) and let the rest stay in the table.
 */
static int reorder_downSimple(int var0)
{
   int toBeProcessed = 0;
   int var1 = bddlevel2var[bddvar2level[var0]+1];
   int vl0 = levels[var0].start;
   int size0 = levels[var0].size;
   int n;

   levels[var0].nodenum = 0;
   
   for (n=0 ; n<size0 ; n++)
   {
      int r;

      r = bddnodes[n + vl0].hash;
      bddnodes[n + vl0].hash = 0;

      while (r != 0)
      {
	 BddNode *node = &bddnodes[r];
	 int next = node->next;

	 if (VAR(LOWp(node)) != var1  &&  VAR(HIGHp(node)) != var1)
	 {
 	       /* Node does not depend on next var, let it stay in the chain */
	    node->next = bddnodes[n+vl0].hash;
	    bddnodes[n+vl0].hash = r;
	    levels[var0].nodenum++;
	 }
	 else
	 {
   	       /* Node depends on next var - save it for later procesing */
	    node->next = toBeProcessed;
	    toBeProcessed = r;
#ifdef SWAPCOUNT
	    bddcachestats.swapCount++;
#endif
   
	 }

	 r = next;
      }
   }
   
   return toBeProcessed;
}


/* Now process all the var 0 nodes that depends on var 1.
 *
 * It is extremely important that no rehashing is done inside the makenode
 * calls, since this would destroy the toBeProcessed chain.
 */
static void reorder_swap(int toBeProcessed, int var0)
{
   int var1 = bddlevel2var[bddvar2level[var0]+1];
   
   while (toBeProcessed)
   {
      BddNode *node = &bddnodes[toBeProcessed];
      int next = node->next;
      int f0 = LOWp(node);
      int f1 = HIGHp(node);
      int f00, f01, f10, f11, hash;
      
         /* Find the cofactors for the new nodes */
      if (VAR(f0) == var1)
      {
	 f00 = LOW(f0);
	 f01 = HIGH(f0);
      }
      else
	 f00 = f01 = f0;
      
      if (VAR(f1) == var1)
      {
	 f10 = LOW(f1);
	 f11 = HIGH(f1);
      }
      else
	 f10 = f11 = f1;

         /* Note: makenode does refcou. */
      f0 = reorder_makenode(var0, f00, f10);
      f1 = reorder_makenode(var0, f01, f11);
      node = &bddnodes[toBeProcessed];  /* Might change in makenode */

         /* We know that the refcou of the grandchilds of this node
	  * is greater than one (these are f00...f11), so there is
	  * no need to do a recursive refcou decrease. It is also
	  * possible for the LOWp(node)/high nodes to come alive again,
	  * so deref. of the childs is delayed until the local GBC. */

      DECREF(LOWp(node));
      DECREF(HIGHp(node));
      
         /* Update in-place */
      VARp(node) = var1;
      LOWp(node) = f0;
      HIGHp(node) = f1;
	    
      levels[var1].nodenum++;
      
         /* Rehash the node since it got new childs */
      hash = NODEHASH(VARp(node), LOWp(node), HIGHp(node));
      node->next = bddnodes[hash].hash;
      bddnodes[hash].hash = toBeProcessed;

      toBeProcessed = next;
   }
}


/* Now go through the var 1 chains. The nodes live here have survived
 * the call to reorder_swap() and may stay in the chain.
 * The dead nodes are reclaimed.
 */
static void reorder_localGbc(int var0)
{
   int var1 = bddlevel2var[bddvar2level[var0]+1];
   int vl1 = levels[var1].start;
   int size1 = levels[var1].size;
   int n;

   for (n=0 ; n<size1 ; n++)
   {
      int hash = n+vl1;
      int r = bddnodes[hash].hash;
      bddnodes[hash].hash = 0;

      while (r)
      {
	 BddNode *node = &bddnodes[r];
	 int next = node->next;

	 if (node->refcou > 0)
	 {
	    node->next = bddnodes[hash].hash;
	    bddnodes[hash].hash = r;
	 }
	 else
	 {
	    DECREF(LOWp(node));
	    DECREF(HIGHp(node));
	    
	    LOWp(node) = -1;
	    node->next = bddfreepos; 
	    bddfreepos = r;
	    levels[var1].nodenum--;
	    bddfreenum++;
	 }

	 r = next;
      }
   }   
}




#ifdef USERESIZE

static void reorder_swapResize(int toBeProcessed, int var0)
{
   int var1 = bddlevel2var[bddvar2level[var0]+1];
   
   while (toBeProcessed)
   {
      BddNode *node = &bddnodes[toBeProcessed];
      int next = node->next;
      int f0 = LOWp(node);
      int f1 = HIGHp(node);
      int f00, f01, f10, f11;
      
         /* Find the cofactors for the new nodes */
      if (VAR(f0) == var1)
      {
	 f00 = LOW(f0);
	 f01 = HIGH(f0);
      }
      else
	 f00 = f01 = f0;
      
      if (VAR(f1) == var1)
      {
	 f10 = LOW(f1);
	 f11 = HIGH(f1);
      }
      else
	 f10 = f11 = f1;

         /* Note: makenode does refcou. */
      f0 = reorder_makenode(var0, f00, f10);
      f1 = reorder_makenode(var0, f01, f11);
      node = &bddnodes[toBeProcessed];  /* Might change in makenode */

         /* We know that the refcou of the grandchilds of this node
	  * is greater than one (these are f00...f11), so there is
	  * no need to do a recursive refcou decrease. It is also
	  * possible for the LOWp(node)/high nodes to come alive again,
	  * so deref. of the childs is delayed until the local GBC. */

      DECREF(LOWp(node));
      DECREF(HIGHp(node));
      
         /* Update in-place */
      VARp(node) = var1;
      LOWp(node) = f0;
      HIGHp(node) = f1;
	    
      levels[var1].nodenum++;
      
         /* Do not rehash yet since we are going to resize the hash table */
      
      toBeProcessed = next;
   }
}


static void reorder_localGbcResize(int toBeProcessed, int var0)
{
   int var1 = bddlevel2var[bddvar2level[var0]+1];
   int vl1 = levels[var1].start;
   int size1 = levels[var1].size;
   int n;

   for (n=0 ; n<size1 ; n++)
   {
      int hash = n+vl1;
      int r = bddnodes[hash].hash;
      bddnodes[hash].hash = 0;

      while (r)
      {
	 BddNode *node = &bddnodes[r];
	 int next = node->next;

	 if (node->refcou > 0)
	 {
	    node->next = toBeProcessed;
	    toBeProcessed = r;
	 }
	 else
	 {
	    DECREF(LOWp(node));
	    DECREF(HIGHp(node));
	    
	    LOWp(node) = -1;
	    node->next = bddfreepos; 
	    bddfreepos = r;
	    levels[var1].nodenum--;
	    bddfreenum++;
	 }

	 r = next;
      }
   }

      /* Resize */
   if (levels[var1].nodenum < levels[var1].size)
      levels[var1].size = MIN(levels[var1].maxsize, levels[var1].size/2);
   else
      levels[var1].size = MIN(levels[var1].maxsize, levels[var1].size*2);

   if (levels[var1].size >= 4)
      levels[var1].size = bdd_prime_lte(levels[var1].size);

      /* Rehash the remaining live nodes */
   while (toBeProcessed)
   {
      BddNode *node = &bddnodes[toBeProcessed];
      int next = node->next;
      int hash = NODEHASH(VARp(node), LOWp(node), HIGHp(node));
	 
      node->next = bddnodes[hash].hash;
      bddnodes[hash].hash = toBeProcessed;

      toBeProcessed = next;
   }   
}

#endif /* USERESIZE */


static int reorder_varup(int var)
{
   if (var < 0  ||  var >= bddvarnum)
      return bdd_error(BDD_VAR);
   if (bddvar2level[var] == 0)
      return 0;
   return reorder_vardown( bddlevel2var[bddvar2level[var]-1]);
}


#if 0
static void sanitycheck(void)
{
   int n,v,r;
   int cou=0;
   
   for (v=0 ; v<bddvarnum ; v++)
   {
      int vcou=0;
      
      for (n=0 ; n<levels[v].size ; n++)
      {
	 r = bddnodes[n+levels[v].start].hash;
	 
	 while (r)
	 {
	    assert(VAR(r) == v);
	    r = bddnodes[r].next;
	    cou++;
	    vcou++;
	 }
      }

      assert(vcou == levels[v].nodenum);
   }

   for (n=2 ; n<bddnodesize ; n++)
   {
      if (bddnodes[n].refcou > 0)
      {
	 assert(LEVEL(n) < LEVEL(LOW(n)));
	 assert(LEVEL(n) < LEVEL(HIGH(n)));
	 cou--;
      }
   }

   assert(cou == 0);
}
#endif

static int reorder_vardown(int var)
{
   int n, level;

   if (var < 0  ||  var >= bddvarnum)
      return bdd_error(BDD_VAR);
   if ((level=bddvar2level[var]) >= bddvarnum-1)
      return 0;

   resizedInMakenode = 0;
   
   if (imatrixDepends(iactmtx, var, bddlevel2var[level+1]))
   {
      int toBeProcessed = reorder_downSimple(var);
#ifdef USERESIZE
      levelData *l = &levels[var];
      
      if (l->nodenum < (l->size)/3  ||
	  l->nodenum >= (l->size*3)/2  &&  l->size < l->maxsize)
      {
	 reorder_swapResize(toBeProcessed, var);
	 reorder_localGbcResize(toBeProcessed, var);
      }
      else
#endif
      {
	 reorder_swap(toBeProcessed, var);
	 reorder_localGbc(var);
      }
   }
   
      /* Swap the var<->level tables */
   n = bddlevel2var[level];
   bddlevel2var[level] = bddlevel2var[level+1];
   bddlevel2var[level+1] = n;
   
   n = bddvar2level[var];
   bddvar2level[var] = bddvar2level[ bddlevel2var[level] ];
   bddvar2level[ bddlevel2var[level] ] = n;
   
      /* Update all rename pairs */
   bdd_pairs_vardown(level);

   if (resizedInMakenode)
      reorder_rehashAll();

   return 0;
}


/*************************************************************************
  User reordering interface
*************************************************************************/

/*
NAME    {* bdd\_swapvar *}
SECTION {* reorder *}
SHORT   {* Swap two BDD variables *}
PROTO   {* int bdd_swapvar(int v1, int v2) *}
DESCR   {* Use {\tt bdd\_swapvar} to swap the position (in the current
           variable order) of the two BDD
           variables {\tt v1} and {\tt v2}. There are no constraints on the
	   position of the two variables before the call. This function may
	   {\em not} be used together with user defined variable blocks.
	   The swap is done by a series of adjacent variable swaps and
	   requires the whole node table to be rehashed twice for each call
	   to {\tt bdd\_swapvar}. It should therefore not be used were
	   efficiency is a major concern. *}
RETURN  {* Zero on succes and a negative error code otherwise. *}
ALSO    {* bdd\_reorder, bdd\_addvarblock *}
*/
int bdd_swapvar(int v1, int v2)
{
   int l1, l2;

      /* Do not swap when variable-blocks are used */
   if (vartree != NULL)
      return bdd_error(BDD_VARBLK);
	 
      /* Don't bother swapping x with x */
   if (v1 == v2)
      return 0;

      /* Make sure the variable exists */
   if (v1 < 0  ||  v1 >= bddvarnum  ||  v2 < 0  ||  v2 >= bddvarnum)
      return bdd_error(BDD_VAR);

   l1 = bddvar2level[v1];
   l2 = bddvar2level[v2];

      /* Make sure v1 is before v2 */
   if (l1 > l2)
   {
      int tmp = v1;
      v1 = v2;
      v2 = tmp;
      l1 = bddvar2level[v1];
      l2 = bddvar2level[v2];
   }

   reorder_init();
   
      /* Move v1 to v2's position */
   while (bddvar2level[v1] < l2)
      reorder_vardown(v1);

      /* Move v2 to v1's position */
   while (bddvar2level[v2] > l1)
      reorder_varup(v2);

   reorder_done();
   
   return 0;
}


void bdd_default_reohandler(int prestate)
{
   static long c1;

   if (verbose > 0)
   {
      if (prestate)
      {
	 printf("Start reordering\n");
	 c1 = clock();
      }
      else
      {
	 long c2 = clock();
	 printf("End reordering. Went from %d to %d nodes (%.1f sec)\n",
		usednum_before, usednum_after, (float)(c2-c1)/CLOCKS_PER_SEC);
      }
   }
}


/*
NAME    {* bdd\_disable\_reorder *}
SECTION {* reorder *}
SHORT   {* Disable automatic reordering *}
PROTO   {* void bdd_disable_reorder(void) *}
DESCR   {* Disables automatic reordering until {\tt bdd\_enable\_reorder}
           is called. Reordering is enabled by default as soon as any variable
	   blocks have been defined. *}
ALSO    {* bdd\_enable\_reorder *}
*/
void bdd_disable_reorder(void)
{
   reorderdisabled = 1;
}


/*
NAME    {* bdd\_enable\_reorder *}
SECTION {* reorder *}
SHORT   {* Enables automatic reordering *}
PROTO   {* void bdd_enable_reorder(void) *}
DESCR   {* Re-enables reordering after a call to {\tt bdd\_disable\_reorder}. *}
ALSO    {* bdd\_disable\_reorder *}
*/
void bdd_enable_reorder(void)
{
   reorderdisabled = 0;
}


int bdd_reorder_ready(void)
{
   if (bddreordermethod == BDD_REORDER_NONE  ||  vartree == NULL  ||
       bddreordertimes == 0  ||  reorderdisabled)
      return 0;
   return 1;
}

   
void bdd_reorder_auto(void)
{
   if (!bdd_reorder_ready)
      return;
   
   if (reorder_handler != NULL)
      reorder_handler(1);

   bdd_reorder(bddreordermethod);
   bddreordertimes--;
   
   if (reorder_handler != NULL)
      reorder_handler(0);
}


static int reorder_init(void)
{
   int n;

   if ((levels=NEW(levelData,bddvarnum)) == NULL)
      return -1;
   
   for (n=0 ; n<bddvarnum ; n++)
   {
      levels[n].start = -1;
      levels[n].size = 0;
      levels[n].nodenum = 0;
   }
   
      /* First mark and recursive refcou. all roots and childs. Also do some
       * setup here for both setLevellookup and reorder_gbc */
   if (mark_roots() < 0)
      return -1;

      /* Initialize the hash tables */
   reorder_setLevellookup();

      /* Garbage collect and rehash to new scheme */
   reorder_gbc();

   return 0;
}


static void reorder_done(void)
{
   int n;
   
   for (n=0 ; n<extrootsize ; n++)
      SETMARK(extroots[n]);
   for (n=2 ; n<bddnodesize ; n++)
   {
      if (MARKED(n))
	 UNMARK(n);
      else
	 bddnodes[n].refcou = 0;

         /* This is where we go from .var to .level again!
	  * - Do NOT use the LEVEL macro here. */
      bddnodes[n].level = bddvar2level[bddnodes[n].level];
   }

#if 0
   for (n=0 ; n<bddvarnum ; n++)
      printf("%3d\n", bddlevel2var[n]);
   printf("\n");
#endif
   
#if 0
   for (n=0 ; n<bddvarnum ; n++)
      printf("%3d: %4d nodes , %4d entries\n", n, levels[n].nodenum,
	     levels[n].size);
#endif
   free(extroots);
   free(levels);
   imatrixDelete(iactmtx);
   bdd_gbc();
}


static int varseqCmp(const void *aa, const void *bb)
{
   int a = bddvar2level[*((const int*)aa)];
   int b = bddvar2level[*((const int*)bb)];

   if (a < b)
      return -1;
   if (a > b)
      return 1;
   return 0;
}


static BddTree *reorder_block(BddTree *t, int method)
{
   BddTree *this;
   
   if (t == NULL)
      return NULL;

   if (t->fixed == BDD_REORDER_FREE  &&  t->nextlevel!=NULL)
   {
      switch(method)
      {
      case BDD_REORDER_WIN2:
	 t->nextlevel = reorder_win2(t->nextlevel);
	 break;
      case BDD_REORDER_WIN2ITE:
	 t->nextlevel = reorder_win2ite(t->nextlevel);
	 break;
      case BDD_REORDER_SIFT:
	 t->nextlevel = reorder_sift(t->nextlevel);
	 break;
      case BDD_REORDER_SIFTITE:
	 t->nextlevel = reorder_siftite(t->nextlevel);
	 break;
      case BDD_REORDER_WIN3:
	 t->nextlevel = reorder_win3(t->nextlevel);
	 break;
      case BDD_REORDER_WIN3ITE:
	 t->nextlevel = reorder_win3ite(t->nextlevel);
	 break;
      case BDD_REORDER_RANDOM:
	 t->nextlevel = reorder_random(t->nextlevel);
	 break;
      }
   }

   for (this=t->nextlevel ; this ; this=this->next)
      reorder_block(this, method);

   if (t->seq != NULL)
      qsort(t->seq, t->last-t->first+1, sizeof(int), varseqCmp);
	 
   return t;
}


/*
NAME    {* bdd\_reorder *}
SECTION {* reorder *}
SHORT   {* start dynamic reordering *}
PROTO   {* void bdd_reorder(int method) *}
DESCR   {* This function initiates dynamic reordering using the heuristic
           defined by {\tt method}, which may be one of the following
	   \begin{description}
	     \item {\tt BDD\_REORDER\_WIN2}\\
	       Reordering using a sliding window of size 2. This algorithm
	       swaps two adjacent variable blocks and if this results in
	       more nodes then the two blocks are swapped back again.
	       Otherwise the result is kept in the variable order. This is
	       then repeated for all variable blocks.
	     \item {\tt BDD\_REORDER\_WIN2ITE}\\
	       The same as above but the process is repeated until no further
	       progress is done. Usually a fast and efficient method.
	     \item {\tt BDD\_REORDER\_WIN3}\\
	       The same as above but with a window size of 3.
	     \item {\tt BDD\_REORDER\_WIN2ITE}\\
	       The same as above but with a window size of 3.
	     \item {\tt BDD\_REORDER\_SIFT}\\
	       Reordering where each block is moved through all possible
	       positions. The best of these is then used as the new position.
	       Potentially a very slow but good method.
	     \item {\tt BDD\_REORDER\_SIFTITE}\\
	       The same as above but the process is repeated until no further
	       progress is done. Can be extremely slow.
	     \item {\tt BDD\_REORDER\_RANDOM}\\
	       Mostly used for debugging purpose, but may be usefull for
	       others. Selects a random position for each variable.
	   \end{description}
	   *}
ALSO    {* bdd\_autoreorder, bdd\_reorder\_verbose, bdd\_addvarblock, bdd\_clrvarblocks *}
*/
void bdd_reorder(int method)
{
   BddTree *top;
   int savemethod = bddreordermethod;
   int savetimes = bddreordertimes;
   
   bddreordermethod = method;
   bddreordertimes = 1;

   if ((top=bddtree_new(-1)) == NULL)
      return;
   if (reorder_init() < 0)
      return;

   usednum_before = bddnodesize - bddfreenum;
   
   top->first = 0;
   top->last = bdd_varnum()-1;
   top->fixed = 0;
   top->next = NULL;
   top->nextlevel = vartree;

   reorder_block(top, method);
   vartree = top->nextlevel;
   free(top);
   
   usednum_after = bddnodesize - bddfreenum;
   
   reorder_done();
   bddreordermethod = savemethod;
   bddreordertimes = savetimes;
}


/*
NAME    {* bdd\_reorder\_gain *}
SECTION {* reorder *}
SHORT   {* Calculate the gain in size after a reordering *}
PROTO   {* int bdd_reorder_gain(void) *}
DESCR   {* Returns the gain in percent of the previous number of used
           nodes. The value returned is
	   \[ (100 * (A - B)) / A \]
	   Where $A$ is previous number of used nodes and $B$ is current
	   number of used nodes.
	*}
*/
int bdd_reorder_gain(void)
{
   if (usednum_before == 0)
      return 0;
   
   return (100*(usednum_before - usednum_after)) / usednum_before;
}


/*
NAME    {* bdd\_reorder\_hook *}
SECTION {* reorder *}
SHORT   {* sets a handler for automatic reorderings *}
PROTO   {* bddinthandler bdd_reorder_hook(bddinthandler handler) *}
DESCR   {* Whenever automatic reordering is done, a check is done to see
           if the user has supplied a handler for that event. If so then
	   it is called with the argument {\tt prestate} being 1 if the
	   handler is called immediately {\em before} reordering and
	   {\tt prestate} being 0 if it is called immediately after.
	   The default handler is
	   {\tt bdd\_default\_reohandler} which will print information
	   about the reordering.

	   A typical handler could look like this:
	   \begin{verbatim}
void reorderhandler(int prestate)
{
   if (prestate)
      printf("Start reordering");
   else
      printf("End reordering");
}
\end{verbatim} *}
RETURN  {* The previous handler *}
ALSO    {* bdd\_reorder, bdd\_autoreorder, bdd\_resize\_hook *}
*/
bddinthandler bdd_reorder_hook(bddinthandler handler)
{
   bddinthandler tmp = reorder_handler;
   reorder_handler = handler;
   return tmp;
}


/*
NAME    {* bdd\_blockfile\_hook *}
SECTION {* reorder *}
SHORT   {* Specifies a printing callback handler *}
PROTO   {* bddfilehandler bdd_blockfile_hook(bddfilehandler handler) *}
DESCR   {* A printing callback handler is used to convert the variable
           block identifiers into something readable by the end user. Use
	   {\tt bdd\_blockfile\_hook} to pass a handler to BuDDy. A typical
	   handler could look like this:
\begin{verbatim}
void printhandler(FILE *o, int block)
{
   extern char **blocknames;
   fprintf(o, "%s", blocknames[block]);
}
\end{verbatim}
           \noindent
           The handler is then called from {\tt bdd\_printorder} and
	   {\tt bdd\_reorder} (depending on the verbose level) with
           the block numbers returned by {\tt bdd\_addvarblock} as arguments.
	   No default handler is supplied. The argument {\tt handler} may be
	   NULL if no handler is needed. *}
RETURN  {* The old handler *}
ALSO    {* bdd\_printorder *}
*/
bddfilehandler bdd_blockfile_hook(bddfilehandler handler)
{
   bddfilehandler tmp = reorder_filehandler;
   reorder_filehandler = handler;
   return tmp;
}


/*
NAME    {* bdd\_autoreorder *}
EXTRA   {* bdd\_autoreorder\_times *}
SECTION {* reorder *}
SHORT   {* enables automatic reordering *}
PROTO   {* int bdd_autoreorder(int method)
int bdd_autoreorder_times(int method, int num) *}
DESCR   {* Enables automatic reordering using {\tt method} as the reordering
           method. If {\tt method} is {\tt BDD\_REORDER\_NONE} then
           automatic reordering is disabled. Automatic
	   reordering is done every time the number of active nodes in the
	   node table has been doubled and works by interrupting the current
	   BDD operation, doing the reordering and the retrying the operation.

	   In the second form the argument {\tt num} specifies the allowed
	   number of reorderings. So if for example a "one shot" reordering
	   is needed, then the {\tt num} argument would be set to one.

	   Values for {\tt method} can be found under {\tt bdd\_reorder}.
	   *}
RETURN  {* Returns the old value of {\tt method} *}
ALSO    {* bdd\_reorder *}
*/
int bdd_autoreorder(int method)
{
   int tmp = bddreordermethod;
   bddreordermethod = method;
   bddreordertimes = -1;
   return tmp;
}


int bdd_autoreorder_times(int method, int num)
{
   int tmp = bddreordermethod;
   bddreordermethod = method;
   bddreordertimes = num;
   return tmp;
}


/*
NAME    {* bdd\_var2level *}
SECTION {* reorder *}
SHORT   {* Fetch the level of a specific BDD variable *}
PROTO   {* int bdd_var2level(int var) *}
DESCR   {* Returns the position of the variable {\tt var} in the current
           variable order. *}
ALSO    {* bdd\_reorder, bdd\_level2var *}
*/
int bdd_var2level(int var)
{
   if (var < 0  ||  var >= bddvarnum)
      return bdd_error(BDD_VAR);

   return bddvar2level[var];
}


/*
NAME    {* bdd\_level2var *}
SECTION {* reorder *}
SHORT   {* Fetch the variable number of a specific level *}
PROTO   {* int bdd_level2var(int level) *}
DESCR   {* Returns the variable placed at position {\tt level} in the
           current variable order. *}
ALSO    {* bdd\_reorder, bdd\_var2level *}
*/
int bdd_level2var(int level)
{
   if (level < 0  ||  level >= bddvarnum)
      return bdd_error(BDD_VAR);

   return bddlevel2var[level];
}


/*
NAME    {* bdd\_getreorder\_times *}
SECTION {* reorder *}
SHORT   {* Fetch the current number of allowed reorderings *}
PROTO   {* int bdd_getreorder_times(void) *}
DESCR   {* Returns the current number of allowed reorderings left. This
           value can be defined by {\tt bdd\_autoreorder\_times}. *}
ALSO    {* bdd\_reorder\_times, bdd\_getreorder\_method *}
*/
int bdd_getreorder_times(void)
{
   return bddreordertimes;
}


/*
NAME    {* bdd\_getreorder\_method *}
SECTION {* reorder *}
SHORT   {* Fetch the current reorder method *}
PROTO   {* int bdd_getreorder_method(void) *}
DESCR   {* Returns the current reorder method as defined by
           {\tt bdd\_autoreorder}. *}
ALSO    {* bdd\_reorder, bdd\_getreorder\_times *}
*/
int bdd_getreorder_method(void)
{
   return bddreordermethod;
}


/*
NAME    {* bdd\_reorder\_verbose *}
SECTION {* reorder *}
SHORT   {* enables verbose information about reorderings *}
PROTO   {* int bdd_reorder_verbose(int v) *}
DESCR   {* With {\tt bdd\_reorder\_verbose} it is possible to set the level
           of information which should be printed during reordering. A value
	   of zero means no information, a value of one means some information
	   and any greater value will result in a lot of reordering
	   information. The default value is zero. *}
RETURN  {* The old verbose level *}
ALSO    {* bdd\_reorder *}
*/
int bdd_reorder_verbose(int v)
{
   int tmp = verbose;
   verbose = v;
   return tmp;
}


/*
NAME    {* bdd\_reorder\_probe *}
SECTION {* reorder *}
SHORT   {* Define a handler for minimization of BDDs *}
PROTO   {* bddsizehandler bdd_reorder_probe(bddsizehandler handler) *}
DESCR   {* Reordering is typically done to minimize the global number of
           BDD nodes in use, but it may in some cases be usefull to minimize
	   with respect to a specific BDD. With {\tt bdd\_reorder\_probe} it
	   is possible to define a callback function that calculates the
	   size of a specific BDD (or anything else in fact). This handler
	   will then be called by the reordering functions to get the current
	   size information. A typical handle could look like this:
\begin{verbatim}
int sizehandler(void)
{
   extern BDD mybdd;
   return bdd_nodecount(mybdd);
}
\end{verbatim}
	   No default handler is supplied. The argument {\tt handler} may be
	   NULL if no handler is needed. *}
	   *}
RETURN  {* The old handler *}
ALSO    {* bdd\_reorder *}
*/
bddsizehandler bdd_reorder_probe(bddsizehandler handler)
{
   bddsizehandler old = reorder_nodenum;
   if (handler == NULL)
      return reorder_nodenum;
   reorder_nodenum = handler;
   return old;
}


/*
NAME    {* bdd\_clrvarblocks *}
SECTION {* reorder *}
SHORT   {* clears all variable blocks *}
PROTO   {* void bdd_clrvarblocks(void) *}
DESCR   {* Clears all the variable blocks that has been defined by calls
           to bdd\_addvarblock. *}
ALSO    {* bdd\_addvarblock *}
*/
void bdd_clrvarblocks(void)
{
   bddtree_del(vartree);
   vartree = NULL;
   blockid = 0;
}


/*
NAME    {* bdd\_addvarblock *}
EXTRA   {* bdd\_intaddvarblock *}
SECTION {* reorder *}
SHORT   {* adds a new variable block for reordering *}
PROTO   {* int bdd_addvarblock(BDD var, int fixed)
int bdd_intaddvarblock(int first, int last, int fixed) *}
DESCR   {* Creates a new variable block with the variables in the variable
           set {\tt var}. The variables in {\tt var} must be contiguous.
	   In the second form the argument {\tt first} is the first variable
	   included in the block and {\tt last} is the last variable included
	   in the block. This order does not depend on current variable
	   order.

	   The variable blocks are ordered as a tree, with the largest
	   ranges at top and the smallest at the bottom. Example: Assume
	   the block 0-9 is added as the first block and then the block 0-6.
	   This yields the 0-9 block at the top, with the 0-6 block as a
	   child. If now the block 2-4 was added, it would become a child
	   of the 0-6 block. A block of 0-8 would be a child of the 0-9
	   block and have the 0-6 block as a child. Partially overlapping
	   blocks are not allowed.

	   The {\tt fixed} parameter sets the block to be fixed (no
	   reordering of its child blocks is allowed) or free, using
	   the constants {\tt BDD\_REORDER\_FIXED} and {\tt
	   BDD\_REORDER\_FREE}.  Reordering is always done on the top
	   most blocks first and then recursively downwards.

	   The return value is an integer that can be used to identify
	   the block later on - with for example {\tt bdd\_blockfile\_hook}.
	   The values returned will be in the sequence $0,1,2,3,\ldots$.
	   *}
RETURN  {* A non-negative identifier on success, otherwise a negative error code. *}
ALSO {* bdd\_varblockall, fdd\_intaddvarblock, bdd\_clrvarblocks *} */
int bdd_addvarblock(BDD b, int fixed)
{
   BddTree *t;
   int n, *v, size;
   int first, last;
   
   if ((n=bdd_scanset(b, &v, &size)) < 0)
      return n;
   if (size < 1)
      return bdd_error(BDD_VARBLK);

   first = last = v[0];
   
   for (n=0 ; n<size ; n++)
   {
      if (v[n] < first)
	 first = v[n];
      if (v[n] > last)
	 last = v[n];
   }

   if ((t=bddtree_addrange(vartree, first,last, fixed,blockid)) == NULL)
      return bdd_error(BDD_VARBLK);
   
   vartree = t;
   return blockid++;
}


int bdd_intaddvarblock(int first, int last, int fixed)
{
   BddTree *t;

   if (first < 0  ||  first >= bddvarnum  ||  last < 0  ||  last >= bddvarnum)
      return bdd_error(BDD_VAR);
   
   if ((t=bddtree_addrange(vartree, first,last, fixed,blockid)) == NULL)
      return bdd_error(BDD_VARBLK);

   vartree = t;
   return blockid++;
}


/*
NAME    {* bdd\_varblockall *}
SECTION {* reorder *}
SHORT   {* add a variable block for all variables *}
PROTO   {* void bdd_varblockall(void) *}
DESCR   {* Adds a variable block for all BDD variables declared so far.
           Each block contains one variable only. More variable blocks
	   can be added later with the use of {\tt bdd\_addvarblock} --
	   in this case the tree of variable blocks will have the blocks
	   of single variables as the leafs. *}
ALSO    {* bdd\_addvarblock, bdd\_intaddvarblock *}
*/
void bdd_varblockall(void)
{
   int n;

   for (n=0 ; n<bddvarnum ; n++)
      bdd_intaddvarblock(n,n,1);
}


/*
NAME    {* bdd\_printorder *}
SECTION {* reorder *}
SHORT   {* prints the current order *}
PROTO   {* void bdd_printorder(void)
bdd_fprint_order(FILE *f)*}
DESCR   {* Prints an indented list of the variable blocks, showing the top
           most blocks to the left and the lower blocks to the right.
	   Example:\\
	   \begin{verbatim}
  2{
     0
     1
  2}
  3
  4
\end{verbatim}
           This shows 5 variable blocks. The first one added is block zero,
	   which is on the same level as block one. These two blocks are then
	   sub-blocks of block two and block two is on the same level as
	   block three and four. The numbers are the identifiers returned
	   from {\tt bdd\_addvarblock}. The block levels depends on the
	   variables included in the blocks.
	   *}
ALSO    {* bdd\_reorder, bdd\_addvarblock *}
*/
void bdd_printorder(void)
{
   bdd_fprintorder(stdout);
}


/*
NAME    {* bdd\_setvarorder *}
SECTION {* reorder *}
SHORT   {* set a specific variable order *}
PROTO   {* void bdd_setvarorder(int *neworder) *}
DESCR   {* This function sets the current variable order to be the one
           defined by {\tt neworder}. The parameter {\tt neworder} is
	   interpreted as a sequence of variable indecies and the new
	   variable order is exactly this sequence. The array {\em must}
	   contain all the variables defined so far. If for instance the
	   current number of variables is 3 and {\tt neworder} contains
	   $[1,0,2]$ then the new variable order is $v_1 < v_0 < v_2$. *}
ALSO    {* bdd\_reorder, bdd\_printorder *}
*/
void bdd_setvarorder(int *neworder)
{
   int level;

      /* Do not set order when variable-blocks are used */
   if (vartree != NULL)
   {
      bdd_error(BDD_VARBLK);
      return;
   }
   
   reorder_init();
   
   for (level=0 ; level<bddvarnum ; level++)
   {
      int lowvar = neworder[level];

      while (bddvar2level[lowvar] > level)
	 reorder_varup(lowvar);
   }
   
   reorder_done();
}


static void print_order_rec(FILE *o, BddTree *t, int level)
{
   if (t == NULL)
      return;

   if (t->nextlevel)
   {
      fprintf(o, "%*s", level*3, "");
      if (reorder_filehandler)
	 reorder_filehandler(o,t->id);
      else
	 fprintf(o, "%3d", t->id);
      fprintf(o, "{\n");
      
      print_order_rec(o, t->nextlevel, level+1);
      
      fprintf(o, "%*s", level*3, "");
      if (reorder_filehandler)
	 reorder_filehandler(o,t->id);
      else
	 fprintf(o, "%3d", t->id);
      fprintf(o, "}\n");
      
      print_order_rec(o, t->next, level);
   }
   else
   {
      fprintf(o, "%*s", level*3, "");
      if (reorder_filehandler)
	 reorder_filehandler(o,t->id);
      else
	 fprintf(o, "%3d", t->id);
      fprintf(o, "\n");
      
      print_order_rec(o, t->next, level);
   }
}



void bdd_fprintorder(FILE *ofile)
{
   print_order_rec(ofile, vartree, 0);
}



/* EOF */
