/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * exfont.h
 * header file for eq - defines the layout of the ex font.
 */


/*
 * These aren't actually characters, but correspond
 * to character codes that shouldn't be used for real
 * characters
 */

#define EX_BAR		1
#define EX_OVER		2
#define EX_VINCULUM	3

#define EX_SPACE	5
#define EX_CTHIN	(EX_SPACE+(int)CTHIN)
#define EX_THIN		(EX_SPACE+(int)THIN)
#define EX_CMED		(EX_SPACE+(int)CMED)
#define EX_MED		(EX_SPACE+(int)MED)
#define EX_CTHICK	(EX_SPACE+(int)CTHICK)
#define EX_THICK	(EX_SPACE+(int)THICK)


/*
 * Extendable symbols in ex font
 */

#define EX_NROOTS	8		/* characters per root */
#define EX_ROOT		020

#define EX_NSLASH	5		/* characters per slash */
#define EX_SLASH	030

#define EX_NPARENS	8		/* characters per paren */
#define EX_LPAREN	040
#define EX_RPAREN	050
#define EX_LSQUARE	060
#define EX_RSQUARE	070
#define EX_LBRACE	0100
#define EX_RBRACE	0110
#define EX_LANGLE	0120
#define EX_RANGLE	0130


/*
 * Extenders - in each font
 */

#define EX_NEXTENDERS	4		/* chars per extender */

#define EX_VERT1	0160
#define EX_VERT2	0164
#define EX_HORIZ1	0170
#define EX_HORIZ2	0174

#define EX_ANGLE_DL	0140
#define EX_ANGLE_DR	0144
#define EX_SLASH_DL	0140
#define EX_ROOT_EXT	0150
