/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <view.ih>

class describer[describe] {
methods:
    Describe(struct view * viewToDescribe, char * format, FILE * file, long rock) returns enum view_DescriberErrs;
};
