/*LIBS: -lutil
*/

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/environ.C,v 3.7 1995/11/07 20:17:10 robr Stab74 $";
#endif

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

 

/* environ.c
 * Class procedures to get at user environment and preference parameters.
 */



ATK_IMPL("environ.H")
#include <environ.H>

#include <util.h>

extern char ProgramName[];	/* blechhh */

/* used to avoid problems with null pointers expected to be
  treated as null strings */
static char *nullstring="";


ATKdefineRegistry(environ, ATK, NULL);
static boolean varcmp(register const char  *variable, register const char  *envEntry);
static char * strncpyMovePointer(char *dest, const char *src, const int numchars);

static char *strncpyMovePointer(char *dest, const char *src, const int numchars)
{
    strncpy(dest, src, numchars);
    dest += numchars;
    *dest = '\0';
    return dest;
}

/* Quick and simple routine for checking if a environment entry is for a given
 * variable, probably faster than raw strlen and strncmp combinations.
 */
static boolean varcmp(register const char  *variable, register const char  *envEntry)
        {

    while (*variable != '\0' && *variable++ == *envEntry++)
	;
    return (*variable == '\0' && (*envEntry == '=' || *envEntry == '\0'));
}

void environ::SetProgramName(const char  *s)
{
    if(s==NULL) s=nullstring;
    strcpy(ProgramName,s);
}

void environ::PutPrinter(const char *value)
{
    environ::Put("LPDEST", value);
    environ::Put("PRINTER", value);
}

const char *environ::GetPrinter()
{
    const char *result=environ::Get("LPDEST");
    if(result==NULL) result=environ::Get("PRINTER");
    return result;
}

void environ::DeletePrinter()
{
    environ::Delete("LPDEST");
    environ::Delete("PRINTER");
}

/* This function inherently leaks core under certain circumstances. This
 * shouldn't be a problem since it shouldn't be called too much.
 */
void environ::Put(const char    *variable , const char    *value)
        {

#undef environ
    extern char **environ;
/* lastEnviron and lastEnvironLength are an attempt to do intelligent allocation
 * of environment space while still maintaining compatability with the UNIX
 * notion of an envrment. We check them each time we add an element because
 * it is possible (but very unlikely) that some other piece of code has
 * replaced the environment since we last touched it...
 */
    static char **lastEnviron = NULL; /* The last environment we allocated. */
    static int lastEnvironLength = 0; /* The maximum number of entries in lastEnviron. */
    register char **p;

    /* check for a NULL value so we can make it the expected null string */
    if(value==NULL) value=nullstring;
    for (p = environ; *p != NULL && !varcmp(variable, *p); p++)
	;

    if (*p == NULL) {
        if (environ != lastEnviron || p - environ > lastEnvironLength) {

	    register char **np;

	    lastEnvironLength = (p - environ) + 12; /* 1 for new entry, 1 for NULL, and 10 for expansion. */
	    if (lastEnviron != NULL)
	        lastEnviron = (char **) realloc(lastEnviron, sizeof(char *) * lastEnvironLength);
	    else
	        lastEnviron = (char **) malloc(sizeof(char *) * lastEnvironLength);

	    for (p = lastEnviron, np = environ; *np;)
		*p++ = *np++;

	    environ = lastEnviron;
	}
	p[1] = NULL;
    }

    *p = (char *) malloc (strlen(variable) + strlen(value) + 2);
    sprintf (*p, "%s=%s", variable, value);
#define environ environclass
}

void environ::Delete(const char	 *variable)
        {
#undef environ
    extern char **environ;
    register char **p;

    for (p = environ; *p != NULL && !varcmp(variable, *p); p++)
	;
    for (; *p != NULL; p++)
	    *p = p[1];
#define environ environclass
}

const char *environ::Get(const char  *variable)
        {


    return(getenv(variable));
}

const char *environ::GetProfile(const char  *preference)
        {

    return getprofile(preference);
}

boolean environ::GetProfileSwitch(const char  *preference, boolean  defaultValue)
            {

    return getprofileswitch(preference, defaultValue);
}

long environ::GetProfileInt(const char  *preference, long  defaultValue)
            {

    return getprofileint(preference, defaultValue);
}

boolean environ::ProfileEntryExists(const char  *preference, boolean  useDefault)
            {
    return profileentryexists(preference, (int) useDefault);
}

const char *environ::GetConfiguration(const char  *key)
        {
    return (char *) ::GetConfiguration(key);
}

const char *environ::AndrewDir(const char  *str)
        {
    return ::AndrewDir(str);
}

const char *environ::LocalDir(const char  *str)
        {
    return ::LocalDir(str);
}

struct configurelist *environ::ReadConfigureFile(char  *filename)
        {
    return (struct configurelist *) ::ReadConfigureFile(filename);
}

char *environ::GetConfig(struct configurelist  *cList, char  *key, boolean  usedefault)
                {
    return (char *) ::GetConfig(cList, key, usedefault);
}

void environ::FreeConfigureList(struct configurelist  *cList)
        {
    ::FreeConfigureList(cList);
}

const char *environ::GetHome(const char  *user)
{
    return gethome(user);
}


const char *environ::GetFirstProfileFileName()
{
    return ::GetFirstProfileFileName();
}

const char *environ::GetProfileFileName()
{
    return ::GetProfileFileName();
}

/* Expand all environment variables imbedded in fromString
 * placing the results in toString, whose length, upon
 * return, will be <= maxsize. If results will cause
 * toString to be overrun, toString will be truncated to
 * maxsize characters.
 * $XXX, ${XXX}, and $(XXX) are valid strings for env vars.
 * $$ will expand to $ (and suppress expansion of env var)
 */
void environ::ExpandEnvVars(char *toString, const char *fromString, int maxsize)
{
    char *dollar, *tx, *dx, *ex;
    char envname[MAXPATHLEN+1];
    const char *envval;
    char endch;
    const char *fs = fromString;
    int strfull = FALSE;

    tx = toString;
    *toString = '\0';	// make sure we start from scratch
      while (!strfull) {
	  dollar = (char *)strchr(fs, '$');
	  if (dollar == NULL) break;
	  dx = dollar+1;
	  if(*dx=='$') {
	      if ((strlen(toString) + (dx-fs)+1) <= maxsize) 
		  tx = strncpyMovePointer(tx, fs, dx-fs);
	      else {
		  tx = strncpyMovePointer(tx, fs, (maxsize-strlen(toString)-1));
		  strfull=TRUE;
	      }
	      fs=dx+1;
	      continue;
	  }
	  if (*dx == '{')  {dx++;  endch = '}';}
	  else if (*dx == '(') {dx++;   endch = ')';}
	  else endch = '\0';
	  ex = envname;
	  while (isalnum(*dx)) {*ex++ = *dx++;}
	  *ex = '\0';
	  if (*dx && *dx == endch) dx++;

	  /* get the value of the environment variable */
	  if(strcmp(envname, "ANDREWDIR")!=0) envval = environ::Get(envname);
	  else envval=environ::AndrewDir("");
	  if (envval == NULL) {
	      /* leave $xxx in place */
	      if ((strlen(toString) + (dx-fs)+1) <= maxsize) 
		  tx = strncpyMovePointer(tx, fs, dx-fs);
	      else {
		  tx = strncpyMovePointer(tx, fs, (maxsize-strlen(toString)-1));
		  strfull=TRUE;
	      }
	  }
	  else {

	      if ((strlen(toString) + (dollar-fs)+1) <= maxsize) {
		  tx = strncpyMovePointer(tx, fs, dollar-fs);
	      }
	      else {
		  tx = strncpyMovePointer(tx, fs, (maxsize-strlen(toString)-1));
		  strfull=TRUE;
	      }
	      if (*envval != '\0') {
		  if ((strlen(toString)+strlen(envval)+1) <= maxsize) {
		      strcpy(tx, envval);
		      // move tx ptr to end of string
			tx = tx+strlen(envval);
		      *tx = '\0';
		  }
		  else {
		      tx = strncpyMovePointer(tx, envval, (maxsize-strlen(toString)-1));
		      strfull=TRUE;
		  }
	      }
	  }
	  fs = dx;
      }
    if (!strfull) {
	if ((strlen(toString)+strlen(fs)+1) <= maxsize)
	    strcpy(tx, fs);
	else 
	    tx = strncpyMovePointer(tx, fs, maxsize-strlen(toString)-1);
    }

}
/* Add a preference to the running environment.
 * This preference overrides all other preferences (if applicable)
 * and overrides any other preferences previously added by
 * AddStringProfile.
 */
void environ::AddStringProfile(const char *string)
{
    addstringprofile((char *)string);
}
