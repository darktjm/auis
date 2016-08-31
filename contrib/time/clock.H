#ifndef _clock_H_
#define _clock_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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

/* #include <time.h> */
#include <fontdesc.H>
#include <event.H>

struct clock_time {
  double hours, minutes, seconds;
};

enum border_shapes {circle = 0, square = 1};

struct clock_options {
  long timestamp;
  int hours_width, minutes_width, seconds_width, border_width;
  int hours_length, minutes_length, seconds_length; /* -100 -> 100 */
  enum border_shapes border_shape;
  int major_ticks, minor_ticks;
  int tick_length;
  int num_labels;
  const char * const *labels;
  const char *fontfamily;
  int fontface;
};

#include <dataobject.H>
class clock: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	clock();
      
	virtual ~clock();
    
       long Read (FILE *fp, long id)  ;
       long Write (FILE *fp, long id, int level)  ;
    
      virtual void SetOptions(struct clock_options *options);
    
      inline struct clock_time   *ReadClock() { return (&(this->clockface)); }
      inline struct clock_options   *GetOptions() { return (&(this->options));; }
    
      struct clock_time clockface;
      struct clock_options options;
      time_t now;
      long epoch;
      class event *ev;
};


#endif /* _clock_H_ */
