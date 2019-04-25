#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "bdd.h"

float dummyStateNum;  // Use to remove the number of states defined by the
                      // next-state variables

#define SIZE 33       // Size of the board
#define CENTER 16     // Location of the center place

bdd boardC[SIZE];     // Current state variables
bdd boardN[SIZE];     // Next state variables
bdd I;                // Initial state
bdd T;                // Transition relation

bdd currentvar;       // All current state variables
bddPair *pair;        // Renaming pair


   // All the possible moves. Note that the numbering starts from '1'
int moves[][3] = 
{ {1,4,9}, {1,2,3},
  {2,5,10},
  {3,2,1}, {3,6,11},
  {4,5,6}, {4,9,16},
  {5,10,17},
  {6,5,4}, {6,11,18},
  {7,8,9}, {7,14,21},
  {8,9,10}, {8,15,22},
  {9,8,7}, {9,10,11}, {9,4,1}, {9,16,23},
  {10,9,8}, {10,11,12}, {10,5,2}, {10,17,24},
  {11,10,9}, {11,12,13}, {11,6,3}, {11,18,25},
  {12,11,10}, {12,19,26},
  {13,12,11}, {13,20,27},
  {14,15,16},
  {15,16,17},
  {16,15,14}, {16,17,18}, {16,9,4}, {16,23,28},
  {17,16,15}, {17,18,19}, {17,10,5}, {17,24,29},
  {18,17,16}, {18,19,20}, {18,11,6}, {18,25,30},
  {19,18,17},
  {20,19,18},
  {21,22,23}, {21,14,7},
  {22,23,24}, {22,15,8},
  {23,22,21}, {23,24,25}, {23,16,9}, {23,28,31},
  {24,23,22}, {24,25,26}, {24,17,10}, {24,29,32},
  {25,24,23}, {25,26,27}, {25,18,11}, {25,30,33},
  {26,25,24}, {26,19,12},
  {27,26,25}, {27,20,13},
  {28,29,30}, {28,23,16},
  {29,24,17},
  {30,29,28}, {30,25,18},
  {31,32,33}, {31,28,23},
  {32,29,24},
  {33,32,31}, {33,30,25},
  {0,0,0}
};





// Setup the variables needed for the board
void make_board(void)
{
   bdd_setvarnum(SIZE*2);
   
   for (int n=0 ; n<SIZE ; n++)
   {
      boardC[n] = bdd_ithvar(n*2);
      boardN[n] = bdd_ithvar(n*2+1);
   }
}


// Make the initial state predicate
void make_initial_state(void)
{
   I = bddtrue;
   
   for (int n=0 ; n<SIZE ; n++)
      if (n == CENTER)
	 I &= !boardC[n];
      else
	 I &= boardC[n];
}


// Make sure all other places does nothing when
// there's a move from 'src' to 'dst' over 'tmp'
bdd all_other_idle(int src, int tmp, int dst)
{
   bdd idle = bddtrue;
   
   for (int n=0 ; n<SIZE ; n++)
   {
      if (n != src  &&  n != tmp  &&  n != dst)
	 idle &= bdd_biimp(boardC[n], boardN[n]);
   }

   return idle;
}


// Encode one move from 'src' to 'dst' over 'tmp'
bdd make_move(int src, int tmp, int dst)
{
   bdd move = boardC[src] & boardC[tmp] & !boardC[dst] &
              !boardN[src] & !boardN[tmp] & boardN[dst];

   move &= all_other_idle(src, tmp, dst);
   
   return move;
}


void make_transition_relation(void)
{
   using namespace std ;
   T = bddfalse;
   
   for (int n=0 ; moves[n][0]!=moves[n][1] ; n++)
      T |= make_move(moves[n][0]-1, moves[n][1]-1, moves[n][2]-1);

   cout << "Transition relation: " << bdd_nodecount(T) << " nodes, "
	<< bdd_satcount(T)/dummyStateNum << " states\n";
}


// Make renaming pair and current state variables
void make_itedata(void)
{
   pair = bdd_newpair();
   for (int n=0 ; n<SIZE ; n++)
      bdd_setpair(pair, n*2+1, n*2);
   
   currentvar = bddtrue;
   for (int n=0 ; n<SIZE ; n++)
      currentvar &= boardC[n];
}


// Do the forward iteration
void iterate(void)
{
   bdd tmp;
   bdd reachable = I;
   int cou = 1;

   make_itedata();
   
   do
   {
      tmp = reachable;
      bdd next = bdd_appex(reachable, T, bddop_and, currentvar);
      next = bdd_replace(next, pair);
      reachable |= next;

      std::cout << cou << ": " << bdd_nodecount(reachable)
	   << " nodes, " << bdd_satcount(reachable)/dummyStateNum
	   << " states\n" << std::endl ;
      cou++;
   }
   while (tmp != reachable);
}


void iterate_front(void)
{
   bdd tmp;
   bdd reachable = I;
   bdd front = reachable;
   int cou = 1;

   make_itedata();
   
   do
   {
      tmp = reachable;
      bdd next = bdd_appex(front, T, bddop_and, currentvar);
      next = bdd_replace(next, pair);
      front = next - reachable;
      reachable |= front;

      std::cout << cou << ": " << bdd_nodecount(reachable)
	   << " , " << bdd_satcount(reachable)/dummyStateNum << std::endl;
      std::cout << cou << ": " << bdd_nodecount(front)
	   << " , " << bdd_satcount(front)/dummyStateNum << std::endl;
      cou++;
   }
   while (tmp != reachable);
}


void setup(void)
{
   bdd_init(100000,1000);
   bdd_setcacheratio(64);
   bdd_setmaxincrease(500000);
   
   dummyStateNum = pow(2.0, SIZE);

   make_board();
   make_transition_relation();
   make_initial_state();
}


int main(void)
{
   setup();
   iterate();

   system("ps aux | grep \"./solitare\" | grep -v \"grep\"");
}

