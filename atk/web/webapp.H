/* Copyright 1995 Carnegie Mellon University -- All rights reserved */
#ifndef _webapp_H_
#define _webapp_H_
ATK_INTER


#include <mflex.H>

class buffer;
DEFINE_MFLEX_CLASS(webapp_histlist, buffer *, 5);

struct webapp_webhist {
    webapp_histlist history;
    class im *im;
    struct webapp_webhist *next;
    size_t hpos;
};

#include <ezapp.H>
class webapp: public ezapp{
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
    
	 boolean ParseArgs(int argc, const char **argv)  ;
	 boolean Start()  ;
        	 int Run()  ;
        	 boolean Fork()  ;

	webapp();
	virtual ~webapp();

	static struct webapp_webhist * getwebhist(class view *view)  ;

	class value *bt;
	class wbuttonv *bv;
	class lpair *lpMain;
};

#endif /* _webapp_H_ */
