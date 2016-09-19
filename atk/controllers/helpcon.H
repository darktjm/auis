ATK_INTER
#ifndef _helpcon_H_
#define _helpcon_H_
/* user code begins here for HeaderInfo */

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
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
