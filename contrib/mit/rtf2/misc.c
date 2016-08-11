/*  
 misc.c of

 Rtf2: A facility to convert RTF files to files 
 compatible with the ATK file format.

 Rtf2 is copyright (c) 1991 by the Massachusetts Institute of
 Technology.

 RTF is a product of the Microsoft Corporation.

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
 
 Rtf2 was written by Jeremy Paul Kirby, jpkirby@ATHENA.MIT.EDU and Scott Rixner, rixner@ATHENA.MIT.EDU

 $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/rtf2/RCS/misc.c,v 1.3 1996/02/09 19:37:26 susan Stab74 $
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/rtf2/RCS/misc.c,v 1.3 1996/02/09 19:37:26 susan Stab74 $";
#endif

#include <andrewos.h>
#include <stdio.h>
#include <ctype.h>
#include "rtf2.h"
#include "input.h"

extern TABLE Table;
extern FILESTACK FileStack;
int offset(), roffset();
void AbsorbSpace(), AbsorbWhiteSpace();
char *makelower();


char *makelower(instruction)
     char *instruction;
{
  int i;
  
  for(i=0; i<strlen(instruction); i++)
    {
      if(isupper(instruction[i]))
	instruction[i] = tolower(instruction[i]);
    }

  return(instruction);
}
  
/* For Development Use Only:  Comment Out for Final Production */
void TempPrintList()
{
  TABLE tmp=Table;

  while(tmp != NULL)
    {
      printf("rtfword is %s  mode is %d ez is ", tmp->rtfword, tmp->mode);
      if(tmp->mode & COMMAND)
	printf("COMMAND\n");
      else 
	printf("%s\n", tmp->ez.word);
      tmp = tmp-> next;
    }
}


int offset(string, character)
     char *string, character;
{
  char *loc;

  loc = strchr(string, character);

  if (loc) return (strlen(string) - strlen(loc));
  else return(strlen(string));
}


int roffset(string, character)
     char *string, character;
{
  char *loc;

  loc = strrchr(string, character);

  if (loc) return (strlen(string) - strlen(loc));
  else return(strlen(string));
}


void usage()
{
  fprintf(stderr, "\nUsage:  %s %s\n\n", me,
	 "[rtffile [ezfile]] [-t transtable] [-e stderrfile]");
  exit(0);
}  


void CloseFiles()
{
  fprintf(fout, "\n\\enddata{text, %d}\n", MasterToken);

  fclose(fout);
  fclose(ftrans);
  if(ferr != NULL)
    fclose(ferr);
}


void AbsorbSpace()
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


void AbsorbNewlines()
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


void AbsorbWhiteSpace()
{
  int in;
  char ch;

  while((in=fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if (ch=='\n' || ch=='\r')
	CurrLine++;
      else if(ch != ' ' && ch!='\t')
	{
	  ungetc(ch, fin);
	  break;
	}
    }
}

reverse(s)
char s[];
{
    int c, i, j;
    for(i=0, j=strlen(s)-1; i<j; i++, j--)
    {
	c=s[i];
	s[i]=s[j];
	s[j]=c;
    }
}

itoa(n, s)
int n;
char s[];
{
    int i, sign;
    if ((sign=n) < 0)
	n = -n;
    i=0;
    do{
	s[i++] = n % 10 + '0';
    } while ((n /=10)>0);
    if(sign<0)
	s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

void CloseBraces()
{
   for(; Levels>0; Levels--)
      fputc('}', fout);
   FontSize = 24;
}
