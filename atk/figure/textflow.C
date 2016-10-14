/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
*/

#include <andrewos.h>
ATK_IMPL("textflow.H")
#include <textflow.H>

ATKdefineRegistryNoInit(textflow, figure);

textflow::textflow()
{
    THROWONFAILURE( TRUE);
}

textflow::~textflow()
{
}

const char *textflow::ViewName()
{
    return "textflowview";
}

