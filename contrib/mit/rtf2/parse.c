/*  
 parse.c of

 Rtf2: a facility to convert RTF files to files 
 compatible with the ATK file format.

 Rtf2 is copyright (c) 1991 by the Massachusetts Institute of Technology.

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
*/

#include <andrewos.h>
#include <stdio.h>
#include <ctype.h>
#include "rtf2.h"
#include "input.h"

static int ExecuteSpecial(char *character);
static int ReplaceText(char *instruction, int mode, char tofind);

void ParseMain(char *Filein, char *Fileout)
/*
 *
 *  Takes care of the beginning of the input and output files.
 *    adds needed styles
 *    detects character set
 */
{
  int in, token, foundver;
  char ch, tmp_instruction[TMP_SIZE];

  CurrLine=1;
/*  token = R2UniqueID();*/
  token = 20001;
  MasterToken = token;

  fprintf(fout, "\\begindata{text, %d}\n", MasterToken);
  fprintf(fout, "\\textdsversion{%d}\n", TextDSVersion);

  printf("* Processing file %s\n", Filein);
  printf("* Translating to %s\n\n", Fileout);

  /* Checks that beginning of rtf file is: "{\rtf" */

  AbsorbSpace();
  ch = (char) getc(fin);
  if(ch != '{')
    {
      printf("* %s: %s is not an RTF file.\n", me, Filein);
      CloseFiles();
      exit(0);
    }

  while((in = getc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch=='\\')
	break;
    }

  if(in == EOF)
    {
      printf("* %s: %s is not an RTF file.\n",
	     me, Filein);
      CloseFiles();
      exit(0);
    }

  strcpy(tmp_instruction, GetInstruction());
  strcpy(tmp_instruction, makelower(tmp_instruction));

  if(strncmp(tmp_instruction, "rtf", 3))
     {
       printf("* %s: %s is not an RTF file.\n", me, Filein);
       CloseFiles();
       exit(0);
     }

  sscanf(tmp_instruction, "rtf%d", &foundver);

  if(foundver > RTFverceiling)
    {
      printf("* %s: Only RTF versions %d and lower supported.\n", 
	     me, RTFverceiling);
      printf("  File %s is written in RTF version %d.\n", Filein,
	     foundver);
      CloseFiles();
      exit(0);
    }

  printf("RTF version %d detected.\n", foundver);

  while((in = getc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch=='\\')
	  break;
    }

  if(in == EOF)
    {
      printf("* %s: %s is not an RTF file.\n",
	     me, Filein);
      CloseFiles();
      exit(0);
    }

  /*  Looks for character set declaration */

  strcpy(tmp_instruction, GetInstruction());
  strcpy(tmp_instruction, makelower(tmp_instruction));

  if(!strcmp(tmp_instruction, "ansi"))
    {
      CharSet = ANSI;
      printf("ANSI Character set detected.\n");
    }
  else if(!strcmp(tmp_instruction, "mac"))
    {
      CharSet = MAC;
      printf("Macintosh Character set detected.\n");
    }
  else if(!strncmp(tmp_instruction, "pc", 2))
    {
      CharSet = PC;
      printf("IBM PC Character set detected.\n");
    }
  else
    {
      printf("* %s: Character set not specified.\n", me);
      exit(0);
    }

  ParseText(EOF, NORMAL, PRINTTOFILE);
  CloseFiles();
  printf("* Finished processing %ld lines of %s.\n", CurrLine, Filein);
}

long int ParseText(int tofind, int transform, int action)
/*
 *
 *  Actually goes through the input file, character by
 *  character, and detects commands and performs the
 *  appropriate action.
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
  char ch, instruction[TMP_SIZE], tmp_instruction[TMP_SIZE];
  TABLE tmp, FindNode();
  int in;

  while((in = fgetc(fin)) != tofind)
    {
      ch = (char) in;
      if(ch == '\\')
	{
	  ch = (char) fgetc(fin);
	  if((ch=='\\' || ch=='{' || ch=='}') && (!action))
	    {
	      /* Backslash characters */

	      sprintf(tmp_instruction, "\\%c", ch);
	      fputs(tmp_instruction, fout);
	    }
	  else if(ch==':')
	    fputs("++", fout);
	  else if(strchr(RTFchars, ch))
	    ExecuteSpecial(&ch);
	  else
	    {
	      /* rtf commands */

	      ungetc(ch, fin);
	      strcpy(tmp_instruction, GetInstruction());
	      sscanf(tmp_instruction, "%[A-z]", instruction);
	      strcpy(instruction, makelower(instruction));
	      tmp = FindNode(RTFCOLUMN, instruction);
	      if(tmp == NULL && (!action))
		{
		  fprintf(ferr, "* RTF command \\%s not recognized\n",
			  instruction);
		  fprintf(fout, "\\%s", tmp_instruction);
		}
	      else if((tmp != NULL) && (!action))
		if(Execute(tmp_instruction) == tofind)
		  break;

	    }
	}
      else if(ch=='\r' || ch=='\n')
	CurrLine++;
      else if(ch=='}')
	{
	  printf("Closing brace found.  RTF File closed.\n");
	  return(CurrLine++);
	}
      else if((ch=='{') && (!action))
	   R2Begin();
      else if((ch=='{') && (action))
	   ParseText('}', NORMAL, action);
   
      else if(!action)
	{
	  /* Perform transformations */

	  if(!transform)
	    fputc(ch, fout);
	  else if(transform == LOWERCASE && isupper(ch))
	    fputc(tolower(ch), fout);
	  else if(transform == CAPS && islower(ch))
	    fputc(toupper(ch), fout);
	  else
	    fputc(ch, fout);
	}
    }
   
  
  if(tofind == EOF)
    {
      return(CurrLine);
    }
  else if(tofind < 256)
    {
      ch = (char) fgetc(fin);
      if(ch != '\n' && ch != '\r')
	ungetc(ch, fin);
      else
	CurrLine++;
    }
  return(CurrLine);
}


TABLE FindNode(int field, char *string)
/*
 *
 *  Look up command in table created from the trans file.
 *
 */
{
  TABLE tmp=Table;

  if(field == RTFCOLUMN)
    {
      while(tmp != NULL)
	{
	  if(!strcmp(tmp->rtfword, string))
	    return(tmp);
	  else
	    tmp = tmp->next;
	}
      return(NULL);
    }
  
  if(field == EZCOLUMN)
    {
      while(tmp != NULL)
	{
	  if(!strcmp(tmp->ez.word, string))
	    return(tmp);
	  else
	    tmp = tmp->next;
	}
      return(NULL);    
    }

  printf("* Unknown error!\n* %s:debug  Invalid mode sent to FindNode().\n", me);
  exit(0);
}


static int ExecuteSpecial(char *character)
/*
 *
 *  Handle special rtf characters.
 *
 */
{
  TABLE tmp;
  char string[2];
  
  sprintf(string, "%c", *character);
  
  tmp = FindNode(RTFCOLUMN, string);
  
  if(tmp == NULL)
    {
      fprintf(ferr, "* Fatal error:\n* %s: %s\n%s%c%s\n%s\n%s\n",        me,
	      "  Defective translation table:  ~", 
	      "  RTFchars entry promises meaning for \"", *character, "\",",
	      "  but there is no corresponding entry.",
	      "  Program terminating.");
      exit(0);
    }
  else
    return Execute(string);
}


char *GetInstruction(void)
/*
 *
 *  Read command in from the input file.
 *
 */
{
  char character, *instruction;
  int in;

  instruction = (char *) calloc (TMP_SIZE, sizeof(char));

  while((in = fgetc(fin)) != EOF)
    {
      character = (char) in;
      if(character == '\\' || character == '{' || character == '}')
	{
	  ungetc(character, fin);
	  return(instruction);
	}
      else if(character==' ' || character=='\t')
	return(instruction);
      else if(character=='\n' || character=='\r')
	{ 
	  CurrLine++;
	  return(instruction);
	}
      else
	sprintf(instruction, "%s%c", instruction, character);
    }
  
  fprintf(ferr, "* Error:\n* %s: %s\n%s  ", me,
	  "Failure to find instruction after '\\' character detected...",
	  "End of file reached.");
  
  CloseFiles();
  exit(1);
}



int Execute(char *instruction)
/*
 *
 *  Read in the delimiter associated with INSTRUCTION,
 *  if any, then call the associated function with the 
 *  appropriate parameters.
 *
 */
{
  TABLE tmp;
  char alphastring[TMP_SIZE], numdelstring[TMP_SIZE];
  int i, j, numdel, lettersaintover, unaryminusfound;

  alphastring[0]='\0';
  numdelstring[0]='\0';
  lettersaintover = 1;
  unaryminusfound = 0;

  for(i=0; i<(int)strlen(instruction); i++)
       {
	 if(((isalpha(instruction[i])) || (strchr(RTFchars, instruction[i]) && instruction[i]!='-') || (!strcmp(instruction, "-"))) && lettersaintover)

	      sprintf(alphastring, "%s%c", alphastring, instruction[i]);
      else if(instruction[i]=='-' && !unaryminusfound)
	{
	  unaryminusfound = 1;
	  lettersaintover = 0;
	  sprintf(numdelstring, "-");
	}
      else if(isdigit(instruction[i]))
	{
	  lettersaintover = 0;
	  sprintf(numdelstring, "%s%c", numdelstring, instruction[i]);
	}
      else
	{
	  printf("* %s: Invalid character '%c' in RTF command %s\n", me,
		 instruction[i], instruction);
	  printf("%s", "                                          ");
	  for(j=0; j<((int)strlen(me) + i); j++)
	    putchar(' ');
	  putchar('\n');
	  exit(0);
	}
    }

  numdel = atoi(numdelstring);
  tmp = FindNode(RTFCOLUMN, alphastring);

  if(tmp->mode & QUOTEDCHAR)
    {
    fputc(tmp->ez.quote, fout);
    return CONTINUE; /* presumably; was nothing - tjm */
    }
  else if(tmp->mode & NAKED)
    {
      if(!(tmp->mode & COMMAND))
	return(ReplaceText(tmp->ez.word, tmp->mode, ' '));
      else
	return(tmp->ez.fun(alphastring, numdel, ' '));
    }
  else
    {
      if(!(tmp->mode & COMMAND))
	return(ReplaceText(tmp->ez.word, tmp->mode, '}'));
      else
	return(tmp->ez.fun(alphastring, numdel, '}'));
    }
}


static int ReplaceText(char *instruction, int mode, char tofind)
/*
 *
 *  If the INSTRUCTION is naked, write the instruction to
 *  the outfile, either with or without empty braces, as
 *  appropriate.  Otherwise, write the instruction and
 *  continue processing the file.
 *
 */
{
  if(mode & NAKED)
    {
      fprintf(fout, "\n\\%s", instruction);      
      if(!(mode & SUPPRESSDELS))
	fputs("{}\n", fout);
      else
	fputs("\n", fout);
      return CONTINUE;
    }
  else
    {
      Levels++;
      fprintf(fout, "\\%s{", instruction);

      ParseText(tofind, NORMAL, PRINTTOFILE);
      
      CloseBraces();
      return tofind;
    }
}
