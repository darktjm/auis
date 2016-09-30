#ifndef _prefval_H_
#define _prefval_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

#include <andrewos.h>
/** \addtogroup libprefs
 * @{ */
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
	const char *sval;
	boolean bval;
	const char *fval;
	const char *dval;
	const char *fontval;
	const char *cval;
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
    static enum prefval_Type StringToType(const char *str)  ;


    virtual void SetAppName(const char *name);
    virtual void SetPrefName(const char *name);
    virtual void SetSeparator(const char *sep);
    virtual void SetChoices(int nchoices, char **choices, struct prefval_value *cvalues);
    virtual void SetValues(int nvalues, struct prefval_value *tvalues);
    virtual void SetIndexValue(int which, struct prefval_value *tvalue);
    virtual void SetValue(struct prefval_value *tvalue);
    virtual void SetChoice(int which, char *choice, struct prefval_value *tvalue);
    virtual struct prefval_value * StringToValue(const char *str)  ;
    virtual const char * IndexValueString(int which)  ;
    virtual const char * TypeString()  ;
    virtual long ReadDataPart(FILE *fp, int dsversion)  ;
    virtual const char *PreferenceString();
    virtual void SetFromPreferenceString(const char *str);
    virtual void ClearValues();
    virtual void ClearChoices();
    virtual void FreeValue(struct prefval_value *v);
    virtual void InitValue(struct prefval_value *v);
    virtual void CopyValue(struct prefval_value *v1, struct  prefval_value *v2);
    virtual void SortChoices();
    virtual void SetCondition(char *str);
    virtual void SetDefault();
    virtual const char * FullPreferenceString()  ;

     long Write(FILE *fp, long writeID, int level)  ;
     long Read(FILE *fp, long id)  ;
     void SetModified();
  


    inline const char  * GetAppName() { return (this->appname); }
    inline const char  * GetPrefName(){ return (this->prefname); }
    inline char  ** GetChoiceNames() { return (this->choices); }
    inline char *GetIndexChoiceName(int which) { return (this->choices?this->choices[which]:NULL); }
    inline struct prefval_value *GetIndexChoiceValue(int which) { return (this->cvalues?&this->cvalues[which]:NULL); }
    inline struct prefval_value *GetIndexValue(int which) { return (this->values?&this->values[which]:NULL); }
    inline int   GetListSize() { return (this->vlistsize); }
    inline int   GetChoiceListSize() { return (this->clistsize); }
    inline struct prefval_value *GetValue() { return (this->values ? &this->values[0] : NULL); }
    inline void SetType(enum prefval_Type t) { ((this)->type=(t)); }
    inline prefval_Type   GetType() { return (this->type); }
    inline const char *ValueString() { return ((this)->IndexValueString( 0)); }
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
/** @} */
#endif /* _prefval_H_ */
