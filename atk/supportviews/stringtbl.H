ATK_INTER
#ifndef _stringtbl_H_
#define _stringtbl_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

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


 

/*
stringtbl.H

String table data object

	This module supports storage for a set of string values.  (At most 32 strings)
	A bit vector associated with the set specifies which are "selected" (0, 1, or many)
	Strings are checked for backslash and brackets and they are deleted.
	Strings are truncated to 255 character.
	All operations are case sensitive.
	(AddString checks for length and backslash, but the others do not.  Thus
		a string may be added and then not found.)

	An "accession number" is associated with each string.  This will be a small integer;
	counting restarts each time the table is cleared.  The functions with OfEntry in their
	names have the accession number as argument.

	The character string arguments to the functions are tested for NULL or '\0' and
	ignored if so.   In this case, AddString returns 0;  it also returns zero if the table is full.

*/

#define  stringtbl_PROGRAMMERVERSION    1

/* XXX ??? I think there is as yet no way to say I want these only in .eh */
#define MAXSTRINGSINTABLE 32
#define MAXSTRINGENTRYLENGTH 255

#define stringtbl_STRINGSCHANGED 1
#define stringtbl_BITSCHANGED 2

#include <dataobject.H>

class stringtbl : public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();




	 long Read( /* struct stringtbl *self, */ FILE *file, long id )  ;
	 long Write( /* struct stringtbl *self, */ FILE *file, long id, int level )
	 ;

	/* The file format for a stringtbl is the bitvector in hex, 
		a newline, 	and then a sequence of the strings, each 
		followed by a newline.
	*/
	 const char * ViewName()  ;


	virtual void Clear( /* struct stringtbl *self */ );		
		/* Clears the string table of existing strings */
	virtual short AddString( /* struct stringtbl *self, */ const char *s)  ;
		/* Adds a string to the set of strings in the table, 
			returns its accession number 
			returns -1 if table is full */
	virtual void RemoveString( /* struct stringtbl *self, */ const char *s);
			/* Removes the specified string from the table.
			If the string is absent, the call is ignored */
	virtual void SetBit( /* struct stringtbl *self, */ const char *s, boolean val);
		/* finds the named string in the table 
			and sets its associated bit to the given val */
	virtual boolean GetBit( /* struct stringtbl *self, */ const char *s)  ;
		/* finds the named string in the table and 
			returns the current value of its bit 
		   if s is absent, returns FALSE anyway */
	virtual void ClearBits( /* struct stringtbl *self */ );
		/* sets all bits to zero */
	virtual void RemoveEntry( /* struct stringtbl *self, */ short accnum );
		/* remove the entry with the given accnum 
			If the string is absent, the call is ignored */
	virtual void SetBitOfEntry( /* struct stringtbl *self, */  short accnum, boolean val );
		/* set the bit associated with the given accnum 
			If the string is absent, the call is ignored */
	virtual boolean GetBitOfEntry( /* struct stringtbl *self, */  short accnum )  ;
		/* return the bit for the given accnum 
			If the string is absent, returns FALSE */
	virtual char * GetStringOfEntry( /* struct stringtbl *self, */  short accnum )  ;
		/* return the string for the given accnum 
			If the string is absent, returns NULL */
	virtual short GetEntryOfString( /* struct stringtbl *self, */ char *string, short startIndex)  ;
		/* return the index of the entry that matches string, the search starts at startIndex.  Returns -1 if no item was found  Returns -2 if a bad string was sent in */	



	/* (the instances of " + 0 " prevent assignment to these fields) 
		It is a gross error to modify the string pointed at by IthString
		or to retain the pointer beyond the next AddString or RemoveString */

	inline short NStrings( /* struct stringtbl *self */ ) { return ((short)(this->used + 0)); }
	inline char * IthString( /* struct stringtbl *self, */  short i )  { return ((char *)(this->item[i] + 0)); }
	inline short NumAcc( /* struct stringtbl *self */ ) { return ((short)(this->numacc+0)); }



	
	stringtbl();
	
	virtual ~stringtbl();



	char *(item[MAXSTRINGSINTABLE]);	/* pointers to the strings */
	unsigned long highlight;		/* the bits.  bit (1<<i) is for item[i] */
	short used;		/* the first -used- of the elements in -item- have values */
	short numacc;		/* number of accessions */
	short accmap[32];	/* the accession number for each entry */
	boolean ContainsInitialStrings;	/* TRUE until client first changes table */
};


#endif /* _stringtbl_H_ */
