/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <util.h>

int fpacheck(void)
{
    float f;

    f = 1000.0 * 0.5;
    f /= 2;
    f /= 2.0;
    return ((f == 125) ? 0 : 1);
}
