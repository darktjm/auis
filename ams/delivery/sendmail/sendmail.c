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

/*
		sendmail.c -- Replacement for sendmail on workstations.
*/


#include <pwd.h>
#include <stdio.h>
#include <system.h>
#include <andrewos.h>
#ifdef HAS_SYSEXITS
#include <sysexits.h>
#endif /* HAS_SYSEXITS */

#include <util.h>
#include <mailconf.h>

typedef unsigned char bool;
#define FALSE	0
#define TRUE	1

#define NIL	0

bool readto, ignoredots;

main(argc, argv)
    int argc;
    char *argv[];
{
    int i, nrecps;

    nrecps = 0;
    ignoredots = FALSE;

    CheckAMSConfiguration();
    /* Process arguments */
    for (i=1; argv[i] != NIL; i++) {
	char *arg;

	arg = argv[i];
	switch (*arg) {

	    case '-':	switch (*++arg) {
			    case 'v':	fputs("[SENDMAIL] Warning: -v is no longer supported\n", stderr);
					break;
			    case 't':	unsupp(argv, argv[i]);
				        break;
			    case 'b':	if (*++arg == 'm') break;
					unsupp(argv, argv[i]);
			    case 'o':	if (*++arg == 'i')
				switch (*++arg) {
				case 'i':
				    ignoredots = TRUE;
				    break;
				case 'e':
				    /* set error level with next char (SunOS cron does this) - ignore */
				    break;
				default:
				    unsupp(argv, argv[i]);
				    break;
				}
				break;
			    case 'i':
			    case 'm':
			    case 's':	break;
			    default:	unsupp(argv, argv[i]);
			}
			break;

	    default:	nrecps++;
			break;
	}
    }

    if (nrecps == 0) unsupp(argv, "No recipient addresses specified");

    /* It's okay, just deliver the mail */
    exit(deliver(argv, nrecps));
}

static deliver(argv, nrecps)
    char *argv[];
    int nrecps;
{
    char **qargs;
    FILE *qmail;
    bool lookfordot;
    int i, q;
    int chk_date = 0;

    /* Get space for args */
    /* Allow space for arg[0], -i & NIL */
    qargs = (char **) calloc(sizeof(char *), nrecps+4);
    if (qargs == NIL) {
	fputs("[SENDMAIL] Out of memory, delivery aborted\n", stderr);
	postmaster(argv, "Out of memory in deliver");
	exit(EX_OSERR);
    }

    /* Construct command -- add list of recipients */
    qargs[0] = "queuemail";
    qargs[1] = "-i";
    qargs[2] = "-a";
    for (i=1, q=3; argv[i]!=NIL; i++)
	if (*argv[i] != '-')	/* Must be a recipient */
	    qargs[q++] = argv[i];

    /* Null-terminate it */
    qargs[q] = NIL;

    /* Fork queuemail & send it message */
    qmail = qopen(queuemail, qargs, "w");
    if (qmail == NIL) {
	fputs("[SENDMAIL] Can't invoke queuemail\n", stderr);
	exit(EX_UNAVAILABLE);
    }

    lookfordot = FALSE;
    for (;;) {
	int c;

	c = getchar();
	switch (c) {
	    case '\n':	if (chk_date == 1) { /* end of headers and no date */
	                   fprintf(qmail, "Date: %s", arpadate());
	                   chk_date = 6;
	                }
	                if (chk_date < 6) chk_date = 1;
                        lookfordot = TRUE;
			break;
	    case EOF:	return qclose(qmail);
	    case '.':	if (!ignoredots && lookfordot) {
			    c = getchar();
			    if (c == '\n')
				return qclose(qmail);
			    fputc('.', qmail);
			}
	                break;
	    case 'D':
	    case 'd':   if (chk_date == 1)
	                   chk_date++;
	                else if (chk_date != 6)
			  chk_date = 0;
	                break;

	    case 'A':
	    case 'a':   if (chk_date == 2)
	                   chk_date++;
	                else if(chk_date != 6)
			  chk_date = 0;
	                break;

	    case 'T':
	    case 't':   if (chk_date == 3)
	                   chk_date++;
	                else if(chk_date != 6)
			  chk_date = 0;
	                break;

	    case 'E':
	    case 'e':   if (chk_date == 4)
	                   chk_date++;
	                else if(chk_date != 6)
			  chk_date = 0;
	                break;

	    case ':':   if (chk_date == 5)
	                   chk_date++;
	                else if(chk_date != 6)
			  chk_date = 0;
	                break;

  	    default:	lookfordot = FALSE;
	                if (chk_date != 6)
			  chk_date = 0;
	                break;
	}

	putc(c, qmail);
    }
}

static unsupp(argv, problem)
    char *argv[], *problem;
{
    fputs("[SENDMAIL] /usr/lib/sendmail was invoked with an unimplemented\n", stderr);
    fprintf(stderr, "\tor improper option: %s\n", problem);
    fputs("Postmaster is being made aware of this problem...", stderr);
    fflush(stderr);

    postmaster(argv, problem);
    exit(EX_USAGE);
}

static char *host()
{
#define NAMELEN 256
    static char name[NAMELEN];

    if (GetHostDomainName(name, NAMELEN) < 0)
	strcpy(name, "UNKNOWN");
    else
	name[NAMELEN-1] = '\0';
    return name;
}

static char *user()
{
    struct passwd *pw;

    pw = getpwuid(getuid());
    if (pw == NULL)
	return "NO ENTRY";
    else
	return pw -> pw_name;
}

static postmaster(argv, problem)
    char *argv[], *problem;
{
    static char *args[] = { "queuemail", "-i", NIL, NIL }; 
    FILE *fout;
    int i;

    args[2] = PostmasterTitle;
    fout = qopen(queuemail, args, "w");
    if (fout == NULL)
	return fputs("WARNING: can't send mail to postmaster\n", stderr);

    /* Header */
    fprintf(fout, "Date: %s", arpadate());
    fprintf(fout, "From: /usr/lib/sendmail <%s%s@%s>\n", PostmasterName,
		(AMS_DeliverySystem ? "+" : ""), WorkstationCell);
    fprintf(fout, "Subject: /usr/lib/sendmail invoked (%s)\n\n", problem);

    /* Body of message */
    fprintf(fout, "/usr/lib/sendmail was invoked on host %s at %s\n", host(), arpadate());
    fprintf(fout, "User %s, WS cell %s\n", user(), WorkstationCell);
    fprintf(fout, "Problem: %s\n", problem);
    fprintf(fout, "\nCommand: %s", argv[0]);
    for (i=1; argv[i]!=NULL; i++)
	fprintf(fout, " %s", argv[i]);
    fputc('\n', fout);

    qclose(fout);
    fputs("done\n", stderr);
}
