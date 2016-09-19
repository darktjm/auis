/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("GetRecV.H")
#include <GetRecV.H>
#include <fontdesc.H>
#include <rect.h>

#include <value.H>
#include <proctable.H>
#include <graphic.H>
#include <view.H>
#undef MIN
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))


/****************************************************************/
/*		private functions				*/
/****************************************************************/



/****************************************************************/
/*		class procedures 				*/
/****************************************************************/





ATKdefineRegistry(GetRecV, valueview, GetRecV::InitializeClass);

boolean GetRecV::InitializeClass()
{
    return TRUE;
}




#define BADVAL -22222
/****************************************************************/
/*		instance methods				*/
/****************************************************************/
GetRecV::GetRecV()
{
	ATKinit;

    this->x = this->y = this ->width = this->height = 0;
    this->tmpval = NULL;
    this->firstx =this->lastx = this->firsty = this->lasty = 0;
    THROWONFAILURE( TRUE);
}


void GetRecV::LookupParameters()
{

}
#define IsEqualRect(LHS, RHS)( (LHS->left == RHS->left) && (LHS->top == RHS->top) && (LHS->width == RHS->width) && (LHS->height == RHS->height) )
#define CopyRect(LHS, RHS) rectangle_SetRectSize(LHS, rectangle_Left(RHS),rectangle_Top(RHS),rectangle_Width(RHS),rectangle_Height(RHS))
void GetRecV::DrawFromScratch(long  x,long  y,long  width,long  height)
{
    class value *w = (this)->Value();
    if((w)->GetUpdateCount() == 0){
	/* ignore read in value */
	(w)->SetValue(0);
    }
    if (width > 0 && height > 0)
    {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	if(width > ((height * 17) / 22) ) {
	    width = ((height * 17) / 22);
	    (this)->SetTransferMode(  graphic_COPY );	    
	    (this)->FillRectSize(x + width,y,this->width - width,height,(this)->GrayPattern(8,16) );
	}
	else{
	    height = ((width * 22 )/17);
	    (this)->SetTransferMode(  graphic_COPY );	    
	    (this)->FillRectSize(x,y + height, width,this->height - height,(this)->GrayPattern(8,16) );
	}
	if(this->tmpval == NULL){
	    /* initialize view */
	    if((w)->GetValue() == 0){
		/* initialize value */
		this->tmpval = (struct GetRecV_recpair *) malloc(sizeof(struct GetRecV_recpair));
		(w)->SetValue((long) this->tmpval);
		rectangle_SetRectSize(&(this->tmpval->child),x,y,width,height);
	    }
	    else this->tmpval = (struct GetRecV_recpair *) (w)->GetValue();
	}
	else {
	    /* resize child */
	    if(IsEqualRect((&(this->tmpval->parent)),(&(this->tmpval->child))))
	       	rectangle_SetRectSize(&(this->tmpval->child),x,y,width,height);
	    else {
		long xx,yy,ww,hh ;
		float xoff,yoff;
		xoff = (float) width / rectangle_Width(&(this->tmpval->parent)); 
		yoff = (float) height / rectangle_Height(&(this->tmpval->parent));
		rectangle_GetRectSize(&(this->tmpval->child),&xx,&yy,&ww,&hh);
		rectangle_SetRectSize(&(this->tmpval->child),(long) (xoff * xx),(long) (yoff * yy), (long) (xoff * ww), (long) (yoff * hh));
	    }
	}
	rectangle_SetRectSize(&(this->tmpval->parent),x,y,width,height);
	(this)->SetTransferMode(  graphic_INVERT );	    
	(this)->DrawRect(&(this->tmpval->child));
    }
}

void GetRecV::DrawDehighlight()
{
    (this)->SetTransferMode(  graphic_COPY );
    (this)->EraseRect( &(this->tmpval->parent));
    (this)->SetTransferMode(  graphic_INVERT );	    
    (this)->DrawRect(&(this->tmpval->child));

}
void GetRecV::DrawHighlight()
{
  /*
    GetRecV_SetTransferMode( self, graphic_COPY );
    GetRecV_EraseRect( self,&(self->tmpval->parent));
    GetRecV_SetTransferMode( self, graphic_INVERT );	    
    GetRecV_DrawRect(self,&(self->tmpval->child));
*/
}
void GetRecV::DrawNewValue( )
{
    (this)->SetTransferMode(  graphic_COPY );
    (this)->EraseRect( &(this->tmpval->parent));
    (this)->SetTransferMode(  graphic_INVERT );	    
    (this)->DrawRect(&(this->tmpval->child));

}
#define CREC(rec,self) rectangle_SetRectSize(&rec,MIN(self->firstx,self->lastx),MIN(self->firsty,self->lasty),ABS(self->firstx - self->lastx),ABS(self->firsty - self->lasty))
#define OutBounds(SELF,X,Y)((X  + rectangle_Left(&(SELF->tmpval->parent))> rectangle_Width(&(SELF->tmpval->parent))) || (Y + rectangle_Top(&(SELF->tmpval->parent)))> rectangle_Height(&(SELF->tmpval->parent)))
class valueview * GetRecV::DoHit( enum view_MouseAction  type,long  x,long  y,long  hits )
{
    class value *tt = (this)->Value();
    struct rectangle rec;
    switch(type){
	case view_LeftDown:
	    if(OutBounds(this,x,y)){
		this->firstx = -1;
		break;
	    }
	    (this)->SetTransferMode(  graphic_COPY );
	    (this)->EraseRect( &(this->tmpval->parent)); 
	    (this)->SetTransferMode(  graphic_INVERT );	    
	    this->lasty = this->firsty = y;
	    this->lastx = this->firstx = x;
	    break;
	case view_LeftMovement	:   
	    if( OutBounds(this,x,y)) break;
	case view_LeftUp:
            if(this->firstx == -1) break;
	    if(this->lastx != this->firstx || this->lasty != this->firsty){
		CREC(rec,this);
		(this)->DrawRect(&rec);
	    }
	    if(!OutBounds(this,x,y)){
		this->lastx = x;
		this->lasty = y;
	    }
	    if(type == view_LeftUp && this->lastx == this->firstx && this->lasty == this->firsty){
		CopyRect(&(this->tmpval->child),&(this->tmpval->parent));
		(tt)->SetValue((long)this->tmpval);
		break;
	    }
	    CREC(rec,this);
	    (this)->DrawRect(&rec);
	    if(type ==  view_LeftMovement) break;
	    CREC((this->tmpval->child),this);
	    (tt)->SetValue((long)this->tmpval);
	    break;
	default:
	    break;
    }

    return this;
}





