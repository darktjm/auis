#ifndef _srctext_H_
#define _srctext_H_
/** \addtogroup libsrctext Object srctext
 *  A dynamically loadable ATK class file.  Contains srctext, srctextview,
 *  cpptextview, ctext, ctextview, hlptext, idltext, idltextview, ltext,
 *  ltextview, m3text, m3textview, modtext, modtextview, mtext, mtextview,
 *  perltext, perltextview, ptext, ptextview, rawtext, rawtextview,
 *  rexxtext, rexxtextview
 * @{ */
/* File srctext.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   srctext: a text subclass specialized for dealing with source code. */

/* for keyword hash-table lookup */
#define TABLESIZE 53
#define HASH(word) ((*(word)+word[strlen(word)-1]*strlen(word))%TABLESIZE)

/* arbitrary number for how many Tab Stops can be set */
#define MAX_TABSTOPS 80

/* kinds of styles that are used in hash table (other style numbers are defined in subclasses) */
#define MAX_STYLES 16
#define UPRCSE 0            /* a null style array entry for uppercase words */
                            /* values from 1 to MAX_STYLES-2 can be used for subclass-specific styles */
#define USRDEF MAX_STYLES-1 /* user-defined keywords */

typedef struct DictStruct {
    const char    *stng;
    short   val;    /*bits contain indentation info*/
    short   kind;    /*used for multiple style types*/
    struct DictStruct *next;
} Dict;

#include <stdio.h>

typedef void (*srctext_writefptr)(class srctext *self, FILE *file, long writeID, int level);

#include <text.H>
class srctext: public text {
  public:
    virtual ATKregistryEntry *ATKregistry();

    long AlwaysInsertFile(FILE * file,char *filename,long position);
    void Clear();
    long GetLineForPos(long pos);
    long GetModified();
    long GetPosForLine(long line);
    boolean InsertCharacters(long pos, const char *str, long len); /*RSK91overstrike*/
    long Read(FILE *file, long id);
    long ReadSubString(long pos, FILE *file, boolean quoteCharacters);
    long ReadTemplate(const char *templateName, boolean inserttemplatetext);
    void SetAttributes(struct attributes *atts);
    long Write(FILE *file, long writeID, int level);
    void WriteSubString(long pos, long len, FILE *file, boolean quoteCharacters);

    /* Polymorphism Enablers */
    virtual void BackwardCheckWord(long from,long to);
    virtual long CheckComment(long start);
    virtual long CheckLinecomment(long start);
    virtual long CheckString(long start);
    virtual long CheckWord(long i,long end);
    virtual long InCommentStart(long pos);
    virtual int Indentation(long pos);
    virtual boolean IsTokenChar(char ch);
    virtual char *Keywordify(char *buff, boolean checkforceupper);
    virtual void RedoStyles();

    /* Normal Methods */
    virtual void BackwardCheckLabel(long pos);
    virtual long BackwardCopyWord(long from,long to,char buffer[]);
    virtual long BackwardSkipString(long pos,char delim);
    virtual boolean Breakable(long pos);
    virtual void BreakLine(mark *endofline);
    virtual void CheckLineLengths(int maxlen,view *view);
    virtual long CopyWord(long pos,long end,char buffer[]);
    virtual int CurrentIndent(long pos);
    virtual int CurrentColumn(long pos);
    virtual boolean DoMatch(long pos,const char *str,int len);
    virtual boolean ExtendToOutdent(int indent,long *pos,long *len);
    virtual void FindSrcInsets();
    virtual environment *GetEnvironment(long pos);
    virtual style *GetStyle(long pos);
    virtual void GetToken(long *pos, long *len);
    virtual long Indent(mark *range); /* override this if subclass does stuff to comments that are on lines with code (plxtext) */
    virtual boolean InString(long pos);
    virtual boolean JustInsertCharacters(long pos,const char *str,long len); /*RSK91overstrike*/
    virtual boolean LinesTooLong(int maxlen);
    virtual int NextTabStop(int curcol);
    virtual boolean OutputSrcInset(FILE *file,long writeID,dataobject *inset);
    virtual void OverstrikeAChar(long pos); /*RSK91overstrike*/
    virtual boolean Quoted(long pos); /* override this if subclass can have backslash-quoted characters (like \" in C) */
    virtual boolean ReflowComment(long pos); /* override this if subclass' comments are column-sensitive or something (plxtext) */
    virtual void ReindentLine(long pos); /* override this if subclass pads end-of-comments to right margin (plxtext) */
    virtual void RemoveStyles();
    virtual long ReverseBalance(long pos); /* override this if subclass can have nested comments (modtext) */
    virtual void SetWriteCallbacks(srctext_writefptr pre,srctext_writefptr post);
    virtual void SetupStyles();
    virtual long SkipWhitespace(long pos,long end);
    virtual long TabAndOptimizeWS(long pos, int target);
    virtual long TranslateSrcInset(long pos);
    virtual void WrapStyle(long pos,long len,style *style,boolean begflag,boolean endflag);
    virtual void WrapStyleNow(long pos,long len,style *style,boolean begflag,boolean endflag);

    srctext();
    virtual ~srctext();

    static void BuildTable(const char *classname, Dict *hashTable[], const Dict wordlist[]);
    static void HashInsert(Dict *hashTable[], const Dict *word);
    static Dict *Lookup(Dict *hashTable[], const char *word);
    static void CopyrightScramble(text *txt, long pos, long len);

    inline boolean GetForceUpper() { return ((this)->forceUpper); }
    inline void SetForceUpper(boolean newValue) { (((this)->forceUpper) = (newValue)); }
    inline long GetMaxLineLength() { return ((this)->maxlinelength); }
    inline void SetMaxLineLength(long newValue) { (((this)->maxlinelength)= (newValue)); }
    inline boolean IndentingEnabled() { return ((this)->indentingEnabled); }
    inline boolean IsInOverstrikeMode() { return ((this)->OverstrikeMode) /*RSK91overstrike*/; }
    inline void ChangeOverstrikeMode(boolean newValue) { (((this)->OverstrikeMode) = (newValue)) /*RSK91overstrike*/; }


    Dict **words;
    boolean forceUpper, useTabs, reindentComments, indentingEnabled;
    char *copyright_key;

    /* Generic styles. Specific styles go into kindStyle array. */
    style *comment_style, *linecomment_style, *function_style, *label_style, *string_style;
    style *kindStyle[MAX_STYLES];

    /* Variables used to control the indenting style. */
    int commentIndent, contIndent, levelIndent;
    int tabSize, numTabStops, tabStop[MAX_TABSTOPS];

    /* Variables used by Esc-1 and Esc-2 comment insertion */
    const char *commentString, *linecommentString;
    int commentCol, linecommentCol;
    boolean commentFixed, linecommentFixed;
    int remarkPadding;

    /* Proc callback pointers. */
    srctext_writefptr preWriteFctn, postWriteFctn;

    /* "Temporary" stuff. maxlinelength stuff should go away as soon as programmers stop using punch cards (and fixed-length records) for their source code. RSK91overstrike stuff would be better implemented in ATK's text(VIEW) object. */
    long maxlinelength;
    boolean OverstrikeMode; /*RSK91overstrike*/ /*ideally, this should be associated with the view; unfortunately, the view is not known to srctext_InsertCharacters, which is where the overstriking is being done. But doing it in _InsertCharacters was a mistake *anyway*, so...*/
};

inline boolean is_whitespace(char ch)
{ return (ch==' ')||(ch=='\t'); }

inline char *makeupper(char *str)
{
    char *st;
    st=str;
    while(*str!=0) {
	if(islower(*str)) *str=toupper(*str);
	str++;
    }
    return st;
}
/** @} */
#endif /* _srctext_H_ */
