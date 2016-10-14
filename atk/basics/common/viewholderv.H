/* File viewholderv.H created by Jon Tollefson at 12:37:43 on Thu Jan 19 1995. */
/* Copyright 1995 Carnegie Mellon University All rights reserved. */
/** \addtogroup libbasics
 * @{ */
/*
 Tool: viewholder

*/

/*
 * Abstract class to hold a view.  A container you might call it.
 */

class viewholderv;
#ifndef _VIEWHOLDERV_H
#define _VIEWHOLDERV_H


#include <view.H>

class viewholderv: public view {
  public:
    /****************************************************************************/
    /*                         Constructors/Destructors                         */
    /****************************************************************************/
    TRACED_CLASS(viewholderv);


    /****************************************************************************/
    /*                            General methods                               */
    /****************************************************************************/
    virtual view *getChild() {return NULL;};
    virtual void setChild(view *childView) {};



    /****************************************************************************/
    /*                      IMPLEMENTATION DETAILS BELOW                        */
    /****************************************************************************/	
  private:
};
/** @} */
#endif /* _VIEWHOLDERV_H */

/* Change Log
 *
 * $Log: viewholderv.H,v $
 * Revision 1.2  1995/11/13  00:00:32  robr
 * Copyright/disclaimer update.
 *
 * Revision 1.1  1995/11/08  01:33:40  robr
 * Initial revision
 *
 * Revision 1.1.1.1  1995/02/18  16:59:42  tinglett
 * Implementation from Jon.
 *
 *
 */
