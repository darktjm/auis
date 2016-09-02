/* Copyright 1993 Carnegie Mellon University All rights reserved.
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

#include <andrewos.h>
ATK_IMPL("messitem.H")
#include <sys/param.h>
#include <sys/stat.h>
#include <ctype.h>
#include <environ.H>
#include <messitem.H>

#define DEFAULTMESS '0'
#define LEFTMESS '1'
#define RIGHTMESS '2'
#define MIDDLEMESS '3'

struct messitemi {
  char *oldmess, *newmess;
  long oldlen, newlen;
  char *dispmess;
  char where;
  struct messitemi *next;
};

static struct messitemi *currentmess=NULL; 
static struct messitemi *firstmess=NULL; 


ATKdefineRegistry(messitem, ATK, NULL);
static struct messitemi *messitem_Create( char  *s1 , char  *s2, char  wh );
static char *MapMessFile(const char  *filename, long  *fileLength /* OUT */);
static int ReadMessFile(const char  *filename, boolean  executeImmediately);


static struct messitemi *messitem_Create( char  *s1 , char  *s2, char  wh )
    {
  struct messitemi *mess;
  mess = (struct messitemi *) malloc(sizeof(struct messitemi));
  if (s1 != NULL && *s1 != '\0') {
    mess->oldlen = strlen(s1);
    mess->oldmess = (char *) malloc(mess->oldlen + 1);
    strcpy(mess->oldmess,s1);
  } else {
    mess->oldlen = 0;
    mess->oldmess = NULL;
  }
  if (s2 != NULL && *s2 != '\0') {
    mess->newlen = strlen(s2);
    mess->newmess = (char *) malloc(mess->newlen + 1);
    strcpy(mess->newmess,s2);
  } else {
    mess->newlen = 0;
    mess->newmess = NULL;
  }
  mess->where = wh;
  mess->dispmess = NULL;
  mess->next = NULL;
  if (currentmess == NULL) firstmess=mess; else currentmess->next = mess;
  currentmess = mess;
  return mess;
}

struct messitemi *messitem::Exists(char  *str)
    {

  struct messitemi *tmess = firstmess;

  if (str == NULL || *str == '\0') return NULL;
  while (tmess && tmess->oldmess && strcmp(tmess->oldmess,str) != 0)
    tmess = tmess->next;
  return tmess;
}

const char *messitem::Replace(const char  *str)
    {
  struct messitemi *mess = firstmess;
  if (str == NULL || *str == '\0') return str;

  while (mess) {
    switch (mess->where) {
    case DEFAULTMESS:
      if (mess->oldmess && strcmp(mess->oldmess,str) != 0)
	mess = mess->next;
      else {
	  return mess->newmess;
      }
      break;
    case LEFTMESS:
       if (mess->oldmess && strncmp(mess->oldmess,str,mess->oldlen) != 0)
	 mess = mess->next;
       else {
	int displen = mess->newlen + strlen(str) - mess->oldlen;
	const char *s = str;
	int i = 0;
	while (i++ < mess->oldlen) s++;
	if (mess->dispmess != NULL)
	  free(mess->dispmess);
	mess->dispmess = (char *) malloc(displen + 1);
	strcpy(mess->dispmess,mess->newmess);
	strcat(mess->dispmess,s);
	return mess->dispmess;
       }
      break;
    case RIGHTMESS:
       { 
	 int len = strlen(str);
	 if (len < mess->oldlen) 
	   mess = mess->next;
	 else {
	   const char *s = str+len;
	   int i = len - mess->oldlen;
	   while (i-- > 0) s--;
	   if (mess->oldmess && strncmp(mess->oldmess,s,mess->oldlen) != 0)
	     mess = mess->next;
	   else {
	     int displen = mess->newlen + len - mess->oldlen;
	     if (mess->dispmess != NULL)
	       free(mess->dispmess);
	     mess->dispmess = (char *) malloc(displen + 1);
	     strncpy(mess->dispmess,s, len - mess->oldlen);
	     strcat(mess->dispmess,mess->newmess);
	     return mess->dispmess;
	   }
	 }
       }
      break;
    }
  }

  return str;
}

#define INITIALSIZE 512

/* Hacked routine to rea a "whole file" into memory. */
static char *MapMessFile(const char  *filename, long  *fileLength /* OUT */)
        {

    int fd;
    char *buffer;
    long length = 0;

    if ((fd = open(filename, O_RDONLY, 0)) >= 0) {

        struct stat statBuf;

        if (fstat(fd, &statBuf) >= 0) {

            long bufferSize; /* Current size of malloced block. */
            long bytesRead = 0;

            /* Find the size. In the case of special files, use a suitable default. */
            if ((statBuf.st_mode & S_IFMT) == S_IFREG)
                bufferSize = statBuf.st_size ;
            else
                bufferSize = INITIALSIZE;

            buffer = (char *) malloc(bufferSize + 1); /* +1 for NUL at end. */

            while (buffer != NULL && (bytesRead = read(fd, buffer + length, bufferSize - length )) > 0) {
                length += bytesRead;
                if (length >= bufferSize) {
                    bufferSize *= 2;
                    buffer = (char *) realloc(buffer, bufferSize + 1); /* +1 for NUL at end. */
                }
            }
            if (bytesRead < 0) {
                free(buffer);
                buffer = NULL;
            }
            else
                buffer[length] = '\0';
        }
        else
            buffer = NULL;

        close(fd);
    }
    else
        buffer = NULL;

    if (fileLength != NULL)
        *fileLength = length;
    return buffer;
}

#define UnmapMessFile(mappedMemory) free(mappedMemory)

static int ReadMessFile(const char  *filename, boolean  executeImmediately)
{

    char *buffer;
    long length;

    if ((buffer = MapMessFile(filename, &length)) != NULL) {

        char *p;
        int currentLine;

        currentLine = 0;

        p = buffer;
        while (p < buffer + length) {
	    char s1[400],s2[400];
	    char where = DEFAULTMESS;
	    char firstcar=*p;
	    int i = 0;
            ++currentLine;
	    /* Skip to the end of line. */
	    if (firstcar != '#') {
	      if (isdigit(*p)) where = *p++;
	      while (*p == ' ') p++; /* Skip blank */
	      i = 0;
	      if (*p == '"') p++;
	      while (*p != '\n' && *p != '\0' && *p != '"')
		s1[i++] = *p++; /* First sentence found */
	      s1[i] = '\0';
	      if (*p == '"') p++;
	    } else
	      s1[0] = '\0';
	    if (s1[0] != '\0') {
	      struct messitemi *mess;
	      while (*p == ' ') p++; /* Skip blank */
	      i = 0;
	      if (*p == '"') p++;
	      while (*p != '\n' && *p != '\0' && *p != '"')
		  s2[i++] = *p++; /* Second sentence found */
	      s2[i] = '\0';
	      if ((mess=messitem::Exists(s1)) == NULL)
		mess = messitem_Create(s1,s2,where);
	      else {
		free(mess->newmess);
		mess->newmess = (char *) malloc(strlen(s2) + 1);
		strcpy(mess->newmess,s2);
	      }
	    }

	    while (*p != '\n' && *p != '\0') /*end of line */
	      p++;
	    if (*p == '\n') p++;
	  }

        UnmapMessFile(buffer);

        return 0;
    }
    else
        return -1;
}    

static boolean isMessFileLoaded = 0;

void messitem::InitMessFile( )
    {
    if (isMessFileLoaded == 0) {
	const char *al=environ::Get("ANDREWLANGUAGE");
	const char *alf=environ::Get("ANDREWLANGUAGEFILE");
	if(al==NULL) al=environ::GetProfile("AndrewLanguage");
	if(alf==NULL) alf=environ::GetProfile("AndrewLanguageFile");
	if(alf==NULL && al) {
	    char MessFile[MAXPATHLEN];
	    strcpy(MessFile, environ::AndrewDir("/lib/"));
	    strcat(MessFile, al);
	    strcat(MessFile, ".msg");
	    ReadMessFile(MessFile, TRUE);
	} else if(alf) ReadMessFile(alf, TRUE);
	isMessFileLoaded = 1;
    }
}
