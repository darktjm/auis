ATK_INTER
#ifndef _phlumph_H_
#define _phlumph_H_

/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
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
#include <dataobject.H>

#define phlumph_AtEnd (-1)
#define phlumph_DatastreamVersion (1)
#define phlumph_DATACHANGED 1
#define phlumph_PAGESCHANGED 2

class phlumph : public dataobject {
  public:
    virtual ATKregistryEntry *ATKregistry();

    phlumph();
    virtual ~phlumph();

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

#endif /* _phlumph_H_ */
