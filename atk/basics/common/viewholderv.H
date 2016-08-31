/* File viewholderv.H created by Jon Tollefson at 12:37:43 on Thu Jan 19 1995. */
/* Copyright 1995 Carnegie Mellon University All rights reserved.
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
    virtual ATKregistryEntry *ATKregistry();
    virtual ~viewholderv();


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


