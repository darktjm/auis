/*  
 misc.c of

 Scribetext: a facility to convert Scribe manuscript files to files 
 compatible with the ATK file format.

 Scribetext is copyright (c) 1989, 1990 by the Massachusetts Institute of
 Technology.

 Scribe is a registered trademark of Scribe Systems, Inc.

 Permission to use, copy, modify, and distribute this software and
 its documentation for any purpose and without fee is hereby granted,
 provided that the above copyright notice and the name of the author(s)
 appear in all copies; that both that copyright notice, the name of
 the author(s) and this permission notice appear in supporting
 documentation; and that the name of the Massachusetts Institute of
 Technology not be used in advertising or publicity pertaining to
 distribution of the software without specific, written prior
 permission.  The Massachusetts Institute of Technology makes no
 representations about the suitability of this software for any purpose.
 It is provided "as is" without express or implied warranty.
 
 Scribetext was written entirely by Jeremy Paul Kirby, jpkirby@ATHENA.MIT.EDU
*/

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

#include <andrewos.h>
#include <ctype.h>
#include "scribetext.h"

char *makelower(char *instruction)
{
  int i;
  
  for(i=0; i<(int)strlen(instruction); i++)
    {
      if(isupper(instruction[i]))
	instruction[i] = tolower(instruction[i]);
    }

  return(instruction);
}
  

#if 0 /* unused */
void TempPrintList(void)
{
  TABLE tmp=Table;

  while(tmp != NULL)
    {
      printf("scribeword is %s  mode is %d ez is ", tmp->scribeword, tmp->mode);
      if(tmp->mode & COMMAND)
	printf("COMMAND\n");
      else 
	printf("%s\n", tmp->ez.word);
      tmp = tmp-> next;
    }
}
#endif


int offset(char *string, char character)
{
  char *loc;

  loc = strchr(string, character);

  if (loc) return (strlen(string) - strlen(loc));
  else return(strlen(string));
}


int roffset(char *string, char character)
{
  char *loc;

  loc = strrchr(string, character);

  if (loc) return (strlen(string) - strlen(loc));
  else return(strlen(string));
}


void usage(void)
{
  fprintf(stderr, "\nUsage:  %s %s\n\n", me,
	 "[scribefile [ezfile]] [-t transtable] [-e stderrfile]");
  exit(0);
}  


void CloseFiles(void)
{
  fprintf(fout, "\\enddata{text, %d}\n", MasterToken);

  fclose(fout);
  fclose(ftrans);
  if(ferr != NULL)
    fclose(ferr);
}


void AbsorbSpace(void)
{
  int in;
  char ch;

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch != ' ' && ch != '\t')
	{
	  ungetc(ch, fin);
	  break;
	}
    }
}


void AbsorbNewlines(void)
{
  int in;
  char ch;

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch != '\n' && ch != '\r')
	{
	  ungetc(ch, fin);
	  break;
	}
      else
	CurrLine++;
    }
}


void PushFile(char *filename)
{
  FILESTACK new;

  new = (FILESTACK) malloc (sizeof(struct FileStackStruct));
  
  new->ptr = fin;
  new->line = CurrLine;

  if(FileStack == NULL)
    new->next = NULL;
  else
    new->next = FileStack;
  
  FileStack = new;

  if((fin = fopen(filename, "r")) == NULL)
    {
      fprintf(ferr, "* Fatal unknown error opening %s for access.\n", 
	      filename);
      CloseFiles();
    }
CurrLine=1;
}


int PopFile(void)
{
  fclose(fin);
  
  if(FileStack == NULL)
    {
      CloseFiles();
      return(0);
    }
  else
    {
      fin = FileStack->ptr;
      CurrLine = FileStack->line;
      if(FileStack->next == NULL)
	FileStack = NULL;
      else
	FileStack = FileStack->next;
    }

  return(1);
}

