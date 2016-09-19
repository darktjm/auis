#ifndef _clock_H_
#define _clock_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
