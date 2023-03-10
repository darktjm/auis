 /*
 main.c of 

 Rtf2: a facility to convert RTF files to files 
 compatible with the ATK file format.

 Rtf2 is copyright (c) 1991 by the Massachusetts Institute of
 Technology.

 RTF is a product of the Microsoft Corporation

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
#include <util.h>
#include <stdio.h>
#include <sys/file.h>  /* access() */
#ifdef POSIX_ENV
#include <unistd.h>
#endif
#include <errno.h>
#include "rtf2.h"
#include "input.h"

TABLE Table=NULL;
VALUES Values=NULL;
FONT Font=NULL;
static char *Filein, *Fileout;
double LeftMargin = .75;
double RightMargin = 6.5;

char *me, *RTFchars;

int Token, MasterToken, TextDSVersion;
int RTFverceiling, CharSet, Levels, FontSize;
int flag, left_indented, right_indented, fnote, tabs;
long int CurrLine;

FILE *fin, *fout, *ftrans, *ferr;

static FILE *FileProcess(const char *prompt, char *filename, const char *mode);
static void MakeTable(void);
/* static void AddValue(char *name, char *value); */ /* use commented out */

int main(int argc, const char *argv[])
/*
 *
 *  Handle command line arguments, and get pointers to the
 *  appropriate files.  Pass control on to ParseMain,
 *  with the names of the input and output files as
 *  arguments.
 *
 */
{
  char filein[TMP_SIZE], fileout[TMP_SIZE], filetrans[TMP_SIZE], 
  fileerr[TMP_SIZE];
  int i, CommandErr=FALSE, OptionErr=FALSE;

/* void SetupEnvironment();*/

  /*  Get date and time into char *datestamp as quick as posssible */

  filein[0] = '\0';
  fileout[0] = '\0';
  filetrans[0] = '\0';
  fileerr[0] = '\0';

  Levels = 0;
  FontSize = 24;
  flag = 0;
  left_indented = right_indented = 0;
  fnote = 0;
  tabs = 0;

  me = (char *) calloc ((strlen(argv[0]) + 1), sizeof(char));
  strcpy(me, argv[0]);
  for(i=1; i<argc; i++)
    {
      if(strcmp(argv[i], "-t") == 0)
	{
	  if(i+1 < argc && argv[i+1][0] != '-')
	    strcpy(filetrans, argv[++i]);
	  else
	    {
	      fprintf(stderr, "%s: Filename must follow -t option.\n", me);
	      OptionErr = TRUE;
	    }
	}
      else if(strcmp(argv[i], "-e") == 0)
	{
	  if(i+1 < argc && argv[i+1][0] != '-')
	    strcpy(fileerr, argv[++i]);
	  else
	    {
	      fprintf(stderr, "* Error:\n* Filename must follow -e option.\n");
	      OptionErr=TRUE;
	    }
	}
      else
	{
	  if(argv[i][0] != '-')
	    {
	      if(strcmp(filein, "") == 0)
		strcpy(filein, argv[i]);
	      else if(strcmp(fileout,  "") == 0)
		strcpy(fileout, argv[i]);
	      else
		{
		  if(CommandErr == FALSE)
		    {
		      CommandErr = TRUE;
		      fprintf(stderr, "%s: Too many parameters.\n", me);
		    }
		  fprintf(stderr, "Extraneous parameter '%s'.\n", argv[i]);
		}
	    }
	  else
	    {
	      OptionErr = TRUE;	      
	      fprintf(stderr, "%s: Invalid command option %s\n", me, argv[i]);
	    }
	}
    if(CommandErr == TRUE || OptionErr == TRUE)
      usage();
    }

  printf("\nRtf2 Version %s\n\n", VERSION);

  ferr = FileProcess("Press ENTER and CALL THE BUG POLICE!!!", fileerr, "e");

  ftrans = FileProcess("Translation file?", filetrans, "t");

  fin = FileProcess("RTF file to convert?", filein, "r");

  if(!strcmp(fileout, ""))
    fout = FileProcess("Output file?", filein, "n");
  else
    fout = FileProcess("Output file?", fileout, "w");

  MakeTable();

/*  SetupEnvironment(Filein); */

  ParseMain(Filein, Fileout);
}


static FILE *FileProcess(const char *prompt, char *filename, const char *mode)
/*
 *
 *  Return a pointer to the file associated with filename,
 *  or ask for filename, if appropriate, using prompt, and
 *  return a pointer to that associated file.
 *
 *  modes:
 *
 *     e - error file
 *     t - trans file
 *     r - input file/all read files
 *     n - output file with unknown name
 *     w - output file with known name/all write files
 *
 *  If no trans file is specified, use 2rtf.trans,
 *  ultimately, "$ANDREWDIR/lib/2rtf.trans", but currently
 *  "./2rtf.trans".
 *
 *  If no output file is specified, create one using the
 *  input file - strip off ".ez" if it is there, and add
 *  ".rtf" regardless. (this is the 'n' mode case)
 *
 *  If no error file is specified, stderr is used.
 *
 *  If no input file has been specified, always use prompt
 *  and ask for one - this is the general case; if at this
 *  point filename is still "" the function uses prompt to
 *  ask for a filename.
 *
 */
{
  int accessible, readable, len;
  FILE *fpt;
  char *filename2 = NULL, number[20], instruction[TMP_SIZE];
  const char *fullspec;

  if(!strcmp(filename, "") && !strcmp(mode, "t"))
  {
    fullspec = AndrewDir("/lib/rtf2.trans"); 
    accessible = access(fullspec, F_OK);
    if(accessible != 0)
      {
	  printf("* Unable to find standard translation file.\n\n");
	  strcpy(filename, "");
      }
      else
      {
	  filename2 = strdup(fullspec);
	  filename = filename2;
      }
  }

  if(!strcmp(mode, "n"))
    {
      if(rindex(filename, '.') != NULL)
	{
	  if(!ULstrcmp(rindex(filename, '.'), ".rtf"))
	    {
	      len = roffset(filename, '.');
	      filename2 = (char *) malloc ((len + 8));
	      memcpy(filename2, filename, len);
	      filename2[len] = '\0';
	    }	 
	}
      else
	{
	filename2 = (char *) malloc ((strlen(filename) + 8));
	strcpy(filename2, filename);
	}

      strcat(filename2, ".ez");
      filename = filename2;

      accessible = access(filename, F_OK);
      if(!accessible)
	{
	  sprintf(instruction, "mv %s %s~", filename, filename);
	  system(instruction);
	}
    }

  if(!strcmp(mode , "t"))
    mode = "r";

  if(!strcmp(mode, "n"))
    mode = "w";

  if(!strcmp(filename, ""))
    {
      if(!strcmp(mode, "e"))
	  return(stderr);      
      printf("%s ", prompt);
      /* scanf("%s", filename); */
      input_read_up_to_whitespace(filename, TMP_SIZE, stdin);
    }
  
  accessible = access(filename, F_OK);
  
  if(!strcmp(mode, "r"))
    strcpy(number, "does not exist");
  else
    {
      accessible = !accessible;
      strcpy(number, "exists");
    }
  
  if(accessible != 0)
    {  
      fprintf(stderr, "* Error!\n* %s: File %s %s\n", me, filename, number);
      usage();
    }
  
  if(!strcmp(mode, "r"))
    {
      readable = access(filename, R_OK);
      if(readable)
	{
	  fprintf(stderr, "* Error!\n* %s: Unauthorized to read from %s.\n", me, filename);
	  usage();
	}
    }
  
  if(!strcmp(mode, "e"))
    mode = "w";

  if((fpt=fopen(filename, mode)) == NULL)
    {
      fprintf(stderr, "* Unknown error!\n* %s: Unrecognizable error opening %s for access.\n\n", 
	      me, filename);
      fprintf(stderr, "* %s.debug: Error %d\n", me, errno);
      usage();
    }

  if(!strcmp(mode, "r"))
    {
      Filein = strdup(filename);
    }
  else if(!strcmp(mode, "w"))
    {
      Fileout = strdup(filename);
    }

  if(filename2)
	free(filename2);

  return(fpt);
}


static void MakeTable(void)
/*
 *
 *  Create internal translation table by reading in ftrans.
 *
 */
{
  TABLE tmp;
  char ch, *rtfword, ezword[TMP_SIZE];  
  int in, len, rtfc=0, dsv=0, rtfvc=0;

  rtfword = (char *) calloc(TMP_SIZE, sizeof(char));

  printf("* Creating internal translation table...");
  while(1)
    {
      in = fgetc(ftrans);
      if(in==EOF)
	break;
      ch = (char) in;
      if(ch == '#' || ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t')
	{
	  while(ch != '\n' && ch != '\r')
	    ch = (char) fgetc(ftrans);
	  continue;
	}
      else
	ungetc(ch, ftrans);

      /* if(fscanf(ftrans, "%s%s", rtfword, ezword)==EOF)
	break; */
      if (input_read_up_to_whitespace(rtfword, TMP_SIZE, ftrans) == EOF)
	  break;
      input_skip_whitespace(ftrans);
      if (input_read_up_to_whitespace(ezword, TMP_SIZE, ftrans) == EOF)
	  break;

      rtfword = makelower(rtfword);

      if(ezword[0]=='~')
	{
	  len = strlen(rtfword) + 1;

	  if(!strcmp(ezword, "~RTFchars"))
	    {
	      RTFchars = (char *) malloc (len);
	      strcpy(RTFchars, rtfword);
	      rtfc = 1;
	    }
	  else if(!strcmp(ezword, "~TextDSVersion"))
	    {
	      TextDSVersion = atoi(rtfword);
	      dsv = 1;
	    }
	  else if(!strcmp(ezword, "~RTFverceiling"))
	    {
	      RTFverceiling = atoi(rtfword);
	      rtfvc = 1;
	    }
	  else
	    {
	      fprintf(stderr, "\n* Error!\n* %s: %s\n%s%s%s%s\n", me,
		      "Corrupt translation table:  Presence of invalid environment entry:",
		      "Corrupt entry:  \"", rtfword, "\" --> ", ezword);
	    }
	}
      else
	{
	  tmp = (TABLE) malloc (sizeof(struct TableStruct));
	  tmp->rtfword = (char *) calloc (strlen(rtfword) + 1, 
					     sizeof(char));
	  strcpy(tmp->rtfword, rtfword);
	  tmp->mode = 0;

	  if( (ezword[0]=='@') || (ezword[0]=='!') || (ezword[0]=='#') || 
	     (ezword[0]=='$') || (ezword[0]=='^'))
	    {
	      switch ((char) ezword[0])
		{
		case '@':
		  tmp->mode=COMMAND | RTFCOMMAND;
		  break;
		case '!':
		  tmp->mode=COMMAND | EZCOMMAND;
		  break;
      		case '#':
		  tmp->mode=COMMAND | RTFCOMMAND | NAKED | SUPPRESSDELS;
		  break;
		case '$':
		  tmp->mode=COMMAND | RTFCOMMAND | NAKED | USEDELS;
		  break;
		case '^':
		  tmp->mode=QUOTEDCHAR;
		}
	      if(ezword[0]=='^')
		tmp->ez.quote = (char) atoi(&ezword[1]);
	      else
		tmp->ez.fun = AssignFunc(&ezword[1]);
	    }
	  else
	    {
	      tmp->mode = WORD;
	      tmp->ez.word = (char *) calloc(strlen(ezword) + 1, sizeof(char));
	      strcpy(tmp->ez.word, ezword);
	    }
	  if(Table == NULL)
	    {
	      Table = tmp;
	      Table->next = NULL;
	    }
	  else
	    {
	      tmp->next = Table;
	      Table = tmp;
	    }
	}
    }

  if(!rtfc || !dsv || !rtfvc)
    {
      fprintf(stderr, "\n* Error!\n* %s: %s\n\t%s %s %s\n%s", me, 
	      "Fatal error in translation table:  environment entry:",
	      !rtfc ? "~RTFchars" : "", 
	      !dsv ? "~TextDSVersion" : "", 
	      !rtfvc ? "~RTFverceiling" : "",
	      "weren't found and are necessary for operation.");
      exit(0);    
    }
  fclose(ftrans);

  printf("done\n");
}




/* Not necessary for rtf2, very necessary in 2rtf */

/* 
void SetupEnvironment(rootfile)
     char *rootfile;
{
  char *username, *wd, *getlogin(), *getwd(), *trimroot, *fullman;

  Go through all predefined string fields and define them
      and add them to the transtable  

  Take care of title, subject, author, operator, keywords, versionz, doccomm, vernz, creatim, revtim, printim, buptim, edminsz, yrz, moz, dyz, hrz, minz, nofpagesz, nofwordsz, nofcharsz, idz


  trimroot = (char *) malloc(1024);

  trimroot = &rootfile[roffset(rootfile, '/')];
  AddValue("title", trimroot);
  
  username=getlogin();
  AddValue("username", username);

  }

void AddValue(char *name, char *value)
{
  VALUES tmp;

  tmp = (VALUES) malloc(sizeof(struct ValuesStruct));

  tmp->name = (char *) calloc(strlen(name) + 1, sizeof(char));
  tmp->name = name;
  if (value == NULL) 
    {
      tmp->value = (char *) malloc (sizeof(char));
      tmp->value[0] = '\0';
    }
  else
    {
      tmp->value = (char *) calloc(strlen(value) + 1, sizeof(char));
      tmp->value = value;
    }

  if(Values == NULL)
    {
      Values = tmp;
      Values->next = NULL;
    }
  else
    {
      tmp->next = Values;
      Values = tmp;
    }
}
*/
