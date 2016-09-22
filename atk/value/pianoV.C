/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("pianoV.H")
#include <pianoV.H>
#include <fontdesc.H>
#include <rect.h>
#include <value.H>
#include <buffer.H>
#include <proctable.H>
#include <atom.H>
#include <atomlist.H>
#include <graphic.H>
#include <rm.H>
#include <view.H>
static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_label;

static class atom *  A_long;
static class atom *  A_string;

#define InternAtoms ( \
   AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
   AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
   AL_label = atomlist::StringToAtomlist("':' separated labels") ,\
   A_long = atom::Intern("long") ,\
   A_string = atom::Intern("string") )


#define Min(X,Y) ((X) < (Y) ? (X) : (Y))
#define FUDGE 2
#define FUDGE2 4


/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(pianoV, valueview, pianoV::InitializeClass);
static void CarveFonts(class pianoV  * self);
static int locateHit(class pianoV  * self,int  x,int  y);
static void parselabels(class pianoV  * self,char  *chr);
static void Drawpiano(class pianoV  * self,boolean  full);


static void CarveFonts(class pianoV  * self)
{
    self->normalfont = fontdesc::Create( self->fontname, fontdesc_Plain, self->fontsize );
    self->boldfont   = fontdesc::Create( self->fontname, fontdesc_Bold,  self->fontsize );
    self->valuefont = fontdesc::Create( "values", fontdesc_Plain, 25);
    self->activefont = self->mouseIsOnTarget ? self->boldfont : self->normalfont;
}  
static int masks[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096 };
static int wn[] = { 1,3,5,6,8,10,12,-1};
static int bn[] = { 2,4,0,7,9,11,-1, 0 };
static int locateHit(class pianoV  * self,int  x,int  y)
{
    int hy,*ip;
    float wid,place;
    hy = (self->height + self->y) / 2;
    if(y < hy){ /* posible black note */
	wid = self->width / 21.0;
	for(place = self->x + wid + wid,ip = bn; x > place; ip++,place += wid){
	    place += wid + wid;
	    if(x < place){
		if(*ip> 0 && x > self->x + wid + wid) return (*ip);
		break;
	    }
	}
    }
    wid = self->width / 7.0 ;
    for(place = self->x + wid,ip = wn; x > place && *ip != 12; ip++,place += wid) ;
    return(*ip);
}

static void parselabels(class pianoV  * self,char  *chr)
{ 
    int i,j;
    for(i = 0 ; i < pianov_NUMLABELS; i++){
	self->label[i] = NULL;
    }
    if(chr == NULL) return;
    i = 0;
    do{
	j = 0;
	self->label[i] = chr;
	while(*chr != ':' && *chr != '\0'){
	    chr++;
	    j++;
	}
	self->lsize[i] = j;
	if(*chr == '\0') break;
	chr++;
    }while (++i < pianov_NUMLABELS);
}
#define NoteOn(self,i) (self->tmpval & masks[i])
#define LabelChanged(self,i) ((self->tmpval & masks[i]) != (self->lastval & masks[i]))
static void Drawpiano(class pianoV  * self,boolean  full)
{
    int hy,*ip;
    float wid,place;
    if(full){
    (self)->SetTransferMode(  graphic_COPY );

    (self)->EraseRectSize( self->x,self->y,self->width,self->height);
    }
    (self)->SetTransferMode(  graphic_BLACK );
    if(full) (self)->DrawRectSize(self->x,self->y,self->width,self->height);
    hy = (self->height + self->y) / 2;
    wid = self->width / 21.0;
    for(place = self->x + wid + wid,ip = bn; *ip != -1 ; ip++,place += wid){
	if(*ip){
	    if(full){
		(self)->MoveTo((int) place,self->y);
		(self)->DrawLineTo((int) place,hy);
	    }
	    place += wid + wid;
	    if(full){
		(self)->DrawLineTo((int) place,hy);
		(self)->DrawLineTo((int) place,self->y);
	    }
	    if(self->label[*ip] && (full ||  LabelChanged(self,*ip))){
		if(!full){
		    (self)->SetTransferMode(  graphic_COPY );
		    (self)->EraseRectSize( (int)(place + 1 - wid - wid),self->y + 1,(int)(wid + wid - 2),(int)(hy - 2));
		    (self)->SetTransferMode(  graphic_BLACK );
		}
		(self)->MoveTo((int)(place - wid), self->y + (hy / 2));
		(self)->SetFont(NoteOn(self,*ip)? self->boldfont : self->normalfont);
		(self)->DrawText(self->label[*ip], self->lsize[*ip],
				      graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM );
	    }

	}
	else 	    place += wid + wid;

    }
    wid = self->width / 7.0 ;
    for(place = self->x + wid,ip = wn;*ip != -1; ip++,place += wid) {
	if(full){
	    (self)->MoveTo((int) place,self->y + self->height);
	    if(*ip != 5)
		(self)->DrawLineTo((int) place,hy);
	    else 
		(self)->DrawLineTo((int) place,self->y);
	}
	if(self->label[*ip]  && (full ||  LabelChanged(self,*ip))){
	    if(!full){
		(self)->SetTransferMode(  graphic_COPY );
		(self)->EraseRectSize( (int)(place - wid + 1),(int)(hy + 1),(int)(wid - 2),(int)hy - 2);
		(self)->SetTransferMode(  graphic_BLACK );
	    }
	    (self)->MoveTo((int)(place - (wid / 2)), self->y + hy +(hy / 2));
	    (self)->SetFont(NoteOn(self,*ip)? self->boldfont : self->normalfont);
	    (self)->DrawText(self->label[*ip], self->lsize[*ip],
				  graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM );
	}
    }
    self->lastval = self->tmpval;
}

/****************************************************************/
/*		class procedures 				*/
/****************************************************************/




boolean pianoV::InitializeClass()
{
    InternAtoms;
    return TRUE;
}




#define BADVAL -22222
/****************************************************************/
/*		instance methods				*/
/****************************************************************/
pianoV::pianoV()
{
	ATKinit;
   
    int i;
    for(i = 0 ; i < pianov_NUMLABELS; i++)
	this->label[i] = NULL;
    this->fontname = NULL;
    this->fontsize = 0;
    this->tmpval = this->lastval =0;
    THROWONFAILURE( TRUE);
}


void pianoV::LookupParameters()
{
    const char * fontname;
    long fontsize;
    static struct resourceList parameters[] = {
	{ AL_label, A_string }, /* 0 */
	{ AL_bodyfont, A_string }, /* 1 */
	{ AL_bodyfont_size, A_long }, /* 2 */
	{ NULL, NULL }
    };

    (this)->GetManyParameters( parameters, NULL, NULL);

    if (parameters[0].found)
	parselabels(this,(char *)parameters[0].data);
    else
	parselabels(this,NULL);

    if (parameters[1].found)
	fontname = (char *)parameters[1].data;
    else
	fontname = "andytype";

    if (parameters[2].found)
	fontsize = parameters[2].data;
    else
	fontsize = 10;

    if (fontsize != this->fontsize || fontname != this->fontname)
    {
	this->fontsize = fontsize;
	this->fontname = fontname;
	CarveFonts(this);
    }
}


void pianoV::DrawFromScratch(long  x,long  y,long  width,long  height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    if (width > 0 && height > 0)
    {
	class value *w = (this)->Value();
	if(this->tmpval == BADVAL){
	    this->tmpval = (w)->GetValue();
	}
	this->activefont = this->mouseIsOnTarget ?
	  this->boldfont : this->normalfont;
	Drawpiano(this,TRUE);	
    }
}


void pianoV::DrawDehighlight()
{

    class value *w = (this)->Value();
    this->activefont = this->normalfont;
    this->tmpval = (w)->GetValue();
    Drawpiano(this,FALSE);	

}

void pianoV::DrawHighlight()
{
/*
    struct value *w = pianoV_Value(self);
    self->activefont = self->boldfont;
    self->tmpval = value_GetValue(w);
    Drawpiano(self);	
*/
}


void pianoV::DrawNewValue( )
{
    class value *w = (this)->Value();
    this->tmpval = (w)->GetValue();
    Drawpiano(this,FALSE);	
}

#define flipbit(A,B) ((A & B)? (A & ~B) : (A | B))

class valueview * pianoV::DoHit( enum view_MouseAction  type,long  x,long  y,long  hits )
{
    class value *tt = (this)->Value();
    int tmp,v,m;
    switch(type){
	case view_LeftDown:
	    v = (tt)->GetValue();
	    m = masks[locateHit(this,x,y)];
	    this->tmpval =  flipbit(v,m);
#ifdef DEBUG
printf("m = %d, self->tmpval = %d, v = %d\n",m,this->tmpval,v);
#endif /* DEBUG */
	    Drawpiano(this,FALSE);
	    break;
	case view_LeftMovement:
	    v = (tt)->GetValue();
	    m = masks[locateHit(this,x,y)];
	    tmp =  flipbit(v,m);
	    if(this->tmpval != tmp){
		this->tmpval = tmp;
		Drawpiano(this,FALSE);
	    }
	    break;
	case view_LeftUp:
	    (tt)->SetValue(this->tmpval);
	    break;
	default:
	    break;
    }  

    return this;
}





