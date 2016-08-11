/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/champ/RCS/month.C,v 1.3 1994/08/13 18:13:51 rr2b Stab74 $";
#endif

#include <andrewos.h>	/* time.h */
#include <stdio.h>

#include <month.H>


ATKdefineRegistry(month, dataobject, NULL);
#ifndef NORCSID
#endif


month::month()
{
    time_t clock = time(0);
    struct tm *thisdate = localtime(&clock);

    this->mon = thisdate->tm_mon;
    this->year = thisdate->tm_year;
    THROWONFAILURE((TRUE));
}

long month::Write(FILE  *fp, long  id, int  level)
{
    time_t clock;
    struct tm *thisdate;

    if (id != (this)->GetWriteID()) {
	/* New write operation */
	(this)->SetWriteID( id);
	clock = time(0);
	thisdate = localtime(&clock);
	fprintf(fp, "\\begindata{%s,%d}\n%d\n%d\n\\enddata{%s,%d}\n",
		(this)->GetTypeName(), (this)->UniqueID(),
		this->mon - thisdate->tm_mon, this->year - thisdate->tm_year,
		(this)->GetTypeName(), (this)->UniqueID());
    }
    return((this)->UniqueID());
}

long month::Read(FILE  *fp, long  id)
{
    char LineBuf[250];
    time_t clock = time(0);
    struct tm *thisdate = localtime(&clock);

    (this)->SetID( (this)->UniqueID());
    if (fgets(LineBuf,sizeof(LineBuf), fp) == NULL) {
	return(dataobject_PREMATUREEOF);
    }
    this->mon = thisdate->tm_mon + atoi(LineBuf);
    if (fgets(LineBuf,sizeof(LineBuf), fp) == NULL) {
	return(dataobject_PREMATUREEOF);
    }
    this->year = thisdate->tm_year + atoi(LineBuf);
    /* Now read in the \enddata line */
    if (fgets(LineBuf,sizeof(LineBuf), fp) == NULL) {
	return(dataobject_PREMATUREEOF);
    }
    if (strncmp(LineBuf, "\\enddata", 8)) {
	return(dataobject_MISSINGENDDATAMARKER);
    }
    return(dataobject_NOREADERROR);
}

void month::SetMonthAndYear(int  mon , int  year)
{
    this->mon = mon;
    this->year = year;
    (this)->NotifyObservers( 0);
}
