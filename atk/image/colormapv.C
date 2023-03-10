#include <andrewos.h>
ATK_IMPL("colormapv.H")
#include <colormapv.H>
#include <view.H>
#include <rect.h>
#include <message.H>
#include <lpair.H>
#include <suite.H>
/* #include <slider.H> */ /* just to shut up makedep; this file is not built */
#include <sliderV.H>
#include <color.H>
#include <colorv.H>
/* #include <colormap.H> */ /* just to shut up makedep; this file is not built */

ATKdefineRegistry(colormapv, view, colormapv::InitializeClass);
static long SliderChanged( long rock, long  inten );
class view * Color_Choice( class colormapv  *self, class suite  *suite, struct suite_item  *item, enum view::UpdateType  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
class view * Control_Choice( class colormapv  *self, class suite  *suite, struct suite_item  *item, enum view::UpdateType  type, enum view::MouseAction  action, long  x , long  y , long  clicks );


static suite_Specification cmap_entries[] = {
    suite_HitHandler( Color_Choice ),
    suite_ItemCaptionFontName( "andysans12b" ),
    suite_Arrangement( suite_Matrix | suite_Fixed ),
    suite_Rows( 16 ), suite_Columns( 16 ),
    suite_ItemHighlightStyle( suite_None ),
    0
};

#define COLORMAPV_NUMSLIDERS 3
#define COLORMAPV_REDCODE 0
#define COLORMAPV_GREENCODE 1
#define COLORMAPV_BLUECODE 2

static suite_Specification redSlider[] = {
    suite_ItemDatum(COLORMAPV_REDCODE),
    suite_ItemForegroundColor("red"),
    suite_ItemBackgroundColor("white"),
    suite_ItemTitleCaption("00000"),
    0
};

static suite_Specification greenSlider[] = {
    suite_ItemDatum(COLORMAPV_GREENCODE),
    suite_ItemForegroundColor("green"),
    suite_ItemBackgroundColor("white"),
    suite_ItemTitleCaption("00000"),
    0
};

static suite_Specification blueSlider[] = {
    suite_ItemDatum(COLORMAPV_BLUECODE),
    suite_ItemForegroundColor("blue"),
    suite_ItemBackgroundColor("white"),
    suite_ItemTitleCaption("00000"),
    0
};

static suite_Specification control_entries[] = {
    suite_HitHandler( Control_Choice ),
    suite_ItemCaptionFontName( "andysans12b" ),
    suite_Arrangement( suite_Row ),
    suite_ItemHighlightStyle( suite_None ),
    suite_Item(redSlider),
    suite_Item(greenSlider),
    suite_Item(blueSlider),
    0
};

struct sliderCBData {
    class colormapv *cmapv;
    struct suite_item *item;
};


boolean
colormapv::InitializeClass( )
  {
    return(TRUE);
}


colormapv::colormapv( )
    {
	ATKinit;

  this->map = suite::Create(cmap_entries, (long) this);
  this->controlPanel = suite::Create(control_entries, (long) this);
  this->top = new lpair;
  (this->top)->VSplit( (class view*) this->map, (class view*) this->controlPanel, 30, TRUE);
  (this->top)->LinkTree( (class view *) this);
  this->currentColor = NULL;
  THROWONFAILURE((TRUE));
}

colormapv::~colormapv( )
    {
	ATKinit;

}

static long
SliderChanged( long rock, long  inten )
        {
    struct sliderCBData *callBackData = (struct sliderCBData *) rock;
    class colormapv *self = callBackData->cmapv;
    class suite *s = self->controlPanel;
    struct suite_item *item = callBackData->item;
    class colormap *cmap = (class colormap *) (self)->GetDataObject();
    class color *c;
    if(c = self->currentColor) {
	unsigned short R, G, B;
	char title[128];
	(c)->GetRGB( R, G, B);
	switch((s)->ItemAttribute( item, suite_itemdatum )) {
	    case COLORMAPV_REDCODE:
		if(R != inten)
		    (c)->SetRGB( inten, G, B);
		break;
	    case COLORMAPV_GREENCODE:
		if(G != inten)
		    (c)->SetRGB( R, inten, B);
		break;
	    case COLORMAPV_BLUECODE:
		if(G != inten) 
		    (c)->SetRGB( R, G, inten);
		break;
	}
	(cmap)->ChangeColor( c);
	(cmap)->SetModified();
	sprintf(title, "%ld", inten);
	(s)->ChangeItemAttribute( item, suite_ItemTitleCaption(title));
    }
    return(0);
}

void
colormapv::FullUpdate( enum view::UpdateType  type, long  left , long  top , long  width , long  height )
      {
  struct rectangle r;
  class colormap *cmap = (class colormap*) (this)->GetDataObject();
  boolean firstTime = FALSE;

  (this)->GetVisualBounds( &r);
  if((this->map)->ItemCount() == 0) {
      int size = (cmap)->Used();
      int i, j;
      struct suite_item *item;

      firstTime = TRUE;
      for( i = 0; i < size; i++ ) {
	  class color *c = (cmap)->NthColor( i );
	  class colorv *cv = new colorv;
	  (cv)->SetDataObject( c );
	  item = (this->map)->CreateItem( NULL, i );
	  (this->map)->SetItemDataObject( item, (class dataobject *) c );
	  (this->map)->SetItemViewObject( item, (class view *) cv );
      }

      for( i = 0; i < COLORMAPV_NUMSLIDERS; i++ ) {
	  class suite *s = this->controlPanel;
	  item = (s)->ItemOfDatum( i);
	  if(item) {
	      class slider *slider = new slider;
	      class sliderV *sldv = new sliderV;
	      struct sliderCBData *sliderCBRock;
	      (s)->SetItemDataObject( item, (class dataobject *) slider );
	      (s)->SetItemViewObject( item, (class view *) sldv );
	      (sldv)->curval = 0;
	      (sldv)->range = 65535;
	      (sldv)->SetDataObject( slider );
	      sliderCBRock = (struct sliderCBData*) malloc( sizeof(struct sliderCBData) );
	      sliderCBRock->cmapv = this;
	      sliderCBRock->item = item;
	      (sldv)->SetCallback( (sliderV_updatefptr) SliderChanged, (long) sliderCBRock );
	  }
      }	
  }

  (this->top)->InsertView( this, &r);
  (this->top)->FullUpdate( type, left, top, width, height);

#if 0
  if(firstTime)
      (this->map)->HighlightItem( (this->map)->ItemAtPosition( 1));
#endif
}

void
colormapv::Update( )
  {
  (this)->view::Update();
}

class view *
Color_Choice( class colormapv  *self, class suite  *suite, struct suite_item  *item, enum view::UpdateType  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {
  if(item && action == view::LeftDown) {
      class suite *s = self->controlPanel;
      class color *c;
      unsigned short R, G, B;
      int i;

      c = (class color *) (self->map)->ItemDataObject( item);
      (c)->GetRGB( R, G, B);
      self->currentColor = c;
      for(i = 0; i < COLORMAPV_NUMSLIDERS; i++) {
	  struct suite_item *item = (s)->ItemOfDatum( i );
	  class sliderV *sldv = (class sliderV *) (s)->ItemViewObject( item );
	  switch(i) {
	      case COLORMAPV_REDCODE:
		  (sldv)->SetCurval( R );
		  break;
	      case COLORMAPV_GREENCODE:
		  (sldv)->SetCurval( G );
		  break;
	      case COLORMAPV_BLUECODE:
		  (sldv)->SetCurval( B );
		  break;
	  }
      }
  }
  return(NULL);
}

class view *
Control_Choice( class colormapv  *self, class suite  *suite, struct suite_item  *item, enum view::UpdateType  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {
  if(item && action == view::LeftDown) {
      message::DisplayString(self, 0, (char *) ((suite)->ItemAttribute( item, suite_itemcaption )) );
  }
  return(NULL);
}

void
colormapv::PostMenus( class menulist  *menulist )
        {
  (this)->view::PostMenus( menulist);
}

class view *
colormapv::Hit( enum view::MouseAction  action, long  x, long  y, long  numberOfClicks)
                    {
    return((class view *) (this->top)->Hit( action, x, y, numberOfClicks));
}
