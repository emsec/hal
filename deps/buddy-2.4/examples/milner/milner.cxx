#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bdd.h"

int N;                // Number of cyclers
bdd normvar;          // Current state variables
bdd primvar;          // Next state variables
bddPair *renamepair;  // Variable pairs for renaming


/* Build a BDD expressing that all other variables than 'z' is unchanged.
 */
bdd A(bdd* x, bdd* y, int z)
{
   bdd res = bddtrue;
   int i;
   
   for(i=0 ; i<N ; i++)
      if(i != z)
	 res &= bdd_apply(x[i],y[i],bddop_biimp);
   
   return res;
}


/* Loop through all cyclers and create the transition relation for each
   of them.
*/
bdd transitions(bdd* t, bdd* tp, bdd* h, bdd* hp, bdd* c, bdd* cp)
{
   int i;
   bdd P;            // Cycler i's handling of the token
   bdd E;            // Cycler i's handling of it's task
   bdd T = bddfalse; // The monolithic transition relation
   
   for(i=0 ; i<N ; i++)
   {
      P = ((c[i]>cp[i]) & (tp[i]>t[i]) & hp[i] & A(c,cp,i)
	   & A(t,tp,i) & A(h,hp,i))
	 | ((h[i]>hp[i]) & cp[(i+1)%N] & A(c,cp,(i+1)%N) & A(h,hp,i)
	    & A(t,tp,N));
      
      E = t[i] & !tp[i] & A(t,tp,i) & A(h,hp,N) & A(c,cp,N);
      
      T |= P | E;
   }
   
   return T;
}


/* Create a BDD for the initial state.
 */
bdd initial_state(bdd* t, bdd* h, bdd* c)
{
   int i;
   bdd I = c[0] & !h[0] & !t[0];
   
   for(i=1; i<N; i++)
      I &= !c[i] & !h[i] & !t[i];
   
   return I;
}


/* Calculate the reachable states.
 */
bdd reachable_states(bdd I, bdd T)
{
   bdd R = I, // Reachable state space
       prevR, // Previously reached state space
       tmp;

   do
   {
      prevR = R;

#if 0
         // Apply and exist as different operations => slow
      tmp = T & bx;
      tmp = bdd_exist(tmp, normvar);
#else
         // Apply and exist as one operation => fast
      tmp = bdd_appex(R, T, bddop_and, normvar);
#endif
      tmp = bdd_replace(tmp, renamepair);
      R |= tmp;
   }
   while(prevR != R);
   
   return R;
}


int main(int argc, char** argv)
{
   using namespace std ;
   int n;
   if(argc < 2)
   {
      cerr << "usage: milner N\n";
      cerr << "       N  number of cyclers\n";
      exit(1);
   }
   
   N = atoi(argv[1]);
   if (N <= 0)
   {
      cerr << "The number of cyclers must be more than zero\n";
      exit(2);
   }
   
   bdd_init(500000, 50000);
   bdd_setvarnum(N*6);
   
   bdd* c  = new bdd[N];
   bdd* cp = new bdd[N];
   bdd* t  = new bdd[N];
   bdd* tp = new bdd[N];
   bdd* h  = new bdd[N];
   bdd* hp = new bdd[N];
   
   int *nvar = new int[N*3];
   int *pvar = new int[N*3];
   
   for (n=0 ; n<N*3 ; n++)
   {
      nvar[n] = n*2;
      pvar[n] = n*2+1;
   }
   
   normvar = bdd_makeset(nvar, N*3);
   primvar = bdd_makeset(pvar, N*3);
   renamepair = bdd_newpair();
   bdd_setpairs(renamepair, pvar, nvar, N*3);
   
   for (n=0 ; n<N ; n++)
   {
      c[n]  = bdd_ithvar(n*6);
      cp[n] = bdd_ithvar(n*6+1);
      t[n]  = bdd_ithvar(n*6+2);
      tp[n] = bdd_ithvar(n*6+3);
      h[n]  = bdd_ithvar(n*6+4);
      hp[n] = bdd_ithvar(n*6+5);

#if 0
      bdd_addvarblock(c[n], BDD_REORDER_FIXED);
      bdd_addvarblock(cp[n], BDD_REORDER_FIXED);
      bdd_addvarblock(t[n], BDD_REORDER_FIXED);
      bdd_addvarblock(tp[n], BDD_REORDER_FIXED);
      bdd_addvarblock(h[n], BDD_REORDER_FIXED);
      bdd_addvarblock(hp[n], BDD_REORDER_FIXED);
      bdd_addvarblock(c[n] & cp[n] & t[n] & tp[n] & h[n] & hp[n],
		      BDD_REORDER_FREE);
#endif
   }

   bdd I = initial_state(t,h,c);
   bdd T = transitions(t,tp,h,hp,c,cp);
   bdd R = reachable_states(I,T);
   
   bddStat s;
   bdd_stats(&s);

   cout << "SatCount R = " << bdd_satcount(R) << endl;
   cout << "Calc       = " << (double)N*pow(2.0,1.0+N)*pow(2.0,3.0*N) << endl;
   cout << "Nodes      = " << s.produced << endl;
   cout << endl << "Number of nodes in T is " << bdd_nodecount( T ) << endl;
   cout << "Number of nodes in R is " << bdd_nodecount( R ) << endl << endl;

   //bdd_printstat();
   cout << "Nodenum: " << bdd_getnodenum() << endl;
   bdd_done();
   return 0;
}

