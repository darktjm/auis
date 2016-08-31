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
 *      This module contains abstract command parsing routines.
 *      A syntax described by a switch table and a positional
 *      table is supported.  Command arguments are classified
 *      and deposited into the strings indicated in these two
 *      tables.
 *
 *      A switch may start with either a '-' or a '/', and must
 *      be followed immediately by a single unique character.
 *      If the switch requires an argument, it may follow immediately
 *      after the switch identifier or as the next command operand.
 *      If the switch does not require an argument, it may optionally
 *      be followed immediately by another switch (with or without an
 *      intervening '-' or '/').
 *
 *      Command operands not determined to be switch arguments are
 *      assigned sequentially to the strings defined in the
 *      positional table.
 *
 *      Command operands may be quoted with " or ' to enclose blanks
 *      or switch characters.  Switches and positional arguments
 *      may also be intermixed.
 *
 */



#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <cmdargs.h>
#include <vuidebug.h>

char *GetArgument (i, arg, argc, argv)
int *i, argc;
char *arg, **argv;
    {
    char endingchar;
    char *s;

    if (*arg == '"' || *arg == '\'') {
        s = (char *) malloc (128);
        s[0] = 0;
        endingchar = *arg;
        strcat (s, arg+1);
        for (;;) {
            if (s[strlen(s)-1] == endingchar) {
                s[strlen(s)-1] = 0;
                return (s);
                }
            strcat (s, " ");
            if (++(*i) < argc) {
                strcat (s, argv[*i]);
                }
            else
                ERROR (("unterminated quoted string %c%s\n", endingchar, s));
            }
        }
    else
        return (arg);
    }

char *GetSwitchArgument (i, argc, argv)
int *i, argc;
char **argv;
    {
    char *t = NULL;

    if (strlen (argv[*i]) == 2)
        if (++(*i) < argc)
            t = argv[*i];
        else
            ERROR (("missing operand for %s\n", argv[*i-1]));
    else
        t = argv[*i]+2;

    return (GetArgument (i, t, argc, argv));
    }

PRIVATE int Empty ()
{
    return(0);
}

ParseArgs (argc, argv, switchtable, positionaltable, DisplayUsage)
int argc;
char **argv;
STABLE switchtable[];
PTABLE positionaltable[];
int (*DisplayUsage)();
    {
    int i,j;
    char match = 0;
    int standalone_option;

    for (i=0; positionaltable[i]; i++)
        *positionaltable[i] = NULL;

    if (DisplayUsage == NULL)
        DisplayUsage = Empty;

    debug((2,"argc=%d, argv[1]=%s\n",argc, argv[1]));
    if (argv[1] != NIL && *argv[1] == '?') {
        (*DisplayUsage) ();
        exit (-1);
        }

    for (i=1; i<argc; i++) {
        do {
            standalone_option = FALSE;
            if (*argv[i] == '/' || *argv[i] == '-') {
                match = FALSE;
                for (j=0; switchtable[j].option; j++) {
                    if (switchtable[j].option == *(argv[i]+1)) {
                        if (switchtable[j].handler != NULL)
                            i = (*switchtable[j].handler) (i, argc, argv);
                        else {
                            if (switchtable[j].reqarg)
                                *switchtable[j].target =
                                    GetSwitchArgument (&i, argc, argv);
                            else {
                                *switchtable[j].target = "1";
                                standalone_option = TRUE;
                                if ((*(argv[i]+2) == '/') ||
                                    (*(argv[i]+2) == '-'))
                                    strcpy (argv[i], argv[i]+2);
                                else if (*(argv[i]+2))
                                    strcpy (argv[i]+1, argv[i]+2);
                                else
                                    standalone_option = FALSE;
                                }
                            }
                        match = TRUE;
                        break;
                        }
                    }
                if (match == FALSE) {
                    printf ("Error: unrecognized switch '%c%c'\n",
                        *argv[i], *(argv[i]+1));
                    (*DisplayUsage) ();
                    exit (-1);
                    }
                }
            else {
                if (*(argv[i])) {
                    match = FALSE;
                    for (j=0; positionaltable[j]; j++)
                        if (*positionaltable[j] == NULL) {
                            *positionaltable[j] =
                                GetArgument (&i, argv[i], argc, argv);
                            match = TRUE;
                            break;
                            }
                    if (match == FALSE) {
                        printf ("Error: unrecognized argument '%s'\n", argv[i]);
                        (*DisplayUsage) ();
                        exit (-1);
                        }
                    }
                }
            } while (standalone_option && match);
        }
    }
