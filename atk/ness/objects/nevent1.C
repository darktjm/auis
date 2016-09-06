/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1993 - All Rights Reserved
\* ********************************************************************** */

/*
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

#include <andrewos.h>	/* for index() */
#include <im.H>
#include <view.H>
#include <message.H>
#include <scroll.H>
#include <matte.H>
#include <lpair.H>
#include <arbiterview.H>
#include <celview.H>
#include <cel.H>
#include <valueview.H>
#include <text.H>
#include <textview.H>
#include <frame.H>
#include <buffer.H>
#include <viewref.H>

#include <ness.H>


#define CLASS(d) ((Concat(d,Class)==NULL)?(Concat(d,Class) = ATK::LoadClass(Stringize(d))) : Concat(d,Class))

static struct ATKregistryEntry  *scrollClass = NULL;
static struct ATKregistryEntry  *matteClass = NULL;
static struct ATKregistryEntry  *frameClass = NULL;
static struct ATKregistryEntry  *bufferClass = NULL;
static struct ATKregistryEntry  *arbiterviewClass = NULL;


ATK *ProperPtr(ATK *ptr, const struct ATKregistryEntry *type);
class text *FrameMark(class ness *ness, class nessmark *m, char *title, 
	char *pgmnm, boolean enablecommands, long *pPos , long *pLen);


/* ProperPtr(ptr, type)
	looks around in neighborhood of ptr for a pointer of the specified type
	If ptr is celview, the neighborhood includes child and TrueChild
	If ptr is view, the neighborhood includes its data object.  If the ptr is a view 
		and nothing is found in the neighborhood, the ancestors of the view are used.
*/
ATK  *
ProperPtr(ATK   *ptr, const struct ATKregistryEntry   *type) {
	class dataobject *dobj;
	class view *view;
	long i;
	boolean issame = FALSE;

	if (ptr == NULL) return ptr;
	if ((ptr)->IsType( type)) return ptr;

	if ( ! (ptr)->IsType( viewClass)) {
		/* if ptr is not a view, maybe it is a cel.  
				Try its dataObject */
		if ((ptr)->IsType( celClass)) {
			dobj = ((class cel *)ptr)->GetObject();
			if ((dobj)->IsType( type))
				return (ATK  *)dobj;
		}
		/* maybe it's a buffer */
		if ((ptr)->IsType( CLASS(buffer))) {
			dobj = ((class buffer *)ptr)->GetData();
			if ((dobj)->IsType( type))
				return (ATK  *)dobj;
		}
		
		return NULL;
	}
	/* it is a view.  If data object is wanted, check this one's */
	if ((type)->IsType( dataobjectClass)) {
		dobj = ((class view *)ptr)->dataobject;
		if ((dobj)->IsType( type))
			return (ATK  *)dobj;
	}


	/* it is a view, try various kinds of containers */

	if ((ptr)->IsType( CLASS(arbiterview))) {
		/* sometimes an arbiter isn't really in the tree */
		/* sometimes it has a child pointer pointing to a bogus place */
		return NULL;
	}

#define NCLASSES 9
	for (i = 1; i <= NCLASSES; i++) {
	    view = NULL;
	    switch (i) {
	    case 1:   if ( (issame = (ptr)->IsType( celviewClass)) ) 
			view = ((class celview *)ptr)->GetApplication();
		break;

	    case 2: if (issame) 
			view = ((class celview *)ptr)->GetTrueChild();
		break;

	    case 3:   if ( (issame = (ptr)->IsType( lpairClass)) ) 
			view = ((class lpair *)ptr)->GetNth( 0);
		break;

	    case 4: if (issame) 
			view = ((class lpair *)ptr)->GetNth( 1);
		break;

	    case 5:   if ( (issame = (ptr)->IsType( CLASS(scroll))) ) 
			view = ((class scroll *)ptr)->GetChild();
		break;

	    case 6: if (issame) 
		view = ((class scroll *)ptr)->GetScrollee();
		break;

	    case 7: if ((ptr)->IsType( CLASS(matte))) 
			view = ((class matte *)ptr)->child;
		break;

	    case 8: if ((ptr)->IsType( imClass)) 
			view = ((class im *)ptr)->topLevel;
		break;

	    case 9: if ((ptr)->IsType( CLASS(frame))) 
			view = ((class frame *)ptr)->GetChildView();
		break;

	    } /* end of switch(i) */

	    /* test the value of view left by the latest case */
	    if (view != NULL && 
		(view=(class view *) ProperPtr((ATK  *)view, type)) != NULL)
			return (ATK  *)view;
	} /* end of for(i) */
#undef NCLASSES

	/* as a last resort try the ancestors of the view we were passed. */
	view = ((class view *)ptr)->parent;

	while(view && !(view)->IsType( type)) view=view->parent;
	
	return (ATK  *)view;
}

	class text *
FrameMark(class ness  *ness, class nessmark  *m, char  *title, char  *pgmnm, 
		boolean  enablecommands, long  *pPos , long  *pLen) {
	class text *text;
	class textview *textviewp;
	class buffer *buffer;
	class frame *frame;
	class im *window;
	char *oldpgmnm;
	char *lastslash;
	int pos, len;
	class dataobject *dobj;
 
	text = (struct text *)m->GetText();
	pos = (m)->GetPos();
	len = (m)->GetLength();

	text = (class text *)(m)->GetText();

	oldpgmnm = im::GetProgramName();
	if (pgmnm != NULL && *pgmnm != '\0')
		im::SetProgramName(pgmnm);

	lastslash = strrchr(title, '/');
	if (lastslash == NULL)
		lastslash = title;
	else lastslash++;

	/* create buffer.  If text has exactly one object, use it instead of text */
	dobj = (class dataobject *)text;
	if (pos == 0 && len == 1 && text->GetLength() == 1) {
		class viewref *vr  = text->FindViewreference(0, 1);
		if (vr != NULL)
			dobj = vr->dataObject;
	}
 	buffer = buffer::Create(lastslash, NULL, NULL, dobj);

	if ((frame = frame::Create(buffer)) == NULL)
		return NULL;
	if ((window = im::Create(NULL)) == NULL) 
		return NULL;
	(window)->SetView( frame);
	(frame)->PostDefaultHandler( "message", 
			(frame)->WantHandler( "message"));
	message::DisplayString(frame, 0, "");
	(frame)->SetBuffer( buffer, TRUE);
	(frame)->SetCommandEnable( enablecommands);

	if (dobj == (class dataobject *)text) {
		textviewp = (class textview *)(frame)->GetView();
		(textviewp)->WantInputFocus( textviewp);
		(ness)->SetArbiter( arbiterview::FindArb((class view *)textviewp));
	}
	else {
		class view *v = (frame)->GetView();
		(v)->WantInputFocus(v);
		(ness)->SetArbiter(arbiterview::FindArb(v));
		textviewp = new textview;
		(textviewp)->SetDataObject(text);
	}

	(ness)->SetDefaultText( textviewp);
	(textviewp)->SetDotPosition(pos);
	(textviewp)->SetDotLength(len);
	(textviewp)->FrameDot( (m)->GetPos());

	neventPost (ness, FALSE);
	/* I can't see what would/should be observed here.  Ness doesn't
	 have a ObservedChanged method... -rr2b Dec 30,1993.
	 (ness)->AddObserver( ness);
	 */

	im::KeyboardProcessor();	/* DO IT - wait while user interacts*/

	*pPos = (textviewp)->GetDotPosition();
	*pLen = (textviewp)->GetDotLength();
	if (dobj != (class dataobject *)text) 
 		(textviewp)->Destroy();
	im::SetProgramName(oldpgmnm);
	return text;
}


