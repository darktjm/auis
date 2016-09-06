/*								      HTAlert.c
**	DISPLAYING MESSAGES AND GETTING INPUT FOR LINEMODE BROWSER
**
**	(c) COPYRIGHT CERN 1994.
**	Please first read the full copyright statement in the file COPYRIGH.
**
**	REPLACE THIS MODULE with a GUI version in a GUI environment!
**
** History:
**	   Jun 92 Created May 1992 By C.T. Barker
**	   Feb 93 Simplified, portablised TBL
**	   Sep 93 Corrected 3 bugs in HTConfirm() :-( AL
*/

/* Library include files */
#include "tcp.h"
#include "HTUtils.h"
#include "HTString.h"
#include "HTAlert.h"
#include <stdio.h>
#ifndef STDIO
#define STDIO
#endif
#ifndef GOT_PASSWD
#define GOT_PASSWD
#endif
PUBLIC BOOL HTInteractive=YES;		    /* Any prompts from the Library? */

PUBLIC void HTAlert (CONST char * Msg)
{
#ifdef NeXTStep
    NXRunAlertPanel(NULL, "%s", NULL, NULL, NULL, Msg);
#else
    fprintf(TDEST, "WWW1 Alert:  %s\n", Msg);
#endif
}


PUBLIC void HTProgress (CONST char * Msg)
{
    fprintf(TDEST, "WWW9  %s ...\n", Msg);
}


PUBLIC BOOL HTConfirm (CONST char * Msg)
{
  char Reply[4];	/* One more for terminating NULL -- AL */
  char *URep;
  
  fprintf(TDEST, "WWW2: %s\n", Msg);	   /* (y/n) came twice -- AL */
#ifdef STDIO
  if (!HTInteractive || !fgets(Reply, 4, stdin))   /* get reply, max 3 chars */
#endif
      return NO;
  URep=Reply;
  while (*URep) {
    if (*URep == '\n') {
	*URep = (char)0;	/* Overwrite newline */
	break;
    }
    *URep=TOUPPER(*URep);
    URep++;	/* This was previously embedded in the TOUPPER */
                /* call an it became evaluated twice because   */
                /* TOUPPER is a macro -- AL */
  }

  if ((strcmp(Reply,"YES")==0) || (strcmp(Reply,"Y")==0))
    return(YES);
  else
    return(NO);
}

/*	Prompt for answer and get text back. Reply text is either NULL on
**	error or a dynamic string which the caller must free.
*/
PUBLIC char * HTPrompt (CONST char * Msg, CONST char * deflt)
{
    char Tmp[200];
    char * rep = 0;
    fprintf(TDEST, "WWW3: %s\n", Msg);
    if (deflt) fprintf(TDEST, " (RETURN for [%s]) ", deflt);
#ifdef STDIO
    if (!HTInteractive || !fgets(Tmp, 200, stdin))
#endif
	return NULL;		       	     /* NULL string on error, Henrik */
    Tmp[strlen(Tmp)-1] = (char) 0;			/* Overwrite newline */

    StrAllocCopy(rep, *Tmp ? Tmp : deflt);
    return rep;
}


/*	Prompt for password without echoing the reply. Reply text is
**	either NULL on error or a dynamic string which the caller must free.
*/
PUBLIC char * HTPromptPassword (CONST char * Msg)
{
    static char result[MAXPASSWDLEN],*c;
    *result = '\0';
#ifdef GOT_PASSWD
    if (HTInteractive) {
	fprintf(TDEST,"WWW4: %s\n",Msg ? Msg : "Password: ");
	fgets(result,MAXPASSWDLEN,stdin);
	*(result+MAXPASSWDLEN-1) = '\0';
	if((c = rindex(result,'\n')) != NULL) *c = '\0';
    }
#endif
    return result;
}


/*	Prompt both username and password	HTPromptUsernameAndPassword()
**	---------------------------------
** On entry,
**	Msg		is the prompting message.
**	*username and
**	*password	are char pointers; they are changed
**			to point to result strings.
**
**			If *username is not NULL, it is taken
**			to point to  a default value.
**			Initial value of *password is
**			completely discarded.
**
** On exit,
**	*username and *password point to newly allocated
**	strings -- original strings pointed to by them
**	are NOT freed.
**	
*/
PUBLIC void HTPromptUsernameAndPassword (CONST char *	Msg,
					      char **		username,
					      char **		password)
{
    char Tmp[200];
    char * rep = 0;
    char * rep2 = 0;
    if (!HTInteractive) {
	*username = "";
	*password = "";
	return;
    }
    fprintf(TDEST, "WWW5: %s\n", Msg);
    if (!HTInteractive || !fgets(Tmp, 200, stdin))
	return;		       	     /* NULL string on error, Henrik */
    Tmp[strlen(Tmp)-1] = (char) 0;			/* Overwrite newline */

    StrAllocCopy(rep, Tmp);
    *username = rep;
    if (!HTInteractive || !fgets(Tmp, 200, stdin))
	return ;		       	     /* NULL string on error, Henrik */
    Tmp[strlen(Tmp)-1] = (char) 0;			/* Overwrite newline */

    StrAllocCopy(rep2, Tmp);
    *password = rep2;

}

