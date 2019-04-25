/*************************************************************************
  $Header: /cvsroot/buddy/buddy/examples/bddcalc/slist.h,v 1.1.1.1 2004/06/25 13:21:48 haimcohen Exp $
  FILE:  slist.h
  DESCR: Single linked list
  AUTH:  Jorn Lind
  DATE:  (C)
*************************************************************************/

#ifndef _SLIST_H
#define _SLIST_H

#include <string.h>


/* === Base void list ==================================================*/

class voidSList;

class voidSList
{
protected:
   class voidSListElem
   {
   public:
      void *data;
      voidSListElem *next;
      ~voidSListElem(void) { delete next; }
      
   private:
      voidSListElem(void *d) { data=d; next=NULL; }
      friend class voidSList;
   };
   
public:
   voidSList(void)       { head=tail=NULL; len=0; }
   ~voidSList(void)      { delete head; }
   void *append(void *);
   void *append(voidSListElem *p, void *d);
   void *insert(void *);
   int size(void) const  { return len; }
   int empty(void) const { return len==0; }
   void reverse(void);
   void eraseHead(void);
   void eraseAll(void)   { delete head; head=tail=NULL; len=0; }
   
protected:
   int len;
   voidSListElem *head, *tail;
};


#ifdef IMPLEMENTSLIST

void *voidSList::append(void *d)
{
   voidSListElem *elem = new voidSListElem(d);

   if (tail)
      tail->next = elem;
   else
      head = elem;

   tail = elem;
   len++;

   return elem->data;
}


void *voidSList::append(voidSListElem *p, void *d)
{
   voidSListElem *elem = new voidSListElem(d);

   if (p)
   {
      elem->next = p->next;
      p->next = elem;
      if (p == tail)
	 tail = p->next;
   }
   else
   {
      if (tail)
	 tail->next = elem;
      else
	 head = elem;
      tail = elem;
   }
   
   len++;

   return elem->data;
}


void *voidSList::insert(void *d)
{
   voidSListElem *elem = new voidSListElem(d);

   if (tail == NULL)
      tail = elem;
   
   elem->next = head;
   head = elem;
   len++;
   
   return elem->data;
}


void voidSList::reverse(void)
{
   voidSListElem *newTail = head;
   voidSListElem *tmpHead = NULL;
   
   if (len < 2)
      return ;
   
   while (head != NULL)
   {
      voidSListElem *tmpNext = head->next;

      head->next = tmpHead;
      tmpHead = head;
      
      head = tmpNext;
   }

   tail = newTail;
   head = tmpHead;
}


void voidSList::eraseHead(void)
{
   if (head != NULL)
   {
      head = head->next;
      if (head == NULL)
	 tail = NULL;
      len--;
   }
}

#endif /* IMPLEMENTSLIST */


/* === Base void list ==================================================*/

//sorting

template <class T>
class SList : private voidSList
{
public:
   class ite
   {
   public:
      ite(void)                            { next=NULL; }
      ite(const ite &start)                { next=start.next;}
      ite(voidSListElem *start)            { next=start; }
      int more(void) const                 { return next!=NULL; }
      ite &operator=(const ite &start)     { next=start.next; return *this; }
      ite operator++(void)
         { if (next) next=next->next; return *this; }
      ite operator++(int)
         { ite tmp=*this; if (next) next=next->next; return tmp; }
      T &operator*(void) const             { return *((T*)next->data); }
      int operator==(ite x)                { return x.next==next; }
   private:
      voidSListElem *next;
      friend class SList<T>;
   };

   ~SList(void)          { for (ite x=first() ; x.more() ; x++) delete &(*x); }
   T &append(const T &d) { return *((T*)voidSList::append(new T(d))); }
   T &insert(const T &d) { return *((T*)voidSList::insert(new T(d))); }
   T &head(void) const   { return *((T*)voidSList::head->data); }
   T &tail(void) const   { return *((T*)voidSList::tail->data); }
   ite first(void) const { return ite(voidSList::head); }
   int empty(void) const { return voidSList::empty(); }
   int size(void) const  { return voidSList::size(); }
   void reverse(void)    { voidSList::reverse(); }
   void filter(int (*)(T&));
   void append(SList<T> &l)
      { for (ite x=l.first() ; x.more() ; x++) append(*x); }
   T &append(ite &i, const T &d)
      { return *((T*)voidSList::append(i.next, new T(d))); }
   void eraseHead(void)
      { delete ((T*)voidSList::head->data); voidSList::eraseHead(); }
   void eraseAll(void)
      { for (ite x=first() ; x.more() ; x++) delete &(*x); voidSList::eraseAll();}
   void map(void (*f)(T&))
      { for (ite x=first() ; x.more() ; x++) f(*x); }
};


template <class T>
void SList<T>::filter(int (*f)(T&))
{
   voidSListElem *prev=NULL, *next=voidSList::head;

   while (next)
   {
      if (f(*((T*)next->data)))
      {
	 prev = next;
	 next = next->next;
      }
      else
      {
	 voidSListElem *n = next->next;
	 
	 if (prev == NULL)
	    voidSList::head = next->next;
	 else
	    prev->next = next->next;
	 if (voidSList::head == NULL)
	    voidSList::tail = NULL;

	 delete next->data;
	 next->next = NULL;
	 delete next;

	 next = n;
	 len--;
      }
   }
}


#endif /* _SLIST_H */


/* EOF */
