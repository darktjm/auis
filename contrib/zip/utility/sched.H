#ifndef _sched_H_
#define _sched_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Sched Data-object Specification

MODULE	sched.ch

NOTICE	IBM Internal Use Only

DESCRIPTION
	This defines the suite of Methods that support the LightTable Data-object.

HISTORY
  01/20/88	Created (TCP)

END-SPECIFICATION  ************************************************************/

#define  sched_VERSION    1

#include <dataobject.H>
class sched : public  dataobject {
public:
  virtual ATKregistryEntry *ATKregistry();
  virtual void Set_Debug( boolean debug );
  static class sched *Create( char *stream_name );
  sched();

  class zip			     *zipp;
  struct zip_stream		     *stream;
  char				      stream_name[512];
  long				      id;
};


#endif /* _sched_H_ */
