/* figoins.c - fig element object: inset */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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
#include <andrewos.h>
ATK_IMPL("figoins.H")
#include <figoins.H>

#include <view.H>
#include <dataobject.H>
#include <figview.H>
#include <figtoolview.H>
#include <figattr.H>
#include <text.H>
#include <message.H>
#include <im.H>

ATKdefineRegistry(figoins, figorect, NULL);


figoins::figoins()
{
    (this)->IsInset() = TRUE;
    (this)->AttributesUsed() = 0;
    this->dobj = NULL;
    this->moribund = FALSE;
    this->paste=FALSE;
    THROWONFAILURE( TRUE);
}

class figoins *figoins::Create(long  left , long  top , long  width , long  height, char  *dataobjectname)
{
    class figoins *res = new figoins;
    if (!res) return NULL;
    if (!ATK::IsTypeByName(dataobjectname, "dataobject")) return NULL;

    (res)->PosX() = left;
    (res)->PosY() = top;
    (res)->PosW() = width;
    (res)->PosH() = height;
    res->dobj = (class dataobject *)ATK::NewObject(dataobjectname);
    if (!res->dobj) return NULL;

    (res)->RecomputeBounds();

    return res;
}

class figobj * figoins::Instantiate(class figtoolview *v, long rock)  {
    class figobj *res=figorect::Instantiate(v,rock);
    if(rock) ((class figoins *)res)->paste=TRUE;
    return res;
}

const char *figoins::ToolName(class figtoolview  *v, long  rock)
{
    if(rock==0) return "Inset";
    else return "Paste Inset";
}

/* set bounding box and handle list in fig coordinates */
void figoins::RecomputeBounds()
{
    long left, width, top, height;

    if ((this)->PosW() >= 0) {
	left = (this)->PosX();
	width = (this)->PosW();
    }
    else {
	left = (this)->PosX()+(this)->PosW();
	width = -(this)->PosW();
    }

    if ((this)->PosH() >= 0) {
	top = (this)->PosY();
	height = (this)->PosH();
    }
    else {
	top = (this)->PosY()+(this)->PosH();
	height = -(this)->PosH();
    }

    (this)->SetBoundsRect( left, top, width, height);

    (this)->SetHandle( 0, (this)->PosX()+(this)->PosW()/2, (this)->PosY()+(this)->PosH()/2);
    (this)->SetHandle( 1, (this)->PosX()+(this)->PosW(), (this)->PosY()+(this)->PosH()/2);
    (this)->SetHandle( 2, (this)->PosX()+(this)->PosW(), (this)->PosY());
    (this)->SetHandle( 3, (this)->PosX()+(this)->PosW()/2, (this)->PosY());
    (this)->SetHandle( 4, (this)->PosX(), (this)->PosY());
    (this)->SetHandle( 5, (this)->PosX(), (this)->PosY()+(this)->PosH()/2);
    (this)->SetHandle( 6, (this)->PosX(), (this)->PosY()+(this)->PosH());
    (this)->SetHandle( 7, (this)->PosX()+(this)->PosW()/2, (this)->PosY()+(this)->PosH());
    (this)->SetHandle( 8, (this)->PosX()+(this)->PosW(), (this)->PosY()+(this)->PosH());

    (this)->ComputeSelectedBounds();

    (this)->UpdateParentBounds();
}

void figoins::Draw(class figview  *v) 
{
    long x, y, w, h;

    if ((this)->PosW() >= 0) {
	x = (v)->ToPixX( (this)->PosX());
	w = (v)->ToPixW( (this)->PosW());
    }
    else {
	x = (v)->ToPixX( (this)->PosX()+(this)->PosW());
	w = (v)->ToPixW( -(this)->PosW());
    }
    
    if ((this)->PosH() >= 0) {
	y = (v)->ToPixY( (this)->PosY());
	h = (v)->ToPixH( (this)->PosH());
    }
    else {
	y = (v)->ToPixY( (this)->PosY()+(this)->PosH());
	h = (v)->ToPixH( -(this)->PosH());
    }

    (v)->SetTransferMode( graphic_COPY);
    (v)->SetForegroundColor( "black", 0, 0, 0); 
    if (this->dobj) {
	// (v)->DrawRectSize( x-1, y-1, w+1, h+1); 
    }
    else {
	(v)->FillRectSize( x, y, w, h, (v)->WhitePattern());
	(v)->DrawRectSize( x, y, w, h); 
    }
}

static figobj_Status ObjectPrompt(class figoins *this_c, class figview *v) {
    long ix;
    char buffer[30];
	ix = message::AskForString(v, 40, "Data object to insert:  ", NULL, buffer, 30); 
	if (this_c->moribund) {
	    /* oh dear, someone cancelled the question. Us, probably. Time ro cut and run like hell. */
	    return figobj_NotDone;
	}
	if (ix<0 || strlen(buffer)==0) {
	    message::DisplayString(v, 10, "Cancelled.");
	    return figobj_Failed;
	}
	ATK::LoadClass(buffer);
	if (!ATK::IsTypeByName(buffer, "dataobject")) {
	    char buf2[70];
	    sprintf(buf2, "%s is not a dataobject.", buffer);
	    message::DisplayString(v, 10, buf2);
	    return figobj_Failed;
	}
	this_c->dobj = (class dataobject *)ATK::NewObject(buffer);
	if (!this_c->dobj) {
	    char buf2[70];
	    sprintf(buf2, "Cannot load %s; object aborted.", buffer);
	    message::DisplayString(v, 10, buf2);
	    return figobj_Failed;
	}
	if (ATK::IsTypeByName(buffer, "text")) {
	    message::DisplayString(v, 0, "Reading template...");
	    ((class text *)this_c->dobj)->ReadTemplate( "default", FALSE);
	}
	return figobj_Done;
}

static figobj_Status PasteInset(class figoins *this_c, class figview  *v)
{
    FILE *fp;
    static const char hdr[] = "\\begindata{";
    const char *hx = hdr;
    int c, ix, id = 0;
    char classname[100], *cp;

    fp = ((v)->GetIM())->FromCutBuffer();

    /* if the file does not begin with a fig, we may as well scan until we find one. Well, actually that's not so cool. */

    while ((c = getc(fp)) != EOF && *hx)
	if (c == *hx) hx++;
	else hx = (c == *hdr) ? hdr+1 : hdr;

    if (*hx) {
	message::DisplayString(v, 0, "No inset found in cut buffer.");
	((v)->GetIM())->CloseFromCutBuffer( fp);
	return figobj_Failed;
    }

    cp=classname;
    do {
	*cp++=c;
    } while((c=getc(fp))!=EOF && c!=',' && (cp-classname<(int)sizeof(classname)-2));
    *cp='\0';

    while(!isdigit(c)) c=getc(fp);
    if(!isdigit(c)) {
	id=0;
    } else do {
	id*=10;
	id+=c-'0';
    } while((c=getc(fp))!=EOF && isdigit(c));
        
    /* skip to end of header */
    while ((c=getc(fp)) != '\n' && c != EOF);

    ATK::LoadClass(classname);
    if (!ATK::IsTypeByName(classname, "dataobject")) {
	char buf2[170];
	sprintf(buf2, "%s is not an inset.", classname);
	message::DisplayString(v, 10, buf2);
	return figobj_Failed;
    }
    this_c->dobj=(class dataobject *)ATK::NewObject(classname);
    if(this_c->dobj==NULL) {
	char buf2[170];
	sprintf(buf2, "Inset %s could not be created.", classname);
	message::DisplayString(v, 10, buf2);
	return figobj_Failed;
    }
    ix=this_c->dobj->Read(fp, id);
    if (ix!=dataobject_NOREADERROR) {
	message::DisplayString(v, 0, "Unable to read inset from cut buffer.");
	((v)->GetIM())->CloseFromCutBuffer( fp);
	return figobj_Failed;
    }
    if (ATK::IsTypeByName(classname, "text")) {
	message::DisplayString(v, 0, "Reading template...");
	((class text *)this_c->dobj)->ReadTemplate( "default", FALSE);
    }
    ((v)->GetIM())->CloseFromCutBuffer( fp);
    return figobj_Done;
}

enum figobj_Status figoins::Build(class figview  *v, enum view_MouseAction  action, long  x , long  y /* in fig coords */, long  clicks)   
{
    enum figobj_Status res;

    if (this->moribund)
	return figobj_NotDone;

    if (clicks==0) {
	this->moribund = TRUE;
	message::CancelQuestion(v);
	message::DisplayString(v, 10, "Object aborted.");
	return figobj_Failed;
    }

    res = (this)->figorect::Build( v, action, x, y, clicks);
    if (res==figobj_Done) {
	if(this->paste) {
	    res=PasteInset(this, v);
	    if(res!=figobj_Done) return res;
	} else {
	    res=ObjectPrompt(this, v);
	    if(res!=figobj_Done) return res;
	}
	(this)->SetModified();
	return res;
    }
    else
	return res;
}

void figoins::WriteBody(FILE  *fp)
{
    (this)->figorect::WriteBody( fp);

    if (this->dobj) {
	(this->dobj)->Write( fp, (this)->GetWriteID(), 2); /* Two?! Two?! Look, man, it's an arbitrary non-zero number. Just deal with it. */
    }
    else {
	fprintf(fp, "\\nodata\n");
    }
}

long figoins::ReadBody(FILE  *fp, boolean  recompute)
{
#define LINELENGTH (256)
    char buf[LINELENGTH+1];
    long tid, ix;
    char namebuf[100];
    const char *np=namebuf;

    ix = (this)->figorect::ReadBody( fp, FALSE);
    if (ix!=dataobject_NOREADERROR) return ix;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    if (!strncmp(buf, "\\nodata", 7)) {
	this->dobj = NULL;
    }
    else {
	ix = sscanf(buf, "\\begindata{%[^,],%ld}", namebuf, &tid);
	if (ix!=2) return dataobject_BADFORMAT;
	if(ATK::LoadClass(np)==NULL) {
	    np="unknown";
	}
	if (!ATK::IsTypeByName(np, "dataobject")) {
	    np="unknown";
	}
	this->dobj = (class dataobject *)ATK::NewObject(np);
	if(this->dobj==NULL) {
	    this->dobj=(class dataobject *)ATK::NewObject("unknown");
	}
	if (!this->dobj) 
	    return dataobject_OBJECTCREATIONFAILED;
	(this->dobj)->Read( fp, tid);
    }

    if (recompute) {
	(this)->RecomputeBounds();
	(this)->SetModified();
    }

    return dataobject_NOREADERROR;
}

void figoins::PrintObject(class figview  *v, FILE  *file, const char  *prefix, boolean newstyle)
{
    long x, y, w, h;

    fprintf(file, "%s  %% inset\n", prefix);

    if ((this)->PosW() >= 0) {
	x = (v)->ToPrintPixX( (this)->PosX());
	w = (v)->ToPrintPixW( (this)->PosW());
    }
    else {
	x = (v)->ToPrintPixX( (this)->PosX()+(this)->PosW());
	w = (v)->ToPrintPixW( -(this)->PosW());
    }
    
    if ((this)->PosH() >= 0) {
	y = (v)->ToPrintPixY( (this)->PosY());
	h = (v)->ToPrintPixH( (this)->PosH());
    }
    else {
	y = (v)->ToPrintPixY( (this)->PosY()+(this)->PosH());
	h = (v)->ToPrintPixH( -(this)->PosH());
    }

    x--;
    y--;
    w+=2;
    h+=2;
    fprintf(file, "%s  %ld %ld moveto  %ld %ld lineto  %ld %ld lineto  %ld %ld lineto closepath\n", prefix, x, y,  x, y+h,  x+w, y+h,  x+w, y);

    fprintf(file, "%s  gsave\n", prefix);
    fprintf(file, "%s  1 setgray\n", prefix);
    fprintf(file, "%s  fill grestore\n", prefix);

    fprintf(file, "%s  1 setlinewidth\n", prefix);
    fprintf(file, "%s  0 setgray\n", prefix);
    fprintf(file, "%s  stroke\n", prefix);
}
