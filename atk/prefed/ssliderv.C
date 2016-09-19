/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("ssliderv.H")
#include <math.h>


#include <scroll.H>
#include <prefval.H>
#include "ssliderv.H"

#define zfree(x) do { if(x) { free(x); (x)=NULL;}} while (0)
#define DATA(self) ((class prefval *)(self)->GetDataObject())


ATKdefineRegistry(ssliderv, scroll, NULL);
static void getinfo(class ssliderv  *self, struct range  *total , struct range  *seen , struct range  *dot);
static long whatisat(class ssliderv  *self, long  numerator , long  denominator);
static void setframe(class ssliderv  *self, long  position , long  numerator , long  denominator);
static void endzone(class ssliderv  *self, int  end, enum view_MouseAction  action);


ssliderv::ssliderv()
{
    (this)->SetScrollee( this);
    THROWONFAILURE( TRUE);
}

ssliderv::~ssliderv()
{
}

static void getinfo(class ssliderv  *self, struct range  *total , struct range  *seen , struct range  *dot)
{
    (self)->GetInfo( total, seen, dot);
}

void ssliderv::GetInfo(struct range  *total , struct range  *seen , struct range  *dot)
{    
}

static long whatisat(class ssliderv  *self, long  numerator , long  denominator)
{
    return (self)->WhatIsAt( numerator, denominator);
}

long ssliderv::WhatIsAt(long  numerator , long  denominator)
{
    return 0;
}

static void setframe(class ssliderv  *self, long  position , long  numerator , long  denominator)
{
    (self)->SetFrame( position, numerator, denominator);
}

void ssliderv::SetFrame(long  pos , long  num , long  denom)
{
}

static void endzone(class ssliderv  *self, int  end, enum view_MouseAction  action)
{
    (self)->Endzone( end, action);
}

void ssliderv::Endzone(int  end, enum view_MouseAction  action)
{
}
    
static const struct scrollfns scrollInterface = {(scroll_getinfofptr)getinfo, (scroll_setframefptr)setframe, (scroll_endzonefptr)endzone, (scroll_whatfptr)whatisat};

const void *ssliderv::GetInterface(const char  *name)
{
    return &scrollInterface;
}

void ssliderv::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    ((class scroll *)this)->desired.location=((class scroll *)this)->ideal_location;
    (this)->scroll::FullUpdate( type, left, top, width, height);
}


view_DSattributes ssliderv::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dWidth, long  *dHeight)
{
   unsigned int loc=(unsigned int)(this)->GetLocation();
   int w=0, h=0;
   view_DSattributes retval=view_Fixed;
   if(loc&(scroll_LEFT|scroll_RIGHT)) {
       w+=(this)->GetWidth()+10;
       h+=128;
       retval=(view_DSattributes)(((int)retval) || view_HeightFlexible);
   }
   if(loc&(scroll_TOP|scroll_BOTTOM)) {
       h+=(this)->GetWidth()+10;
       w+=128;
       retval=(view_DSattributes)(((int)retval) || view_WidthFlexible);
   }
  *dWidth=w;
  *dHeight=h;
  return retval;
}

void ssliderv::GetOrigin(long  width, long  height, long  *originX, long  *originY)
{
    *originX=0;
    *originY=height;
}
