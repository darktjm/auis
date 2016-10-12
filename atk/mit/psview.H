#ifndef _psview_H_
#define _psview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	Copyright Carnegie Mellon 1996                                    *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libnote
 * @{ */
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
    void DesiredPrintSize(long width, long height, enum view::DSpass pass, 
			long *desiredwidth, long *desiredheight);
    boolean Gifify(const char *filename, long *pmaxw, long *pmaxh, 
			struct rectangle *visrect);
    class menulist *menus;
    int interpret;
};
/** @} */
#endif /* _psview_H_ */
