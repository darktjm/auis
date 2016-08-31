#ifndef _psview_H_
#define _psview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon 1996
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


#include <iconview.H>
class psview : public  iconview
{
  public:
    virtual ATKregistryEntry *ATKregistry();
    psview();
    virtual ~psview();
    static boolean InitializeClass();
    static boolean atk_inited;

    void Print(FILE *file, const char *processor, const char *finalFormat, 
		boolean topLevel);
    void PostMenus(class menulist *menulist);
    void SetDataObject(class dataobject * dobj);
    void PrintPSDoc(FILE *outfile, long pagew, long pageh);
    void *GetPSPrintInterface(const char *printtype);
    void PrintPSRect(FILE *outfile, 
			long logwidth, long logheight, struct rectangle *visrect);
    void DesiredPrintSize(long width, long height, enum view_DSpass pass, 
			long *desiredwidth, long *desiredheight);
    boolean Gifify(const char *filename, long *pmaxw, long *pmaxh, 
			struct rectangle *visrect);
    class menulist *menus;
    int interpret;
};

#endif /* _psview_H_ */
