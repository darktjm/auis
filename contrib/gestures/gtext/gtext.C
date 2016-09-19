#include <andrewos.h>
#include <gtext.H>


ATKdefineRegistry(gtext, text, gtext::InitializeClass);

boolean
gtext::InitializeClass( )
    {
  return TRUE;
}

gtext::gtext( )
        {
	ATKinit;

  THROWONFAILURE( TRUE);
}

gtext::~gtext( )
        {
	ATKinit;

}

const char *
gtext::ViewName( )
    {
  return((char*)"gtextv");
}
