/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>	/* time.h */
#include <stdio.h>

#include <month.H>


ATKdefineRegistry(month, dataobject, NULL);


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
	fprintf(fp, "\\begindata{%s,%ld}\n%d\n%d\n\\enddata{%s,%ld}\n",
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
