#include <andrewos.h>

#ifndef NULL
#define NULL 0
#endif

#include "date.h"

char	*date64_Parse(date)
char	*date;
{
    char    *convlongto64();

    if(lcstrcmp(date,"creation")==0)
	return date64_CREATION;
    else{
	int year,month,day,hour,minut,sec,wday;
	int gtm;
	if(MS_ParseDate(date,&year,&month,&day,&hour,&minut,&sec,&wday,&gtm)!=0)
	    return NULL;
	return convlongto64(gtm,0);
    }
}

char	*niceTime(tm)
struct tm   *tm;
{
    static char	buf[80],
/*		*weekdays[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"},    */
		*months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
			   "Sep","Oct","Nov","Dec"};
    int	    hr=tm->tm_hour;
    int    am=hr<12;

    if(!am)
	hr-=12;
    else if(hr==0)
	hr=12;

    sprintf(buf,"%d-%s-%d %d:%02d:%02d%s",
	    tm->tm_mday,months[tm->tm_mon],tm->tm_year+1900,
	    hr,tm->tm_min,tm->tm_sec,(am?"am":"pm"));

    return buf;
}

char	*date64_Unparse(date64)
char	*date64;
{
    char    *asctime();
    struct tm	*localtime();
    long    clock,conv64tolong();

    clock=conv64tolong(date64);
    if(clock<=0)
	return "creation";
    else
	return niceTime(localtime(&clock));
}
