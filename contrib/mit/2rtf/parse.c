/*  
 parse.c of

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "2rtf.h"
#include "input.h"

static struct TableStruct *FindNode(int field, char *string);
static char *GetInstruction(void);
static int Execute(char *instruction, int transform);
static void ReplaceText(char *instruction, int mode, int transform, char tofind);

void ParseMain(char *Filein, char *Fileout)
/*
 *
 *  Handle standard beginning to all rtf files.  Make main
 *  call to ParseText(), Close files when done.  Print
 *  useful messages to users.
 *
 */
{
  CurrLine=1;

  fprintf(fout, "{\\rtf%d\\mac\\deff%d ", RTFVersion, NEW_YORK);

  /* font table - standard */

  fprintf(fout, "{\\fonttbl");
  fprintf(fout, "{\\f%d\\fswiss Chicago;}", CHICAGO);
  fprintf(fout, "{\\f%d\\froman New York;}", NEW_YORK);
  fprintf(fout, "{\\f%d\\fmodern Monaco;}", MONACO);
  fprintf(fout, "{\\f%d\\ftech Symbol;}", SYMBOL);
  fprintf(fout, "}");

  /* color table - may be omitted */

  fputs("\\ftnbj\\sectd\n", fout);
  fputs("\\linemod0\\linex0\\cols1\\endnhere ", fout);
  fputs("\\pard\\plain ", fout);

  printf("* Processing manuscript file %s\n", Filein);
  printf("* Translating to %s\n\n", Fileout);

  ParseText(EOF, NORMAL, PRINTTOFILE);
  CloseFiles();
  printf("* Finished processing %ld lines of %s.\n", CurrLine, Filein);
}

long int ParseText(int tofind, int transform, int action)
/*
 *
 *  tofind:     used to tell when this recursion of
 *              ParseText should fall back.
 *  transform:  used to check if individual characters
 *              should be transformed into anything.
 *              i.e. change to all caps
 *  action:     if set to PRINTTOFILE (0), normal
 *              parsing occurs.  Presently the only
 *              other option is NOP, which causes
 *              everything to be passed over until
 *              tofind is encountered, at which point
 *              it falls back.
 *
 */
{
   char ch, tmp_instruction[TMP_SIZE];
   struct TableStruct *tmp;
   struct StyleStackStruct *stytmp;
   int in;

   while((in = fgetc(fin)) != tofind)
   {
      ch = (char) in;
      if(paragraph && ch != '\\')
         paragraph = 0;
      if(ch == '\\')
      {
         ch = (char) fgetc(fin);
	 if((ch == '\\' || ch == '{' || ch == '}') && (!action))
            fprintf(fout, "\\%c", ch);
         else if(isspace(ch))
            continue;
	 else
	 {
	    ungetc(ch, fin);
	    strcpy(tmp_instruction, GetInstruction());
	    strcpy(tmp_instruction, makelower(tmp_instruction));
	    tmp = FindNode(EZCOLUMN, tmp_instruction);
	    if(tmp == NULL && (!action))
	    {
               /* Check if command is in stylesheet before
                  declaring an error. */

               stytmp = Style;
               while(stytmp != NULL)
               {
                  if(!strcmp(tmp_instruction, stytmp->name))
                     break;
                  stytmp = stytmp->next;
	       }

               if(stytmp == NULL)
               {
		 /* Because we don't interpret templates, we may find one of
		  * these.  Signal an error, and insert the command as a
		  * style annotation. */
       	         fprintf(ferr, "* EZ command %s not recognized\n",
			 tmp_instruction);
		 fprintf(fout, "{{\\v STYLE: %s}", tmp_instruction);

		 ch = ' ';
		 while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
		 {
		     ch = (char) fgetc(fin);
		     if(ch=='\n' || ch=='\r')
			 Newlines();
		 }

		 if(ch != '{')
		     ungetc(ch, fin);
   
		 ParseText('}', NORMAL, PRINTTOFILE);
		 putc('}', fout);
	       }
               else
               {
                  RStyleApply(stytmp);
	       }
            }
	    else if(tmp != NULL && !action)
               if(Execute(tmp_instruction, transform) == tofind)
	          break;
	 }
      }   
      else if(ch == '\r' || ch == '\n')
         Newlines();
      else if(ch == '\t')
         fputs("\\tab ", fout);
      else if(!action)
      {
         if(!transform)
            fputc(ch, fout);
         else if(transform == INDEX && ch == '+')
         {
            ch = (char) fgetc(fin);
            if(ch == '+')
	       fputs("\\:", fout);
            else
            {
               ungetc(ch, fin);
               fputs("+", fout);
	    }
         }
         else if(transform == HEADER && ch == '$')
         {
            ch = (char) fgetc(fin);
            if(ch == '$')
               fputs("$", fout);
            else
            {
		sprintf(tmp_instruction, "$");
		while(isalpha(ch))
		{
		    sprintf(tmp_instruction, "%s%c", tmp_instruction, ch);
		    ch = (char) fgetc(fin);
		}
		ungetc(ch, fin);
                strcpy(tmp_instruction, makelower(tmp_instruction));
	        tmp = FindNode(EZCOLUMN, tmp_instruction);
                if(tmp != NULL)
                {
                   if(!(tmp->mode & COMMAND))
                      fprintf(fout, "\\%s ", tmp->rtf.word);
                   else
                      tmp->rtf.fun(tmp_instruction, transform, tofind);
		}
                else
                   fputs(tmp_instruction, fout);
	    }
	 }
	 else if(transform == LOWERCASE && isupper(ch))
	    fputc(tolower(ch), fout);
	 else if(transform == CAPS && islower(ch))
	    fputc(toupper(ch), fout);
	 else
	    fputc(ch, fout);
      }	
   }
  
   if(tofind == EOF)
      return(CurrLine++);
/*   else if(tofind < 256)
   {
      ch = (char) fgetc(fin);
      if(ch != '\n' && ch != '\r')
	 ungetc(ch, fin);
      else
         Newlines();
   }*/

   return(CurrLine);
}


static struct TableStruct *FindNode(int field, char *string)
/*
 *
 *  Look up string in internal translation table.
 *
 */
{
  struct TableStruct *tmp=Table;

  if(field == EZCOLUMN)
    {
      while(tmp != NULL)
	{
	  if(!strcmp(tmp->ezword, string))
	    return(tmp);
	  else
	    tmp = tmp->next;
	}
      return(NULL);
    }
  
  if(field == RTFCOLUMN)
    {
      while(tmp != NULL)
	{
	  if(!strcmp(tmp->rtf.word, string))
	    return(tmp);
	  else
	    tmp = tmp->next;
	}
      return(NULL);    
    }

  printf("* Unknown error!\n* %s:debug  Invalid mode sent to FindNode().\n", me);
  exit(0);
}

static char *GetInstruction(void)
/*
 *
 *  Read in commands that occur after "\".
 *
 */
{
  char character, *instruction;
  int in;

  instruction = (char *) calloc (TMP_SIZE, sizeof(char));

  while((in = fgetc(fin)) != EOF)
    {
      character = (char) in;
      if(character == '{' || character == '\\')
      {
        ungetc(character, fin);
        return(instruction);
      }
      else if(character == ' ' || character == '\t')
        return(instruction);
      else if(character == '\n' || character == '\r')
      {
        Newlines();
	return(instruction);
      }
      else
	sprintf(instruction, "%s%c", instruction, character);
    }
  
  fprintf(ferr, "* Error:\n* %s: %s\n%s  ", me,
	  "Failure to find instruction after '@' character detected...",
	  "End of file reached.");
  
  CloseFiles();
  exit(1);
}


static int Execute(char *instruction, int transform)
/*
 *
 *  Handle instruction read by GetInstruction() accordingly.
 *
 */
{
  struct TableStruct *tmp;
  char ch;

  tmp = FindNode(EZCOLUMN, instruction);

  if(tmp->mode & QUOTEDCHAR)
    fputc(tmp->rtf.quote, fout);
  else if(tmp->mode & NAKED)
    {
      if(!(tmp->mode & COMMAND))
	ReplaceText(tmp->rtf.word, tmp->mode, transform, ' ');
      else
	return(tmp->rtf.fun(instruction, transform, ' '));
    }
  else
    {
      ch = ' ';
      while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
	{
	  ch = (char) fgetc(fin);
	  if(ch=='\n' || ch=='\r')
	    Newlines();
	}
      
      if(ch == '{')
	 if(!(tmp->mode & COMMAND))
	    ReplaceText(tmp->rtf.word, tmp->mode, transform, '}');
	 else
	    return(tmp->rtf.fun(instruction, transform, '}'));
      else
	{
	  fprintf(ferr, "* Fatal error:\n* %s: Lack of parameters for command %s\n%s", me,
		  tmp->ezword, "Program terminating");
	  exit(0);
	}
    }
  return CONTINUE;
}


static void ReplaceText(char *instruction, int mode, int transform, char tofind)
/*
 *
 *  Make simple replacement of text if ezword was not
 *  mapped to a command in the transtable.
 *
 */
{
  if(mode & NAKED)
     fprintf(fout, "\n\\%s ", instruction);
  else
  {
     fprintf(fout, "{\\%s ", instruction);

     ParseText(tofind, transform, PRINTTOFILE);

     fputc('}', fout);
  }
}
