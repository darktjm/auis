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

#include <andrewos.h>
ATK_IMPL("graphic.H")
#include <fontdesc.H>
#include <region.H>
#include <graphic.H>
#include <physical.h>
#include <im.H>

static char *foregroundColorName = NULL; /* Name of the default foreground color. */
static char *backgroundColorName = NULL; /* Name of the default background color. */


ATKdefineRegistryNoInit(graphic, observable);


void graphic::MoveToPt(struct point  * NewPosition )
{
    (this)->MoveTo(point_X(NewPosition),point_Y(NewPosition));
}


void graphic::MoveTo(long  NewX, long  NewY)
{
    point_SetPt(&this->currentPoint,NewX,NewY);
}

void graphic::Move(long  DeltaX, long  DeltaY )
{
    point_OffsetPoint(&this->currentPoint,DeltaX,DeltaY);
}

void graphic::GetCurrentPt(struct point  * Pt)
{
    *Pt = this->currentPoint;
}

void graphic::DrawLineToPt(struct point  * LineEnd)
{
    (this)->DrawLineTo(point_X(LineEnd),point_Y(LineEnd));
}

void graphic::DrawLineTo(long  XEnd, long  YEnd )
{
    point_SetPt(&this->currentPoint,XEnd,YEnd);
}

void graphic::DrawLine(long  DeltaX, long  DeltaY )
{
    point_OffsetPoint(&this->currentPoint,DeltaX,DeltaY);
}

void graphic::DrawString(const char  * Text, short  Operation )
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: DrawString not implemented for this wm\n");
    }
}

void graphic::DrawText(const char  * Text, long  TextLength, short  Operation)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: DrawText not implemented for this wm\n");
    }
}

void graphic::SetSpaceShim(short  Amount)
{
    this->spaceShim = Amount;
}

short graphic::GetSpaceShim()
{

    return this->spaceShim;
}


void graphic::SetFont(class fontdesc  * ChosenFont)
{

    if (ChosenFont) {
	this->currentFont = ChosenFont;
    }
    /* Damn, nothing there, so switch back to default */
    else {
	this->currentFont = fontdesc::Create("andysans", fontdesc_Plain, 12);
    }
}

class fontdesc * graphic::GetFont()
{
    return this->currentFont;
}

void graphic::DrawRectSize(long  x, long  y,long  width,long  height)
{
    long left = x;
    long right = x+width;
    long top = y;
    long bottom = y+height;

    if (left > right || top > bottom) return;

    (this)->MoveTo(left,top);
    (this)->DrawLineTo(right,top);
    (this)->DrawLineTo(right,bottom);
    (this)->DrawLineTo(left,bottom);
    (this)->DrawLineTo(left,top);
}

void graphic::DrawRect(struct rectangle  * Rect)
{
    (this)->DrawRectSize(rectangle_Left(Rect),rectangle_Top(Rect), rectangle_Width(Rect), rectangle_Height(Rect));
}

void graphic::DrawPolygon(struct point  * PointArray, short  PointCount)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: DrawPolygon not implemented for this wm\n");
    }
}

void graphic::DrawPath(struct point  * PointArray, short  PointCount )
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: DrawPath not implemented for this wm\n");
    }
}

void graphic::DrawOvalSize(long  x,long  y,long  width,long  height)
{
    /* An approximation for now */
    (this)->DrawRectSize(x,y,width,height);
}

void graphic::DrawOval(struct rectangle  * Rect)
{
    (this)->DrawOvalSize(rectangle_Left(Rect),
			  rectangle_Top(Rect),rectangle_Width(Rect),
			  rectangle_Height(Rect));
}

void graphic::DrawArcSize(long  x,long  y,long  width,long  height, short  StartAngle, short  OffsetAngle)
{
    /* Cheap imitation by a diagonal line */
    (this)->MoveTo(x,y);
    (this)->DrawLineTo(x+width,y+height);
}

void graphic::DrawArc(struct rectangle  * EnclRect, short  StartAngle, short  OffsetAngle)
{

    (this)->DrawArcSize(rectangle_Left(EnclRect),
			 rectangle_Top(EnclRect), rectangle_Width(EnclRect),
			 rectangle_Height(EnclRect), StartAngle, OffsetAngle);
}

void graphic::DrawRRectSize(long  x,long  y,long  width,long  height,long  cornerWidth,long  cornerHeight )
{
    /* Handle pathologic cases in system indepedent manner
      (luser desires to bite bullet in efficiency) */

    if ( (2*cornerHeight >= height) || (2*cornerWidth >= width)) {
	/* Bizarre -- corners are bigger than rectangle, so 
	 make an appropriate looking oval */
	if ( (2*cornerHeight >= height) && (2*cornerWidth >= width))
	    (this)->DrawOvalSize(x,y,width,height);
	else if (2*cornerHeight >= height) {
	    /* Draw left semi-oval */
	    (this)->DrawArcSize(x,y,2*cornerWidth,height,0,-180);
	    /* Draw Top line */
	    (this)->MoveTo(x+cornerWidth,y);
	    (this)->DrawLine(width-2*cornerWidth,0);
	    /* Draw right semi-oval */
	    (this)->DrawArcSize(x+width-2*cornerWidth,y,2*cornerWidth,height,0,180);
	    /* Draw bottom line */
	    (this)->MoveTo(x+cornerWidth,y+height);
	    (this)->DrawLine(width-2*cornerWidth,0);
	}
	else { /* assuming (2*cornerWidth >= width) */
	    /* Draw top semi-oval */
	    (this)->DrawArcSize(x,y,width,2*cornerHeight,-90,180);
	    /* Draw right line */
	    (this)->MoveTo(x+width,y+cornerHeight);
	    (this)->DrawLine(0,height-2*cornerHeight);
	    /* Draw bottom semi-oval */
	    (this)->DrawArcSize(x,y+height-2*cornerHeight,width,2*cornerHeight,90,180);
	    /* Draw left line */
	    (this)->MoveTo(x,y+cornerHeight);
	    (this)->DrawLine(0,height-2*cornerHeight);
	}
	return;
    }


    /* Generic rr rect -- go around the loop */
    /* Upper left corner */
    (this)->DrawArcSize(x,y,cornerWidth*2, cornerHeight*2,-90,90);
    /* Top line */
    (this)->MoveTo(x+cornerWidth,y);
    (this)->DrawLine(width-2*cornerWidth,0);
    /* upper right corner */
    (this)->DrawArcSize(x+width-2*cornerWidth,y,cornerWidth*2,cornerHeight*2,0,90);
    /* right side */
    (this)->MoveTo(x+width,y+cornerHeight);
    (this)->DrawLine(0,height-2*cornerHeight);
    /* lower right corner */
    (this)->DrawArcSize(x+width-2*cornerWidth,y+height-2*cornerHeight,cornerWidth*2,cornerHeight*2,90,90);
    /* bottom line */
    (this)->MoveTo(x+width-cornerWidth,y+height);
    (this)->DrawLine(-(width-2*cornerWidth),0);
    /* lower left corner */
    (this)->DrawArcSize(x,y+height-2*cornerHeight,2*cornerWidth,2*cornerHeight,180,90);
    /* right side */
    (this)->MoveTo(x,y+height-cornerHeight);
    (this)->DrawLine(0,-(height-2*cornerHeight));
}

void graphic::DrawRRect(struct rectangle  * OuterBox, struct rectangle  * InnerBox)
{
    (this)->DrawRRectSize(rectangle_Left(OuterBox), rectangle_Top(OuterBox), rectangle_Width(OuterBox), rectangle_Height(OuterBox), rectangle_Width(InnerBox), rectangle_Height(InnerBox));
}

void graphic::DrawRgn(class region  * Rgn)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: DrawRgn not impletemented for this wm\n");
    }
}

void graphic::DrawTrapezoid(long  topX , long  topY , long  topWidth , long  bottomX , long  bottomY , long  bottomWidth)
{
    (this)->MoveTo(topX,topY);
    (this)->DrawLine(topWidth,0);
    (this)->DrawLineTo(bottomX+bottomWidth,bottomY);
    (this)->DrawLine(-bottomWidth,0);
    (this)->DrawLineTo(topX,topY);
}


void graphic::FillRectSize(long  x,long  y,long  width,long  height,class graphic  * Tile )
{
    static boolean printed = FALSE;

    if (width <= 0 || height <= 0) return;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: FillRectSize not implemented for this wm\n");
    }
}

void graphic::FillRect(struct rectangle  * Rect, class graphic  * Tile)
{
    (this)->FillRectSize( rectangle_Left(Rect),
			  rectangle_Top(Rect), rectangle_Width(Rect),
			  rectangle_Height(Rect),Tile);
}

void graphic::FillPolygon(struct point  * PointArray, short  PointCount, class graphic  * Tile)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: FillPolygon not implemented for this wm\n");
    }
}

void graphic::FillOvalSize(long  x,long  y,long  width,long  height,class graphic  * Tile)
{
    /* ************************
      **** test dummy code ****
      ********************** */
    (this)->FillRectSize(x,y,width,height,Tile);
}

void graphic::FillOval(struct rectangle  * Rect, class graphic  * Tile)
{
    (this)->FillOvalSize( rectangle_Left(Rect), rectangle_Top(Rect), rectangle_Width(Rect), rectangle_Height(Rect), Tile);
}

void graphic::FillArcSize(long  x,long  y,long  width,long  height,short  StartAngle, short  OffsetAngle,class graphic  * Tile)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: FillArcSize not implemented for this wm\n");
    }
    (this)->FillRectSize(x,y,width,height,Tile);
}

void graphic::FillArc(struct rectangle  * EnclRect,short  StartAngle, short  OffsetAngle,class graphic  * Tile)
{
    (this)->FillArcSize( rectangle_Left(EnclRect), rectangle_Top(EnclRect),
			 rectangle_Width(EnclRect), rectangle_Height(EnclRect),
			 StartAngle, OffsetAngle, Tile);
}

void graphic::FillRRectSize(long  x,long  y,long  width,long  height,long  cornerWidth ,long  cornerHeight,class graphic  * Tile)
{
    static boolean printed = FALSE;

    /* Handle pathologic cases in system indepedent manner
      (luser desires to bite bullet in efficiency) */

    if ( (2*cornerHeight >= height) || (2*cornerWidth >= width)) {
	/* Bizarre -- corners are bigger than rectangle, so 
	 make an appropriate looking oval */
	if ( (2*cornerHeight >= height) && (2*cornerWidth >= width))
	    (this)->FillOvalSize(x,y,width,height,Tile);
	else if (2*cornerHeight >= height) {
	    /* Fill left semi-oval */
	    (this)->FillArcSize(x,y,2*cornerWidth,height,0,-180,Tile);
	    /* Fill vertical rectangle */
	    (this)->FillRectSize(x+cornerWidth,y,width-2*cornerWidth,height,Tile);
	    /* Fill right semi-oval */
	    (this)->FillArcSize(x+width-2*cornerWidth,y,2*cornerWidth,height,0,180,Tile);
	}
	else { /* assuming (2*cornerWidth >= width) */
	    /* Fill top semi-oval */
	    (this)->FillArcSize(x,y,width,2*cornerHeight,-90,180,Tile);
	    /* Fill horizontal rectangle */
	    (this)->FillRectSize(x,y+cornerHeight,width,height-2*cornerHeight,Tile);
	    /* Fill bottom semi-oval */
	    (this)->FillArcSize(x,y+height-2*cornerHeight,width,2*cornerHeight,90,180,Tile);
	}
	return;
    }

    /* ************************
      **** test dummy code ****
      ********************** */
    (this)->FillRectSize(x,y,width,height,Tile);
    if (!printed){
	printed = TRUE;
	fprintf(stderr, "graphic: FillRRectSize not implemented in this wm\n");
    }
}

void graphic::FillRRect(struct rectangle  * OuterBox,struct rectangle  * InnerBox,class graphic  * Tile)
{
    (this)->FillRRectSize( rectangle_Left(OuterBox),rectangle_Top(OuterBox),
			   rectangle_Width(OuterBox), rectangle_Height(OuterBox),
			   rectangle_Width(InnerBox), rectangle_Height(InnerBox), Tile);
}

void graphic::FillRgn(class region  * Rgn,class graphic  * Tile)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: FillRgn not implemented for this wm\n");
    }
}

void graphic::FillTrapezoid(long  topX , long  topY , long  topWidth , long  bottomX , long  bottomY , long  bottomWidth , class graphic  *Tile)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: FillTrapezoid not implemented for this wm\n");
    }
}


void graphic::EraseRect(struct rectangle  * Rect)
{
    short mode=GetTransferMode();
    if(mode!=graphic_COPY) SetTransferMode(graphic_COPY);
    (this)->FillRect( Rect, (this)->WhitePattern());
    if(mode!=graphic_COPY) SetTransferMode(mode);
}

void graphic::EraseRectSize(long  x, long  y, long  width, long  height)
{
    short mode=GetTransferMode();
    if(mode!=graphic_COPY) SetTransferMode(graphic_COPY);
    (this)->FillRectSize( x, y, width, height, (this)->WhitePattern());
    if(mode!=graphic_COPY) SetTransferMode(mode);
}

void graphic::EraseVisualRect()
{
    short mode=GetTransferMode();
    if(mode!=graphic_COPY) SetTransferMode(graphic_COPY);
    (this)->FillRectSize(
			  rectangle_Left(&this->visualBounds),
			  rectangle_Top(&this->visualBounds),
			  rectangle_Width(&this->visualBounds),
			  rectangle_Height(&this->visualBounds),
			  (this)->WhitePattern());
    if(mode!=graphic_COPY) SetTransferMode(mode);
}


void graphic::BitBltSize(long  srcX , long  srcY , long  dstX , long  dstY , long  Width , long  Height, class graphic  * DstGraphic, long  clipX , long  clipY , long  clipWidth , long  clipHeight)
{
    struct rectangle tmpSrcRect, tmpClipRect;
    struct point tmpDestOrigin;
    struct rectangle * passedClipRect;

    if ((clipWidth <= 0) || (clipHeight <= 0)) passedClipRect = NULL;
    else {
	passedClipRect = &tmpClipRect;
	rectangle_SetRectSize(&tmpClipRect,clipX,clipY,clipWidth,clipHeight);
    }
    rectangle_SetRectSize(&tmpSrcRect,srcX, srcY, Width, Height);
    point_SetPt(&tmpDestOrigin,dstX,dstY);
    (this)->BitBlt(&tmpSrcRect,DstGraphic,&tmpDestOrigin,passedClipRect);
}

void graphic::BitBlt(struct rectangle  * SrcRect, class graphic  *DstGraphic, struct point  * DstOrigin, struct rectangle  * ClipRect)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: BitBlt not implemented for this wm\n");
    }
}


void graphic::WriteImage(long  DestX , long  DestY , class image  *image, long  SrcX , long  SrcY , long  width , long  height)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: WritePixImage not implemented for this wm\n");
    }
}

void graphic::WriteImage(long  DestX , long  DestY , ddimage  &image, long  SrcX , long  SrcY , long  width , long  height)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: WritePixImage not implemented for this wm\n");
    }
}

void graphic::ReadImage(long  SrcX , long  SrcY , class image  *DestImage, long  DestX , long  DestY , long  width , long  height)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: ReadImage not implemented for this wm\n");
    }
}

void graphic::WritePixImage(long  DestX , long  DestY , class pixelimage  *SrcPixels, long  SrcX , long  SrcY , long  width , long  height)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: WritePixImage not implemented for this wm\n");
    }
}

void graphic::ReadPixImage(long  SrcX , long  SrcY , class pixelimage  *DestPixels, long  DestX , long  DestY , long  width , long  height)
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: ReadPixImage not implemented for this wm\n");
    }
}

void graphic::SetBitAtLoc(long  XPos,long  YPos, boolean  NewValue)
{
    struct point tempPt;
    short tempLineWidth, tempTMode, dashStyle, tempLineCap, tempLineJoin;
    int dashOffset;
    char *dashPattern;

    /* Slow, but correct -- instead of filling a rectangle, we should
      probably draw a one pixel long, one pixel wide line. */
    (this)->GetCurrentPt(&tempPt);
    tempLineWidth = (this)->GetLineWidth();
    tempTMode = (this)->GetTransferMode();
    (this)->GetLineDash(  &dashPattern, &dashOffset, &dashStyle );
    tempLineCap = (this )->GetLineCap( );
    tempLineJoin = (this )->GetLineJoin( );

    if (NewValue==TRUE)
	(this)->SetTransferMode(graphic_BLACK);
    else if (NewValue==FALSE)
	(this)->SetTransferMode(graphic_WHITE);
    else fprintf(stderr, "graphic_SetBitAtLoc: Bad bit value: %d\n",NewValue);

    (this)->MoveTo(XPos,YPos);
    (this)->SetLineWidth(1);
    (this)->SetLineDash(  NULL, 0, graphic_LineSolid );
    (this)->SetLineCap(  1 );
    (this)->SetLineJoin(  0);
    (this)->DrawLineTo(XPos,YPos); /* yep, this draws one dot in wm */

    (this)->MoveToPt(&tempPt);
    (this)->SetLineWidth(tempLineWidth);
    (this)->SetLineDash(  dashPattern, dashOffset, dashStyle );
    if(dashPattern)
	free(dashPattern);
    (this)->SetLineCap(  tempLineCap );
    (this)->SetLineJoin(  tempLineJoin );
    (this)->SetTransferMode(tempTMode);

}

void graphic::MoveLogicalOrigin(long  DeltaX, long  DeltaY)
{
    point_OffsetPoint(&this->savedOrigin,DeltaX,DeltaY);
    rectangle_OffsetRect(&this->localBounds,DeltaX,DeltaY);
    rectangle_OffsetRect(&this->visualBounds,DeltaX,DeltaY);
    if (this->localRegion) {
	(this->localRegion)->OffsetRegion( DeltaX, DeltaY);
    }
    if (this->visualRegion) {
	(this->visualRegion)->OffsetRegion( DeltaX, DeltaY);
    }
    if (this->clippingRegion) {
	(this->clippingRegion)->OffsetRegion( DeltaX, DeltaY);
    }
    point_OffsetPoint(&this->currentPoint, DeltaX, DeltaY);

    /* *********Previous BUG statement ************* */
    /* Added code for doing offsets to clipping and currnet point */
    /* Still might be wrong */
    /*   Must change clipping rectangle  */
    /* specification uncertainity: should currentpoint be mnoved? */
    /* view bug : all children should have enclosed origins reset */
    /* *************** */
}

void graphic::SetLogicalOrigin(long  NewX ,long  NewY)
{
    (this)->MoveLogicalOrigin(NewX-(this)->GetLogicalLeft(),
			       NewY-(this)->GetLogicalTop());
}

void graphic::SetLogicalOriginPt(struct point  * Pt)
{
    (this)->SetLogicalOrigin(point_X(Pt),point_Y(Pt));
}


void graphic::GetLogicalBounds(struct rectangle  * Rect)
{
    *Rect = this->localBounds;
}

long graphic::GetLogicalRight()
{
    return (this)->GetLogicalLeft() + (this)->GetLogicalWidth();
}

long graphic::GetLogicalBottom()
{
    return (this)->GetLogicalTop() + (this)->GetLogicalHeight();
}

void graphic::GetEnclosedBounds(struct rectangle  * Rect)
{
    rectangle_SetRectSize(Rect,
			   point_X(&this->enclosedOrigin),
			   point_Y(&this->enclosedOrigin),
			   rectangle_Width(&this->localBounds),
			   rectangle_Height(&this->localBounds));
}


long graphic::GetEnclosedRight()
{
    return (this)->GetEnclosedLeft() + (this)->GetEnclosedWidth();
}

long graphic::GetEnclosedBottom()
{
    return (this)->GetEnclosedTop() + (this)->GetEnclosedHeight();
}


void graphic::SetClippingRegion(class region  *regionp)
{
    if (this->clippingRegion == NULL)  {
	this->clippingRegion = new region;
    }
    region::CopyRegion(this->clippingRegion, regionp);
}

class region *graphic::GetClippingRegion(class region  *retRegion)
{
    if (retRegion != NULL)  {
	if (this->clippingRegion != NULL)  {
	    region::CopyRegion(retRegion, this->clippingRegion);
	}
	else {
	    return NULL;
	}
    }
    return retRegion;
}

void graphic::SetClippingRect(struct rectangle  * AdditionalRect)
{
    /* Note: we need some way to remove the clipping rectangle so that
      resizing does the "right" thing */

    if (this->clippingRegion == NULL)  {
	this->clippingRegion = region::CreateRectRegion(AdditionalRect);
    }
    else {
	(this->clippingRegion)->RectRegion( AdditionalRect);
    }
}

void graphic::SetClippingRectSize(long  x , long  y , long  w , long  h)
{
    struct rectangle r;
    rectangle_SetRectSize(&r, x, y, w, h);
    (this)->SetClippingRect( &r);
}

void graphic::ClearClippingRect()
{

    if (this->clippingRegion)
	delete this->clippingRegion;
    this->clippingRegion = NULL;
}

void graphic::GetClippingRect(struct rectangle  * Rect)
{
    if (this->clippingRegion) (this->clippingRegion)->GetBoundingBox( Rect);
    else (this)->GetVisualBounds(Rect);
}

void graphic::SetLineWidth(short  NewLineWidth)
{

    this->lineWidth = NewLineWidth;

}

short graphic::GetLineWidth()
{
    return this->lineWidth;
}

void graphic::SetLineDash( const char		 *dashPattern, int		 dashOffset, short		 dashType )
{
char		*oldDash = this->lineDashPattern;

    this->lineDashType = dashType;
    this->lineDashOffset = dashOffset;
    if ( dashPattern && ( this->lineDashPattern = strdup( dashPattern ) ))
    {
      if ( oldDash ) free( oldDash );
    }
    else this->lineDashPattern = oldDash;
}

void graphic::GetLineDash( char		 **dashPattern, int		 *dashOffset, short		 *dashType )
{
    if ( dashOffset ) *dashOffset = this->lineDashOffset;
    if ( dashType ) *dashType = this->lineDashType;
    if ( dashPattern )
    {
      if ( this->lineDashPattern )
      {
        *dashPattern = strdup(this->lineDashPattern);
      }
      else *dashPattern = NULL;
    }
}

void graphic::SetLineCap( short		 newLineCap )
{
    this->lineCap = newLineCap;
}

short graphic::GetLineCap( )
{
    return this->lineCap;
}

void graphic::SetLineJoin( short		 newLineJoin )
{
    this->lineJoin = newLineJoin;
}

short graphic::GetLineJoin( )
{
    return this->lineJoin;
}

void graphic::SetTransferMode(short  NewTransferMode)
{
    this->transferMode = 0xFF & NewTransferMode;
}

short graphic::GetTransferMode()
{
    return this->transferMode;
}

void graphic::SetPatternOrigin(long  xpos , long  ypos)
{
    point_SetPt(&(this->patternOrigin), xpos, ypos);
}

void graphic::GetPatternOrigin(long  *xpos , long  *ypos)
{
    if (xpos)
	*xpos = point_X(&(this->patternOrigin));
    if (ypos)
	*ypos = point_Y(&(this->patternOrigin));
}

void graphic::GetVisualBounds(struct rectangle  *Rect)
{
    *Rect = this->visualBounds;
}


long graphic::GetVisualRight()
{
    return (this)->GetVisualLeft() + (this)->GetVisualWidth();
}

long graphic::GetVisualBottom()
{
    return (this)->GetVisualTop() + (this)->GetVisualHeight();
}


void graphic::InsertGraphicRegion(class graphic  * EnclosingGraphic, class region  *regionp)
{
    /* Fill in the local bounds for the rectangle (always 0,0 based
						    upon creation) */

    if ((regionp)->IsRegionEmpty()) {
	(this)->InsertGraphicSize( EnclosingGraphic, 0,0, 0, 0);
	return;
    }

    if (this->localRegion == NULL)  {
	this->localRegion = new region;
    }
    region::CopyRegion(this->localRegion, regionp);
    (this->localRegion)->GetBoundingBox( &this->localBounds);

    point_X(&this->enclosedOrigin) = rectangle_Left(&this->localBounds);
    point_Y(&this->enclosedOrigin) = rectangle_Top(&this->localBounds);

    rectangle_Left(&this->localBounds) = point_X(&this->savedOrigin);
    rectangle_Top(&this->localBounds) = point_Y(&this->savedOrigin);

    (this->localRegion)->OffsetRegion(
			 point_X(&this->savedOrigin) - point_X(&this->enclosedOrigin),
			 point_Y(&this->savedOrigin) - point_Y(&this->enclosedOrigin));


    /* Calculate the visual bounds for the newly enclosed rectangle
	in terms of the parent. Start with entire requested area,
	and then clip (intersect) to enclosing graphic */

    if (this->visualRegion == NULL)  {
	this->visualRegion = new region;
    }
    region::CopyRegion(this->visualRegion, regionp);
    if (EnclosingGraphic->visualRegion != NULL) {
	(this->visualRegion)->IntersectRegion( EnclosingGraphic->visualRegion,
			       this->visualRegion);
    }
    else {
	class region *tmpRegion;

	tmpRegion = region::CreateRectRegion(&EnclosingGraphic->visualBounds);
	(this->visualRegion)->IntersectRegion( tmpRegion,
			       this->visualRegion);
	delete tmpRegion;
    }

    (this->visualRegion)->OffsetRegion(
			 point_X(&this->savedOrigin) - point_X(&this->enclosedOrigin),
			 point_Y(&this->savedOrigin) - point_Y(&this->enclosedOrigin)); 

    (this->visualRegion)->GetBoundingBox( &this->visualBounds);

    /* Since the clippingRegion is relative to the visual rect, we
      define the clipping region as disappearing with an
      insertion operation */
    if (this->clippingRegion)
	delete this->clippingRegion;
    this->clippingRegion = NULL;


    /* Now figure out how the local window manager offsets should be
      reset. Note: the origin that we are calculating should be the
      upper left hand corner of the local bounds (0,0 point) of the
      newly created graphic. This way we know that any references to
      a point in the local graphic are exactly offset by the
      the origin we are about to calculate. */

    this->physicalOrigin =
      EnclosingGraphic->physicalOrigin;
    point_OffsetPoint(&this->physicalOrigin,
		       point_X(&this->enclosedOrigin),
		       point_Y(&this->enclosedOrigin));
}

void graphic::InsertGraphicSize(class graphic  * EnclosingGraphic, long  xOriginInParent ,
				 long  yOriginInParent , long  width , long  height)
{
    struct rectangle r;

    rectangle_SetRectSize(&r,xOriginInParent, yOriginInParent,
			   width,height);
    (this)->InsertGraphic(EnclosingGraphic,&r);
}

void graphic::InsertGraphic(class graphic  * EnclosingGraphic, struct rectangle  * EnclosedRectangle)
{

    /* Fill in the local bounds for the rectangle (always 0,0 based
						    upon creation) */

    rectangle_SetRectSize(&this->localBounds,point_X(&this->savedOrigin),
			   point_Y(&this->savedOrigin),
			   rectangle_Width(EnclosedRectangle),
			   rectangle_Height(EnclosedRectangle));

    /* Calculate the visual bounds for the newly enclosed rectangle
	in terms of the parent. Start with entire requested area,
	and then clip (intersect) to enclosing graphic */

    this->visualBounds = *EnclosedRectangle;
    rectangle_IntersectRect(&this->visualBounds,&this->visualBounds,
			     &EnclosingGraphic->visualBounds);
    /* Now reorient the rect to local (savedOrigin based) coordinates. */
    rectangle_OffsetRect(&this->visualBounds,
			  point_X(&this->savedOrigin)-rectangle_Left(EnclosedRectangle),
			  point_Y(&this->savedOrigin)-rectangle_Top(EnclosedRectangle));

    if (this->visualRegion != NULL) {
	delete this->visualRegion;
	this->visualRegion = NULL;
    }
    if (EnclosingGraphic->visualRegion != NULL) {
	this->visualRegion = region::CreateRectRegion(EnclosedRectangle);
	(this->visualRegion)->IntersectRegion( EnclosingGraphic->visualRegion,
			       this->visualRegion);
	(this->visualRegion)->OffsetRegion(
			    point_X(&this->savedOrigin)-rectangle_Left(EnclosedRectangle),
			    point_Y(&this->savedOrigin)-rectangle_Top(EnclosedRectangle)); 
    }

    /* Install the origin relative to the enclosing grphic (just
							     copy over the rectangle that it specified! */
    point_SetPt(&this->enclosedOrigin,
		 rectangle_Left(EnclosedRectangle),
		 rectangle_Top(EnclosedRectangle));

    /* Since the clippingRegion is relative to the visual rect, we
      define the clipping region as disappearing with an
      insertion operation */
    if (this->clippingRegion)
	delete this->clippingRegion;
    this->clippingRegion = NULL;


    /* Now figure out how the local window manager offsets should be
      reset. Note: the origin that we are calculating should be the
      upper left hand corner of the local bounds (0,0 point) of the
      newly created graphic. This way we know that any references to
      a point in the local graphic are exactly offset by the
      the origin we are about to calculate. */

    this->physicalOrigin =
      EnclosingGraphic->physicalOrigin;
    point_OffsetPoint(&this->physicalOrigin,
		       rectangle_Left(EnclosedRectangle),
		       rectangle_Top(EnclosedRectangle));

}

void graphic::SetVisualRegion(class region  *regionp)
{
    if ((regionp)->IsRegionEmpty())  {
	rectangle_SetWidth(&this->visualBounds, 0);
	rectangle_SetHeight(&this->visualBounds, 0);
    }
    else {
	if (this->visualRegion == NULL)  {
	    this->visualRegion = new region;
	}
	region::CopyRegion(this->visualRegion, regionp);
	(this->visualRegion)->OffsetRegion(
			    point_X(&this->savedOrigin),
			    point_Y(&this->savedOrigin)); 
	(this->visualRegion)->GetBoundingBox( &this->visualBounds);
    }
}

class region *graphic::GetVisualRegion(class region  *retRegion)
{
    if (retRegion != NULL)  {
	if (this->visualRegion != NULL) {
	    region::CopyRegion(retRegion, this->visualRegion);
	}
	else {
	    class region *tmpRegion;

	    tmpRegion = region::CreateRectRegion(&this->visualBounds);
	    region::CopyRegion(retRegion, tmpRegion);
	    delete tmpRegion;
	}
    }
    return retRegion;
}

void graphic::RestoreGraphicsState()
{

}

void graphic::FlushGraphics()
{
}

class graphic * graphic::WhitePattern()
{
    return (this)->GrayPattern(0,100);
}

class graphic * graphic::BlackPattern()
{
    return (this)->GrayPattern(100,100);
}

class graphic * graphic::GrayPattern(short  IntensityNum , short  IntensityDenom )
{
    static boolean printed = FALSE;
    if (! printed) {
	printed = TRUE;
	fprintf(stderr, "graphic: GrayPattern not implemented for this wm\n");
    }
    return (class graphic *) 0;
}

static color fg="black";
static color bg="white";

void graphic::SetDefaultColors(const char  *foreground, const char  *background)
{

    char *tempString;

    if (foregroundColorName != NULL)
	free(foregroundColorName);
    if (foreground != NULL && *foreground && (tempString = strdup(foreground)) != NULL)
	foregroundColorName = tempString;
    else
	foregroundColorName = NULL;

    if (backgroundColorName != NULL)
	free(backgroundColorName);
    if (background != NULL && *background && (tempString = strdup(background)) != NULL)
	backgroundColorName = tempString;
    else
	backgroundColorName = NULL;
    if(foregroundColorName) fg=foregroundColorName;
    if(backgroundColorName) bg=backgroundColorName;
}

void graphic::GetDefaultColors(const char  **foreground, const char  **background)
{

    if (foreground != NULL)
	*foreground = foregroundColorName;
    if (background != NULL)
	*background = backgroundColorName;
}

void graphic::SetForegroundColor(color *c )
{
    if(c!=fore) {
	icolor *old=icolor::Find(fore);
	if(old) old->Destroy();
	fore=c;
    }
}

void graphic::SetBackgroundColor(color *c )
{
    if(c!=back) {
	icolor *old=icolor::Find(back);
	if(old) old->Destroy();
	back=c;
    }
}
    
void graphic::SetForegroundColor(const char  *colorName, long  red , long  green, long  blue )
{
    icolor *nw=colors.Alloc(colorName, red, green, blue);
    if(nw) {
	nw->Reference();
	SetForegroundColor(nw);
    } else {
	SetForegroundColor(&fg);
    }
}

void graphic::GetForegroundColor(const char  **colorName, long  *red , long  *green, long  *blue )
{

    if ( colorName && fore)
	*colorName = (const char *)fore->Name();
    unsigned short r=0, g=0, b=0;
    if(fore) {
	fore->HardwareRGB(*CurrentColormap(), r, g, b);
    }
    if (red) *red = r;
    if (green) *green = g;
    if (blue) *blue = b;
}

void graphic::SetBackgroundColor(const char  *colorName, long  red , long  green, long  blue )
{
    icolor *nw=colors.Alloc(colorName, red, green, blue);
    if(nw) {
	nw->Reference();
	SetBackgroundColor(nw);
    } else {
	SetBackgroundColor(&bg);
    }
}

void graphic::GetBackgroundColor(const char  **colorName, long  *red , long  *green, long  *blue )
{
    if ( colorName )
	*colorName = (const char *)back->Name();
    unsigned short r=0, g=0, b=0;
    if(back) {
	back->HardwareRGB(*CurrentColormap(), r, g, b);
    }
    if (red) *red = r;
    if (green) *green = g;
    if (blue) *blue = b;
}

void graphic::SetFGColor( double  red , double  green , double  blue )
{
    icolor *nw=colors.Alloc(( unsigned short ) ( red * 65535.0 + 0.5),
				 ( unsigned short ) ( green * 65535.0 + 0.5), ( unsigned short ) ( blue * 65535.0 + 0.5));
    if(nw) {
	nw->Reference();
	SetForegroundColor(nw);
    } else {
	SetForegroundColor(&fg);
    }
}

void graphic::GetFGColor( double  *red , double  *green , double  *blue )
{
    unsigned short r=0, g=0, b=0;
    if(fore) {
	fore->HardwareRGB(*CurrentColormap(), r, g, b);
    }
    if (red) *red = ((double)r) / 65535.0;
    if (green) *green = ((double)g) / 65535.0;
    if (blue) *blue = ((double)b) / 65535.0;
}

void graphic::SetBGColor( double  red , double  green , double  blue )
{
    icolor *nw=colors.Alloc(( unsigned short ) ( red * 65535.0 + 0.5),       ( unsigned short ) ( green * 65535.0 + 0.5),  ( unsigned short ) ( blue * 65535.0 + 0.5));
    if(nw) {
	nw->Reference();
	SetBackgroundColor(nw);
    } else {
	SetBackgroundColor(&bg);
    }
}

void graphic::GetBGColor( double  *red , double  *green , double  *blue )
{
    unsigned short r=0, g=0, b=0;
    if(back) {
	back->HardwareRGB(*CurrentColormap(), r, g, b);
    }
    if (red) *red = ((double)r) / 65535.0;
    if (green) *green = ((double)g) / 65535.0;
    if (blue) *blue = ((double)b) / 65535.0;
}

long graphic::GetHorizontalResolution()
{
    return 80L;
}

long graphic::GetVerticalResolution()
{
    return 80L;
}

const char * graphic::GetWindowManagerType()
{
    return "";
}

long graphic::GetDevice()
{
    return 0;
}

long graphic::DisplayClass( )
{
    return graphic_Monochrome | graphic_StaticGray;
}

/* declares whether images pulled back from the server via graphic_ReadPixelImage() are inverted */
boolean graphic::IsImageInverted()
{
    return FALSE;
}

/* -------------------------------------------------- */
/*   Predefined procedures */
/* -------------------------------------------------- */

graphic::graphic()
{
    static class fontdesc *defaultFont = NULL;

    fore=&fg;
    back=&bg;
    this->colormap = NULL;
    this->inheritedColormap = NULL;
    rectangle_SetRectSize(&this->localBounds,0,0,0,0);
    rectangle_SetRectSize(&this->visualBounds,0,0,0,0);
    this->localRegion = NULL;
    this->visualRegion = NULL;
    point_SetPt(&this->savedOrigin,0,0);
    point_SetPt(&this->enclosedOrigin,0,0);
    if (defaultFont == NULL)
	defaultFont = fontdesc::Create("andysans", 0, 12);
    this->currentFont = defaultFont;
    this->spaceShim = 0;
    this->transferMode = graphic_COPY;
    this->lineWidth = 1;
    this->lineDashType = 0;
    this->lineDashOffset = 0;
    this->lineDashPattern = NULL;
    this->lineCap = 1;
    this->lineJoin = 0;
    this->clippingRegion = (class region * ) NULL;
    point_SetPt(&this->currentPoint,0,0);

    point_SetPt(&this->physicalOrigin,0,0);
    point_SetPt(&(this->patternOrigin), 0, 0);

    THROWONFAILURE( TRUE);
}

/* ***************************************** */
/*       Class procedures                  */
/* ***************************************** */

class graphic * graphic::CreateGraphic(class view  *v)
{
    class graphic *g;
    g = im::GetGraphic();
    return(g);
}

graphic::~graphic()
{
    if (this->clippingRegion) {
	delete this->clippingRegion;
    }
    if (this->localRegion)  {
	delete this->localRegion;
    }
    if (this->visualRegion) {
	delete this->visualRegion;
    }
    if (this->lineDashPattern) {
	free(this->lineDashPattern);
    }

}

void
graphic::SetFGColorCell(color  *c)
{
    SetForegroundColor(c);
}

void
graphic::SetBGColorCell(color  *c)
{
    SetBackgroundColor(c);
}

void graphic::SetFGToShadow(int  shadow)
{
    long br, bg, bb;
    unsigned short rr, rg, rb;
    
    (this)->GetBackgroundColor( NULL, &br, &bg, &bb);
    
    shadows_ComputeColor((unsigned short)br, (unsigned short)bg, (unsigned short)bb, &rr, &rg, &rb, shadow);

    (this)->SetForegroundColor( NULL, (long)rr, (long)rg, (long)rb);
}

void graphic::ComputeShadow(long  br , long  bg , long  bb, long  *rr , long  *rg , long  *rb, int  shadow)
{
    unsigned short srr, srg, srb;
    
    shadows_ComputeColor((unsigned short)br, (unsigned short)bg, (unsigned short)bb, &srr, &srg, &srb, shadow);
    
    *rr=(long)srr;
    *rg=(long)srg;
    *rb=(long)srb;
}

void graphic::ComputeShadowDouble(double  br , double  bg , double  bb, double  *rr , double  *rg , double  *rb, int  shadow)
{
    unsigned short srr, srg, srb;
    
    shadows_ComputeColor((unsigned short)(br*65535.0), (unsigned short)(bg*65535.0), (unsigned short)(bb*65535.0), &srr, &srg, &srb, shadow);
    
    *rr=((double)srr)/65535.0;
    *rg=((double)srg)/65535.0;
    *rb=((double)srb)/65535.0;
}
