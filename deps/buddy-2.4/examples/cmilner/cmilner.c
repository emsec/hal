#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bdd.h"

int N;          /* Number of cyclers */
int *normvar;   /* Current state variables */
int *primvar;   /* Next state variables */

bdd normvarset;
bddPair *pairs;

bdd A(bdd* x, bdd* y, int z)
{
   bdd res = bddtrue, tmp1, tmp2;
   int i;
   
   for(i=0 ; i<N ; i++)
      if(i != z)
      {
	 bdd_addref(res);
	 tmp1 = bdd_addref(bdd_apply(x[i],y[i],bddop_biimp));
	 tmp2 = bdd_apply(res, tmp1, bddop_and);
	 bdd_delref(tmp1);
	 bdd_delref(res);
	 res = tmp2;
      }
   
   return res;
}


bdd transitions(bdd* t, bdd* tp, bdd* h, bdd* hp, bdd* c, bdd* cp)
{
   int i;
   bdd P, E, T = bddfalse;
   bdd tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
   
   for(i=0; i<N; i++)
   {
      bdd_addref(T);

      tmp1 = bdd_addref( bdd_apply(c[i], cp[i], bddop_diff) );
      tmp2 = bdd_addref( bdd_apply(tp[i], t[i], bddop_diff) );
      tmp3 = bdd_addref( A(c, cp, i) );
      tmp4 = bdd_addref( A(t, tp, i) );
      tmp5 = bdd_addref( A(h, hp, i) );

      tmp6 = bdd_addref( bdd_apply(tmp1, tmp2, bddop_and) );
      bdd_delref(tmp1);
      bdd_delref(tmp2);

      tmp1 = bdd_addref( bdd_apply(tmp6, hp[i], bddop_and) );
      bdd_delref(tmp6);

      tmp2 = bdd_addref( bdd_apply(tmp1, tmp3, bddop_and) );
      bdd_delref(tmp1);
      bdd_delref(tmp3);

      tmp3 = bdd_addref( bdd_apply(tmp2, tmp4, bddop_and) );
      bdd_delref(tmp2);
      bdd_delref(tmp4);
      
      tmp1 = bdd_addref( bdd_apply(tmp3, tmp5, bddop_and) );
      bdd_delref(tmp3);
      bdd_delref(tmp5);

      
      tmp4 = bdd_addref( bdd_apply(h[i], hp[i], bddop_diff) );
      
      tmp5 = bdd_addref( bdd_apply(tmp4, cp[(i+1)%N], bddop_and) );
      bdd_delref(tmp4);

      tmp6 = bdd_addref( A(c, cp, (i+1)%N) );

      tmp2 = bdd_addref( bdd_apply(tmp5, tmp6, bddop_and) );
      bdd_delref(tmp5);
      bdd_delref(tmp6);

      tmp3 = bdd_addref( A(h, hp, i) );

      tmp4 = bdd_addref( bdd_apply(tmp2, tmp3, bddop_and) );
      bdd_delref(tmp2);
      bdd_delref(tmp3);

      tmp5 = bdd_addref( A(t, tp, N) );

      tmp6 = bdd_addref( bdd_apply(tmp4, tmp5, bddop_and) );
      bdd_delref(tmp4);
      bdd_delref(tmp5);

      P = bdd_addref( bdd_apply(tmp1, tmp6, bddop_or) );
      bdd_delref(tmp1);
      bdd_delref(tmp6);


      tmp1 = bdd_addref( bdd_apply(t[i], tp[i], bddop_diff) );

      tmp2 = bdd_addref( A(t, tp, i) );
      
      tmp3 = bdd_addref( bdd_apply(tmp1, tmp2, bddop_and) );
      bdd_delref(tmp1);
      bdd_delref(tmp2);

      tmp4 = bdd_addref( A(h, hp, N) );
      tmp5 = bdd_addref( A(c, cp, N) );

      tmp6 = bdd_addref( bdd_apply(tmp3, tmp4, bddop_and) );
      bdd_delref(tmp3);
      bdd_delref(tmp4);

      E = bdd_addref( bdd_apply(tmp6, tmp5, bddop_and) );
      bdd_delref(tmp6);
      bdd_delref(tmp5);
      

      tmp1 = bdd_addref( bdd_apply(P, E, bddop_or) );
      bdd_delref(P);
      bdd_delref(E);

      tmp2 = bdd_apply(T, tmp1, bddop_or);
      bdd_delref(T);
      T = tmp2;
   }

   return T;
}

   
bdd initial_state(bdd* t, bdd* h, bdd* c)
{
   int i;
   bdd I, tmp1, tmp2, tmp3;

   tmp1 = bdd_addref( bdd_not(h[0]) );
   
   tmp2 = bdd_addref( bdd_apply(c[0], tmp1, bddop_and) );
   bdd_delref(tmp1);

   tmp1 = bdd_addref( bdd_not(t[0]) );

   I = bdd_apply(tmp1, tmp2, bddop_and);
   bdd_delref(tmp1);
   bdd_delref(tmp2);

   
   for(i=1; i<N; i++)
   {
      bdd_addref(I);

      tmp1 = bdd_addref( bdd_not(c[i]) );
      tmp2 = bdd_addref( bdd_not(h[i]) );

      tmp3 = bdd_addref( bdd_apply(tmp1, tmp2, bddop_and) );
      bdd_delref(tmp1);
      bdd_delref(tmp2);

      tmp1 = bdd_addref( bdd_not(t[i]) );
      tmp2 = bdd_addref( bdd_apply(tmp3, tmp1, bddop_and) );
      bdd_delref(tmp3);
      bdd_delref(tmp1);

      tmp1 = bdd_apply(I, tmp2, bddop_and);
      bdd_delref(tmp2);
      bdd_delref(I);

      I = tmp1;
   }
   
   return I;
}


bdd reachable_states(bdd I, bdd T)
{
   bdd C, by, bx = bddfalse;
   bdd tmp1;
   
   do
   {
      bdd_addref(bx);
      
      by = bx;
#if 1
      tmp1 = bdd_addref( bdd_apply(T, bx, bddop_and) );
      C = bdd_addref( bdd_exist(tmp1, normvarset) );
      bdd_delref(tmp1);
#else
      C = bdd_addref( bdd_appex(bx, T, bddop_and, normvar, N*3) );
#endif
      
      tmp1 = bdd_addref( bdd_replace(C, pairs) );
      bdd_delref(C);
      C = tmp1;

      tmp1 = bdd_apply(I, C, bddop_or);
      bdd_delref(C);

      bdd_delref(bx);
      bx = tmp1;
      
      /*printf("."); fflush(stdout);*/
   }
   while(bx != by);
   
   printf("\n");
   return bx;
}

#if 0
int has_deadlocks(bdd R, bdd T)
{
   bdd C = bddtrue;
   
   for(int i=0; i<N; i++)
      C &= bdd_exist(T, primvar, N*3);
   //C &= bdd_exist(bdd_exist(bdd_exist(T,i*6+3),i*6+5),i*6+1);
   
   if(C != bddfalse && R != bddfalse)
      return 0;
   
   return 1;
}
#endif

int main(int argc, char** argv)
{
   bdd *c, *cp, *h, *hp, *t, *tp;
   bdd I, T, R;
   int n;
   
   if(argc < 2)
   {
      printf("usage: %s N\n",argv[0]);
      printf("\tN  number of cyclers\n");
      exit(1);
   }
   
   N = atoi(argv[1]);
   if (N <= 0)
   {
      printf("The number of cyclers must more than zero\n");
      exit(2);
   }
   
   bdd_init(100000, 10000);
   bdd_setvarnum(N*6);
      
   c  = (bdd *)malloc(sizeof(bdd)*N);
   cp = (bdd *)malloc(sizeof(bdd)*N);
   t  = (bdd *)malloc(sizeof(bdd)*N);
   tp = (bdd *)malloc(sizeof(bdd)*N);
   h  = (bdd *)malloc(sizeof(bdd)*N);
   hp = (bdd *)malloc(sizeof(bdd)*N);
   
   normvar = (int *)malloc(sizeof(int)*N*3);
   primvar = (int *)malloc(sizeof(int)*N*3);
   
   for (n=0 ; n<N*3 ; n++)
   {
      normvar[n] = n*2;
      primvar[n] = n*2+1;
   }
   normvarset = bdd_addref( bdd_makeset(normvar, N*3) );
   pairs = bdd_newpair();
   bdd_setpairs(pairs, primvar, normvar, N*3);
   
   for (n=0 ; n<N ; n++)
   {
      c[n]  = bdd_ithvar(n*6);
      cp[n] = bdd_ithvar(n*6+1);
      t[n]  = bdd_ithvar(n*6+2);
      tp[n] = bdd_ithvar(n*6+3);
      h[n]  = bdd_ithvar(n*6+4);
      hp[n] = bdd_ithvar(n*6+5);
   }
   
   I = bdd_addref( initial_state(t,h,c) );
   T = bdd_addref( transitions(t,tp,h,hp,c,cp) );
   R = bdd_addref( reachable_states(I,T) );
   
   /*if(has_deadlocks(R,T))
     printf("Milner's Scheduler has deadlocks!\n"); */
   
   printf("SatCount R = %.0f\n", bdd_satcount(R));
   printf("Calc       = %.0f\n", (double)N*pow(2.0,1.0+N)*pow(2.0,3.0*N));
   
   bdd_done();
   
   return 0;
}

