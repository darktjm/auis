/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("hwview.H")
#include "graphic.H"
#include "hwview.H"


ATKdefineRegistry(helloworldview, view, NULL);

void helloworldview::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
                        {
    int x,y;
    struct rectangle VisualRect;		

    (this)->GetVisualBounds(&VisualRect);
    x = rectangle_Left(&VisualRect) + rectangle_Width(&VisualRect)/2;
    y = rectangle_Top(&VisualRect) + rectangle_Height(&VisualRect)/2;

    (this)->MoveTo(x,y);
    (this)->DrawString("hello world",
	graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);    

}
