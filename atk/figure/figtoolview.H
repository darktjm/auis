ATK_INTER
#ifndef _figtoolview_H_
#define _figtoolview_H_
/* figtoolv.ch - drawing editor toolset */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
#include <figobj.H>
#include <figtoolview.H>
#include <lpair.H>

struct figtoolview_tool_t {
    const char *name; /* name of the ATK object */
    long rock; /* rock passed to Instantiate() */
};

typedef class view *(*figtoolview_layout_f)(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
typedef void (*figtoolview_mod_f)(class figtoolview  *self, long  rock);
typedef void (*figtoolview_comm_f)(class figtoolview  *self, const char  *rock);

class figtoolview  : public  lpair {
  public:
    virtual ATKregistryEntry *ATKregistry();

    static boolean InitializeClass();
    figtoolview();
    virtual ~figtoolview();

    void ObservedChanged(class observable *observed, long status);
    void PostMenus(class menulist *ml);
    void PostKeyState(class keystate *ks);
    void UnlinkTree();

    virtual boolean SetPrimaryView(class figview *v)  ;
    virtual void SetExpertMode(boolean val);
    virtual void AbortObjectBuilding();

    inline class figview  * GetPrimaryView() { return ((this)->primaryview); }
    inline int   GetToolnum() { return ((this)->toolnum); }
    inline figtoolview_layout_f GetToolProc() { return ((this)->toolproc); }
    /*inline void SetMoribund(boolean val) { ((this)->moribund = (val)); }*/
    inline int   GetSnapGrid()  { return ((this)->snapgrid); }
    inline void SnapToGrid(long &x, long &y)  { ((((this)->snapgrid) ? ((x) += (this)->snapgrid/2, (x) -= ((x) % (this)->snapgrid)) : 0), (((this)->snapgrid) ? ((y) += (this)->snapgrid/2, (y) -= ((y) % (this)->snapgrid)) : 0)); }


    class menulist *Menus;
    class keystate *Keystate;
    class figview *primaryview;
    boolean expertmode;
    /*boolean moribund;*/

    class figobj **dummylist;
    struct rectangle *rectlist;
    long rect_size;
    long *tmplist;
    long tmp_size;
    long tmpnum;
    class figobj *creating;
    boolean selectonup;
    boolean LockCreateMode;
    long submode, rock, rockx, rocky;
    long lastx, lasty, lastw, lasth, lastb;
    boolean selectdeep;
    class view *rockview;

    class figattr *menuatt;

    class lpair *lp1, *lp2, *lp3, *lp4, *lp5, *lp6, *lp7, *lp8, *lp9;
    class fontsel *fontselp;
    class fontselview *vfontsel;
    class stringtbl *tooltbl;
    class strtblview *vtooltbl;
    boolean toolextras;
    struct figtoolview_tool_t *toollist;
    int toollistnum, toollistsize;
    short *toolacc;
    int toolnum;
    class stringtbl *cmdtbl;
    class strtblview *vcmdtbl;
    short *cmdacc;
    figtoolview_layout_f toolproc;
    class stringtbl *shadetbl;
    class strtblview *vshadetbl;
    short *shadeacc;
    int shadenum;
    class stringtbl *textpostbl;
    class strtblview *vtextpostbl;
    short *textposacc;
    int textposnum;
    class stringtbl *linestyletbl;
    class strtblview *vlinestyletbl;
    short *linestyleacc;
    int linestylenum;
    class stringtbl *linewidthtbl;
    class strtblview *vlinewidthtbl;
    short *linewidthacc;
    short *linewidthlist;
    int linewidths_num;
    int linewidthnum;
    class stringtbl *rrectcornertbl;
    class strtblview *vrrectcornertbl;
    short *rrectcorneracc;
    short *rrectcornerlist;
    int rrectcorners_num;
    int rrectcornernum;
    class stringtbl *colortbl;
    class strtblview *vcolortbl;
    short *coloracc;
    char **colorlist;
    int colors_num;
    int colornum;
    class stringtbl *arrowtbl;
    class strtblview *varrowtbl;
    short *arrowacc;
    short *arrowlist;
    int arrows_num;
    int arrowsizenum, arrowposnum, arrownum;
    class stringtbl *snapgridtbl;
    class strtblview *vsnapgridtbl;
    short *snapgridacc;
    short *snapgridlist;
    int snapgrids_num;
    int snapgridnum;
    int snapgridunit;
    int snapgrid;
};

#endif /* _figtoolview_H_ */
