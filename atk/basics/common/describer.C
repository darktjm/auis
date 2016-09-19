/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("describer.H")
#include <view.H>
#include <describer.H>

/* changes to describe.c */


ATKdefineRegistry(describer, ATK, NULL);

enum view_DescriberErrs describer::Describe(class view  * viewToDescribe,char  * format,FILE  * file,long  rock)
{
    /* Print out a console message saying that this format is not available */
    return view_NoDescriptionAvailable;
}

