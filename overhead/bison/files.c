/* Open and close files for bison,
   Copyright (C) 1984, 1986, 1989, 1992 Free Software Foundation, Inc.
   Modified (1992) from bison-1.19 by 
		Wilfred J. Hansen (wjh+@cmu.edu) 
		Andrew Consortium, Carnegie Mellon University

This file is part of Bison, the GNU Compiler Compiler.

Bison is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Bison is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bison; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


#if defined (VMS) & !defined (__VMS_POSIX)
#include <ssdef.h>
#define unlink delete
#ifndef XPFILE
#define XPFILE "GNU_BISON:[000000]BISON.SIMPLE"
#endif
#ifndef XPFILE1
#define XPFILE1 "GNU_BISON:[000000]BISON.HAIRY"
#endif
#endif

#include <stdio.h>
#include "andrewos.h"
#include "util.h"
#include "files.h"
#include "new.h"
#include "gram.h"
#include "proto.h"

FILE *finput = NULL;
FILE *foutput = NULL;
FILE *fdefines = NULL;
FILE *ftable = NULL;
FILE *fattrs = NULL;
FILE *fguard = NULL;
FILE *faction = NULL;
FILE *fparser = NULL;

/* File name specified with -o for the output file, or 0 if no -o.  */
char *spec_outfile;

char *infile;
char *outfile;
char *defsfile;
char *tabfile;
char *attrsfile;
char *guardfile;
char *actfile;

static FILE *tryopen(const char *name, const char *mode);
static FILE *tryopen_tmp();

int fixed_outfiles = 0;


static char*
stringappend(const char *string1, int end1, const char *string2)
{
  char *ostring;
  int i = strlen(string2);

  ostring = NEW2(i+end1+1, char);

  memcpy(ostring, string1, end1);
  strcpy(ostring + end1, string2);

  return ostring;
}


/* JF this has been hacked to death.  Nowaday it sets up the file names for
   the output files, and opens the tmp files and the parser */
void
openfiles(void)
{
  const char *name_base;
#ifdef MSDOS
  char *cp;
#endif
  char *filename;
  int base_length;
  int short_base_length;

#ifdef MSDOS
  strlwr (infile);
#endif /* MSDOS */

  if (spec_outfile)
    {
      /* -o was specified.  The precise -o name will be used for ftable.
	 For other output files, remove the ".c" or ".tab.c" suffix.  */
      name_base = spec_outfile;
#ifdef MSDOS
      strlwr (name_base);
#endif /* MSDOS */
      /* BASE_LENGTH includes ".tab" but not ".c".  */
      base_length = strlen (name_base);
      if (!strcmp (name_base + base_length - 2, ".c"))
	base_length -= 2;
      /* SHORT_BASE_LENGTH includes neither ".tab" nor ".c".  */
      short_base_length = base_length;
      if (!strncmp (name_base + short_base_length - 4, ".tab", 4))
	short_base_length -= 4;
      else if (!strncmp (name_base + short_base_length - 4, "_tab", 4))
	short_base_length -= 4;
    }
  else if (spec_file_prefix)
    {
      /* -b was specified.  Construct names from it.  */
      /* SHORT_BASE_LENGTH includes neither ".tab" nor ".c".  */
      short_base_length = strlen (spec_file_prefix);
      /* Count room for `.tab'.  */
      base_length = short_base_length + 4;
      name_base = (char *) xmalloc (base_length + 1);
      /* Append `.tab'.  */
      strcpy ((char *)name_base, spec_file_prefix);
#ifdef VMS
      strcat ((char *)name_base, "_tab");
#else
      strcat ((char *)name_base, ".tab");
#endif
#ifdef MSDOS
      strlwr ((char *)name_base);
#endif /* MSDOS */
    }
  else
    {
      /* -o was not specified; compute output file name from input
	 or use y.tab.c, etc., if -y was specified.  */

      name_base = fixed_outfiles ? "y.y" : infile;

      /* BASE_LENGTH gets length of NAME_BASE, sans ".y" suffix if any.  */

      base_length = strlen (name_base);
      if (!strcmp (name_base + base_length - 2, ".y"))
	base_length -= 2;
      short_base_length = base_length;

#ifdef VMS
      name_base = stringappend(name_base, short_base_length, "_tab");
#else
#ifdef MSDOS
      name_base = stringappend(name_base, short_base_length, "_tab");
#else
      name_base = stringappend(name_base, short_base_length, ".tab");
#endif /* not MSDOS */
#endif
      base_length = short_base_length + 4;
    }

  finput = tryopen(infile, "r");

  if (! noparserflag) 
    {
      filename = getenv("BISON_SIMPLE");
#ifdef MSDOS
      /* File doesn't exist in current directory; try in INIT directory.  */
      cp = getenv("INIT");
      if (filename == 0 && cp != NULL)
        {
          filename = xmalloc(strlen(cp) + strlen(PFILE) + 2);
          strcpy(filename, cp);
          cp = filename + strlen(filename);
          *cp++ = '/';
          strcpy(cp, PFILE);
        }
#endif /* MSDOS */
      fparser = tryopen(filename ? filename : PFILE, "r");
    }

  if (verboseflag)
    {
#ifdef MSDOS
      outfile = stringappend(name_base, short_base_length, ".out");
#else
      /* We used to use just .out if spec_name_prefix (-p) was used,
	 but that conflicts with Posix.  */
      outfile = stringappend(name_base, short_base_length, ".output");
#endif
      foutput = tryopen(outfile, "w");
    }

  if (noparserflag)
    {
      /* use permanent name for actions file */
      actfile = stringappend(name_base, short_base_length, ".act");
      faction = tryopen(actfile, "w");
    } 
  else
    faction = tryopen_tmp();
  fattrs = tryopen_tmp();
  ftable = tryopen_tmp();

  if (definesflag)
    {
      defsfile = stringappend(name_base, base_length, ".h");
      fdefines = tryopen_tmp();
    }

	/* These are opened by `done' or `open_extra_files', if at all */
  if (spec_outfile)
    tabfile = spec_outfile;
  else
    tabfile = stringappend(name_base, base_length, ".c");

#ifdef VMS
  attrsfile = stringappend(name_base, short_base_length, "_stype.h");
  guardfile = stringappend(name_base, short_base_length, "_guard.c");
#else
#ifdef MSDOS
  attrsfile = stringappend(name_base, short_base_length, ".sth");
  guardfile = stringappend(name_base, short_base_length, ".guc");
#else
  attrsfile = stringappend(name_base, short_base_length, ".stype.h");
  guardfile = stringappend(name_base, short_base_length, ".guard.c");
#endif /* not MSDOS */
#endif /* not VMS */
}



/* open the output files needed only for the semantic parser.
This is done when %semantic_parser is seen in the declarations section.  */

void
open_extra_files(void)
{
  FILE *ftmp;
  int c;
  char *filename;
#ifdef MSDOS
  char *cp;
#endif

  if (fparser)
    fclose(fparser);

  if (! noparserflag) 
    {
      filename = (char *) getenv ("BISON_HAIRY");
#ifdef MSDOS
      /* File doesn't exist in current directory; try in INIT directory.  */
      cp = getenv("INIT");
      if (filename == 0 && cp != NULL)
        {
          filename = xmalloc(strlen(cp) + strlen(PFILE1) + 2);
          strcpy(filename, cp);
          cp = filename + strlen(filename);
          *cp++ = '/';
          strcpy(cp, PFILE1);
        }
#endif
      fparser= tryopen(filename ? filename : PFILE1, "r");
    }

		/* JF change from inline attrs file to separate one */
  ftmp = tryopen(attrsfile, "w");
  rewind(fattrs);
  while((c=getc(fattrs))!=EOF)	/* Thank god for buffering */
    putc(c,ftmp);
  fclose(fattrs);
  fattrs=ftmp;

  fguard = tryopen(guardfile, "w");

}

	/* JF to make file opening easier.  This func tries to open file
	   NAME with mode MODE, and prints an error message if it fails. */
static FILE *
tryopen(const char *name, const char *mode)
{
  FILE	*ptr;

  ptr = fopen(name, mode);
  if (ptr == NULL)
    {
      fprintf(stderr, "%s: ", program_name);
      perror(name);
      done(2);
    }
  return ptr;
}

static FILE *
tryopen_tmp(void)
{
  FILE	*ptr;

  ptr = tmpfile();
  if (ptr == NULL)
    {
      perror(program_name);
      done(2);
    }
  return ptr;
}

void
done(int k)
{
  if (faction)
    fclose(faction);

  if (fattrs)
    fclose(fattrs);

  if (fguard)
    fclose(fguard);

  if (finput)
    fclose(finput);

  if (fparser)
    fclose(fparser);

  if (foutput)
    fclose(foutput);

	/* JF write out the output file */
  if (k == 0 && ftable)
    {
      FILE *ftmp;
      int c;

      ftmp=tryopen(tabfile, "w");
      rewind(ftable);
      while((c=getc(ftable)) != EOF)
        putc(c,ftmp);
      fclose(ftmp);
      fclose(ftable);

      if (definesflag)
        {
          ftmp = tryopen(defsfile, "w");
          fflush(fdefines);
          rewind(fdefines);
          while((c=getc(fdefines)) != EOF)
            putc(c,ftmp);
          fclose(ftmp);
          fclose(fdefines);
        }
    }

#if defined (VMS) & !defined (__VMS_POSIX)
  if (k==0) sys$exit(SS$_NORMAL);
  sys$exit(SS$_ABORT);
#else
  exit(k);
#endif /* not VMS, or __VMS_POSIX */
}
