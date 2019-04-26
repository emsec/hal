#include "bvec.h"

/* Find a solution to the send-more-money example
 * The problem is to assign values for the digits s,e,n,d,m,o,r,y
 * in such a way that the following equation is true:
 *
 *     s e n d +
 *     m o r e =
 *   m o n e y
 *
 * with the additional constraints that all digits must have different values
 * and s>0 and m>0.
 */

int main(void)
{
   using namespace std ;	
   // Allocate 11 domains with room for up to 3*10
   static int dom[11] = {30,30,30,30,30,30,30,30,30,30,30};
   
   bdd_init(10000,10000);
   fdd_extdomain(dom,11);

      // Assign binary vectors (expressions) to the digits
   
   bvec s = bvec_varfdd(0);  // The 's' digit
   bvec e = bvec_varfdd(1);  // The 'e' digit
   bvec n = bvec_varfdd(2);  // ...
   bvec d = bvec_varfdd(3);
   bvec m = bvec_varfdd(4);
   bvec o = bvec_varfdd(5);
   bvec r = bvec_varfdd(6);
   bvec y = bvec_varfdd(7);
   bvec m1 = bvec_varfdd(8);  // Carry out 1
   bvec m2 = bvec_varfdd(9);  // Carry out 2
   bvec m3 = bvec_varfdd(10); // Carry out 3

      // Create a few constants of the right bit number (5)
   bvec c10 = bvec_con(5,10);
   bvec c2 = bvec_con(5,2);
   bvec c0 = bvec_con(5,0);

      // Create constraints

      // Constraint 1:  addition of the last digits and constraints on
      // the max. value of the involved variables and carry-out
   bdd  t1 = (d + e       ==  y + m1*10) & d<c10 & e<c10 & y<c10 & m1<c2;

   // The use of "m1*10" instead of "m1*c10" avoids a bitnum mismatch since
   // "m1*10" results in 5 bits but "m1*c10" results in 10 bits!
   
      // And so on ...
   bdd  t2 = (n + r + m1  ==  e + m2*10) & n<c10 & r<c10 & m2<c2;
   bdd  t3 = (e + o + m2  ==  n + m3*10) & o<c10 & m3<c2;
   bdd  t4 = (s + m + m3  ==  o +  m*10) & s<c10 & m<c2;
   bdd  t5 = (m > c0  &  s > c0);

      // Make sure all digits are different
   bdd  t6 = (s!=e  &  s!=n  &  s!=d  &  s!=m  &  s!=o  &  s!=r  &  s!=y);
   bdd  t7 = (e!=n  &  e!=d  &  e!=m  &  e!=o  &  e!=r  &  e!=y);
   bdd  t8 = (n!=d  &  n!=m  &  n!=o  &  n!=r  &  n!=y);
   bdd  t9 = (d!=m  &  d!=o  &  d!=r  &  d!=y);
   bdd t10 = (m!=o  &  m!=r  &  m!=y);
   bdd t11 = (o!=r  &  o!=y);
   bdd t12 = (r!=y);

      // Join all constraints
   bdd t = t1 & t2 & t3 & t4 & t5 & t6 & t7 & t8 & t9 & t10 & t11 & t12;

      // Print result
   cout << fddset << t << endl;
   
   return 0;
}
