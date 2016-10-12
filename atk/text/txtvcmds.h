/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

extern long lcKill;
extern long lcYank;
extern long lcMove;
extern long lcDisplayEnvironment;
extern long lcInsertEnvironment;
extern long lcNewLine;

/* yank/delete text flags in txtvcmds.c */
#define YANK	0
#define DELETE	1

/* search direction flag values used in txtvcmds.c */
#define	FORWARD	    0
#define	BACKWARD    1

#define NEW(type) (type *) malloc(sizeof(type))

#define Text(v)	(struct text *) ((v)->GetDataObject())

#define TEXT_VIEWREFCHAR '\377'

#define textview_GetLines(self) ((self)->nLines)

