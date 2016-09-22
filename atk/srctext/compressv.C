/* File compressv.c created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   compressv, a view to display a box where the hidden text lies. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <view.H>
#include <rect.h>
#include <fontdesc.H>
#include <cursor.H>
#include <environ.H>
#include <textview.H>
#include <text.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <proctable.H>
#include <bind.H>
#include <message.H>

#include "compress.H"
#include "compressv.H"

static fontdesc *boxfont;
static const char *boxfgcolor=NULL, *boxbgcolor=NULL;
static int boxwidth=0;
static keymap *c_Map;
static menulist *c_Menus;

ATKdefineRegistry(compressv, view, compressv::InitializeClass);
static void compressLines(textview *self, char *rString);
static void decompressLines(textview *self, char *rString);
static void compressRegion(textview *self, long rock);
static void decompressAll(textview *self, long rock);

static struct bind_Description compressBindings[]={
    {"compressv-compress-lines",NULL,0, NULL,0,0, (proctable_fptr)compressLines, "Compresses lines in the specified range (or selected region) into a box; will prompt if none specified."},
    {"compressv-decompress-lines",NULL,0, NULL,0,0, (proctable_fptr)decompressLines, "Takes any lines within the specified range (or selected region) out of the box they were in."},
    {"compressv-compress-region",NULL,0, NULL,0,0, (proctable_fptr)compressRegion, "Blindly compresses the selected region, doesn't know how to combine compresses or grab entire lines, but useful if you WANT to nest compresses."},
    {"compressv-decompress-all",NULL,0, NULL,0,0, (proctable_fptr)decompressAll, "Expand all compressed regions in document to normal size."},
    NULL
};

void compressv::GetOrigin(long width, long height, long *originX, long *originY)
{
    *originX = 0;
    *originY = height;
    return;
}

/* compressv_BoxText returns the string that should appear in the box that gets drawn. It points to static storage, so either use the return value immediately, or make a copy of it. */
char *compressv::BoxText()
{
    static char boxstr[256];
    compress *cmprs=(compress *)GetDataObject();
    int numlines=(cmprs)->GetLines();
    sprintf(boxstr, "%1d compressed line%s", numlines,(numlines!=1)?"s":"");
    return boxstr;
}

void compressv::Print(FILE *f, const char *process, const char *final, int toplevel)
{
    /* note: the placement of the troff font size changes, \s-2 and \s+2, is not as haphazard as they may seem. If the -2 size includes BOTH spaces, the left edge of the box gets disconnected. If the -2 size includes NEITHER space, the top and bottom edges extend too far left. */
    fprintf(f, "\\(br \\s-2%s \\s+2\\(br\\l'|0\\(rn'\\l'|0\\(ul'", BoxText());
}

view *compressv::Hit(enum view_MouseAction action, long mousex, long mousey, long numberOfClicks)
{
    text *txt=(text *)this->parenttext;
    compress *fn=(compress *)GetDataObject();
    if (action==view_LeftUp || action==view_RightUp) {
	long mod=(txt)->GetModified();
	(fn)->DecompressBox(txt);
	(txt)->RestoreModified(mod);
	(txt)->NotifyObservers(0);
	if (action==view_LeftUp) {
	    /* put cursor around decompressed text for easy re-compression */
	    (this->parentview)->SetDotPosition(fn->loc);
	    (this->parentview)->SetDotLength((fn)->GetLength());
	}
    }
    return (view *)this;
}

view_DSattributes compressv::DesiredSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    (boxfont)->StringSize(GetDrawable(), BoxText(), desiredwidth,desiredheight);
    *desiredwidth= (boxwidth>0) ? boxwidth : *desiredwidth+8; 
    *desiredheight= (boxfont)->FontSummary(GetDrawable()) -> maxHeight;
    return (view_HeightFlexible | view_WidthFlexible);
}

static void DoUpdate(compressv *self, boolean full)
{
    struct rectangle enclosingRect;
    enclosingRect.top = 0; enclosingRect.left = 0;
    enclosingRect.width  = (self)->GetLogicalWidth();
    enclosingRect.height = (self)->GetLogicalHeight();
    (self)->PostCursor(&(enclosingRect),self->cursor);
    (self)->EraseRect(&(enclosingRect));
    enclosingRect.width--; enclosingRect.height--;
    if (!boxbgcolor)
	(self)->DrawRect(&(enclosingRect));

    /* draw string in center of box */
    (self)->MoveTo((enclosingRect.width+1)/2, (enclosingRect.height-1)/2);
    (self)->DrawString((self)->BoxText(),(view_BETWEENTOPANDBOTTOM | view_BETWEENLEFTANDRIGHT));
}

void compressv::LinkTree(view *parent)
{
    compress *fn=(compress *)GetDataObject();
    (this)->view::LinkTree(parent);
    while (!ATK::IsTypeByName((parent)->GetTypeName(), "textview"))
	if ((parent=parent->parent) == NULL) return;
    this->parenttext= (text *)(parent)->GetDataObject();
    this->parentview= (textview *)parent;
    if (GetDrawable() && GetIM()) { /* make sure the graphic & im exist */
	SetFont(boxfont);
	if (boxbgcolor) SetBackgroundColor(boxbgcolor, 0,0,0);
	if (boxfgcolor) SetForegroundColor(boxfgcolor, 0,0,0);
    }
    if (fn) /* make sure we HAVE a dataobject */
	(fn)->SetLines((fn)->GetLineForPos((fn)->GetLength())); /* I wanted to do this in compress_Compress, but the value didn't get copied when the compress box got copied. */
}

void compressv::FullUpdate(enum view_UpdateType type, long left, long top, long width, long height)
{
    DoUpdate(this,TRUE);
}

void compressv::Update()
{
    DoUpdate(this,FALSE);
}

void compressv::ReceiveInputFocus()
{
    (this)->view::ReceiveInputFocus();
    if (this->parentview) {	
	/* this should only happen when the view is created */
	(this->parentview)->SetDotPosition((this->parentview)->GetDotPosition() + 1);
	(this->parentview)->WantInputFocus(this->parentview);
    }
}

/* blow ourselves away if our compress got decompressed */
void compressv::ObservedChanged(observable *changed, long value)
{
    boolean destroy=FALSE;
    /* check this BEFORE we call super_, because super_ will set our dataobject to NULL if it IS destroyed. */
    if (changed==(observable *)GetDataObject() && value==observable_OBJECTDESTROYED)
	destroy= TRUE;
    (this)->view::ObservedChanged(changed,value);
    if (destroy)
	Destroy();
}

compressv::compressv()
{
    ATKinit;
    this->cursor= cursor::Create(this);
    (this->cursor)->SetStandard(Cursor_CrossHairs);
    this->parenttext= NULL;
    this->parentview= NULL;
    THROWONFAILURE(TRUE);
}

compressv::~compressv()
{
    ATKinit;
    if (this->cursor!=NULL) delete this->cursor;
}

boolean compressv::InitializeClass()
{
    char family[256];
    long style=0, size=8;    
    const char *fontname= environ::GetProfile("CompressBoxFont");
    boxwidth= environ::GetProfileInt("CompressBoxWidth", 0);
    boxfgcolor= environ::GetProfile("CompressBoxForegroundColor");
    boxbgcolor= environ::GetProfile("CompressBoxBackgroundColor");
    if (!fontname || !*fontname || !fontdesc::ExplodeFontName(fontname, family,sizeof(family), &style,&size)) {
	/* either fontname is NULL or empty, or the name is invalid */
	if (fontname && *fontname)
	    fprintf(stderr, "Invalid font specified in CompressBoxFont preference.\n");
	strcpy(family, "andysans");
	style= 0;
	size= 8;
    }
    boxfont= fontdesc::Create(family, style, size);
    c_Menus = new menulist;
    c_Map = new keymap;
    bind::BindList(compressBindings,c_Map,c_Menus,ATK::LoadClass("textview"));
    return TRUE;
}

/* engulfBoxes snags any compress insets that are butted up against the ends of the specified region, and includes them in the region */
static void engulfBoxes(text *txt, long *ppos, long *plen)
{
    while (compress::IsThere(txt,*ppos-1) || ((txt)->GetChar(*ppos-1)=='\n' && compress::IsThere(txt,*ppos-2))) {
	--*ppos;
	++*plen;
    }
    while (compress::IsThere(txt,*ppos+*plen) || ((txt)->GetChar(*ppos+*plen)=='\n' && compress::IsThere(txt,*ppos+*plen+1)))
	++*plen;
}

static boolean getLinesFromUser(textview *self, char *rString, const char *prompt, long *pfirst, long *plast)
{
    text *txt=(text *)(self)->GetDataObject();
    char range[32];
    int numbers;
    *range= '\0';
    if (!rString || !*rString || (unsigned long)rString<256) { /* none specified */
	long pos=(self)->GetDotPosition(), len=(self)->GetDotLength();
	if (len>0)
	    sprintf(range, "%ld-%ld", (txt)->GetLineForPos(pos), (txt)->GetLineForPos(pos+len));
	else if (message::AskForString(self, 0, prompt, NULL, range, sizeof(range)) < 0)
	    return FALSE;
    } else
	strcpy(range, rString);
    if (!*range) return FALSE;
    numbers= sscanf(range, " %ld%*[^0123456789]%ld ", pfirst,plast);
    if (numbers<1) return FALSE;
    if (numbers==1) *plast= *pfirst;
    return TRUE;
}

static void compressLines(textview *self, char *rString)
{
    text *txt=(text *)(self)->GetDataObject();
    long mod=(txt)->GetModified();
    long pos,len, first,last;
    if (!getLinesFromUser(self, rString,"First,last line to compress: ", &first,&last))
	return;
    pos= (txt)->GetPosForLine(first);
    len= (txt)->GetPosForLine(last+1) - pos;

    /* make sure we aren't just trying to re-compress a box */
    if (len>1 || !compress::IsThere(txt,pos)) {
	engulfBoxes(txt,&pos,&len);
	last= (txt)->GetLineForPos(pos+len-1); /* find last engulfed line */
	compress::DecompressRange(txt,pos,len);
	len= (txt)->GetPosForLine(last+1) - pos; /* recalculate length */
	compress::Compress(txt,pos,len);
    }
    (txt)->RestoreModified(mod);
    (txt)->NotifyObservers(0);
}

static void decompressLines(textview *self, char *rString)
{
    text *txt=(text *)(self)->GetDataObject();
    long mod=(txt)->GetModified();
    long pos,len, first,last;
    compress *cprs;
    if (!getLinesFromUser(self, rString,"First,last line to decompress: ", &first,&last))
	return;
    pos= (txt)->GetPosForLine(first);
    len= (txt)->GetPosForLine(last+1) - pos;

    /* check for partial decompression at end of region */
    if (len>0 && (cprs= compress::IsThere(txt,pos+len-1)) != NULL) {
	long firstline= (txt)->GetLineForPos(pos+len-1);
	long extractlen= (cprs)->GetPosForLine(last-firstline+2);
	(cprs)->PartialDecompress(txt, 0,extractlen);
    }
    /* check for partial decompression at beginning of region (this may make our pos and len values invalid) */
    if (len>0 && (cprs= compress::IsThere(txt,pos-1)) != NULL) {
	long firstline= (txt)->GetLineForPos(pos-1);
	long extractpos= (cprs)->GetPosForLine(first-firstline+1) -1 /* grab newline too */;
	(cprs)->PartialDecompress(txt, extractpos, (cprs)->GetLength()-extractpos);
    }
    if (len>0) {
	/* recalculate pos and len */
	pos= (txt)->GetPosForLine(first);
	len= (txt)->GetPosForLine(last+1) -pos -1 /* omit newline */;
	compress::DecompressRange(txt,pos,len);
    } else if ((cprs=compress::IsThere(txt,pos-1)) != NULL) {
	/* length is zero, so entire range must be INSIDE a single box */
	long firstline= (txt)->GetLineForPos(pos-1);
	long extractpos= (cprs)->GetPosForLine(first-firstline+1) -1 /* grab newline too */;
	long extractlen= (cprs)->GetPosForLine(last-firstline+2) -extractpos;
	(cprs)->PartialDecompress(txt, extractpos,extractlen);
    }
    (txt)->RestoreModified(mod);
    (txt)->NotifyObservers(0);
}

static void compressRegion(textview *self, long rock)
{
    text *txt=(text *)(self)->GetDataObject();
    long mod=(txt)->GetModified();
    long pos=(self)->GetDotPosition(), len=(self)->GetDotLength();
    if (len>0) {
	compress::Compress(txt,pos,len);
	(txt)->RestoreModified(mod);
	(txt)->NotifyObservers(0);
    }
}

static void decompressAll(textview *self, long rock)
{
    text *txt=(text *)(self)->GetDataObject();
    long mod=(txt)->GetModified();
    compress::DecompressAll(txt);
    (txt)->RestoreModified(mod);
    (txt)->NotifyObservers(0);
}
