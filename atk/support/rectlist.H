ATK_INTER
#ifndef _rectlist_H_
#define _rectlist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define rectlist_VERSION 1

#include <ATK.H>
#include <color.H>
#include <rect.h>
typedef color tempcolor;
struct spotcolor : public rectangle {
    long ox, oy;
    tempcolor color;
    tempcolor bgcolor;
    spotcolor *next, *nextgroup;
};

class rectlist : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    static void ResetList();
    static void AddOldRectangle(long bottom, long top, long left, long right);
    static void AddNewRectangle(long bottom, long top, long left, long right, long startScan);
    static void InvertRectangles(class view *vPtr, spotcolor *spots=NULL);
    static struct rectangle &Bounds();
};

#endif /* _rectlist_H_ */
