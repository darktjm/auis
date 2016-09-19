ATK_INTER
#ifndef _owatch_H_
#define _owatch_H_
/* Copyright 1992 Carnegie Mellon University, All rights reserved. */

struct owatch_data {
    class observable *obj;
    boolean alive;
    struct owatch_data *next, *prev;
    long refs;
};

#include <observable.H>

class owatch : public  observable {
public:
	virtual ATKregistryEntry *ATKregistry();


      
     void ObservedChanged(class observable *changed, long val);

    static inline boolean Check(struct owatch_data *tocheck) { return (tocheck?tocheck->alive:FALSE); }

    static struct owatch_data * Create(class observable *b)  ;
    static void Delete(struct owatch_data *d);
    static boolean CheckAndDelete(struct owatch_data *d)  ;

};

#endif /* _owatch_H_ */
