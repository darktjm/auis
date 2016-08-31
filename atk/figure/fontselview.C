/* fontselv.c - font selection inset view */
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
ATK_IMPL("fontselview.H")
#include <fontselview.H>

#include <dataobject.H>
#include <message.H>
#include <proctable.H>
#include <stringtbl.H>
#include <strtblview.H>
#include <lpair.H>
#include <fontdesc.H>

#include <fontsample.H>
#include <fontsel.H>

#define DEFAULTDEFAULT "<default>"

struct stylelayout_t {
    char name[16]; /* name in the toolset window */
    long style;	    /* fontdesc_Whatever */
};

#define	FAMILIES_NUM_INIT (3)
#define fontselv_family_Other (-1)
#define fontselv_family_Zero (0)
static char familylayout[FAMILIES_NUM_INIT][16] = {
    "andy",
    "andysans", 
    "andytype"
};

#define	STYLE_NUM_INIT (2)
#define fontselv_style_Zero (0)
static struct stylelayout_t stylelayout[STYLE_NUM_INIT] = {
    {"bold",	fontdesc_Bold},
    {"italic",	fontdesc_Italic}
};

#define	SIZES_NUM_INIT (5)
#define fontselv_size_Other (0)
#define fontselv_size_Zero (1)

static char sizelayout[SIZES_NUM_INIT][16] = {
    "<other>",
    "8",
    "10", 
    "12", 
    "14"
};

    



ATKdefineRegistry(fontselview, lpair, fontselview::InitializeClass);
static void InsertSize(class fontselview  *self, short  val);
static void SetSizeProc(class stringtbl  *st, class fontselview  *self, short  accnum);
static void SetStyleProc(class stringtbl  *st, class fontselview  *self, short  accnum);
static void SetFamilyProc(class stringtbl  *st, class fontselview  *self, short  accnum);
static void ShowExtraProc(class fontselview  *self, long  rock);
static char *CopyString(char  *str);


boolean fontselview::InitializeClass()
{
    return TRUE;
}

fontselview::fontselview()
{
	ATKinit;

    int ix;
    class stringtbl *tl;
    class strtblview *tlv;

    this->showdefault = FALSE;
    this->defaultstring = (char *)malloc(strlen(DEFAULTDEFAULT)+1);
    strcpy(this->defaultstring, DEFAULTDEFAULT);

    this->familynum = fontselv_family_Zero;
    this->sizenum = fontselv_size_Zero+2; /* 12 points */
    this->style_mask = fontdesc_Plain;

    this->sample = new fontsample;
    this->lp1 = new lpair;
    this->lp2 = new lpair;

    if (!this->lp1 || !this->lp2 || !this->sample)
	THROWONFAILURE( FALSE);

    /* family table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->families_num = FAMILIES_NUM_INIT;
    this->familyextra = -1;
    this->familyacc = (short *)malloc(sizeof(short) * this->families_num);
    this->familylist = (char **)malloc(sizeof(char *) * this->families_num);
    for (ix=0; ix<this->families_num; ix++) {
	this->familyacc[ix] = (tl)->AddString( familylayout[ix]);
	if (this->familyacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
	if (ix>fontselv_family_Other)
	    this->familylist[ix] = CopyString(familylayout[ix]);
    }
    (tlv)->SetItemHitProc( (strtblview_hitfptr)SetFamilyProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->familyacc[this->familynum], TRUE);
    (tlv)->SetDataObject( tl);
    this->familytbl = tl;
    this->vfamilytbl = tlv;

    /* style table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->styles_num = STYLE_NUM_INIT;
    this->styleextra = -1;
    this->styleacc = (short *)malloc(sizeof(short) * this->styles_num);
    this->stylelist = (long *)malloc(sizeof(long) * this->styles_num);
    for (ix=0; ix<this->styles_num; ix++) {
	this->styleacc[ix] = (tl)->AddString( stylelayout[ix].name);
	if (this->styleacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
	this->stylelist[ix] = stylelayout[ix].style;
    }
    (tlv)->SetItemHitProc( (strtblview_hitfptr)SetStyleProc, (long)this);
    (tl)->ClearBits();
    (tlv)->SetDataObject( tl);
    this->styletbl = tl;
    this->vstyletbl = tlv;

    /* size table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->sizes_num = SIZES_NUM_INIT;
    this->sizeextra = -1;
    this->sizeacc = (short *)malloc(sizeof(short) * this->sizes_num);
    this->sizelist = (short *)malloc(sizeof(short) * this->sizes_num);
    for (ix=0; ix<this->sizes_num; ix++) {
	this->sizeacc[ix] = (tl)->AddString( sizelayout[ix]);
	if (this->sizeacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
	if (ix>fontselv_size_Other)
	    this->sizelist[ix] = atoi(sizelayout[ix]);
    }
    (tlv)->SetItemHitProc( (strtblview_hitfptr)SetSizeProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->sizeacc[this->sizenum], TRUE);
    (tlv)->SetDataObject( tl);
    this->sizetbl = tl;
    this->vsizetbl = tlv;

    (this->lp1)->SetUp( this->vsizetbl, this->vfamilytbl, 50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);
    (this->lp2)->SetUp( this->vstyletbl, this->sample, 50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);
    (this)->SetUp( this->lp1, this->lp2, 30, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);

    THROWONFAILURE( TRUE);
}

fontselview::~fontselview()
{
	ATKinit;

    int ix;

    (this)->SetNth( 0, NULL);
    (this)->SetNth( 1, NULL);

    (this->lp1)->Destroy();
    (this->lp2)->Destroy();

    (this->vsizetbl)->SetDataObject( NULL);
    (this->vstyletbl)->SetDataObject( NULL);
    (this->vfamilytbl)->SetDataObject( NULL);

    (this->sample)->Destroy();
    (this->vsizetbl)->Destroy();
    (this->vstyletbl)->Destroy();
    (this->vfamilytbl)->Destroy();

    (this->sizetbl)->Destroy();
    (this->styletbl)->Destroy();
    (this->familytbl)->Destroy();

    for (ix=0; ix<this->families_num; ix++) {
	if (ix>fontselv_family_Other)
	    free(this->familylist[ix]);
    }
    free(this->sizeacc);
    free(this->sizelist);
    free(this->styleacc);
    free(this->stylelist);
    free(this->familyacc);
    free(this->familylist);
}

void fontselview::SetDataObject(class dataobject  *dobj)
{
    (this)->lpair::SetDataObject( dobj);
    (this->sample)->SetDataObject( dobj);
}

void fontselview::ObservedChanged(class observable  *d, long  status)
{
    class fontsel *dobj=(class fontsel *)d;
    long ix, vnum, accnum;
    const char *cx;

    if (status == observable_OBJECTDESTROYED) {
    }
    else if (status == fontsel_DATACHANGED) {

	if ((dobj)->IsActive( fontsel_Family)) {
	    cx = (dobj)->GetFamily();
	    for (vnum=fontselv_family_Zero; vnum<this->families_num; vnum++)
		if (!strcmp(this->familylist[vnum], cx)) break;
	    if (vnum==this->families_num) {
		vnum = this->families_num - 1;
	    }
	    this->familynum = vnum;
	    accnum = this->familyacc[vnum];
	    if (!(this->familytbl)->GetBitOfEntry( accnum)) {
		(this->familytbl)->ClearBits();
		(this->familytbl)->SetBitOfEntry( accnum, TRUE);
	    }
	}
	else {
	    (this)->ShowExtraOption();
	    this->familynum = (-1);
	    accnum = this->familyextra;
	    if (!(this->familytbl)->GetBitOfEntry( accnum)) {
		(this->familytbl)->ClearBits();
		(this->familytbl)->SetBitOfEntry( accnum, TRUE);
	    }
	}

	if ((dobj)->IsActive( fontsel_Size)) {
	    ix = (dobj)->GetSize();
	    for (vnum=fontselv_size_Zero; vnum<this->sizes_num; vnum++)
		if (this->sizelist[vnum] == ix) break;
	    if (vnum==this->sizes_num) {
		InsertSize(this, ix);
		vnum = this->sizes_num - 1;
	    }
	    this->sizenum = vnum;
	    accnum = this->sizeacc[vnum];
	    if (!(this->sizetbl)->GetBitOfEntry( accnum)) {
		(this->sizetbl)->ClearBits();
		(this->sizetbl)->SetBitOfEntry( accnum, TRUE);
	    }
	}
	else {
	    (this)->ShowExtraOption();
	    this->sizenum = (-1);
	    accnum = this->sizeextra;
	    if (!(this->sizetbl)->GetBitOfEntry( accnum)) {
		(this->sizetbl)->ClearBits();
		(this->sizetbl)->SetBitOfEntry( accnum, TRUE);
	    }
	}

	if ((dobj)->IsActive( fontsel_Style)) {
	    ix = (dobj)->GetStyle();
	    (this->styletbl)->ClearBits();
	    for (vnum=fontselv_style_Zero; vnum<this->styles_num; vnum++) {
		if (this->stylelist[vnum] & ix) {
		    accnum = this->styleacc[vnum];
		    (this->styletbl)->SetBitOfEntry( accnum, TRUE);
		}
	    }
	    this->style_mask = ix;
	}
	else {
	    (this)->ShowExtraOption();
	    accnum = this->styleextra;
	    if (!(this->styletbl)->GetBitOfEntry( accnum)) {
		(this->styletbl)->ClearBits();
		(this->styletbl)->SetBitOfEntry( accnum, TRUE);
	    }
	}

	(this)->WantUpdate( this);
    }
}

static void InsertSize(class fontselview  *self, short  val)
{
    class stringtbl *st = self->sizetbl;   
    char name[16];
    int ix;

    ix = self->sizes_num;
    self->sizeacc = (short *)realloc(self->sizeacc, sizeof(short) * (1+self->sizes_num));
    self->sizelist = (short *)realloc(self->sizelist, sizeof(short) * (1+self->sizes_num));

    sprintf(name, "%d", val);
    
    if (self->sizeextra != (-1)) {
	(st)->RemoveEntry( self->sizeextra);
	self->sizeacc[ix] = (st)->AddString( name);
	self->sizeextra = (st)->AddString( self->defaultstring);
    }
    else {
	self->sizeacc[ix] = (st)->AddString( name);
    }

    if (self->sizeacc[ix] == (-1))
	return; 
    self->sizelist[ix] = val;

    self->sizes_num++;
}

static void SetSizeProc(class stringtbl  *st, class fontselview  *self, short  accnum)
{
    int sizenum;
    class fontsel *fontsel = (class fontsel *)(self)->GetDataObject();

    if (accnum==self->sizeextra) {
	sizenum = (-1);
    }
    else {
	for (sizenum=0; sizenum<self->sizes_num; sizenum++) {
	    if (self->sizeacc[sizenum] == accnum) break;
	}
	if (sizenum==self->sizes_num) {
	    return;
	}
    }

    if (sizenum == fontselv_size_Other) {
	int ix;
	short val;
	char buffer[32];
	int res;

	res = message::AskForString (self, 40, "Enter a new font size:  ", NULL, buffer, 30); 
	if (res<0 || strlen(buffer)==0) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}
	val = atoi(buffer);

	if (val<1) {
	    message::DisplayString(self, 10, "Value must be a positive integer.");
	    return;
	}
	if (val>144) {
	    message::DisplayString(self, 10, "That value is too large.");
	    return;
	}

	for (ix=fontselv_size_Zero; ix<self->sizes_num; ix++)
	    if (self->sizelist[ix] == val) {
		message::DisplayString(self, 10, "That value is already available.");
		sizenum = ix;
		break;
	    }
	if (sizenum == fontselv_size_Other) {
	    InsertSize(self, val);

	    for (sizenum=fontselv_size_Zero; sizenum<self->sizes_num; sizenum++) {
		if (self->sizelist[sizenum] == val) break;
	    }
	    if (sizenum==self->sizes_num) {
		return;
	    }
	}
	accnum = self->sizeacc[sizenum];
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }

    if (self->sizenum != sizenum) {
	self->sizenum = sizenum;
	if (sizenum == (-1))
	    (fontsel)->UnsetSize();
	else
	    (fontsel)->SetSize( self->sizelist[sizenum]);
	(fontsel)->NotifyObservers( fontsel_DATACHANGED);
    }
}

static void SetStyleProc(class stringtbl  *st, class fontselview  *self, short  accnum)
{
    int stylenum;
    class fontsel *fontsel = (class fontsel *)(self)->GetDataObject();

    if (accnum==self->styleextra) {
	stylenum = (-1);
    }
    else {
	for (stylenum=0; stylenum<self->styles_num; stylenum++) {
	    if (self->styleacc[stylenum] == accnum) break;
	}
	if (stylenum==self->styles_num) {
	    return;
	}
    }

    if (stylenum == (-1)) {
	if (!(st)->GetBitOfEntry( accnum)) {
	    self->style_mask = fontsel_default_Style;
	    (fontsel)->SetStyle( self->style_mask);
	    (fontsel)->UnsetStyle();
	    (st)->ClearBits();
	    (st)->SetBitOfEntry( accnum, TRUE);
	}
	else {
	    self->style_mask = fontsel_default_Style;
	    (fontsel)->SetStyle( self->style_mask);
	    (st)->ClearBits();
	}
    }
    else {
	if (!(st)->GetBitOfEntry( accnum)) {
	    (st)->SetBitOfEntry( accnum, TRUE);
	    self->style_mask |= self->stylelist[stylenum];
	}
	else {
	    (st)->SetBitOfEntry( accnum, FALSE);
	    self->style_mask &= ~(self->stylelist[stylenum]);
	}
	if (self->styleextra != (-1))
	    (st)->SetBitOfEntry( self->styleextra, FALSE);
	(fontsel)->SetStyle( self->style_mask);
    }

    (fontsel)->NotifyObservers( fontsel_DATACHANGED);
}

static void SetFamilyProc(class stringtbl  *st, class fontselview  *self, short  accnum)
{
    int familynum;
    class fontsel *fontsel = (class fontsel *)(self)->GetDataObject();

    if (accnum==self->familyextra) {
	familynum = (-1);
    }
    else {
	for (familynum=0; familynum<self->families_num; familynum++) {
	    if (self->familyacc[familynum] == accnum) break;
	}
	if (familynum==self->families_num) {
	    return;
	}
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }

    if (self->familynum != familynum) {
	self->familynum = familynum;
	if (familynum == (-1)) {
	    (fontsel)->UnsetFamily();
	}
	else {
	    (fontsel)->SetFamily( self->familylist[familynum]);
	}
	(fontsel)->NotifyObservers( fontsel_DATACHANGED);
    }
}

void fontselview::SetExtraOptionString(char  *val)
{
    if (!val)
	return;

    if (this->defaultstring)
	free(this->defaultstring);
    this->defaultstring = CopyString(val);
}

void fontselview::ShowExtraOption()
{
    if (this->showdefault)
	return;

    this->showdefault = TRUE;

    this->sizeextra = (this->sizetbl)->AddString( this->defaultstring);
    this->styleextra = (this->styletbl)->AddString( this->defaultstring);
    this->familyextra = (this->familytbl)->AddString( this->defaultstring);
}

static void ShowExtraProc(class fontselview  *self, long  rock)
{
    (self)->ShowExtraOption();
}

static char *CopyString(char  *str)
{
    char *tmp;

    if (str==NULL)
	return NULL;
    tmp = (char *)malloc(strlen(str)+1);
    if (!tmp)
	return NULL;
    strcpy(tmp, str);
    return tmp;
}

