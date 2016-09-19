/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
*/

#include <andrewos.h>
ATK_IMPL("textflow.H")
#include <textflow.H>

ATKdefineRegistry(textflow, figure, NULL);

textflow::textflow()
{
    ATKinit;

    THROWONFAILURE( TRUE);
}

textflow::~textflow()
{
    ATKinit;
}

const char *textflow::ViewName()
{
    return "textflowview";
}

