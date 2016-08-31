ATK_INTER
#ifndef _menulist_H_
#define _menulist_H_
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


 

/* menulist.H
 * Class header file for menulist package.
 *
 */

#define menulist_GetMLVersion(alist)	((alist)->version)
#define menulist_RewindML(alist)		((alist)->curMenu = (alist)->menus)
#define menulist_RewindBeforeMC(alist)	((alist)->curChainBefore = (alist)->menuChainBefore)
#define menulist_RewindAfterMC(alist)	((alist)->curChainAfter = (alist)->menuChainAfter)

struct itemlist {
    struct itemlist *next;		/* next guy in the list */
    char *string;			/* what to put in the menu */
    long enableMask;			/* used to enable this item */ 
    long functionData;			/* sent when we call the procedure */
    struct proctable_Entry *proc;	/* what to call if selected */
};

struct headerlist {
    struct headerlist *next;
    long assocKey;				/* Association key. */
    class menulist *menulist;		/* The menulist this link points to. */
};

#include <ATK.H>

class menulist : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();
 /* Maybe should be subclass of observe. */
    
        virtual class menulist * DuplicateML(class view *view)  ;
        virtual void SetView(class view *view);
        virtual void AddToML(const char *menuString, struct proctable_Entry *menuProc, long functionData, long mask);
        virtual void DeleteFromML(const char *menustring);
        virtual void ClearML();
        virtual boolean NextME(char **outString, long *outData, struct proctable_Entry **outProc)  ;
        virtual class menulist * NextBeforeMC()  ;
        virtual class menulist * NextAfterMC()  ;
        virtual void ChainBeforeML(class menulist *chainee, long key);
        virtual void ChainAfterML(class menulist *chainee, long key);
        virtual void UnchainML(long key);
        virtual class menulist * GetChainedML(long key)  ;
        virtual void ClearChain();
	virtual boolean SetMask(long mask)  ;  /* TRUE if changed */

    
        inline long GetMask() { return (this->selectMask); }
      /*item is of type "struct itemlist *" */
        inline boolean ItemIsEnabled(struct itemlist *item) 
            { return ((this->selectMask&(item)->enableMask)==(item)->enableMask); }

    
    
	menulist();
        
	virtual ~menulist();
        static class menulist * Create(class view *view)  ;
/* Version control functions for use by internal modules only. */
        static int NextMLVersion()  ;
        static void IncrementMLVersion();

    
        long version;				/* version of the menu */
	long menuVersion;			/* version of self->menus */
        ATK  *object;		/* The guy who owns these menus. */
        struct itemlist *menus;			/* ptr to list of words and procs to call */
        struct itemlist *curMenu;		/* the current one */
        struct headerlist *menuChainBefore;	/* Pointer to linked list of before chained menulists */
        struct headerlist *menuChainAfter;	/* Pointer to linked list of after chained menulists */
        struct headerlist *curChainBefore;	/* the current before one */
        struct headerlist *curChainAfter;	/* the current after one */
        int *refcount;                          /* Pointer to a reference count for the menus field of this structure. */
	long selectMask, oldMask;		/* enables various items in list */
/* Window manager dependent fields. */
        long regionID;				/* region id for installed menu list; only needed for wm, wasted space for x. */
	long installVersion;			/* version displayed in wm */
	class im *curIM;			/* the one we displayed in last */
};

#endif /* _menulist_H_ */
