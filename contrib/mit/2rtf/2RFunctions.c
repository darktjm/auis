/*  
 2RFunctions.c of

 2rtf: a facility to convert files in the ATK file format to
 RTF manuscript files.

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

 2rtf was written by Scott Rixner, rixner@ATHENA.MIT.EDU and Jeremy Paul Kirby, jpkirby@ATHENA.MIT.EDU
*/

#include <andrewos.h>
#include <stdio.h>
#include <sys/file.h>
#include <ctype.h>
#include <util.h>
#include "2rtf.h"
#include "input.h"

static int RDelete(const char *command, int transform, int tofind);
static int RNOP(const char *command, int transform, int tofind);
/* static int RError(const char *command, int transform, int tofind); */
static int RBegin(const char *command, int transform, int tofind);
static int REnd(const char *command, int transform, int tofind);
static int RDSVer(const char *command, int transform, int tofind);
static int RText(const char *command, int transform, int tofind);
static int RAnnotation(const char *command, int transform, int tofind);
static int RFootnote(const char *command, int transform, int tofind);
static int RSkip(const char *command, int transform, int tofind);
static int RTitle(const char *command, int transform, int tofind);
static int RNewpage(const char *command, int transform, int tofind);
static int RHeader(const char *command, int transform, int tofind);
static int RSize(const char *command, int transform, int tofind);
static int RScript(const char *command, int transform, int tofind);
static int RFont(const char *command, int transform, int tofind);
static int RTemplate(const char *command, int transform, int tofind);
static int RLeft(const char *command, int transform, int tofind);
static int RIndent(const char *command, int transform, int tofind);
static int RIndex(const char *command, int transform, int tofind);
static int RStyleMain(const char *command, int transform, int tofind);
static int RMajorHeading(const char *command, int transform, int tofind);
static int RHeading(const char *command, int transform, int tofind);
static int RSubHeading(const char *command, int transform, int tofind);
static int RChapter(const char *command, int transform, int tofind);
static int RSection(const char *command, int transform, int tofind);
static int RSubSection(const char *command, int transform, int tofind);
static int RParagraph(const char *command, int transform, int tofind);
static int RCaption(const char *command, int transform, int tofind);
static int RQuotation(const char *command, int transform, int tofind);
static int RDescription(const char *command, int transform, int tofind);
static int RExample(const char *command, int transform, int tofind);
static int RDisplay(const char *command, int transform, int tofind);
static int RVerbatim(const char *command, int transform, int tofind);

static void RStyleConvert(char *p1, char *p2, char *p3, char *p4, struct StyleStackStruct *tmp);
static int Delimeter(char *units, char *n);

struct func_words
{
   const char *word;
   int (*fname)(const char *command, int transform, int tofind);
};

FP AssignFunc(const char *rtfword)
/*
 *
 *  Function that returns a pointer to the function
 *  associated with RTFWORD.
 *
 */
{
  static struct func_words fnlist[FUNCTION_SIZE] = {
      {"begin",		    RBegin},
      {"caption",	    RCaption},
      {"chapter",	    RChapter},
      {"delete",	    RDelete},
      {"description",	    RDescription},
      {"display",	    RDisplay},
      {"dsver",		    RDSVer},
      {"end",		    REnd},
      {"example",	    RExample},
      {"font",		    RFont},
      {"heading",	    RHeading},
      {"indent",	    RIndent},
      {"index",		    RIndex},
      {"left",		    RLeft},
      {"majorheading",	    RMajorHeading},
      {"nop",		    RNOP},
      {"paragraph",	    RParagraph},
      {"quotation",	    RQuotation},
      {"script",	    RScript},
      {"section",	    RSection},
      {"size",		    RSize},
      {"skip",		    RSkip},
      {"style",		    RStyleMain},
      {"subheading",	    RSubHeading},
      {"subsection",	    RSubSection},
      {"template",	    RTemplate},
      {"title",		    RTitle},
      {"verbatim",	    RVerbatim}
  };
  int i;

  for(i = 0; i < FUNCTION_SIZE; i++)
     if(!ULstrcmp(rtfword, fnlist[i].word))
       return(fnlist[i].fname);

  return(RNOP);
}

static int RDelete(const char *command, int transform, int tofind)
/*
 *
 *  Function that parses all of the text associated with COMMAND
 *  without processing it or writing it to the output file.
 *
 */
{
   ParseText(tofind, NORMAL, NOP);
   return CONTINUE;
}

  
static int RNOP(const char *command, int transform, int tofind)
/*
 *
 *  NOP function.
 *
 */
{
    return CONTINUE;
}

#if 0 /* unused */
static int RError(const char *command, int transform, int tofind)
/*
 *
 *  Notification of errors.
 *
 */
{
  fprintf(ferr, "* Unknown error!\n* %s: unknown error in input file.\n", me);
  return CONTINUE; /* presumably; was nothing - tjm */
}
#endif

static int RBegin(const char *command, int transform, int tofind)
/*
 *
 *  On encountering a \begindata in the ez document, set up
 *  an entry on the IdStack to keep track of the new recursion.
 *  Pass POP_JOB as tofind, which is an identifier that
 *  will be used by enddata to finish the recursion.
 *  Look up the data object to see if there is a function
 *  to handle it, if not, process the text normally.
 *
 */
{
   char type[TMP_SIZE];
   long int id = 0;
   struct IdStackStruct *tmp;
   int i;
   FP function = RNOP;
   static struct func_words typelist[TYPE_SIZE] = {
       {"text",			RText},
       {"note",			RAnnotation},
       {"fnote",		RFootnote},
       {"bp",			RNewpage},
       {"header",		RHeader},
       {"table",		RNOP/*RTable*/}
   };

   /* fscanf(fin, "%[^,],%ld}", type, &id); */
   input_read_up_to(type, TMP_SIZE, ',', fin);
   id = input_number(fin);
   input_match("}", fin);

   for(i = 0; i < TYPE_SIZE; i++)
      if(!ULstrcmp(type, typelist[i].word))
      {
         function = typelist[i].fname;
         break;
      }

   tmp = (struct IdStackStruct *) malloc(sizeof(struct IdStackStruct));
   tmp->idnum = id;
   tmp->next = IdStack;
   IdStack = tmp;
   if(function == RNOP)
      ParseText(POP_JOB, transform, PRINTTOFILE);
   else
      function(type, transform, POP_JOB);
   return CONTINUE;
}

static int REnd(const char *command, int transform, int tofind)
/*
 *
 *  On encountering an \enddata in the ez document, check to
 *  see that the id number correctly matches the id number
 *  of the corresponding \begindata, if so pass POP_JOB back
 *  to ParseText() to let it know that the recursion is finished.
 *  This procedure will be called with transform set to
 *  HEADER four times for each headertext data object, and
 *  the first two times, a tab needs to be inserted to keep
 *  consistency with rtf headers.
 *
 */
{
   char type[TMP_SIZE];
   long int id;
   static int headerflag = 0;

   if(transform == HEADER && !headerflag)
      headerflag = 4;

   /* fscanf(fin, "%[^,],%ld}", type, &id); */
   input_read_up_to(type, TMP_SIZE, ',', fin);
   id = input_number(fin);
   input_match("}", fin);

   if((IdStack != NULL) && (id == IdStack->idnum))
   {
      IdStack = IdStack->next;
      if(transform == HEADER)
         if(headerflag-- > 2)
            ungetc('\t', fin);
      return(POP_JOB);
   }
   else
   {
      printf("* Improper \\enddata encountered.  Id #: %ld", id);
   }
   return CONTINUE;
}

static int RDSVer(const char *command, int transform, int tofind)
/*
 *
 *  Check for correct TextDSVersion.
 *
 */
{
   int version;

   /* fscanf(fin, "%d}", &version); */
   version = input_number(fin);
   input_match("}", fin);
   if(version != TextDSVersion)
      fprintf(stderr, "* Error:\n* %s: Wrong TextDSVersion: %d.\n", me, version);
   return CONTINUE;
}

static int RText(const char *command, int transform, int tofind)
/*
 *
 *  Process text normally for a text data object.
 *
 */
{
   ParseText(tofind, transform, PRINTTOFILE);
   return CONTINUE;
}

static int RAnnotation(const char *command, int transform, int tofind)
/*
 *
 *  Process text for an annotation object by converting it
 *  into hidden text in rtf.
 *
 */
{
   /* fscanf(fin, "%d %d %d", &a, &b, &c); */
   /* a = */ input_number(fin);
   input_skip_whitespace(fin);
   /* b = */ input_number(fin);
   input_skip_whitespace(fin);
   /* c = */ input_number(fin);

   fputs("{\\v ", fout);
   ParseText(tofind, NORMAL, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RFootnote(const char *command, int transform, int tofind)
/*
 *
 *  Process footnote data object.
 *
 */
{
   fputs("{\\fs18\\up6 \\chftn {\\footnote \\pard\\plain\n", fout);
   fputs("\\s246 \\fs20 {\\fs18\\up6 \\chftn }", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}}\n", fout);
   return CONTINUE;
}

static int RSkip(const char *command, int transform, int tofind)
/*
 *
 *  Used for unknown commands, or commands that do nothing
 *  in rtf.  Processes the text associated with the command
 *  as if the command weren't there.
 *
 */
{
   ParseText(tofind, transform, PRINTTOFILE);
   return CONTINUE;
}

static int RTitle(const char *command, int transform, int tofind)
/*
 *
 *  Used for the title in an annotation.  This is scripted
 *  into "Author: nnnnnn.  " in rtf.
 *
 */
{
   fputs("Author: ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs(".  ", fout);
   return CONTINUE;
}

static int RNewpage(const char *command, int transform, int tofind)
/*
 *
 *  Handle page break data object.
 *
 */
{
   fputs("\\page ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   return CONTINUE;
}

static int RHeader(const char *command, int transform, int tofind)
/*
 *
 *  Handle Headertext data objects - ParseText() with transform
 *  as HEADER, so that enddata knows to insert tabs, and the
 *  ez $ header commands are treated properly.  Styles 243 and
 *  244 are rtf's default footer and header styles.
 *
 */
{
   char type[TMP_SIZE];
   int header = 0;

   /* fscanf(fin, "\nwhere:%s\n", type); */
   input_skip_whitespace(fin);
   input_match("where:", fin);
   input_read_up_to_whitespace(type, TMP_SIZE, fin);
   input_skip_whitespace(fin);

   if(strcmp(type, "footer"))
      header = 1;

   /* fscanf(fin, "active:%d\n", &active); */
   input_match("active:", fin);
   /* active = */ input_number(fin);
   input_skip_whitespace(fin);

   CurrLine += 3;

   fprintf(fout, "{\\%s \\pard", header ? "header" : "footer");
   fprintf(fout, "\\plain \\s%d", header ? 244 : 243);
   fprintf(fout, "\\tqc\\tx4320\\tqr\\tx8640 ");
   ParseText(tofind, HEADER, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RSize(const char *command, int transform, int tofind)
/*
 *
 *  Handle changes in font sizes.  Increase or decrease
 *  the fontsize, parse the text, then reset the fontsize
 *  to its value before the call.
 *
 */
{
   long oldfont;

   oldfont = State.CurFontSize;

   if(strcmp(command, "smaller"))
       State.CurFontSize += 4;
   else
       State.CurFontSize -= 4;

   fprintf(fout, "{\\fs%ld ", State.CurFontSize);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}", fout);

   State.CurFontSize = oldfont;
   return CONTINUE;
}

static int RScript(const char *command, int transform, int tofind)
/*
 *
 *  Handle changes in script location.  Increase of decrease
 *  the script location, parse the text, then reset the
 *  script location to its value before the call.
 *
 */
{
   int up = 0;
   long oldpos;

   oldpos = State.CurScriptMovement;

   if(strcmp(command, "subscript"))
       up = 1;
   if(up)
       State.CurScriptMovement += 6;
   else
       State.CurScriptMovement -= 6;

   fprintf(fout, "{\\%s%ld ", up ? "up" : "dn", State.CurScriptMovement);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}", fout);

   State.CurScriptMovement = oldpos;
   return CONTINUE;
}

static int RFont(const char *command, int transform, int tofind)
/*
 *
 *  Handle changes in fonts.  New York is the default
 *  font, and Chicago, Monaco, and Symbol were chosen
 *  for their universal availability on the Mac.
 *
 */
{
   static struct FontStruct fonts[FONT_SIZE] = {
       {"sansserif",	CHICAGO},
       {"typewriter",	MONACO},
       {"symbol",	SYMBOL}
   };
   int i;
   int n = NEW_YORK;

   State.CurFontFamily = "andy";
   for(i=0; i<FONT_SIZE; i++)
      if(!strcmp(command, fonts[i].name))
      {
         n = fonts[i].num;
	 State.CurFontFamily = fonts[i].name;
         break;
      }

   fprintf(fout, "{\\f%d ", n);
   ParseText(tofind, transform, PRINTTOFILE);
   fprintf(fout, "}");

   return CONTINUE;
}

static int RTemplate(const char *command, int transform, int tofind)
/*
 *
 *  Currently checks if the template is default.  If
 *  not, it prints an error and continues.  Ultimately
 *  it should send the template file through the style
 *  sheet interpreter.
 *
 */
{
   char temp[TMP_SIZE];

   /* fscanf(fin, "%[^}]}", temp); */
   input_read_up_to(temp, TMP_SIZE, '}', fin);
   if(strcmp(temp, "default"))
       printf("Warning, template other than default: %s.\n", temp);
   fprintf(fout, "{\\v TEMPLATE: %s}", temp);
   return CONTINUE;
}

static int RLeft(const char *command, int transform, int tofind)
/*
 *
 *  Handle changes in the left margin.
 *
 *  RTF imposes a paragraph boundry on indent styles.
 *  EZ's redisplay acts wrong when an indent occurs in
 *  the middle of a paragraph:  the left and right indent
 *  displayed conform to the indent in place at the
 *  beginning of the line.  Converting from EZ to RTF
 *  will cut a paragraph in two if an indent is discovered
 *  in the middle of a paragraph.  This is the right thing!
 *
 */
{
   long oldmar;
   char ch;

   oldmar = State.CurLeftIndentation;

   State.CurLeftIndentation += 720;
   if(!paragraph)
       fputs("\\par", fout);
   fprintf(fout, "\\li%ld ", State.CurLeftIndentation);
   ParseText(tofind, transform, PRINTTOFILE);

   ch = (char) fgetc(fin);
   if(ch == '\n' || ch == '\r')
   {
       CurrLine++;
       ch = (char) fgetc(fin);
       if(ch == '\n' || ch == '\r')
	   Newlines();
       else
	   ungetc(ch, fin);
   }
   else
       ungetc(ch, fin);
   fprintf(fout, "\\par\\li%ld ", oldmar);

   State.CurLeftIndentation = oldmar;
   return CONTINUE;
}

static int RIndent(const char *command, int transform, int tofind)
/*
 *
 *  Handle calls to indent by checking with the left
 *  and right indentations and changing them accordingly.
 *
 *  RTF imposes a paragraph boundry on indent styles.
 *  EZ's redisplay acts wrong when an indent occurs in
 *  the middle of a paragraph:  the left and right indent
 *  displayed conform to the indent in place at the
 *  beginning of the line.  Converting from EZ to RTF
 *  will cut a paragraph in two if an indent is discovered
 *  in the middle of a paragraph.  This is the right thing!
 *
 */
{
   long left, right;
   char ch;

   left = State.CurLeftIndentation;
   right = State.CurRightIndentation;

   State.CurLeftIndentation += 720;
   State.CurRightIndentation += 720;

   if(!paragraph)
      fputs("\\par", fout);
   fprintf(fout, "\\li%ld\\ri%ld ", State.CurLeftIndentation, State.CurRightIndentation);

   ParseText(tofind, transform, PRINTTOFILE);

   ch = (char) fgetc(fin);
   if(ch == '\n' || ch == '\r')
   {
       CurrLine++;
       ch = (char) fgetc(fin);
       if(ch == '\n' || ch == '\r')
	   Newlines();
       else
	   ungetc(ch, fin);
   }
   else
       ungetc(ch, fin);
   fprintf(fout, "\\par\\li%ld\\ri%ld ", left, right);

   State.CurLeftIndentation = left;
   State.CurRightIndentation = right;

   return CONTINUE;
}

static int RIndex(const char *command, int transform, int tofind)
/*
 *
 *  Handle visible and invisible index entries.  For
 *  invisible entries, the transform is set to INDEX
 *  so that "++" seperator gets properly parsed into
 *  "\:".
 *
 */
{
   fputs("{\\v {\\xe\\pard\\plain ", fout);
   if(!strcmp(command, "indexi"))
   {
      fputs("{\\v ", fout);
      ParseText(tofind, INDEX, PRINTTOFILE);
      fputs("}}}", fout);
   }
   else
   {
      ParseText(tofind, NORMAL, PRINTTOFILE);
      fputs("}}", fout);
   }
   return CONTINUE;
}

static int RStyleDefine(char *style_sheet)
/*
 *
 *  Parse through the attributes of a style, and use
 *  RStyleConvert() to actually convert each attribute.
 *  Set up a new entry on the StyleStack that contains
 *  the styleid and its associated expansion string.
 *
 */
{
   char name[TMP_SIZE], type[TMP_SIZE],
        p1[TMP_SIZE], p2[TMP_SIZE], p3[TMP_SIZE], p4[TMP_SIZE],
        ch;
   int i, n;
   struct StyleStackStruct *tmp;
   static int styleid = 0;

   tmp = (struct StyleStackStruct *) malloc(sizeof(struct StyleStackStruct));

   fprintf(fout, "{\\s%d ", ++styleid);
   /* fscanf(fin, "%s", name); */
   input_read_up_to_whitespace(name, TMP_SIZE, fin);
   sprintf(style_sheet, "%s%s\n", style_sheet, name);

   tmp->idnum = styleid;
   strcpy(tmp->name, name);
   for(n=0; n<STATES; n++)
   {
      tmp->state_attribute[n] = style_None;
      tmp->modify[n] = 0;
   }
   tmp->state_count = 0;
   tmp->next = Style;
   tmp->string[0] = '\0';
   Style = tmp;

   while(1)
   {
      AbsorbNewlines();
      /* fscanf(fin, "%[^:}]:", type); */
      input_read_up_to_one_of(type, TMP_SIZE, ":}", fin);
      input_match(":", fin);

      if(!strcmp(type, "menu"))
      {
	 sprintf(style_sheet, "%smenu:", style_sheet);
         ch = ' ';
         while(ch != ']')
         {
            ch = (char) fgetc(fin);
            sprintf(style_sheet, "%s%c", style_sheet, ch);
	 }
         sprintf(style_sheet, "%s\n", style_sheet);
      }
      else if(!strcmp(type, "attr"))
      {
	  sprintf(style_sheet, "%sattr:", style_sheet);
          /* fscanf(fin, "[%s %s %s ", p1, p2, p3); */
	  input_match("[", fin);
	  input_read_up_to_whitespace(p1, TMP_SIZE, fin);
	  input_skip_whitespace(fin);
	  input_read_up_to_whitespace(p2, TMP_SIZE, fin);
	  input_skip_whitespace(fin);
	  input_read_up_to_whitespace(p3, TMP_SIZE, fin);
	  input_skip_whitespace(fin);

          i = 0;
          while((ch = (char) fgetc(fin)) != ']')
             p4[i++] = ch;
          p4[i] = '\0';

          sprintf(style_sheet, "%s[%s %s %s %s]\n", style_sheet, p1, p2, p3, p4);

          /* handle actual conversion to rtf... */
          RStyleConvert(p1, p2, p3, p4, Style);
      }
      else
      {
          ch = ' ';
	  while(ch != ']' && ch != '}')
             ch = (char) fgetc(fin);
	  if(ch == '}')
	     ungetc(ch, fin);
      }

      AbsorbNewlines();
      ch = (char) fgetc(fin);
      if(ch != '}')
         ungetc(ch, fin);
      else
      {
	 sprintf(style_sheet, "%s)\n", style_sheet);
         break;
      }
   }

   fprintf(fout, "\\sbasedon0\\snext%d %s;}", styleid, name);
   return CONTINUE;
}

static int RStyleMain(const char *command, int transform, int tofind)
/*
 *
 *  Should only be called for the first occurence of
 *  \define in the ez document.  Standard styles for
 *  footers, headers, footnote reference, and footnote
 *  text are hard coded into the style sheet (as well
 *  as the Normal style).  As long as "\define"'s are
 *  encountered, RStyleDefine is called to handle them.
 *
 */
{
   char style_sheet[10000];
   char temp[TMP_SIZE], ch;
   int i;
   static int done = 0;

   if(done)
   {
      RDelete(command, transform, tofind);
      return CONTINUE; /* presumably; was nothing - tjm */
   }
   else
      done = 1;

   fputs("{\\stylesheet", fout);
   fputs("{\\s243\\tqc\\tx4320\\tqr\\tx8640 \\sbasedon0\\snext243 footer;}", fout);
   fputs("{\\s244\\tqc\\tx4320\\tqr\\tx8640 \\sbasedon0\\snext244 header;}\n", fout);
   fputs("{\\s245 \\fs18\\up6 \\sbasedon0\\snext0 footnote reference;}", fout);
   fputs("{\\s246 \\fs20 \\sbasedon0\\snext246 footnote text;}\n", fout);
   fputs("{\\sbasedon222\\snext0 Normal;}", fout);

   sprintf(style_sheet, "%s", "|define(");
   RStyleDefine(style_sheet);
   while(1)
   {
      ch = ' ';
      while(isspace(ch))
      {
         AbsorbNewlines();
         ch = (char) fgetc(fin);
      }
      if(ch != '\\')
      {
         ungetc(ch, fin);
         break;
      }

      ch = (char) fgetc(fin);
      if(ch == '\\' || ch == '{' || ch == '}')
      {
          ungetc(ch, fin);
          ungetc('\\', fin);
          break;
      }

      i = 0;
      while(1)
      {
         if(ch == '{' || isspace(ch))
            break;
         temp[i++] = ch;
         ch = (char) fgetc(fin);
      }
      temp[i] = '\0';

      if(!strcmp(temp, "define"))
      {
	 sprintf(style_sheet, "%s|%s(", style_sheet, temp);
         RStyleDefine(style_sheet);
      }
      else
      {
         ungetc(ch, fin);
         for(i--; i>=0; i--)
            ungetc(temp[i], fin);
         ungetc('\\', fin);
         break;
      }
   }

   fputs("}\n", fout);
   fprintf(fout, "{\\v STYLESHEET:\n%s}", style_sheet);
   return CONTINUE;
}

void RStyleApply(struct StyleStackStruct *tmp)
/*
 *
 *  Called when a style is encountered inside the document,
 *  so that ez style name can be replaced by style number
 *  and expansion of style attributes.
 *
 */
{
   char ch;
   int k, space;
   int done = 0;
   int do_new = 0;
   long save[STATES];
   long save2[STATES];

/*   fprintf(fout, "{\\s%d%s ", tmp->idnum, tmp->string);*/
   fprintf(fout, "{{\\v STYLE: %s}%s", tmp->name, tmp->string);

   space = 0;
   for(k=0; k<=tmp->state_count; k++)
   {
       switch(tmp->state_attribute[k])
       {
	   case style_FontSizeAttr:
	       save[k] = State.CurFontSize;
	       State.CurFontSize += tmp->modify[k];
	       fprintf(fout, "\\fs%ld", State.CurFontSize);
	       space = 1;
	       break;
	   case style_ScriptAttr:
	       save[k] = State.CurScriptMovement;
	       State.CurScriptMovement -= tmp->modify[k];
	       if(State.CurScriptMovement > 0)
		  fprintf(fout, "\\up%ld", State.CurScriptMovement);
	       else
		  fprintf(fout, "\\dn%ld", - State.CurScriptMovement);
	       space = 1;
	       break;
	   case style_LeftMarginAttr:
	       save[k] = State.CurLeftMargin;
               State.CurLeftMargin += tmp->modify[k];
	       if(!paragraph)
               {
		   fputs("\\par", fout);
                   paragraph = 1;
	       }
/*               fprintf(fout, "\\li%d", State.CurLeftMargin);*/
	       space = 1;
               break;
	   case style_RightMarginAttr:
	       save[k] = State.CurRightMargin;
               State.CurRightMargin += tmp->modify[k];
	       if(!paragraph)
               {
		   fputs("\\par", fout);
                   paragraph = 1;
	       }
/*               fprintf(fout, "\\ri%d", State.CurRightMargin);*/
	       space = 1;
               break;
	   case style_IndentAttr:
               save[k] = State.CurLeftIndentation;
               save2[k] = State.CurRightIndentation;
               State.CurLeftIndentation += tmp->modify[k];
               State.CurRightIndentation += tmp->modify[k];
	       if(!paragraph)
		   fputs("\\par", fout);
/*               fprintf(fout, "\\li%d\\ri%d", State.CurLeftIndentation, State.CurRightIndentation);*/
	       space = 1;
               break;
	   case style_None:
	   default:
	       break;
       }
   }
   if(space)
      fputs(" ", fout);

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
   fputs("}", fout);

   for(k=0; k<=tmp->state_count; k++)
   {
       switch(tmp->state_attribute[k])
       {
	   case style_FontSizeAttr:
	       State.CurFontSize = save[k];
	       break;
	   case style_ScriptAttr:
	       State.CurScriptMovement = save[k];
	       break;
	   case style_LeftMarginAttr:
               fprintf(fout, "\\li%ld", State.CurLeftMargin);
               State.CurLeftMargin = save[k];
               if(!done)
               {
		   ch = (char) fgetc(fin);
		   if(ch == '\n' || ch == '\r')
		   {
		       CurrLine++;
		       ch = (char) fgetc(fin);
		       if(ch == '\n' || ch == '\r')
			   do_new = 1;
		       else
			   ungetc(ch, fin);
		   }
		   else
		       ungetc(ch, fin);
		   done = 1;
	       }
               break;
	   case style_RightMarginAttr:
               fprintf(fout, "\\ri%ld", State.CurRightMargin);
               State.CurRightMargin = save[k];
               if(!done)
               {
		   ch = (char) fgetc(fin);
		   if(ch == '\n' || ch == '\r')
		   {
		       CurrLine++;
		       ch = (char) fgetc(fin);
		       if(ch == '\n' || ch == '\r')
			   do_new = 1;
		       else
			   ungetc(ch, fin);
		   }
		   else
		       ungetc(ch, fin);
		   done = 1;
	       }
               break;
	   case style_IndentAttr:
               fprintf(fout, "\\li%ld\\ri%ld", State.CurLeftIndentation, State.CurRightIndentation);
               State.CurLeftIndentation = save[k];
	       State.CurRightIndentation = save2[k];
	       ch = (char) fgetc(fin);
	       if(ch == '\n' || ch == '\r')
	       {
		   CurrLine++;
		   ch = (char) fgetc(fin);
		   if(ch == '\n' || ch == '\r')
		       Newlines();
		   else
		       ungetc(ch, fin);
	       }
	       else
		   ungetc(ch, fin);
	       fprintf(fout, "\\par\\li%ld\\ri%ld ", State.CurLeftIndentation, State.CurRightIndentation);
               break;
	   case style_None:
	   default:
	       break;
       }
   }
   if(do_new)
      Newlines();
   if(done)
       fprintf(fout, "\\par\\pard\\li%ld\\ri%ld ", State.CurLeftMargin, State.CurRightMargin);
}

static void RSLMargin(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSRMargin(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSTMargin(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSBMargin(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSIndent(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSIpSpacing(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSAbove(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSBelow(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSIlSpacing(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSFontFamily(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSFontSize(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSFontScript(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSTabChange(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSFontFace(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
static void RSJustify(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);

struct sfunc_words
{
   const char *word;
   void (*fname)(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp);
};

static void RStyleConvert(char *p1, char *p2, char *p3, char *p4, struct StyleStackStruct *tmp)
/*
 *
 *  Call the function that handles the particular type
 *  of style attribute.
 *
 */
{
   static struct sfunc_words Attribs[15] = {
      {"LeftMargin",		RSLMargin},
      {"RightMargin",		RSRMargin},
      {"TopMargin",		RSTMargin},
      {"BottomMargin",		RSBMargin},
      {"Indentation",		RSIndent},
      {"InterparagraphSpacing",	RSIpSpacing},
      {"Above",			RSAbove},
      {"Below",			RSBelow},
      {"InterlineSpacing",	RSIlSpacing},
      {"FontFamily",		RSFontFamily},
      {"FontSize",		RSFontSize},
      {"Script",		RSFontScript},
      {"TabChange",		RSTabChange},
      {"FontFace",		RSFontFace},
      {"Justification",		RSJustify}
   };
   int i;

   for(i=0; i<15; i++)
      if(!strcmp(p1, Attribs[i].word))
      {
         Attribs[i].fname(p2, p3, p4, tmp);
         break;
      }
}

static int Delimeter(char *units, char *n)
/*
 *
 *  Convert the delimeter from whatever it is into twips
 *  for use by rtf.
 *
 *  (A twip is 1/20th of a point)
 *
 */
{
   struct divider
   {
       const char *name;
       float div;
   };
   static struct divider types[UNITS] = {
      {"inch", 45.5},
      {"cm", 114.0},
      {"point", 0.05},
      {"rawdot", 0.05}
   };
   int i, numdel = UNBOUND;

   strcpy(units, makelower(units));

   for(i=0; i<UNITS; i++)
      if(!strcmp(units, types[i].name))
      {
         numdel = ((float) atoi(n)) / types[i].div;
         break;
      }

   if(numdel == UNBOUND)
      fprintf(ferr, "*Error:  Unknown unit of measurement, %s.\n", units);

   return(numdel);
}

static void RSLMargin(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   ConstantMargin
 *   LeftMargin
 *   LeftEdge
 *   RightMargin
 *   RightEdge
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

   fprintf(fout, "\\li%d ", numdel);
   if(!strcmp(basis, "ConstantMargin"))
      sprintf(tmp->string, "%s\\li%d ", tmp->string, numdel);
   else if(!strcmp(basis, "LeftMargin"))
   {
       tmp->modify[tmp->state_count] = numdel;
       tmp->state_attribute[tmp->state_count] = style_LeftMarginAttr;
       tmp->state_count++;
   }
}

static void RSRMargin(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   ConstantMargin
 *   LeftMargin
 *   LeftEdge
 *   RightMargin
 *   RightEdge
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

   fprintf(fout, "\\ri%d ", numdel);
   if(!strcmp(basis, "ConstantMargin"))
      sprintf(tmp->string, "%s\\ri%d ", tmp->string, numdel);
   else if(!strcmp(basis, "RightMargin"))
   {
       tmp->modify[tmp->state_count] = numdel;
       tmp->state_attribute[tmp->state_count] = style_RightMarginAttr;
       tmp->state_count++;
   }
}

static void RSTMargin(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   ConstantMargin
 *   TopMargin
 *   TopEdge
 *   BottomMargin
 *   BottomEdge
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

}

static void RSBMargin(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   ConstantMargin
 *   TopMargin
 *   TopEdge
 *   BottomMargin
 *   BottomEdge
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

}

static void RSIndent(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   ConstantMargin
 *   PreviousIndentation
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

   if(!strcmp(basis, "ConstantMargin"))
   {
       fprintf(fout, "\\li%d\\ri%d ", numdel, numdel);
       sprintf(tmp->string, "%s\\li%d\\ri%d ", tmp->string, numdel, numdel);
   }
   else if(!strcmp(basis, "PreviousIndentation"))
   {
       tmp->modify[tmp->state_count] = numdel;
       tmp->state_attribute[tmp->state_count] = style_IndentAttr;
       tmp->state_count++;
   }
}

static void RSIpSpacing(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

}

static void RSAbove(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   ConstantMargin
 *   AboveSpacing
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

   fprintf(fout, "\\sb%d ", numdel);
   sprintf(tmp->string, "%s\\sb%d ", tmp->string, numdel);
}

static void RSBelow(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   ConstantMargin
 *   BelowSpacing
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

   fprintf(fout, "\\sa%d ", numdel);
   sprintf(tmp->string, "%s\\sa%d ", tmp->string, numdel);
}

static void RSIlSpacing(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   ConstantMargin
 *   InterlineSpacing
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

   fprintf(fout, "\\sl%d ", numdel);
   sprintf(tmp->string, "%s\\sl%d ", tmp->string, numdel);
}

static void RSFontFamily(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
{
   static struct FontStruct type[FTSIZE] = {
       {"Andy",		    NEW_YORK},
       {"AndySans",	    CHICAGO},
       {"AndyType",	    MONACO},
       {"AndySymbol",	    SYMBOL},
       {"Default",	    NEW_YORK}
   };
   int i;

   for(i=0; i<FTSIZE; i++)
      if(!strcmp(basis, type[i].name))
         break;

   if(i<FTSIZE)
   {
      fprintf(fout, "\\f%d ", type[i].num);
      sprintf(tmp->string, "%s\\f%d ", tmp->string, type[i].num);
   }
}

static void RSFontSize(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   PreviousFontSize
 *   ConstantFontSize
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

   numdel /= 10;
   fprintf(fout, "\\fs%d ", numdel);
   if(!strcmp(basis, "ConstantFontSize"))
      sprintf(tmp->string, "%s\\fs%d", tmp->string, numdel);
   else if(!strcmp(basis, "PreviousFontSize"))
   {
      tmp->modify[tmp->state_count] = numdel;
      tmp->state_attribute[tmp->state_count] = style_FontSizeAttr;
      tmp->state_count++;
   }
}

static void RSFontScript(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
/*
 *
 *   PreviousScriptMovement
 *   ConstantScriptMovement
 *
 */
{
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

   numdel /= 10;
   if(!strcmp(basis, "ConstantScriptMovement"))
   {
      if(numdel<0)
      {
         fprintf(fout, "\\up%d ", - numdel);
         sprintf(tmp->string, "%s\\up%d ", tmp->string, - numdel);
      }
      else
      {
         fprintf(fout, "\\dn%d ", numdel);
         sprintf(tmp->string, "%s\\dn%d ", tmp->string, numdel);
      }
   }
   else if(!strcmp(basis, "PreviousScriptMovement"))
   {
      fprintf(fout, "\\dn%d ", numdel);
      tmp->state_attribute[tmp->state_count] = style_ScriptAttr;
      tmp->modify[tmp->state_count] = numdel;
      tmp->state_count++;
   }
}

static void RSTabChange(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
{
   static struct style_words type[TSIZE] = {
       {"LeftAligned",		"tx"},
       {"RightAligned",		"tqr\\tx"},
       {"CenteredOnTab",	"tqc\\tx"},
       {"CenteredBetweenTab",	"tqc\\tx"},
       {"TabDivide",		"tb"}
   };
   int i;
   int numdel;

   if((numdel = Delimeter(unit, operand)) == UNBOUND)
      return;

   for(i=0; i<TSIZE; i++)
      if(!strcmp(basis, type[i].ezword))
         break;

   if(i<TSIZE)
   {
      fprintf(fout, "\\%s%d ", type[i].rtfword, numdel);
      sprintf(tmp->string, "%s\\%s%d ", tmp->string, type[i].rtfword, numdel);
   }
}

static void RSFontFace(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
{
   static struct style_words type[FSIZE] = {
       {"Plain",	"plain"},
       {"Bold",		"b"},
       {"Italic",	"i"}
   };
   int i;

   for(i=0; i<FSIZE; i++)
      if(!strcmp(basis, type[i].ezword))
         break;

   if(i<FSIZE)
   {
      fprintf(fout, "\\%s ", type[i].rtfword);
      sprintf(tmp->string, "%s\\%s ", tmp->string, type[i].rtfword);
   }
}

static void RSJustify(char *basis, char *unit, char *operand, struct StyleStackStruct *tmp)
{
   static struct style_words type[JSIZE] = {
       {"LeftJustified",		"ql"},
       {"RightJustified",		"qr"},
       {"LeftAndRightJustified",	"qj"},
       {"LeftThenRightJustified",	"qj"},
       {"Centered",			"qc"}
   };
   int i;

   for(i=0; i<JSIZE; i++)
      if(!strcmp(basis, type[i].ezword))
         break;

   if(i<JSIZE)
   {
      fprintf(fout, "\\%s ", type[i].rtfword);
      sprintf(tmp->string, "%s\\%s ", tmp->string, type[i].rtfword);
   }
}

static int RMajorHeading(const char *command, int transform, int tofind)
/*
 *
 *  Handle MajorHeading style.
 *
 */
{
   int oldsize;

   oldsize = State.CurFontSize;
   State.CurFontSize += 8;

   fprintf(fout, "{\\fs%ld\\qc ", State.CurFontSize);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);

   State.CurFontSize = oldsize;
   return CONTINUE;
}

static int RHeading(const char *command, int transform, int tofind)
/*
 *
 *  Handle Heading style.
 *
 */
{
   fputs("{\\b\\ql\\fi-350 ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RSubHeading(const char *command, int transform, int tofind)
/*
 *
 *  Handle SubHeading style.
 *
 */
{
   fputs("{\\b\\ql ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RChapter(const char *command, int transform, int tofind)
/*
 *
 *  Handle Chapter style.
 *
 */
{
   int oldsize;

   oldsize = State.CurFontSize;
   State.CurFontSize += 8;

   fprintf(fout, "{\\fs%ld\\b\\ql ", State.CurFontSize);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);

   State.CurFontSize = oldsize;
   return CONTINUE;
}

static int RSection(const char *command, int transform, int tofind)
/*
 *
 *  Handle Section style.
 *
 */
{
   int oldsize;

   oldsize = State.CurFontSize;
   State.CurFontSize += 4;

   fprintf(fout, "{{\\v STYLE: section}\\fs%ld\\b\\ql ", State.CurFontSize);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);

   State.CurFontSize = oldsize;
   return CONTINUE;
}

static int RSubSection(const char *command, int transform, int tofind)
/*
 *
 *  Handle SubSection style.
 *
 */
{
   fputs("{\\b\\ql ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RParagraph(const char *command, int transform, int tofind)
/*
 *
 *  Handle Paragraph style.
 *
 */
{
   fputs("{\\i\\ql ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RCaption(const char *command, int transform, int tofind)
/*
 *
 *  Handle Caption style.
 *
 */
{
   fputs("{\\b\\qc ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RQuotation(const char *command, int transform, int tofind)
/*
 *
 *  Handle Quotation style.
 *
 */
{
   fputs("{\\i\\li720\\ri720 ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RDescription(const char *command, int transform, int tofind)
/*
 *
 *  Handle Description style.
 *
 */
{
   fputs("{\\fi-720\\li720 ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RExample(const char *command, int transform, int tofind)
/*
 *
 *  Handle Example style.
 *
 */
{
   fprintf(fout, "{\\ql\\li720\\f%d ", MONACO);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RDisplay(const char *command, int transform, int tofind)
/*
 *
 *  Handle Display style.
 *
 */
{
   fputs("{\\ql\\li720\\ri720 ", fout);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}

static int RVerbatim(const char *command, int transform, int tofind)
/*
 *
 *  Handle Verbatim style.
 *
 */
{
   fprintf(fout, "{\\ql\\f%d ", MONACO);
   ParseText(tofind, transform, PRINTTOFILE);
   fputs("}\n", fout);
   return CONTINUE;
}
