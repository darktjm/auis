#include <andrewos.h>
ATK_IMPL("colorv.H")
#include <rect.h>
#include <view.H>
#include <color.H>
#include <colorv.H>


ATKdefineRegistry(colorv, view, colorv::InitializeClass);

boolean
colorv::InitializeClass( )
  {
  return(TRUE);
}

colorv::colorv( )
    {
	ATKinit;

  THROWONFAILURE((TRUE));
}

colorv::~colorv( )
    {
	ATKinit;


}

void
colorv::FullUpdate( enum view::UpdateType  type, long  left , long  top , long  width , long  height )
      {
  class color *c = (class color*) (this)->GetDataObject();
  unsigned short R, G, B;
  struct rectangle r;

  (c)->GetRGB( R, G, B);
  (this)->SetForegroundColor( (c)->Name(), R, G, B);
  (this)->GetVisualBounds( &r);
  (this)->FillRect( &r, (this)->BlackPattern());
}

void
colorv::Update( )
  {
  (this)->FullUpdate( view::FullRedraw, 0, 0, 0, 0);
}
