/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("helloworld.H")
#include "helloworld.H"


ATKdefineRegistry(helloworld, dataobject, NULL);

helloworld::helloworld()
{
    this->x = POSUNDEF;
    this->y = POSUNDEF;
    this->blackOnWhite = TRUE;
    THROWONFAILURE( TRUE);
}
