/*************************************************************************
  $Header: /cvsroot/buddy/buddy/examples/bddcalc/hashtbl.cxx,v 1.1.1.1 2004/06/25 13:21:41 haimcohen Exp $
  FILE:  hashtbl.cc
  DESCR: Compiler hash table
  AUTH:  Jorn Lind
  DATE:  (C) september 1998
*************************************************************************/
#include "hashtbl.h"

/*************************************************************************
*************************************************************************/

/*======================================================================*/

hashTable::hashTable(void)
{
   table = NULL;
   clear();
}


hashTable::~hashTable(void)
{
   delete[] table;
}


void hashTable::add(hashData &d)
{
   if (freepos == -1)
      reallocate_table();

   unsigned int h = hashval(d.id);
   int tmppos = table[freepos].next;

   table[freepos].data = d;
   table[freepos].next = table[h].first;
   table[h].first = freepos;
   freepos = tmppos;
}


int hashTable::exists(const char *id)
{
   if (size == 0)
      return 0;
   
   int p = table[hashval(id)].first;
   
   while (p != -1)
   {
      if (strcmp(id, table[p].data.id) == 0)
         return 1;
      p = table[p].next;
   }

   return 0;
}


int hashTable::lookup(const char *id, hashData &d) const
{
   if (size == 0)
      return -1;
   
   int p = table[hashval(id)].first;

   while (p != -1)
   {
      if (strcmp(id, table[p].data.id) == 0)
      {
 	     d = table[p].data;
	     return 0;
      }
      p = table[p].next;
   }

   return -1;
}


int hashTable::remove(const char *id)
{
   if (size == 0)
      return -1;

   int h = hashval(id);
   int next = table[h].first;
   int prev = -1;

   while (next != -1)
   {
      if (strcmp(id, table[next].data.id) == 0)
      {
	 if (prev == -1)
	    table[h].first = table[next].next;
	 else
	    table[prev].next = table[next].next;

	 table[next].next = freepos;
	 freepos = next;

	 return 0;
      }

      prev = next;
      next = table[next].next;
   }

   return -1;
}


void hashTable::clear(void)
{
   delete[] table;
   freepos = -1;
   size = 0;
}


void hashTable::reallocate_table(void)
{
   hashElement *newtable;
   int oldsize = size;
   int n;

   if (size > 0)
   {
      size *= 2;
      newtable = new hashElement[size];
      for (int n=0 ; n<oldsize ; n++)
	 newtable[n] = table[n];
      delete[] table;
      table = newtable;
   }
   else
   {
      size = 10;
      table = new hashElement[size];
   }
   
   for (n=0 ; n<oldsize ; n++)
      table[n].first = -1;

   for (n=oldsize ; n<size ; n++)
   {
      table[n].first = -1;
      table[n].next = n+1;
   }
   table[size-1].next = -1;
   freepos = oldsize;

   for (n=0 ; n<oldsize ; n++)
   {
      unsigned int h = hashval(table[n].data.id);
      table[n].next = table[h].first;
      table[h].first = n;
   }
}


unsigned int hashTable::hashval(const char *s) const
{
   unsigned int h = 0;
   
   for (const char *p=s ; *p!=0 ; p++)
      h = (h + *p) % size;

   return h;
}


/* EOF */
