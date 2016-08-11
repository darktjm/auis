#ifndef _amsutil_H_
#define _amsutil_H_
/***********************************************************
		Copyright IBM Corporation 1991

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/* Conversion notes:
  debug_XXX procedures used to be called dbg_XXX.
  fdplumber_XXX used to be called fdplumb_XXX.
*/
 

/* Constants for the customization interface */

#define EXP_FILEINTO 0 /* Turns on classification, also "create,delete,rename" menus */
/* EXP_FILEICONMAIL 1 */ /* not used */
/* EXP_FILEICONOTHER 2 */ /* not used */
#define EXP_FILEICONCAPTIONS 3
#define EXP_FILEINTOMENU 4
/* EXP_SHOWALL 5 */ /* not used */
#define EXP_SHOWCLASSES 6
/* EXP_SHOWSUBSCRIBED 7 */ /* not used */
/* EXP_UNDENSEMAIL 8 */ /* not used */
/* EXP_UNDENSEOTHERS 9 */ /* not used */
/* EXP_SHOWDUPS 10 */ /* not used */
#define EXP_FIXCAPTIONS 11
#define EXP_PURGEONQUIT 12
#define EXP_SUBSEXPERT 13
/* EXP_NOSHOWUNCHANGED 14 */ /* not used */
#define EXP_WHITESPACE 15
/* EXP_ERASER 16 */ /* not used */
/* EXP_UPDATESTATE 17 */ /* not used */
#define EXP_THREEREPLIES 18
#define EXP_SHOWNOHEADS 19
#define EXP_MARKING 20
#define EXP_SETQUITHERE 21
#define EXP_SHOWMORENEXT 22
#define EXP_MARKASUNREAD 23
#define EXP_APPENDBYNAME 24
#define EXP_MARKEDEXTRAS 25
#define EXP_CLEARAFTER 26
#define EXP_HIDEAFTER 27
#define EXP_KEEPBLIND 28
#define EXP_INSERTHEADER 29
#define EXP_KEYSTROKES 30
/* EXP_ROT13 31 */ /* not used */
#define EXP_BIGSTYLES 32
#define EXP_CHECKRECIP 33
#define EXP_PUNTBUTT 34
/* EXP_MIXEDBAGGAGE 35 */ /* not used */
#define EXP_SHOWALLBUTKEYS 36
#define EXP_PUNTMENU 37
#define EXP_SIDEBYSIDE 38
/*#define EXP_BLOCKPUNTS 39 */ /* not used any longer */
/* #define EXP_BIGOPTIONS 40 */ /* not used */
#define EXP_DUMPCORE 41
#define EXP_CKPONTMP 42
#define EXP_FORCESEND 43
#define EXP_FORMATMENUS 44
/*#define EXP_NOGROW 45 */
#define EXP_SENDEMPTY 46
#define EXP_WARPWINDOW 47
#define EXP_GROWFOLDS 48
#define EXP_SIGNMAIL 49
#define EXP_NOFIRSTFOLDER 50
#define EXP_VANISH 51

/* Changes to the above definitions should be accompanied by changes to the descriptive array in stubs.c, and addittions should be reflected in changes to EXP_MAXUSED */
#define EXP_MAXUSED 52 /* Should be one bigger than biggest above */

#define GETOPTBIT(o, i) ((o)[(i)/32] & (1<<((i)%32)))
#define SETOPTBIT(o, i, v) if (v) {(o)[(i)/32] |= (1<<((i)%32));} else {(o)[(i)/32] &= ~(1<<((i)%32));}

struct OptionState {
    long Opts[EXP_MAXUSED/32+1],
	PermOpts[EXP_MAXUSED/32+1],
	OptMask[EXP_MAXUSED/32+1],
	DefaultOpts[EXP_MAXUSED/32+1];
};

#include <ATK.H>
class amsutil : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      static char ** GetKeyHeadsArray()  ;
      static char ** ParseKeyHeaders()  ;
      static int setprofilestring(char *prog, char *pref, char *val)  ;
      static int GetOptBit(int opt)  ;
      static void SetOptBit(int opt, int val);
      static void BuildOptionPreference(char *buf);
      static int GetPermOptBit(int opt)  ;
      static void SetPermOptBit(int opt, int val);
      static int GetOptMaskBit(int opt)  ;
      static void SetOptMaskBit(int opt, int val);
      static void BreakDownContentTypeField(char *override, char *fmttype, int fmttypelen, char *fmtvers, int fmtverslen, char *fmtresources, int fmtresourceslen);
      static char ** BreakDownResourcesIntoArray(char *fmtresources)  ;
      static int lc2strncmp(char *s1, char *s2, int len)  ;
      /* The above were what was needed for ezprinting (used by text822) */
      static char * StripWhiteEnds(char *s)  ;
      static char * cvEng(int num, int min, int max)  ;
      static char * convlongto64(long t, long pad)  ;
      static long conv64tolong(char *s64)  ;
      static void ReduceWhiteSpace(char *s);
      static void LowerStringInPlace(char *s, int slen);
      static int debug_open(char *name, int flags, int mode)  ;
      static FILE * debug_fopen(char *name, char *mode)  ;
      static int debug_close(int fd)  ;
      static int debug_vclose(int fd)  ;
      static int debug_fclose(FILE *fp)  ;
      static int debug_vfclose(FILE *fp)  ;
      static void fdplumber_SpillGutsToFile(FILE *fp, boolean doublenewlines);
      static void fdplumber_SpillGuts();
      static char * GetDefaultFontName()  ;
      static int ChooseNewStatus(char *nickname, int GivenDefault, boolean ShowAllChoices)  ;
};

#endif /* _amsutil_H_ */
