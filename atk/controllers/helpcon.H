ATK_INTER
#ifndef _helpcon_H_
#define _helpcon_H_
/* user code begins here for HeaderInfo */

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

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

 
/* user code ends here for HeaderInfo */
#include <ATK.H>
class helpcon : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();
 

        helpcon();
	
	static boolean InitializeClass();

/* user code begins here for classprocedures */
/* user code ends here for classprocedures */

	class text *topics;
	class cltextview *topicsView;
	class text *choice;
	class cltextview *choiceView;
	class value *topicschoice;
	class stringV *topicschoiceView;
	class text *body;
	class textview *bodyView;
	class value *choicelabel;
	class stringV *choicelabelView;
/* user code begins here for classdata */
	class text *historytext;
	int ShowHistory;
	char CurrentName[256];
	char CurrentType[256];
	struct dirent **dl;
	long count;
/* user code ends here for classdata */
	class view *v;
	class arbiterview *arbv;
	class helpcon *next;
};


#endif /* _helpcon_H_ */
