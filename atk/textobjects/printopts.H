ATK_INTER
#ifndef _printopts_H_
#define _printopts_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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

#endif /* _printopts_H_ */
