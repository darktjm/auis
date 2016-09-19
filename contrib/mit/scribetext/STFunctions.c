/*  
 STFunctions.c of

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

#include <andrewos.h>
#include <util.h>
#include "scribetext.h"

static int STDevice(const char *command, int tofind);
static int STDelete(const char *command, int tofind);
static int STNOP(const char *command, int tofind);
/* static int STCopy(const char *command, int tofind); */
static int STStrip(const char *command, int tofind);
static int STStupidstrip(const char *command, int tofind);
static int STSymbol(const char *command, int tofind);
static int STItemize(const char *command, int tofind);
/* static int STError(const char *command, int tofind); */
static int STFootnote(const char *command, int tofind);
static int STBegin(const char *command, int tofind);
static int STEnd(const char *command, int tofind);
static int STCaps(const char *command, int tofind);
static int STTilde(const char *command, int tofind);
static int STInclude(const char *command, int tofind);
static int STChapter(const char *command, int tofind);
static int STDefine(const char *command, int tofind);
static int STNewpage(const char *command, int tofind);
static int STLabel(const char *command, int tofind);
static int STRef(const char *command, int tofind);
static int STTsymbol(const char *command, int tofind);
static int STVerbatim(const char *command, int tofind);
static int STBlankspace(const char *command, int tofind);
static int STValue(const char *command, int tofind);

FP AssignFunc(const char *ezword)
{
  if(!ULstrcmp(ezword, "delete"))
    return(STDelete);

  if(!ULstrcmp(ezword, "symbol"))
    return(STSymbol);

  if(!ULstrcmp(ezword, "strip"))
    return(STStrip);

  if(!ULstrcmp(ezword, "device"))
    return(STDevice);

  if(!ULstrcmp(ezword, "imbed"))
    return(STBegin);

  if(!ULstrcmp(ezword, "footnote"))
    return(STFootnote);
  
  if(!ULstrcmp(ezword, "end"))
    return(STEnd);

  if(!ULstrcmp(ezword, "caps"))
    return(STCaps);

  if(!ULstrcmp(ezword, "tilde"))
    return(STTilde);

  if(!ULstrcmp(ezword, "nop"))
    return(STNOP);

  if(!ULstrcmp(ezword, "include"))
    return(STInclude);

  if(!ULstrcmp(ezword, "chapter"))
    return(STChapter);

  if(!ULstrcmp(ezword, "define"))
    return(STDefine);

  if(!ULstrcmp(ezword, "newpage"))
    return(STNewpage);

  if(!ULstrcmp(ezword, "stupidstrip"))
    return(STStupidstrip);

  if(!ULstrcmp(ezword, "verbatim"))
    return(STVerbatim);

  if(!ULstrcmp(ezword, "itemize"))
    return(STItemize);

  if(!ULstrcmp(ezword, "blankspace"))
    return(STBlankspace);

  if(!ULstrcmp(ezword, "label"))
    return(STLabel);

  if(!ULstrcmp(ezword, "ref"))
    return(STRef);

  if(!ULstrcmp(ezword, "pageref"))
    return(STRef);

  if(!ULstrcmp(ezword, "tsymbol"))
    return(STTsymbol);

  if(!ULstrcmp(ezword, "define"))
    return(STValue);

  return(STNOP);
}

int STUniqueID(void)
{
  return(++Token);
}

static int STDevice(const char *command, int tofind)
{
  char ch, device[TMP_SIZE];
  int in;

  device[0] = '\0';

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
	  if(ch !='\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      else
	sprintf(device, "%s%c", device, ch);
    }

  if(ULstrcmp(device, "postscript"))
    {
      printf("* Output device %s was specified in input file.\n",
	      device);
      printf("  Please note that printer selections must occur %s\n",
	      "from within the specific");
      printf("  Andrew application.\n");
    }
    return 0; /* only needs to not be POP_JOB - tjm */
}


static int STDelete(const char *command, int tofind)
{
  ParseText(tofind, "", "", NORMAL, NOP);
  return 0; /* only needs to not be POP_JOB - tjm */
}

  
static int STNOP(const char *command, int tofind)
{
  return 0; /* only needs to not be POP_JOB - tjm */
}


#if 0 /* unused */
static int STCopy(const char *command, int tofind)
{
  fputs(command, fout);
  return 0; /* only needs to not be POP_JOB - tjm */
}
#endif


static int STStrip(const char *command, int tofind)
{
  ParseText(tofind, "", "", NORMAL, PRINTTOFILE);
  return 0; /* only needs to not be POP_JOB - tjm */
}



static int STStupidstrip(const char *command, int tofind)
{
  char ch;
  int in;

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      else
	fputc(ch, fout);
    }
  return 0; /* only needs to not be POP_JOB - tjm */
}


static int STSymbol(const char *command, int tofind)
{
  int token;

  token = STUniqueID();

  fprintf(fout, "\\begindata{acc_%s, %d}\n\\enddata{aac_%s, %d}\n",
	  command, token, command, token);
  fprintf(fout, "\\view{aac_%s, %d, 0, 0, 0}", command, token);
  return 0; /* only needs to not be POP_JOB - tjm */
}


static int STItemize(const char *command, int tofind)
{
  fputs("\\itemize{", fout);

  ParseText(tofind, "\\symbol{7}\t", "", NORMAL, PRINTTOFILE);

  fputs("}\n", fout);
  return 0; /* only needs to not be POP_JOB - tjm */
}

#if 0 /* unused */
static int STError(const char *command, int tofind)
{
  fprintf(ferr, "* Unknown error!\n* %s: unknown error in input file.\n", me);
  return 0; /* only needs to not be POP_JOB - tjm */
}
#endif


static int STFootnote(const char *command, int tofind)
{
  int token;

  token = STUniqueID();

  fprintf(fout, "\\footnote{\\\n");
  fprintf(fout, "\\begindata{fnote,%d}\n", token);
  fprintf(fout, "\\textdsversion{%d}\n", TextDSVersion);

  ParseText(tofind, "", "", NORMAL, PRINTTOFILE);

  fprintf(fout, "\\\n\\enddata{fnote,%d}\n\\view{fnotev,%d,3,0,0}}",
	  token, token);
  return 0; /* only needs to not be POP_JOB - tjm */
}


static int STBegin(const char *command, int tofind)
{
  char ch, *instruction, *makelower();
  int in;
  TABLE tmp;

  instruction = (char *) calloc (TMP_SIZE, sizeof(char));

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  ch = (char) fgetc(fin);
	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      else
	sprintf(instruction, "%s%c", instruction, ch);
    }

  instruction = makelower(instruction);

  tmp = FindNode(SCRIBECOLUMN, instruction);
  if(tmp == NULL)
    {
      printf("* Scribe environment @%s not recognized\n", instruction);
      fprintf(fout, "@begin(%s)", instruction);
    }
  else
    {
      if(!(tmp->mode & COMMAND))
	{
	  fprintf(fout, "\\%s{", tmp->ez.word);
	  ParseText(POP_JOB, "", "", NORMAL, PRINTTOFILE);
	  fputc('}', fout);
	}
      else
	  return(tmp->ez.fun(instruction, POP_JOB));
    }
  return 0; /* only needs to not be POP_JOB - tjm */
}


static int STEnd(const char *command, int tofind)
{
  char ch, *instruction, *makelower();
  int in;
  TABLE tmp;

  instruction = (char *) calloc (TMP_SIZE, sizeof(char));

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
 	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      sprintf(instruction, "%s%c", instruction, ch);
    }

  instruction = makelower(instruction);

  tmp = FindNode(SCRIBECOLUMN, instruction);

  if(tmp == NULL)
    {
      printf("* Scribe environment @%s not recognized...\n", instruction);
      fprintf(fout, "@end(%s)", instruction);
      return 0; /* only needs to not be POP_JOB - tjm */
    }
  else
    return(POP_JOB);
}


static int STCaps(const char *command, int tofind)
{
  fputs("\\smaller{", fout);

  ParseText(tofind, "", "", CAPS, PRINTTOFILE);

  fputc('}', fout);
  return 0; /* only needs to not be POP_JOB - tjm */
}


static int STTilde(const char *command, int tofind)
{
  /* ABSORB EVERYTHING UNTIL THE NEXT PRINTABLE CHARACTER */

  char ch;
  int in;
  
  while((in = fgetc(fin) != EOF))
    {
      ch = (char) in;
      if(ch != '\n' && ch != '\r')
	break;
      else
	CurrLine++;
    }
	
  if(in == EOF)
    {
      fprintf(ferr, "* Error!\n* %s: End of file reached after @~ command\n", me);
      CloseFiles();
    }
  return 0; /* only needs to not be POP_JOB - tjm */
}


static int STInclude(const char *command, int tofind)
{
  char ch, instruction[TMP_SIZE];
  int in, accessible, readable;
  long int tempcurrline;

  instruction[0] = 0;

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
 	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      sprintf(instruction, "%s%c", instruction, ch);
    }

  accessible = access(instruction, F_OK);
  readable = access(instruction, R_OK);

  if(accessible)
    {
      fprintf(ferr, "* Unable to process @INCLUDE(%s):\n", instruction);
      fprintf(ferr, "  File %s not found.\n", instruction);
    }
  else if(readable)
    {
      fprintf(ferr, "* Unable to process @INCLUDE(%s):\n", instruction);
      fprintf(ferr, "  Insufficient access to read %s.\n", instruction);
    }
  else
    {
      printf("* Incorporating %s\n", instruction);
      PushFile(instruction);
      tempcurrline = ParseText(EOF, "", "", NORMAL, PRINTTOFILE);
      printf("* Finished incorporating %ld lines of %s\n",  tempcurrline,
	     instruction);
    }
  return 0; /* only needs to not be POP_JOB - tjm */
}


static int STChapter(const char *command, int tofind)
{
  int token;

  token = STUniqueID();

  fprintf(fout, 
	  "\\begindata{bp,%d}\n\\enddata{bp,%d}\n\\view{bpv,%d,1,0,0}\n",
	  token, token, token);
  ReplaceText("chapter", WORD, tofind);
  return 0; /* only needs to not be POP_JOB - tjm */
}



static int STDefine(const char *command, int tofind)
{
  int in, nomoreflag=FALSE;
  char ch, *instruction, *makelower();
  TABLE tmp;

  instruction = (char *) calloc (TMP_SIZE, sizeof(char));
  tmp = (TABLE) malloc(sizeof(struct TableStruct));
  
  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      else if(ch == ',' || ch == '=' || ch == ' ')
	nomoreflag = TRUE;
      else if(nomoreflag == FALSE)
	sprintf(instruction, "%s%c", instruction, ch);
    }

  /* Upon finding a "@define(new, ...)" / "@define(new=old, ...)",
     make an entry in the internal translation table with 
     .scribeword equal to instruction, mode equal to
     (COMMAND | SCRIBECOMMAND | PARAMETERIZED | USEDELS)
     and the command function pointer for STDelete.    */

  instruction = makelower(instruction);

  tmp->scribeword = (char *) calloc((strlen(instruction) + 1), sizeof(char));

  strcpy(tmp->scribeword, instruction);
  tmp->mode = (COMMAND | SCRIBECOMMAND | PARAMETERIZED | USEDELS);
  tmp->ez.fun = STDelete;
  tmp->next = Table;
  Table = tmp;

  printf("* @%s command encountered.  All future occurences of\n", command);
  printf("  @%s will be deleted.\n", instruction);

  return 0; /* only needs to not be POP_JOB - tjm */
}


static int STNewpage(const char *command, int tofind)
{
  char ch, *instruction, *makelower();
  int in, times, token;
  void AbsorbNewlines();

  instruction = (char *) calloc (TMP_SIZE, sizeof(char));

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
 	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      sprintf(instruction, "%s%c", instruction, ch);
    }

  if(strcmp(instruction, ""))
    times = atoi(instruction);
  else
    times = 1;

  for(in=0; in<times; in++)
    {
      token = STUniqueID();
      fprintf(fout, "\\begindata{bp,%d}\n\\enddata{bp,%d}\n", token, token);
      fprintf(fout, "\\view{bpv,%d,1,0,0}\n", token);
    }

  AbsorbNewlines();
  return 0; /* only needs to not be POP_JOB - tjm */
}

static int STLabel(const char *command, int tofind)
{
  char ch, *codeword;
  int in, token;

  codeword = (char *) malloc(TMP_SIZE);

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      sprintf(codeword, "%s%c", codeword, ch);
    }
  
  if(!strcmp(codeword, ""))
    {
      printf("* No codeword specified with %s.  Ignoring.\n", command);
      return 0; /* only needs to not be POP_JOB - tjm */
    }
  else
    {
      token = STUniqueID();
      fputs("\\footnote{\\\n", fout);
      fprintf(fout, "\\begindata{texttag,%d}\n\\textdsversion{%d}\n", token,
	      TextDSVersion);
      fprintf(fout, "%s\\\n\\enddata{texttag,%d}\n\\view{texttagv,%d,0,0,0}}",
	      codeword, token, token);
    }
  return 0; /* only needs to not be POP_JOB - tjm */
}
  
static int STRef(const char *command, int tofind)
{
  char ch, *codeword;
  int in, token;

  codeword = (char *) malloc(TMP_SIZE);

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      sprintf(codeword, "%s%c", codeword, ch);
    }
  
  if(!strcmp(codeword, ""))
    {
      printf("* No codeword specified with %s.  Ignoring.\n", command);
      return 0; /* only needs to not be POP_JOB - tjm */
    }
  else
    {
      token = STUniqueID();
      fputs("\\footnote{\\\n", fout);
      fprintf(fout, "\\begindata{textref,%d}\n\\textdsversion{%d}\n", token,
	      TextDSVersion);
      
      if(!ULstrcmp(command, "pageref"))
	fprintf(fout, "# %s", codeword);
      else
	fprintf(fout, "%s", codeword);

      fprintf(fout, "\\\n\\enddata{textref,%d}\n\\view{textrefv,%d,0,0,0}}",
	      token, token);
    }
  return 0; /* only needs to not be POP_JOB - tjm */
}

static int STTsymbol(const char *command, int tofind)
{
  if(!strcmp(command, "zts"))
    fputs("\\formatnote{\\\\(ts}", fout);

  return(STStupidstrip(command, tofind));

}

  
static int STVerbatim(const char *command, int tofind)
{
  verbatim++;
  ParseText(tofind, "", "", NORMAL, PRINTTOFILE);
  verbatim--;
  return 0; /* only needs to not be POP_JOB - tjm */
}


static int STBlankspace(const char *command, int tofind)
{
char ch, *quan, *qual, *combined, quality;
int in;
double quantity;

quan = (char *) malloc (TMP_SIZE);
qual = (char *) malloc (TMP_SIZE);
combined = (char *) malloc (TMP_SIZE);

while((in=fgetc(fin)) != EOF)
  {
    ch = (char) in;
    if(ch == tofind)
      {
	ch = (char) fgetc(fin);
	if(ch != '\n' && ch != '\r')
	  ungetc(ch, fin);
	else
	  CurrLine++;
	break;
      }
    sprintf(combined, "%s%c", combined, ch);
  }

sscanf(combined, "%s %s", quan, qual);

quantity = atof(quan);

if(!ULstrncmp(qual, "in", 2) || !(ULstrcmp(qual, "\"")))
  quality = 'i';
else if(!ULstrcmp(qual, "cm") || !ULstrncmp(qual, "centimeter", 10))
  quality = 'c';
else if(!ULstrcmp(qual, "mm") || !ULstrncmp(qual, "millimeter", 10))
  {
    quality = 'c';
    quantity *= 0.1;
  }
else if(!ULstrncmp(qual, "pt", 2) || !ULstrncmp(qual, "point", 5))
  quality = 'p';
else if(!ULstrncmp(qual, "pica", 4))
  quality = 'P';
else if(!ULstrncmp(qual, "em", 2) || !ULstrncmp(qual, "quad", 4))
  quality = 'm';
else if(!ULstrncmp(qual, "en", 2))
  quality = 'n';
else if(!ULstrncmp(qual, "line", 4))
  quality = 'v';
else
  {
    printf("* Unit of measurement %s not recognized:\n", qual);
    return 0;
  }

fprintf(fout, "\\formatnote{.sp %lf%c}\n", quantity, quality);
return 0; /* only needs to not be POP_JOB - tjm */
}
  
static int STValue(const char *command, int tofind)
{
  char *instruction, ch;
  int in;

  instruction = (char *) calloc (TMP_SIZE, sizeof(char));

  instruction[0] = '\0';

  while((in = fgetc(fin)) != EOF)
    {
      ch = (char) in;
      if(ch == tofind)
	{
	  ch = (char) fgetc(fin);
 	  if(ch != '\n' && ch != '\r')
	    ungetc(ch, fin);
	  else
	    CurrLine++;
	  break;
	}
      sprintf(instruction, "%s%c", instruction, ch);
    }
  return 0; /* only needs to not be POP_JOB - tjm */
}
    


/* 
   THIS CODE IS NOT IMPLEMENTED ANYWHERE.  IT EXISTS AS A FUTURE POSSIBLE
   SOURCE FOR GREEK TRANSLITERATION USING   ParseText()'s    transform
   FACILITY.

char *STGreek(character)
     char character;
{
  char toreturn[2];

  switch (character)
    {
    case 'c':
      return("ch");
    case 'f':
      return("ph");
    case 'j':
      return("");
    case 'q':
      return("th");
    case 'v':
      return("");
    case 'w':
      return("o");
    case 'y':
      return("ps");
    default:
      toreturn[0]=character;
      toreturn[1]='\0';
      return(toreturn);
    }
}
*/
