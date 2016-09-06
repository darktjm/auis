/*								     HTAnchor.c
**	HYPERTEXT "ANCHOR" OBJECT
**
**	(c) COPYRIGHT CERN 1994.
**	Please first read the full copyright statement in the file COPYRIGH.
**
**	An anchor represents a region of a hypertext document which is
**	linked to another anchor in the same or a different document.
**
** History
**         Nov 1990  Written in Objective-C for the NeXT browser (TBL)
**	24-Oct-1991 (JFG), written in C, browser-independant 
**	21-Nov-1991 (JFG), first complete version
**
*/

/* Library include files */
#include "tcp.h"
#include "HTUtils.h"
#include "HTString.h"
#include "HTFormat.h"
#include "HTParse.h"
#include "HTFWrite.h"					  /* for cache stuff */
#include "HTAnchor.h"					 /* Implemented here */

#define HASH_SIZE 101		/* Arbitrary prime. Memory/speed tradeoff */

typedef struct _HyperDoc Hyperdoc;
#ifdef VMS
struct _HyperDoc {
	int junk;	/* VMS cannot handle pointers to undefined structs */
};
#endif

PRIVATE HTList **adult_table=0;  /* Point to table of lists of all parents */

/*				Creation Methods
**				================
**
**	Do not use "new" by itself outside this module. In order to enforce
**	consistency, we insist that you furnish more information about the
**	anchor you are creating : use newWithParent or newWithAddress.
*/
PRIVATE HTParentAnchor * HTParentAnchor_new (void)
{
    HTParentAnchor *newAnchor =
	(HTParentAnchor *) calloc(1, sizeof (HTParentAnchor));
    newAnchor->parent = newAnchor;
    return newAnchor;
}


PRIVATE HTChildAnchor * HTChildAnchor_new (void)
{
    return (HTChildAnchor *) calloc (1, sizeof (HTChildAnchor));
}


/*	Case insensitive string comparison
**	----------------------------------
** On entry,
**	s	Points to one string, null terminated
**	t	points to the other.
** On exit,
**	returns	YES if the strings are equivalent ignoring case
**		NO if they differ in more than  their case.
*/

PRIVATE BOOL equivalent
   (CONST char *s, CONST char *t)
{
  if (s && t) {  /* Make sure they point to something */
    for ( ; *s && *t ; s++, t++) {
        if (TOUPPER(*s) != TOUPPER(*t))
	  return NO;
    }
    return TOUPPER(*s) == TOUPPER(*t);
  } else
    return s == t;  /* Two NULLs are equivalent, aren't they ? */
}


/*	Create new or find old sub-anchor
**	---------------------------------
**
**	Me one is for a new anchor being edited into an existing
**	document. The parent anchor must already exist.
*/

PUBLIC HTChildAnchor * HTAnchor_findChild
   (HTParentAnchor *parent, CONST char *tag)
{
  HTChildAnchor *child;
  HTList *kids;

  if (! parent) {
    if (ANCH_TRACE)
	fprintf(TDEST, "HTAnchor_findChild called with NULL parent.\n");
    return NULL;
  }
  if ((kids = parent->children)) {  /* parent has children : search them */
    if (tag && *tag) {		/* TBL */
	while ((child = (HTChildAnchor *) HTList_nextObject (kids))) {
	    if (equivalent(child->tag, tag)) { /* Case sensitive 920226 */
		if (ANCH_TRACE) fprintf (TDEST,
	       "AnchorChild. %p of parent %p with name `%s' already exists.\n",
		    (void*)child, (void*)parent, tag);
		return child;
	    }
	}
     }  /*  end if tag is void */
  } else  /* parent doesn't have any children yet : create family */
    parent->children = HTList_new ();

  child = HTChildAnchor_new ();
  /* int for apollo */
  if (ANCH_TRACE)
      fprintf(TDEST, "AnchorChild. New Anchor %p named `%s' is child of %p\n",
	      (void *) child, tag ? tag : (CONST char *) "", (void *) parent);
  HTList_addObject (parent->children, child);
  child->parent = parent;
  StrAllocCopy(child->tag, tag);
  return child;
}


/*	Create or find a child anchor with a possible link
**	--------------------------------------------------
**
**	Create new anchor with a given parent and possibly
**	a name, and possibly a link to a _relatively_ named anchor.
**	(Code originally in ParseHTML.h)
*/
PUBLIC HTChildAnchor * HTAnchor_findChildAndLink
  (
       HTParentAnchor *parent,	/* May not be 0 */
       CONST char *tag,	/* May be "" or 0 */
       CONST char *href,	/* May be "" or 0 */
       HTLinkType *ltype	/* May be 0 */
       )
{
  HTChildAnchor * child = HTAnchor_findChild(parent, tag);
  if (href && *href) {
    char * relative_to = HTAnchor_address((HTAnchor *) parent);
    char * parsed_address = HTParse(href, relative_to, PARSE_ALL);
    HTAnchor * dest = HTAnchor_findAddress(parsed_address);
    HTAnchor_link((HTAnchor *) child, dest, ltype);
    free(parsed_address);
    free(relative_to);
  }
  return child;
}


/*	Create new or find old named anchor
**	-----------------------------------
**
**	Me one is for a reference which is found in a document, and might
**	not be already loaded.
**	Note: You are not guaranteed a new anchor -- you might get an old one,
**	like with fonts.
*/

PUBLIC HTAnchor * HTAnchor_findAddress  (CONST char * address)
{
    char *tag = HTParse (address, "", PARSE_ANCHOR);	        /* Any tags? */
    char *newaddr=NULL;
    
    StrAllocCopy(newaddr, address);		       /* Get a working copy */
    if (!HTImProxy)
	newaddr = HTSimplify(newaddr);	     /* Proxy has already simplified */

    /* If the address represents a sub-anchor, we recursively load its parent,
       then we create a child anchor within that document. */
    if (*tag) {
	char *docAddress = HTParse(newaddr, "", PARSE_ACCESS | PARSE_HOST |
				   PARSE_PATH | PARSE_PUNCTUATION);
	HTParentAnchor * foundParent =
	    (HTParentAnchor *) HTAnchor_findAddress (docAddress);
	HTChildAnchor * foundAnchor = HTAnchor_findChild (foundParent, tag);
	free (docAddress);
	free (tag);
	return (HTAnchor *) foundAnchor;
    } else {		       	     /* Else check whether we have this node */
	int hash;
	CONST char *p;
	HTList * adults;
	HTList *grownups;
	HTParentAnchor * foundAnchor;
	free (tag);
	
	/* Select list from hash table */
	for(p=newaddr, hash=0; *p; p++)
	    hash = (int) ((hash * 3 + (*(unsigned char*)p)) % HASH_SIZE);
	if (!adult_table)
	    adult_table = (HTList**) calloc(HASH_SIZE, sizeof(HTList*));
	if (!adult_table[hash]) adult_table[hash] = HTList_new();
	adults = adult_table[hash];

	/* Search list for anchor */
	grownups = adults;
	while ((foundAnchor = (HTParentAnchor *) HTList_nextObject(grownups))){
	    if (equivalent(foundAnchor->address, newaddr)) {
		if (ANCH_TRACE)
		    fprintf(TDEST, "FindAnchor.. %p with address `%s' already exists.\n",
			    (void*) foundAnchor, newaddr);
		return (HTAnchor *) foundAnchor;
	    }
	}
	
	/* Node not found : create new anchor. */
	foundAnchor = HTParentAnchor_new();
	if (ANCH_TRACE) fprintf(TDEST, "FindAnchor.. %p with hash %d and address `%s' created\n", (void*)foundAnchor, hash, newaddr);
	foundAnchor->address = newaddr;			/* Remember our copy */
	HTList_addObject (adults, foundAnchor);
	return (HTAnchor *) foundAnchor;
    }
}


/*	Delete an anchor and possibly related things (auto garbage collection)
**	--------------------------------------------
**
**	The anchor is only deleted if the corresponding document is not loaded.
**	All outgoing links from parent and children are deleted, and this
**	anchor is removed from the sources list of all its targets.
**	We also try to delete the targets whose documents are not loaded.
**	If this anchor's source list is empty, we delete it and its children.
*/

PRIVATE void deleteLinks
   (HTAnchor *me)
{
  if (! me)
    return;

  /* Recursively try to delete target anchors */
  if (me->mainLink.dest) {
    HTParentAnchor *parent = me->mainLink.dest->parent;
    HTList_removeObject (parent->sources, me);
    if (! parent->document)  /* Test here to avoid calling overhead */
      HTAnchor_delete (parent);
  }
  if (me->links) {  /* Extra destinations */
    HTLink *target;
    while ((target = (HTLink *) HTList_removeLastObject (me->links))) {
      HTParentAnchor *parent = target->dest->parent;
      HTList_removeObject (parent->sources, me);
      if (! parent->document)  /* Test here to avoid calling overhead */
	HTAnchor_delete (parent);
    }
  }
}

PUBLIC BOOL HTAnchor_delete
   (HTParentAnchor *me)
{
  HTChildAnchor *child;

  /* Don't delete if document is loaded */
  if (me->document)
    return NO;

  /* Recursively try to delete target anchors */
  deleteLinks ((HTAnchor *) me);

  if (! HTList_isEmpty (me->sources)) {  /* There are still incoming links */
    /* Delete all outgoing links from children, if any */
    HTList *kids = me->children;
    while ((child = (HTChildAnchor *) HTList_nextObject (kids)))
      deleteLinks ((HTAnchor *) child);
    return NO;  /* Parent not deleted */
  }

  /* No more incoming links : kill everything */
  /* First, recursively delete children */
  while ((child = (HTChildAnchor *) HTList_removeLastObject (me->children))) {
    deleteLinks ((HTAnchor *) child);
    free (child->tag);
    free (child);
  }

  /* Now kill myself */
  HTList_delete (me->children);
  HTList_delete (me->sources);
  free (me->address);
  /* Devise a way to clean out the HTFormat if no longer needed (ref count?) */
  free (me);
  if (me->cacheItems) {
      HTCacheClear(me->cacheItems);
  }
  return YES;  /* Parent deleted */
}


/*		Move an anchor to the head of the list of its siblings
**		------------------------------------------------------
**
**	This is to ensure that an anchor which might have already existed
**	is put in the correct order as we load the document.
*/

PUBLIC void HTAnchor_makeLastChild
  (HTChildAnchor *me)
{
  if (me->parent != (HTParentAnchor *) me) {  /* Make sure it's a child */
    HTList * siblings = me->parent->children;
    HTList_removeObject (siblings, me);
    HTList_addObject (siblings, me);
  }
}

/*	Data access functions
**	---------------------
*/

PUBLIC HTParentAnchor * HTAnchor_parent
   (HTAnchor *me)
{
  return me ? me->parent : NULL;
}

PUBLIC void HTAnchor_setDocument
   (HTParentAnchor *me, HyperDoc *doc)
{
  if (me)
    me->document = doc;
}

PUBLIC HyperDoc * HTAnchor_document
   (HTParentAnchor *me)
{
  return me ? me->document : NULL;
}


#if 0
PUBLIC void HTAnchor_setAddress
   (HTAnchor *,me, char *,addr)
{
  if (me)
    StrAllocCopy (me->parent->address, addr);
}
#endif


PUBLIC char * HTAnchor_address
   (HTAnchor *me)
{
  char *addr = NULL;
  if (me) {
    if (((HTParentAnchor *) me == me->parent) ||
    	!((HTChildAnchor *) me)->tag) {  /* it's an adult or no tag */
      StrAllocCopy (addr, me->parent->address);
    }
    else {  /* it's a named child */
      addr = (char *) malloc (2 + strlen (me->parent->address)
			      + strlen (((HTChildAnchor *) me)->tag));
      if (addr == NULL) outofmem(__FILE__, "HTAnchor_address");
      sprintf (addr, "%s#%s", me->parent->address,
	       ((HTChildAnchor *) me)->tag);
    }
  }
  return addr;
}



PUBLIC void HTAnchor_setFormat
   (HTParentAnchor *me, HTFormat form)
{
  if (me)
    me->format = form;
}

PUBLIC HTFormat HTAnchor_format
   (HTParentAnchor *me)
{
  return me ? me->format : NULL;
}

PUBLIC void HTAnchor_clearIndex
   (HTParentAnchor *me)
{
  if (me)
    me->isIndex = NO;
}

PUBLIC void HTAnchor_setIndex
   (HTParentAnchor *me)
{
  if (me)
    me->isIndex = YES;
}

PUBLIC BOOL HTAnchor_isIndex
   (HTParentAnchor *me)
{
  return me ? me->isIndex : NO;
}


PUBLIC BOOL HTAnchor_hasChildren
   (HTParentAnchor *me)
{
  return me ? ! HTList_isEmpty(me->children) : NO;
}

/*	Title handling
*/
PUBLIC CONST char * HTAnchor_title
   (HTParentAnchor *me)
{
  return me ? me->title : 0;
}

PUBLIC void HTAnchor_setTitle
  (HTParentAnchor *me, CONST char *title)
{
  StrAllocCopy(me->title, title);
}

PUBLIC void HTAnchor_appendTitle
  (HTParentAnchor *me, CONST char *title)
{
  StrAllocCat(me->title, title);
}

/*	Link me Anchor to another given one
**	-------------------------------------
*/

PUBLIC BOOL HTAnchor_link
  (HTAnchor *source, HTAnchor *destination, HTLinkType *type)
{
  if (! (source && destination))
    return NO;  /* Can't link to/from non-existing anchor */
  if (ANCH_TRACE)
      fprintf(TDEST, "LinkAnchor.. Linking anchor %p to anchor %p\n",
	      (void *) source, (void *) destination);
  if (! source->mainLink.dest) {
    source->mainLink.dest = destination;
    source->mainLink.type = type;
  } else {
    HTLink * newLink = (HTLink *) malloc (sizeof (HTLink));
    if (newLink == NULL) outofmem(__FILE__, "HTAnchor_link");
    newLink->dest = destination;
    newLink->type = type;
    if (! source->links)
      source->links = HTList_new ();
    HTList_addObject (source->links, newLink);
  }
  if (!destination->parent->sources)
    destination->parent->sources = HTList_new ();
  HTList_addObject (destination->parent->sources, source);
  return YES;  /* Success */
}


/*	Manipulation of links
**	---------------------
*/

PUBLIC HTAnchor * HTAnchor_followMainLink
   (HTAnchor *me)
{
  return me->mainLink.dest;
}

PUBLIC HTAnchor * HTAnchor_followTypedLink
   (HTAnchor *me, HTLinkType *type)
{
  if (me->mainLink.type == type)
    return me->mainLink.dest;
  if (me->links) {
    HTList *links = me->links;
    HTLink *link;
    while ((link = (HTLink *) HTList_nextObject (links)))
      if (link->type == type)
	return link->dest;
  }
  return NULL;  /* No link of me type */
}


/*	Make main link
*/
PUBLIC BOOL HTAnchor_makeMainLink
   (HTAnchor *me, HTLink *movingLink)
{
  /* Check that everything's OK */
  if (! (me && HTList_removeObject (me->links, movingLink)))
    return NO;  /* link not found or NULL anchor */
  else {
    /* First push current main link onto top of links list */
    HTLink *newLink = (HTLink*) malloc (sizeof (HTLink));
    if (newLink == NULL) outofmem(__FILE__, "HTAnchor_makeMainLink");
    memcpy (newLink, & me->mainLink, sizeof (HTLink));
    HTList_addObject (me->links, newLink);

    /* Now make movingLink the new main link, and free it */
    memcpy (& me->mainLink, movingLink, sizeof (HTLink));
    free (movingLink);
    return YES;
  }
}


/*	Methods List
**	------------
*/

PUBLIC HTList * HTAnchor_methods (HTParentAnchor * me)
{
    if (!me->methods) {
        me->methods = HTList_new();
    }
    return me->methods;
}

/*	Protocol
**	--------
*/

PUBLIC void * HTAnchor_protocol (HTParentAnchor * me)
{
    return me->protocol;
}

PUBLIC void HTAnchor_setProtocol (HTParentAnchor * me,
	void*	protocol)
{
    me->protocol = protocol;
}

/*	Physical Address
**	----------------
*/

PUBLIC char * HTAnchor_physical (HTParentAnchor * me)
{
    return me->physical;
}

PUBLIC void HTAnchor_setPhysical (HTParentAnchor * me,
	char *	physical)
{
    if (!me || !physical) {
	if (ANCH_TRACE)
	    fprintf(TDEST, "HTAnchor.... setPhysical, called with null argument\n");
	return;
    }
    StrAllocCopy(me->physical, physical);
}

