/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

#include <gestures/bool.h>
#include <gestures/util.h>
#include "set.h"
#include "object.h"
#include "iobject.h"
#include <gestures/matrix.h>
#include <gestures/gdev.h>
#include "transform.h"
#include <stdio.h>
#include <math.h>
#include "proto.h"

int	DrawDebug = 0;
#define	D if(DrawDebug)

#define stat mystat

static void DumpObject(Object o);

static void
dmpo(DllElement _o, Pointer ign)
{
	Object o = (Object)_o;  /* I guess */
	printf("	-(");
	DumpObject(o);
}

static void DumpObject(Object o)
{
	int i;
	struct dope *d = &dope[(int) o->type];

	printf("%s <%p> ", d->name, o);
	for(i = 0; i < d->npoints; i++)
		printf(" (%d,%d)", 
			o->point[i].x,
		   	o->point[i].y);
	if(o->text != NULL)
		printf("'%s'", o->text);
	printf(")\n");
	if(o->type == SetOfObjects)
		Map(o->subobjects, dmpo, NULL);
}

static
Bool
Drawable(Object o)
{
	int i;
	struct dope *d = &dope[(int) o->type];
	for(i = 0; i < d->npoints; i++)
		if(o->point[i].x == NOT_A_POINT ||
		   o->point[i].y == NOT_A_POINT)
			return FALSE;
	if(o->type == Text && o->text == NULL)
		return FALSE;
	return TRUE;
}

static
void
ReallyDraw(Object o)
{
	D printf("ReallyDraw("), DumpObject(o);   /* debug */
	if(Drawable(o)) {
		GDEVsets("currentlinetype", o->highlight ? "dotted" : "solid");
		(*dope[(int)o->type].draw)(o);	
	}
}


/* ObjEqual does a straight bit compare */

static Bool
ObjEqual(Object o1, Object o2)
{
	return ! memcmp( (char *) o1, (char *) o2, sizeof(struct object));
}

/* cache of things to draw */

#define	CACHESIZE	1024

static int
hash(Object o)
{
	return  (unsigned) (o->point[0].x + o->point[0].y +
		o->point[1].x + o->point[1].y +
		o->point[2].x + o->point[2].y +
		(int) o->type) % CACHESIZE;
}

static struct cache {
	struct	cache *next;
	struct	cache *prev;
	struct	object	o;
} cache[CACHESIZE];

static struct	cache headspace = { &headspace, &headspace }, *head = &headspace;

static struct {
	int	flushes;
	int	news;
	int	hits;
	int	collisions;
} stat;

void CacheStats(void)
{
	printf("%d new %d hits %d collisions %d flushes\n",
	stat.flushes, stat.news, stat.hits, stat.collisions);
}

static void CacheFlush(void (*fcn)(Object))
{
	struct cache *c;

	stat.flushes++;
	for(c = head->next; c != head; c = c->next) {
		if(fcn)
			(*fcn)(&c->o);
		c->o.type = Nothing;
	}
	head->prev = head->next = head;
}

/* possible cache return values */

#define	C_NEW		0
#define	C_COLLISION	1
#define	C_HIT		2

static int
CacheEnter(Object o)
{
	int h = hash(o);
	struct cache *r = &cache[h];

	D printf("hash=%d ", h);
	if(ObjEqual(&r->o, o)) {
		/* hit - delete from cache */
		r->o.type = Nothing;
		r->prev->next = r->next;
		r->next->prev = r->prev;
		stat.hits++;
		return C_HIT;
	}
	if(r->o.type == Nothing) {
		r->o = *o;
		r->next = head->next;
		r->prev = head;
		head->next->prev = r;
		head->next = r;
		stat.news++;
		return C_NEW;
	}
	stat.collisions++;
	return C_COLLISION;
}

void
Erase(Object o)
{
	D printf("Erase ");
	Draw(o);
}

void
Draw(Object o)
{
	D printf("Draw("), DumpObject(o);   /* debug */
	if(o->type == SetOfObjects)
		ReallyDraw(o);
	else {
		switch(CacheEnter(o)) {
		case C_HIT:	/* already in cache - no need to draw */
			D printf("Cache hit - not drawing\n");
			break;
		case C_NEW:	/* cache slot empty - will be drawn later */
			D printf("Cache empty - maybe draw later\n");
			break;
		case C_COLLISION:    /* cache slot full so draw immediately */
			D printf("Cache collision - draw immediately\n");
			ReallyDraw(o);
			break;
		}
	}
}

void
DrawSync(void)
{
	D printf("DrawSync..");   /* debug */
	CacheFlush(ReallyDraw);
	WmFlush();
	D printf("Done\n");   /* debug */
}


void DrawClear(void)
{
	CacheFlush(NULL);
	WmClear();
}

/*-----------------------------------------------------------------*/

void
LineDraw(Object o)
{
	/* printf("<%d %d %d %d> ", o->point[0].x, o->point[0].y, o->point[1].x, o->point[1].y); fflush(stdout); */
	GDEVline(o->point[0].x, o->point[0].y, o->point[1].x, o->point[1].y);
}

void
LineTransform(Object o, Transformation t)
{
	ApplyTran(o->point[0].x, o->point[0].y, t,
		&o->point[0].x, &o->point[0].y);
	ApplyTran(o->point[1].x, o->point[1].y, t,
		&o->point[1].x, &o->point[1].y);
}

void
RectDraw(Object o)
{
	int x1 = o->point[0].x, y1 = o->point[0].y;
	int x2 = o->point[1].x, y2 = o->point[1].y;
	int x3 = o->point[2].x, y3 = o->point[2].y;
	int x4 = x1+x3-x2, y4 = y1+y3-y2;

	GDEVline(x1, y1, x2, y2);
	GDEVline(x2, y2, x3, y3);
	GDEVline(x3, y3, x4, y4);
	GDEVline(x4, y4, x1, y1);
}

void
RectTransform(Object o, Transformation t)
{
	ApplyTran(o->point[0].x, o->point[0].y, t,
		&o->point[0].x, &o->point[0].y);
	ApplyTran(o->point[1].x, o->point[1].y, t,
		&o->point[1].x, &o->point[1].y);
	ApplyTran(o->point[2].x, o->point[2].y, t,
		&o->point[2].x, &o->point[2].y);
}


void
TextDraw(Object o)
{
	GDEVtext(o->point[0].x, o->point[0].y, "text");
}

void
TextTransform(Object o, Transformation t)
{
	ApplyTran(o->point[0].x, o->point[0].y, t,
		&o->point[0].x, &o->point[0].y);
}

void
CircleDraw(Object o)
{
#define NSEGS   16
#define	SHIFT	8
#define FACTOR	(1<<SHIFT)
	static int called = 0;
	static int _x[NSEGS+1], _y[NSEGS+1];
	int i;
	int x = o->point[0].x, y = o->point[0].y, r = o->point[1].x;

	if(!called) {
		double pi = 4 * atan(1.0);
		for(i = 0; i <= NSEGS; i++) {
			_x[i] = FACTOR * cos( (2*pi*i)/NSEGS );
			_y[i] = FACTOR * sin( (2*pi*i)/NSEGS );
		}
		called = 1;
	}

	for(i = 0; i < NSEGS; i++)
	    GDEVline(x + ((_x[i]*r)>>SHIFT), y + ((_y[i]*r)>>SHIFT),
		     x + ((_x[i+1]*r)>>SHIFT), y + ((_y[i+1]*r)>>SHIFT) );
}

void
CircleTransform(Object o, Transformation t)
{
	ApplyTran(o->point[0].x, o->point[0].y, t,
		&o->point[0].x, &o->point[0].y);
	o->point[1].x *= TransScale(t);
}

int
DistanceToPoint(int x1, int y1, int x2, int y2)
{
	int dx = x2 - x1, dy = y2 - y1;
	return dx * dx + dy * dy;
}

#define	inorder(a, b, c)	( ((a) <= (b) && (b) <= (c)) || \
				  ((c) <= (b) && (b) <= (a)) ) 
/*

   Distance from point to line segment.
   Based on Amon/Agin algorithm found in
 	/../h/usr/amon/NP2/np1.mss

  Let A=(xa,ya) B=(xb,yb) 	X=(x,y)

  Now, the line between A and B is given parametrically by
	V(k) = (1-k)A + kB
  and adding the constraint 0 <= k <= 1 makes V(K) the line sgement from A to B

  Now, the line through X perpendicular to V(k) intersects V(k) when k equals

	    (B - A) . (X - A)
	kp = ------------------
	       | B - A |^2

 So if kp =< 0, X is closest to A, kp >= 1, X is closest to B, and if 
	0 < k < kp, X is closest to V(kp).

*/


static
int
DistanceToLineSeg(int x, int y, int xa, int ya, int xb, int yb)
{
	int xba = xb - xa, yba = yb - ya;
	double kp;
	
	if(xba == 0 && yba == 0)	/* degenerate line */
		return DistanceToPoint(x, y, xa, ya);

	kp = ( (double) ( xba * (x - xa) + yba * (y - ya) ) ) /
	      			(xba * xba + yba * yba);
	if(kp <= 0)
		return DistanceToPoint(x, y, xa, ya);
	else if(kp >= 1)
		return DistanceToPoint(x, y, xb, yb);
	else
		return DistanceToPoint(x, y, (int) ((1 - kp) * xa + kp * xb),
					     (int) ((1 - kp) * ya + kp * yb));
}

int
LineDistance(Object o, int x, int y)
{
	return DistanceToLineSeg(x, y, o->point[0].x, o->point[0].y,
				    o->point[1].x, o->point[1].y);
}

static int
min(int a, int b)
{
	return a < b ? a : b;
}

int
RectDistance(Object o, int x, int y)
{
	int x1 = o->point[0].x, y1 = o->point[0].y;
	int x2 = o->point[1].x, y2 = o->point[1].y;
	int x3 = o->point[2].x, y3 = o->point[2].y;
	int x4 = x1+x3-x2, y4 = y1+y3-y2;

	return  min(DistanceToLineSeg(x, y, x1, y1, x2, y2),
		min(DistanceToLineSeg(x, y, x2, y2, x3, y3),
		min(DistanceToLineSeg(x, y, x3, y3, x4, y4),
		DistanceToLineSeg(x, y, x4, y4, x1, y1))));
}

int
TextDistance(Object o, int x, int y)
{
	return DistanceToPoint(x, y, o->point[0].x, o->point[0].y);
}

int
CircleDistance(Object o, int x, int y)
{
	int dx = o->point[0].x - x, dy = o->point[0].y - y;
	double c =  dx * dx + dy * dy;
	double d = sqrt(c) - o->point[1].x;
	return (int) (d * d);
}

static void mdraw(DllElement e, Pointer p)
{
    Draw((Object)e);
}

void
SetDraw(Object o)
{
	Map(o->subobjects, mdraw, NULL);
}

static void mtransform(DllElement e, Pointer p)
{
    Transform((Object)e, (Transformation)p);
}

void
SetTransform(Object o, Transformation t)
{
	void Transform();

	Map(o->subobjects, mtransform, (Pointer)t);
}

struct size {
	int x, y, min_d;
	int count;
};

static void
sizem(DllElement e, Pointer p)
{
	Object o = (Object)e;
	struct size *s = (struct size *)p;
	int d = Distance(o, s->x, s->y);
	if(d < s->min_d)
		s->min_d = d;
	s->count++;
}

int
SetDistance(Object o, int x, int y)
{
	struct size s;

	s.x = x; s.y = y;
	s.count = 0;
	s.min_d = 2000000;
	Map(o->subobjects, sizem, (Pointer) &s);
	/* printf("(%d,%d) ", s.count, s.min_d); fflush(stdout); */
	return s.min_d;
}

