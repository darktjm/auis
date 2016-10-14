#ifndef _phlumph_H_
#define _phlumph_H_

/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
*/
/** \addtogroup libfigure
 * @{ */
#include <dataobject.H>

#define phlumph_AtEnd (-1)
#define phlumph_DatastreamVersion (1)
#define phlumph_DATACHANGED 1
#define phlumph_PAGESCHANGED 2

class phlumph : public dataobject {
  public:
    TRACED_CLASS(phlumph);

    phlumph();

    const char *ViewName();
    long Write(FILE *fp, long writeid, int level);
    long Read(FILE *file, long id);

    virtual boolean InsertPage(int where);
    virtual void DeletePage(int num);
    virtual void SetRepeat(int num);

    int numpages;
    class textflow **pagelist;
    int listsize;
    int repeat;

    inline int GetRepeat() { return this->repeat; };
    inline int GetNumPages() { return this->numpages; };
    inline class textflow *GetPage(int num) {
	if (num < 0 || num >= this->numpages || !this->pagelist)
	    return NULL;
	else
	    return this->pagelist[num];
    };
};
/** @} */
#endif /* _phlumph_H_ */
