/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>	/* time.h */
#include <stdio.h>
#include <champ.h>
#include <ctype.h>

static const char * const Weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static void PrintNode(struct eventnode  *en, struct tm  *thisdate);


static void PrintNode(struct eventnode  *en, struct tm  *thisdate)
{
    printf("%s, %d/%d/%d: %s\n", Weekdays[thisdate->tm_wday], thisdate->tm_mon+1, thisdate->tm_mday, thisdate->tm_year+1900, en->event);
}

int main(int  argc, char  **argv) 
{
    time_t clock;
    struct tm *thisdate;
    int lefttocheck;

    if (argc < 2) {
	lefttocheck = 7;
    } else {
	lefttocheck = atoi(argv[1]);
	if (lefttocheck <= 0) {
	    printf("Usage: chump [days-to-check]\n");
	    exit(1);
	}
    }
    clock = time(0);
    thisdate = localtime(&clock);
    ReadDatesFromChampPath(NULL);
    while (lefttocheck > 0) {
	ClearAllFlaggedEvents();
	FlagEventsMatchingDate(thisdate);
	IterateFlaggedEvents((champ_ifefptr)PrintNode, (long)thisdate);
	IncrementDate(thisdate);
	--lefttocheck;
    }
    exit(0);
}

