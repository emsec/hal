/* This program creates a transition relation for a finite state machine.
 * This transition relation is then used to find the reachable statespace
 * of the state machine. The state machine has 8 states with state 0 being
 * the initial state. The transitions form a ring:
 *
 *    0 -> 1 -> 2 -> 3 -> 4 -> 5 -> -> 7 -> 0
 */

#include "fdd.h"

/* Use the transition relation "transRel" to iterate through the statespace
 */
void findStateSpace(bdd transRel)
{
   using namespace std ;
      /* Create a new pair for renaming the next-state variables to
       * current-state variables */
   bddPair *p = bdd_newpair();
   fdd_setpair(p,1,0);

      /* Get a BDD that represents all the current-state variables */
   bdd currentStateVar = fdd_ithset(0);

      /* Start with the initial state */
   bdd reachedStates = fdd_ithvar(0,0);

   bdd tmp = bddfalse;

      /* Repeat until no new states are found */
   do
   {
      tmp = reachedStates;

         /* Calculate: Newset = (exists V_cur. transRel & Reached)[cur/next] */
      bdd newset;
      newset = reachedStates & transRel;
      newset = bdd_exist(newset, currentStateVar);
      newset = bdd_replace(newset, p);

      cout << "Front: " << (newset - reachedStates) << endl;

         /* Add the new states to the found states */
      reachedStates = reachedStates | newset;
   }
   while (tmp != reachedStates);
}


int main() {
   using namespace std ;
      /* Initialize BuDDy and declare two interleaved FDD variable blocks
       * with the domain [0..7] */
   int domain[2] = {8,8};

   bdd_init(100,100);
   fdd_extdomain(domain, 2);

      /* Initialize the transition relation with no transitions */
   bdd T = bddfalse;

      /* Add all the transitions (from state 'i' to state 'i+1') */
   for (int i=0 ; i<8 ; i++)
   {
         /* Set the current state to be state 'i' */
      bdd current = fdd_ithvar(0,i);
      
         /* Set the next state to be state 'i+1' */
      bdd next = fdd_ithvar(1, (i+1) % 8);

         /* Add the transition */
      T = T | (current & next);
   }

   cout << fddset << "Transition relation: " << T << endl << endl;

      /* Calculate the reachable statespace */
   findStateSpace(T);
}
