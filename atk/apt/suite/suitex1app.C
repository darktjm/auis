/* Copyright 1993 Carnegie Mellon University All rights reserved.
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
ATK_IMPL("suitex1app.H")
#include <im.H>
#include <frame.H>
#include <suite.H>
#include <message.H>
#include <suitex1app.H>



ATKdefineRegistry(suitex1app, application, NULL);

class view * Flavor_Choice( class suitex1app  *self, register class suite  *suite, register struct suite_item  *item, int type, view_MouseAction action, long x, long y, long clicks );

static suite_Specification vanilla[] = {
    suite_ItemCaption( "   Vanilla" ),
    suite_ItemTitleCaption( "Flavor 1:" ),
    0
};

static suite_Specification strawberry[] = {
    suite_ItemCaption( "   Strawberry" ),
    suite_ItemTitleCaption( "Flavor 2:" ),
    0
};

static suite_Specification chocolate[] = {
    suite_ItemCaption( "   Chocolate" ),
    suite_ItemTitleCaption( "Flavor 3:" ),
    0
};

static suite_Specification grape[] = {
    suite_ItemCaption( "   Grape" ),
    suite_ItemTitleCaption( "Flavor 4:" ),
    0
};

static suite_Specification fudge[] = {
    suite_ItemCaption( "   Fudge" ),
    suite_ItemTitleCaption( "Flavor 5:" ),
    0
};

static suite_Specification licorice[] = {
    suite_ItemCaption( "   Licorice" ),
    suite_ItemTitleCaption( "Flavor 6:" ),
    0
};

static suite_Specification caramel[] = {
    suite_ItemCaption( "   Caramel" ),
    suite_ItemTitleCaption( "Flavor 7:" ),
    0
};

static suite_Specification lemon[] = {
    suite_ItemCaption( "   Lemon" ),
    suite_ItemTitleCaption( "Flavor 8:" ),
    0
};

static suite_Specification orange[] = {
    suite_ItemCaption( "   Orange" ),
    suite_ItemTitleCaption( "Flavor 9:" ),
    0
};


suite_Specification flavors[] = {
    suite_HitHandler( Flavor_Choice ),
    suite_Item(vanilla),
    suite_Item(strawberry), 
    suite_Item(chocolate),
    suite_Item(grape), 
    suite_Item(fudge), 
    suite_Item(licorice),
    suite_Item(caramel), 
    suite_Item(lemon), 
    suite_Item(orange), 
    suite_ItemTitleCaptionAlignment( suite_Top | suite_Left ),
    suite_ItemCaptionAlignment( suite_Middle | suite_Left ),
    suite_BorderSize( 4 ),
    suite_ItemBorderSize( 4 ),
    suite_ItemCaptionFontName( "andysans12b" ),
    suite_GutterSize ( 5 ),
    suite_Arrangement( suite_Matrix | suite_Fixed ),
    suite_Columns( 3 ), suite_Rows( 3 ),
    suite_ForegroundColor( "red" ),
    suite_BackgroundColor( "gray70" ),
    suite_ActiveItemForegroundColor( "blue" ),
    suite_ActiveItemBackgroundColor( "gray" ),
    suite_TitleCaption("9 Wondertastic Flavors!"),
    suite_TitleCaptionFontName( "andy14bi" ),
    0
};


boolean
suitex1app::Start( )
{
    class frame *f = new frame;
    (f)->SetView( suite::Create(flavors, (long)this));
    im::SetPreferedDimensions(0, 0, 300, 300);
    (im::Create(NULL))->SetView( f);
    (f)->PostDefaultHandler( "message", (f)->WantHandler( "message"));
    return(TRUE);
}

class view *
Flavor_Choice( class suitex1app  *self, register class suite  *suite, register struct suite_item  *item, int  type, view_MouseAction action, long x, long y, long clicks )
{
    char msg[100];
    if(action == view_LeftUp) {
	sprintf(msg, "Chosen Flavor is %s.", (char *)(suite)->ItemAttribute( item, suite_itemcaption));
	message::DisplayString(suite, 0, msg);
    }
    return(NULL);
}

