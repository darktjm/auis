ATK_INTER
#ifndef _figoflow_H_
#define _figoflow_H_
/* figoflow.ch - fig element object: text flow unit */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figoflow.H,v 1.1 1995/03/07 20:18:02 Zarf Stab74 $
*/

#include <figorect.H>

class figoflow : public  figorect {
  public:
    virtual ATKregistryEntry *ATKregistry();
    static boolean InitializeClass();

    figoflow();
    static class figoflow * Create(long left, long top, long width, long height);

    void ObservedChanged(class observable  *obs, long  status);
    void PrintObject(class figview *v, FILE *file, char *prefix, boolean newstyle);
    char * ToolName(class figtoolview *v, long rock)  ;
    enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;
    void Draw(class figview *v); 
    void Sketch(class figview *v);
    void SetParent(long  pref, class figure  *ancfig);
    long Read(FILE  *fp, long  id);

    long figmodified;
    long order;
};

#endif /* _figoflow_H_ */
