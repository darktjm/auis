ATK_INTER
#ifndef _prefval_H_
#define _prefval_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

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



enum prefval_Type {
    prefval_None,
    prefval_Integer,
    prefval_Real,
    prefval_String,
    prefval_Boolean,
    prefval_Filename,
    prefval_Directory,
    prefval_Font,
    prefval_Color
};

enum prefval_Changes {
    prefval_Generic,
    prefval_ChoicesChanged,
    prefval_ValuesChanged
};

struct prefval_value {
    union prefval_realvalue {
	int ival;
	float rval;
	char *sval;
	boolean bval;
	char *fval;
	char *dval;
	char *fontval;
	char *cval;
    } v;
    boolean set;
};

#define prefval_DS_VERSION 0

#include <dataobject.H>
class prefval : public  dataobject  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	prefval();
    
	virtual ~prefval();
    static enum prefval_Type StringToType(char *str)  ;


    virtual void SetAppName(char *name);
    virtual void SetPrefName(char *name);
    virtual void SetSeparator(char *sep);
    virtual void SetChoices(int nchoices, char **choices, struct prefval_value *cvalues);
    virtual void SetValues(int nvalues, struct prefval_value *tvalues);
    virtual void SetIndexValue(int which, struct prefval_value *tvalue);
    virtual void SetValue(struct prefval_value *tvalue);
    virtual void SetChoice(int which, char *choice, struct prefval_value *tvalue);
    virtual struct prefval_value * StringToValue(char *str)  ;
    virtual char * IndexValueString(int which)  ;
    virtual char * TypeString()  ;
    virtual long ReadDataPart(FILE *fp, int dsversion)  ;
    virtual char *PreferenceString();
    virtual void SetFromPreferenceString(char *str);
    virtual void ClearValues();
    virtual void ClearChoices();
    virtual void FreeValue(struct prefval_value *v);
    virtual void InitValue(struct prefval_value *v);
    virtual void CopyValue(struct prefval_value *v1, struct  prefval_value *v2);
    virtual void SortChoices();
    virtual void SetCondition(char *str);
    virtual void SetDefault();
    virtual char * FullPreferenceString()  ;

     long Write(FILE *fp, long writeID, int level)  ;
     long Read(FILE *fp, long id)  ;
     void SetModified();
  


    inline char  * GetAppName() { return (this->appname); }
    inline char  * GetPrefName(){ return (this->prefname); }
    inline char  ** GetChoiceNames() { return (this->choices); }
    inline char *GetIndexChoiceName(int which) { return (this->choices?this->choices[which]:NULL); }
    inline struct prefval_value *GetIndexChoiceValue(int which) { return (this->cvalues?&this->cvalues[which]:NULL); }
    inline struct prefval_value *GetIndexValue(int which) { return (this->values?&this->values[which]:NULL); }
    inline int   GetListSize() { return (this->vlistsize); }
    inline int   GetChoiceListSize() { return (this->clistsize); }
    inline struct prefval_value *GetValue() { return (this->values ? &this->values[0] : NULL); }
    inline void SetType(enum prefval_Type t) { ((this)->type=(t)); }
    inline prefval_Type   GetType() { return (this->type); }
    inline char *ValueString() { return ((this)->IndexValueString( 0)); }
    inline void SetListMax(int n) { (this->listmax=(n)); }
    inline int   GetListMax() { return (this->listmax); }
    inline char  * GetSeparator() { return (this->separator); }
    inline void SetCurrentItem(int n) { (this->curitem=(n)); }
    inline int   GetCurrentItem() { return (this->curitem); }
    inline char  * GetCondition() { return (this->condition); }
    inline boolean   GetIsDefault() { return (this->isdefault); }
    inline void SetRangeLow(int l) { (this->low=(l)); }
    inline void SetRangeHigh(int h) { (this->high=(h)); }
    inline long   GetRangeLow() { return (this->low); }
    inline long   GetRangeHigh() { return (this->high); }


    char *prefname;
    char *appname;
    enum prefval_Type type;
    boolean valueset;
    int listmax;
    int vlistsize;
    struct prefval_value *values;
    int clistsize;
    char **choices;
    struct prefval_value *cvalues;
    char *separator;
    int curitem;
    char *condition;
    boolean isdefault;
    long low, high;
};

#endif /* _prefval_H_ */