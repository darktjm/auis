/*  
 misc.c of

 2rtf: a facility to convert files in the ATK file format to files compatible with the RTF format.

 Scribetext is copyright (c) 1989, 1990 by the Massachusetts Institute of
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
 
 2rtf was written by Scott Rixner, rixner@ATHENA.MIT.EDU and Jeremy Paul Kirby, jpkirby@ATHENA.MIT.EDU
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
#include <stdio.h>
#include <ctype.h>
#include "2rtf.h"
#include "input.h"

extern struct TableStruct *Table;
/* extern struct FileStackStruct *FileStack; */

int offset(char *string, char character);
int roffset(char *string, char character);
void AbsorbSpace();
char *makelower();

char *makelower(char *instruction)
{
  int i;
  
  for(i=0; i<strlen(instruction); i++)
    {
      if(isupper(instruction[i]))
	instruction[i] = tolower(instruction[i]);
    }

  return(instruction);
}
  
/*
void TempPrintList()
{
  TABLE tmp=Table;

  while(tmp != NULL)
    {
      printf("ezword is %s  mode is %d rtf is ", tmp->ezword, tmp->mode);
      if(tmp->mode & COMMAND)
	printf("COMMAND\n");
      else 
	printf("%s\n", tmp->rtf.word);
      tmp = tmp->next;
    }
}
*/

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
	 "[ezfile [rtffile]] [-t transtable] [-e stderrfile]");
  exit(0);
}  


void CloseFiles(void)
{
  fputc('}', fout);

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
/* doesn't handle putting in the proper number of "\par"'s. */
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

void Newlines(void)
{
   int n = 0;
   int k;
   char ch, tmp_instruction[TMP_SIZE];

   CurrLine++;
   while(1)
   {
      ch = (char) fgetc(fin);
      if(ch != '\r' && ch != '\n')
      {
         ungetc(ch, fin);
         break;
      }
      CurrLine++;
      n++;
   }

   if(n>0)
      paragraph = 1;

   for(; n>0; n--)
      fputs("\\par\n", fout);
}
