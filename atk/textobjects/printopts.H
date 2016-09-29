#ifndef _printopts_H_
#define _printopts_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtextobjs
 * @{ */
struct p_option {
    struct view_printopt *op;
    class environment *env, *ansenv;
    boolean changed;
    char *answer; /* points to an allocated string of at least ANSWERLEN+1 chars */
};

#include <textview.H>
class printopts  : public  textview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    class view * Hit(enum view_MouseAction action, long x, long y, long n);
    view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *desiredWidth, long *desiredHeight);
    void PostKeyState(class keystate *ks);
    void PostMenus(class menulist *ml);
    struct view_printoptlist *PrintOptions();


    static boolean InitializeClass();
    static boolean atk_inited;


    printopts();

    virtual ~printopts();


    class menulist *menus;
    class keystate *keys;

    class frame *parentframe;
    class view *opview;
    class dataobject *opobj;
    class environment *doneenv, *cancelenv, *printenv, *previewenv, *hiddenenv;
    struct p_option *oplist;
    int numops;
    int printnamenum, filenamenum;

    class environment *hitenv; /* temporary, used in mouse tracking */
};
/** @} */
#endif /* _printopts_H_ */
