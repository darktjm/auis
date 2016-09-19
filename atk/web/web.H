/* Copyright 1995 Carnegie Mellon University  --All rights reserved */
#ifndef _web_H_
#define _web_H_
ATK_INTER

#define web_Limit 1
#define web_IncludeContext 2
#define web_ISMAP 4

#include <htmltext.H>
class web: public htmltext {
	boolean imbedded;
public:
	virtual ATKregistryEntry *ATKregistry();

	web();
	~web();
	static boolean InitializeClass();

	virtual void SetWebcom(class webcom *c);
	virtual long ReadHTML(FILE* file, long id)  ; 
    
	long Read(FILE* file, long id)  ;
	inline class webcom  * getwebcom() 
			{ return ((this)->webcomp); }
	virtual void ImbedTextInTable(table *t, chunk *c);
	inline boolean Imbedded() {
	    return imbedded;
        }
        image *GetImage(const char *file, attlist *atts);
	webcom *webcomp;
};

#endif /* _web_H_ */
