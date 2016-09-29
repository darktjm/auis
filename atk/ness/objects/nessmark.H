#ifndef _nessmark_H_
#define _nessmark_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1993 - All Rights Reserved
\* ********************************************************************** */

/** \addtogroup libness
 * @{ */
#include <simpletext.H>

#include <mark.H>
class  nessmark  : public   mark

{
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	nessmark();
	virtual ~nessmark();


	void UpdateMarks(long pos, long size);
		/* update mark locations for a change to base of size at pos.
			'size' may be negative for deletion. */

	virtual void SetText(class simpletext *text);
		/* changes to refer to 'text'.  Updates links in chain from text. */

	virtual void Set(class simpletext *text, long pos, long length);
		/* sets the mark to the given pos and length in given text */
	virtual void MakeConst(const char * cx);
		/* sets the mark to refer to the constant */
	virtual void Next();
		/* modifies mark to point to the char after the former value */
	virtual void Start();
		/* modifies mark to refer to the start of the former value */
	virtual void Base();
		/* modifies the mark to refer to the entire text */
	virtual void Extent(class nessmark *tail);
		/* modifies mark to extend from its start to the end of tail */
	virtual void Replace(class nessmark *replacement);
		/* replaces contents of the mark with the replacement text */

	virtual boolean Equal(class nessmark *comparand);
		/* compares the text denoted by the two marks */
	virtual boolean IsEmpty();
		/* tests the text of self to see if it has any characters */

	virtual long Length();
		/* returns the number of characters in the mark */
	virtual void NextN(long n);
		/* apply next 'n' times */

	virtual void SetFrom(class nessmark *src);
		/* copies src marker to self  (they share text) */
	virtual char * ToC()  ;
		/* mallocs a string and copies mark into it.  
			The caller must free the value. */


	inline class simpletext * GetText() {
		return ((class simpletext *)
			(((class mark *)this)->GetObject()));
	}

	static class nessmark *FreeMarks;
	static class nessmark *getMark();
	inline void ungetMark() {
		(this)->DetachFromText();
		(this)->SetNext(FreeMarks);
		FreeMarks = this;
	}


	/* DetachFromText and AttachToText are intended for use when moving
	   a mark, as on a stack.  The text is not deleted even if it has
	   no marks after the DetachFromText.  To get rid of a mark, 
	   nessmark_Destroy should be used instead.  To change the text
	   associated with a mark, use SetText instead. */
	inline void DetachFromText() { 
		if ((this)->GetText()) {
			((this)->GetText())->RemoveMark(this);
			(this)->SetObject( NULL); 
	}}
		/* XXX dirty:  Should have an AddMark method in simpletext.H */
	inline void AttachToText(class simpletext *text) { 
		this->SetObject(text);  
		this->SetNext( (text)->markList);  
		(text)->markList = this;  
	}
	inline void InitFrom(class nessmark  *src) {
		/* copies src marker to this  (they share text);
		   this did not previously have a marker value */
		*(this) = *(src);  	/* ??!!  may need copy constructor */
		(this)->AttachToText( (this)->GetText());
	}
};
/** @} */
#endif /* _nessmark_H_ */
