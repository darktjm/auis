/* Copyright 1993 Carnegie Mellon University All rights reserved. */


#include <andrewos.h>
ATK_IMPL("suitex1app.H")
#include <im.H>
#include <frame.H>
#include <suite.H>
#include <message.H>
#include <suitex1app.H>



ATKdefineRegistry(suitex1app, application, NULL);

static class view * Flavor_Choice( class suitex1app  *self, class suite  *suite, struct suite_item  *item, int type, view_MouseAction action, long x, long y, long clicks );

static const suite_Specification vanilla[] = {
    suite_ItemCaption( "   Vanilla" ),
    suite_ItemTitleCaption( "Flavor 1:" ),
    0
};

static const suite_Specification strawberry[] = {
    suite_ItemCaption( "   Strawberry" ),
    suite_ItemTitleCaption( "Flavor 2:" ),
    0
};

static const suite_Specification chocolate[] = {
    suite_ItemCaption( "   Chocolate" ),
    suite_ItemTitleCaption( "Flavor 3:" ),
    0
};

static const suite_Specification grape[] = {
    suite_ItemCaption( "   Grape" ),
    suite_ItemTitleCaption( "Flavor 4:" ),
    0
};

static const suite_Specification fudge[] = {
    suite_ItemCaption( "   Fudge" ),
    suite_ItemTitleCaption( "Flavor 5:" ),
    0
};

static const suite_Specification licorice[] = {
    suite_ItemCaption( "   Licorice" ),
    suite_ItemTitleCaption( "Flavor 6:" ),
    0
};

static const suite_Specification caramel[] = {
    suite_ItemCaption( "   Caramel" ),
    suite_ItemTitleCaption( "Flavor 7:" ),
    0
};

static const suite_Specification lemon[] = {
    suite_ItemCaption( "   Lemon" ),
    suite_ItemTitleCaption( "Flavor 8:" ),
    0
};

static const suite_Specification orange[] = {
    suite_ItemCaption( "   Orange" ),
    suite_ItemTitleCaption( "Flavor 9:" ),
    0
};


static const suite_Specification flavors[] = {
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

static class view *
Flavor_Choice( class suitex1app  *self, class suite  *suite, struct suite_item  *item, int  type, view_MouseAction action, long x, long y, long clicks )
{
    char msg[100];
    if(action == view_LeftUp) {
	sprintf(msg, "Chosen Flavor is %s.", (char *)(suite)->ItemAttribute( item, suite_itemcaption));
	message::DisplayString(suite, 0, msg);
    }
    return(NULL);
}

