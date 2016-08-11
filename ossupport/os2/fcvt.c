/*
 * fcvt -- Convert floating point number to a string
 *         This is a very cheap implementation and may be considered
 *         broken!  This is used in a limited case in atk/table.
 *
 */
#include <stdio.h>
#include <string.h>

static char buf[50];

char *fcvt(double val, int num_dig, int *dec_p, int *sgn)
{
    if (num_dig >= sizeof(buf))
	num_dig = sizeof(buf)-1;
    sprintf(buf, "%*f", val, num_dig);
    if (dec_p) {
	char *p = strchr(buf, '.');
	if (p)
	    *dec_p = p - buf;
	else
	    *dec_p = num_dig;
    }
    if (sgn)
	*sgn = (val < 0.0);
    return buf;
}
