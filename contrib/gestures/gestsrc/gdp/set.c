/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

/* implementation */


#include <gestures/util.h>
#include "set.h"
#include <stdio.h>

/* ------------------------- doubly linked lists ------------------*/

typedef struct dll *Dll, *DllElement;
			/* headed, circular, doubly linked list */

struct dll
{
	DllElement	next;
	DllElement	prev;
	union {
		char	c[sizeof(long)];
		int	datasize;
		void	*p;
	} u;
};

#define	DLL_FIRST(h)		((h)->next)
#define	DLL_LAST(h)		((h)->prev)
#define	DLL_NEXT(h, d)		((d)->next)
#define	DLL_PREV(h, d)		((d)->prev)
#define	DLL_DATA(type, h, d)	(* (type *) (d)->u.c)
#define	DLL_PTR(type, h, d)	((type) (d)->u.p)
#define	DLL_END(h, d)		((h) == (d))

#define	DLL_EMPTY(h)		((h)->next == (h))

static
Dll
DllNull(int datasize)
{
	Dll h = allocate(1, struct dll);
	h->next = h->prev = h;
	h->u.datasize = datasize;
	return h;
}

static void
DllInsertElement(Dll h, DllElement d)
{
	d->next = h;
	d->prev = h->prev;
	h->prev->next = d;
	h->prev = d;
}

static
DllElement
DllInsertData(Dll h, Pointer data)
{
	DllElement d = (Dll) recog_myalloc(1, sizeof(struct dll)
				- sizeof(h->u) + h->u.datasize, "Dll");
#ifdef _IBMR2	/* In the rs6000 I used, mentioning bcopy gives
			".movmen undefined" during the load */
	{ char *r = data, *l = d->u.c; int n=h->u.datasize;
	  while(--n >= 0)
		*l++ = *r++;
	}
#else
	bcopy(data, d->u.c, h->u.datasize);
#endif
	DllInsertElement(h, d);
	return d;
}

static
void
DllDeleteElement(Dll h, DllElement d)
{
	d->prev->next = d->next;
	d->next->prev = d->prev;
}

/* --------------------- sets ------------------------- */

struct set {
	Dll		set;
	Dll		log;
	DllElement	cur_ptr;
	void		(*when_added)();
	void		(*when_deleted)();
	Dll		group;	/* NULL except for groupleaders */
	VersionNumber	v;	/* only valid for groupleaders */
};

/* operation logging */

	/* ops */
#define	OP_INSERT	0
#define	OP_DELETE	1
#define	OP_CHECKPOINT	2

typedef struct log_record {
	int		operation;
	union {
		Element		operand;
		VersionNumber	version;
	} u;
} LogRecord;

#ifdef FANCY_UNDO
struct checkpoint_record {
	Dll	log;
	struct	checkpoint_record *next;
	struct	checkpoint_record *prev;
	struct	checkpoint_record *side;
}
#endif

Set
EmptySet(void (*when_added)(Set, DllElement, Pointer), void (*when_deleted)(Set, DllElement, Pointer), Set groupleader)
{
	Set s = allocate(1, struct set);
	Set ss;

	s->set = DllNull(sizeof(Pointer));
	s->log = DllNull(sizeof(LogRecord));
	s->when_added = when_added;
	s->when_deleted = when_deleted;
	if(groupleader == NULL) { /* This one IS the groupleader */
		s->group = DllNull(sizeof(Set));
		s->v = 1;
		groupleader= s;
	}
	else { /* joining the group */
		if(groupleader->group == NULL)
		 recog_error("EmptySet: The set designated as groupleader isn't one");
		s->group = NULL;
	}
	/* add the set to the group */
	ss = s; /* put in a non-register variable for DllInsertCall-bleech */
	(void) DllInsertData(groupleader->group, (Pointer) &ss);

	return s;
}

#ifdef ANSI_CPP
#define assert(x) if( ! (x)) recog_error("set assertion failed: " #x " line %d", __LINE__)
#else
#define assert(x) if( ! (x)) recog_error("set assertion failed: x line %d", __LINE__)
#endif

Element
AddElement(Set s, Pointer data)
{
	LogRecord lr;
	DllElement d;

	d = DllInsertData(s->set, (Pointer) &data);
	lr.operation = OP_INSERT;
	lr.u.operand = d;
	DllInsertData(s->log, (Pointer) &lr);
	if(s->when_added)
		(*s->when_added)(s, d, data);
	return (Element) d;
}

void
DeleteElement(Set s, Element e)
{
	LogRecord lr;

	DllDeleteElement(s->set, (DllElement) e);
	lr.operation = OP_DELETE;
	lr.u.operand = e;
	DllInsertData(s->log, (Pointer) &lr);
	if(s->when_deleted)
		(*s->when_deleted)(s, e, DLL_PTR(Pointer, s->set, e));
}

void
DumpSet(Set s, void (*pf)(Pointer))
{
	DllElement d;
	LogRecord *lr;

	printf("set: { ");
	for(d = DLL_FIRST(s->set); ! DLL_END(s->set, d);
		d = DLL_NEXT(s->set, d))
			(*pf)(DLL_PTR(Pointer, s->set, d)), printf(" ");
	printf("}\n");

	printf("log: {\n");
	for(d = DLL_FIRST(s->log); ! DLL_END(s->log, d);
		d = DLL_NEXT(s->log, d)) {
			lr = &DLL_DATA(LogRecord, s->log, d);
			switch(lr->operation) {
			case OP_INSERT:
				printf("	Insert ");
				(*pf)(DLL_PTR(Pointer, s->set, lr->u.operand));
				printf("\n");
				break;
			case OP_DELETE:
				printf("	Delete ");
				(*pf)(DLL_PTR(Pointer, s->set, lr->u.operand));
				printf("\n");
				break;
			case OP_CHECKPOINT:
				printf("Checkpoint %d\n", lr->u.version);
				break;
			}
	}
	printf("}\n");
}

VersionNumber
CheckpointSetGroup(Set groupleader)
{
	
	LogRecord lr;
	DllElement d;

	if(groupleader->group == NULL)
		recog_error("CheckpointSetGroup: not groupleader");

	lr.operation = OP_CHECKPOINT;
	lr.u.version = groupleader->v++;
	for(d = DLL_FIRST(groupleader->group); ! DLL_END(groupleader->group, d);
	     d = DLL_NEXT(groupleader->group, d))
		DllInsertData(DLL_PTR(Set, groupleader->group, d)->log,
				(Pointer) &lr);
	return lr.u.version;
}

static void UndoSet(Set s, VersionNumber v);

void
UndoSetGroup(Set groupleader, VersionNumber v)
{
	DllElement d;

	if(groupleader->group == NULL)
		recog_error("UndoSetGroup: not groupleader");
	if(v >= groupleader->v) {
		recog_error("Undo(%d): latest version is %d\n",
			v, groupleader->v);
	}

	for(d = DLL_FIRST(groupleader->group); ! DLL_END(groupleader->group, d);
	    d = DLL_NEXT(groupleader->group, d))
		UndoSet(DLL_PTR(Set, groupleader->group, d), v);
}

static
void
UndoSet(Set s, VersionNumber v)
{
	LogRecord *lr;
	DllElement d;

	for(d = DLL_LAST(s->log); ! DLL_END(s->log, d);
		d = DLL_PREV(s->log, d)) {
			lr = &DLL_DATA(LogRecord, s->log, d);
			switch(lr->operation) {
			case OP_INSERT:
				DeleteElement(s, lr->u.operand);
				break;

			case OP_DELETE:
				AddElement(s, DLL_PTR(Pointer, s->set,
					lr->u.operand) );
				break;

			case OP_CHECKPOINT:
				if(v >= lr->u.version)
					return;
				break;
			}
	}
	return;
}

void
Map(Set s, void (*fp)(DllElement, Pointer), Pointer arg)
{

	DllElement d;

	for(d = DLL_FIRST(s->set); ! DLL_END(s->set, d);
		d = DLL_NEXT(s->set, d))
			(*fp)(DLL_PTR(DllElement, s->set, d), arg);
}

void
MapE(Set s, void (*fp)(DllElement, Pointer), Pointer arg)
{

	DllElement d;

	for(d = DLL_FIRST(s->set); ! DLL_END(s->set, d);
		d = DLL_NEXT(s->set, d))
			(*fp)(d, arg);
}

Pointer
ElementPointer(Element e)
{
	return DLL_PTR(Pointer, (Set) NULL, e);
}

Element
AnElement(Set s)
{
	if(DLL_EMPTY(s->set))
		return NULL;
	return DLL_FIRST(s->set);
}


/* undo functions */

void		DiscardOldVersions();	/* VersionNumber */


/*
		printf("s->v=%d v=%d old=%x new=%x N=%x\n",
			s->v, v, s->old_next, s->new_next, NEXTV(s, v));
*/


void
IterateSet(Set s)
{
	s->cur_ptr = DLL_FIRST(s->set);
}

Element
NextElement(Set s)
{
	Element e;
	e = s->cur_ptr;
	if(DLL_END(s->set, e))
		return NULL;
	s->cur_ptr = DLL_NEXT(s->set, s->cur_ptr);
	return e;
}
