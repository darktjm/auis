/******************************************************************************
 *
 * gtextv - Gesture Text View
 * Medical Informatics 
 * Washington University, St. Louis
 * July 29, 1991
 *
 * Scott Hassan
 * Steve Cousins
 * Mark Frisse
 *
 *****************************************************************************/

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#define NOPOINT -1L

#define MODULE "gtextv"
/*#define DEBUGFLAG */

/*****************************************************************************
 * 
 * gtextv.c -- The Gesture Text View Module
 *
 *****************************************************************************/

#include <andrewos.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>


#include <text.H>
#include <textview.H>
#include <dataobject.H>
#include <mark.H>
#include <environ.H>
#include <proctable.H>

/*****************************************************************************/

#include <gtext.H> /* not really necessary, pulls in gtext when doing auto-magical static linking */
#include <gtextv.H>

#include <math.h>
#define sRead sRead_hidden
#define FvAlloc FvAlloc_hidden
#define FvInit FvInit_hidden
#define FvAddPoint FvAddPoint_hidden
#define sClassify sClassify_hidden
#define FvCalc FvCalc_hidden
#include <gestures/bitvector.h>
#include <gestures/matrix.h>
#include <gestures/sc.h>
#include <gestures/fv.h>
#include <gestures/zdebug.h>

#undef sRead
#undef FvAlloc
#undef FvInit
#undef FvAddPoint
#undef sClassify
#undef FvCalc
extern "C" {
    extern sClassifier sRead(FILE *fp);
    extern FV FvAlloc(int a);
    extern void FvInit(FV fv);
    extern void FvAddPoint(FV f, long x, long y, long ts_timer, long foo);
    extern sClassDope sClassify(sClassifier s, Vector v);
    extern double *FvCalc(FV f);
}

/*****************************************************************************/
ATKdefineRegistry(gtextv, textview, gtextv::InitializeClass);
#ifdef hpux
static unsigned long
#else
static long
#endif
mclock();

static int CalcMiddle(int  *xp, int  *yp, int  length, int  *middlex, int  *middley);
static char *ClassifyFv();
static char *ClassifyVector(Vector  y);

#ifdef hpux
static unsigned long
#else
static long
#endif
mclock()
{
    struct timeval this_c;
    gettimeofday(&this_c, NULL);
    return this_c.tv_usec;
}

/* strdup: return a freshly malloced copy of a string so that it
  can be safely freed later. */

static char *mynewstring(char  *x)
{
    char *p=(char *)malloc(strlen(x) + 1);
    if(!p) return NULL;
    strcpy(p, x);
    return p;
}

static sClassifier fullclassifier;
static sClassifier doneclassifier;
static FV fvb;
char _zdebug_flag[128];




#define GESTURE_FILE "/lib/gestures/gest.cl"

/******************************************************************************
 * InitializeClass(class)
 * 
 * Setup the Gesture Recognizer and Read in the Gesture file.
 *
 *****************************************************************************/

static proctable_fptr textview_CopyRegionCmd = NULL;
static proctable_fptr textview_BeginningOfTextCmd = NULL;
static proctable_fptr textview_EndOfTextCmd = NULL;
static proctable_fptr textview_NextScreenCmd = NULL;
static proctable_fptr textview_PrevScreenCmd = NULL;
static proctable_fptr textview_YankCmd = NULL;
static proctable_fptr textview_ZapRegionCmd = NULL;

boolean gtextv::InitializeClass() 
     {
  FILE *fp;
  char *temp;
  char *filename;
  struct proctable_Entry *pe=NULL;
  doneclassifier = NULL;

  temp = (char *) environ::GetProfile("GestureFile");

  if(temp == NULL) 
    filename = (char *) mynewstring(environ::AndrewDir(GESTURE_FILE));
  else
    filename = (char *) mynewstring(temp);

  fp = fopen(filename,"r");

  if(fp == NULL) {
    printf("gtextv: Cannot find file: %s\n", filename);
    return FALSE;
  }

  fullclassifier = sRead(fp);
  fclose(fp);

  fvb = FvAlloc(0);

/*  free(filename);*/

  pe=proctable::Lookup("textview-copy-region");
  if(pe) textview_CopyRegionCmd = proctable::GetFunction(pe);

  pe=proctable::Lookup("textview-beginning-of-text");
    if(pe) textview_BeginningOfTextCmd = proctable::GetFunction(pe);

  pe=proctable::Lookup("textview-end-of-text");
  if(pe) textview_EndOfTextCmd= proctable::GetFunction(pe);

  pe=proctable::Lookup("textview-next-screen");
  if(pe) textview_NextScreenCmd= proctable::GetFunction(pe);

  pe=proctable::Lookup("textview-prev-screen");
  if(pe) textview_PrevScreenCmd= proctable::GetFunction(pe);

  pe=proctable::Lookup("textview-yank");
  if(pe) textview_YankCmd= proctable::GetFunction(pe);

  pe=proctable::Lookup("textview-zap-region");
  if(pe) textview_ZapRegionCmd= proctable::GetFunction(pe);

  if(textview_CopyRegionCmd && textview_BeginningOfTextCmd && textview_EndOfTextCmd && textview_NextScreenCmd && textview_PrevScreenCmd && textview_YankCmd && textview_ZapRegionCmd) return TRUE;
 
  return FALSE;
}     

/******************************************************************************
 * InitializeObject(class, self)
 * 
 * Set the object's initial data to null values.
 *
 *****************************************************************************/

gtextv::gtextv()
          {
	ATKinit;

  this->parstart = NOPOINT;
  this->parend = NOPOINT;

  this->limit = 500;
  this->xp = (int *)malloc(this->limit * sizeof(int));
  this->yp = (int *)malloc(this->limit * sizeof(int));
  this->index = 0;

  THROWONFAILURE( TRUE);
}
/******************************************************************************
 * FinalizeObject(class, self)
 * 
 * Free up some of the used memory.
 *
 *****************************************************************************/

gtextv::~gtextv()
          {
	ATKinit;

  free(this->xp);
  free(this->yp);
}
     
/**********************************************************************/

static int CalcMiddle(int  *xp, int  *yp, int  length, int  *middlex, int  *middley)
                         {
  int i;


  if(length>0) {
    *middlex = *middley = 0;

    for(i=0; i<length; i++) {
      *middlex += xp[i];
      *middley += yp[i];
    }
    *middlex = *middlex / length;
    *middley = *middley / length;
    return TRUE;
  } else {
    *middlex = NOPOINT;
    *middley = NOPOINT;
    return FALSE;
  }
}

/**********************************************************************/
/******************************************************************************
 * Hit(self, action, x, y, numclicks)
 * 
 * Track the mouse cursor and process the path.
 *
 *****************************************************************************/

class view *gtextv::Hit(enum view_MouseAction  action, long  x, long  y , long  numclicks)
               {
  int i;
  class view *temp;
  int transfer;
  long ts_timer;
  char *class_c;
  struct rectangle VisualRect;
  class view *vptr;
  long newPos;
  int my, mx;

  newPos = (this)->Locate(x,y, &vptr);

  temp = (class view *)this;
  ts_timer = mclock() / 1000;

  (this)->GetVisualBounds( &VisualRect);
  
  (this)->SetTransferMode( graphic_XOR);

  if(action == view_LeftMovement) { /* Track the mouse movements */
    this->xp[this->index] = x;
    this->yp[this->index] = y;
    (this)->MoveTo( this->xp[this->index - 1], this->yp[this->index - 1]);
    (this)->DrawLineTo( x, y);
    this->index++;
    FvAddPoint(fvb, x, VisualRect.height - y, ts_timer, 0);

  } else if(action == view_LeftDown) { /* signals the start of an action */
    this->xp[this->index] = x;
    this->yp[this->index] = y;
    (this)->MoveTo( x, y);
    (this)->DrawLineTo( x, y);
    this->index++;
    FvInit(fvb); /* initialize the gesture handler */
    FvAddPoint(fvb, x, VisualRect.height - y, ts_timer, 0);
    (this)->WantInputFocus( this);

  } else if(action == view_LeftUp) {  /* signals the end of an action */
    i=0;
    (this)->MoveTo( this->xp[i], this->yp[i]);
    for(i=0; i<this->index; i++) {
      (this)->DrawLineTo( this->xp[i], this->yp[i]);
    }
    FvAddPoint(fvb, x, VisualRect.height - y, ts_timer, 0);
    class_c = ClassifyFv(); /* Classify the sampled points into a gesture */
/*    printf("clasifyfv: %s\n", class);*/
/************************************************************ PARSTART Gesture handler *****/
    if(!strcmp(class_c, "parstart")) {
      if(CalcMiddle(this->xp, this->yp, this->index, &mx, &my)==TRUE) {
	this->parstart = (this)->Locate(mx,my, &vptr);
	if(this->parend != NOPOINT) {
	  if(this->parstart < this->parend) {
	    (this)->SetDotPosition( this->parstart);
	    (this)->SetDotLength( this->parend - this->parstart);
	    textview_CopyRegionCmd(this, 0);
	  } else {
	    (this)->SetDotPosition( this->parstart);
	    (this)->SetDotLength( 0);
	  }
	}
      }
/************************************************************ PAREND Gesture handler *****/
    } else if(!strcmp(class_c, "parend")) {
      if(CalcMiddle(this->xp, this->yp, this->index, &mx, &my)==TRUE) {
	this->parend = (this)->Locate(mx,my, &vptr);
	if(this->parstart != NOPOINT) {
	  if(this->parstart > this->parend) {
	    this->parstart = NOPOINT;
	    this->parend = NOPOINT;
	  } else {
	    (this)->SetDotPosition( this->parstart);
	    (this)->SetDotLength( this->parend - this->parstart);
	    textview_CopyRegionCmd(this, 0);
	  }
	}
      }
/************************************************************ .DOT Gesture handler *****/
    } else if(!strcmp(class_c, ".dot")) {
      this->parstart = NOPOINT;
      this->parend = NOPOINT;
      
      (this)->SetDotLength( 0);
      (this)->SetDotPosition( newPos);
/************************************************************ TOP Gesture handler *****/
    } else if(!strcmp(class_c, "top")) {
      textview_BeginningOfTextCmd(this, 0);
/************************************************************ BOTTOM Gesture handler *****/
    } else if(!strcmp(class_c, "bottom")) {
      textview_EndOfTextCmd(this, 0);
/************************************************************ DOWN Gesture handler *****/
    } else if(!strcmp(class_c, "down")) {
      textview_NextScreenCmd(this, 0);
/************************************************************ UP Gesture handler *****/
    } else if(!strcmp(class_c, "up")) {
      textview_PrevScreenCmd(this, 0);
/************************************************************ PASTE Gesture handler *****/
    } else if(!strcmp(class_c, "paste")) {
      newPos = (this)->Locate(this->xp[0],this->yp[0], &vptr);
      (this)->SetDotPosition( newPos);
      (this)->SetDotLength( 0);
      textview_YankCmd(this, 0);
      this->parstart = newPos;
      this->parend = newPos + (this)->GetDotLength();
/************************************************************ SELECT Gesture handler *****/
    } else if(!strcmp(class_c, "select")) {
      this->parstart = (this)->Locate( this->xp[0], this->yp[0], &vptr);
      this->parend = (this)->Locate( this->xp[this->index - 1], 
				     this->yp[this->index - 1], &vptr);
      (this)->SetDotPosition( this->parstart);
      (this)->SetDotLength( this->parend - this->parstart);
      textview_CopyRegionCmd(this, 0);
/************************************************************ DELETE Gesture handler *****/
    } else if(!strcmp(class_c, "delete")) {
      newPos = (this)->Locate(this->xp[0],this->yp[0], &vptr);
      if((this)->GetDotLength() >= 0) {
	if(this->parstart != NOPOINT && this->parend != NOPOINT) {
	  if(newPos >= this->parstart && newPos <= this->parend) {
	    textview_ZapRegionCmd(this, 0);
	    this->parstart = NOPOINT;
	    this->parend = NOPOINT;
	  }
	}
      }
    }
    this->index = 0;
  }

  (this)->SetTransferMode( graphic_COPY);
  return((class view *)temp);
}



static char *ClassifyFv()
{
  Vector v;

  v = FvCalc(fvb);
  return ClassifyVector(v);
}

/**********************************************************************/

static char *ClassifyVector(Vector  y)
     {
  sClassDope cd;
  
  cd = sClassify(fullclassifier, y);
  if(cd == NULL) {
    return NULL;
  } else {
    return(cd->name);
  }
}

