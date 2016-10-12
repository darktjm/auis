/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include "champ.H"
#include "view.H"
#include "enode.H"
#include "chimp.H"


ATKdefineRegistry(chimp, chlist, NULL);
static void ChimpCallBack(struct eventnode  *en, class chimp  *self, enum view::MouseAction  action, long  nclicks);
static void WriteOutEvent(FILE  *fp, struct eventnode  *en);


chimp::chimp()
{
    this->en = new enode;
    this->comment = NULL;

    if (!this->en) THROWONFAILURE((FALSE));
    (this->en)->SetChimp( this);
    THROWONFAILURE((TRUE));
}

static void ChimpCallBack(struct eventnode  *en, class chimp  *self, enum view::MouseAction  action, long  nclicks)
{
    if (action == view::LeftUp || action == view::RightUp) {
	(self->en)->SetEvent( en);
    }
}

long chimp::Read(FILE  *fp, long  id)
{
    char LineBuf[250];
    struct eventnode *en;

    while (fgets(LineBuf, sizeof(LineBuf)-1, fp) != NULL) {
	if (strncmp(LineBuf, "\\enddata{", 9) == 0) {
	    return(dataobject::NOREADERROR);
	}
	en = champ::ReadDateIntoEventNode(LineBuf);
	if (en) {
	    /* need to add event to list */
	    (this)->AddItemToEnd( en->event, (chlist_itemfptr)ChimpCallBack, (long)en);
	} else {
	    struct comment *cm, *nextcm;

	    cm = (struct comment *) malloc(sizeof(struct comment));
	    if (cm) cm->line = strdup(LineBuf);
	    if (cm && cm->line) {
		cm->next = NULL;
		for (nextcm = this->comment; nextcm && nextcm->next; nextcm = nextcm->next) {
		    ;
		}
		if (nextcm) {
		    nextcm->next = cm;
		} else {
		    this->comment = cm;
		}
	    } else {
		printf("Throwing away: %s\n", LineBuf);
	    }		
	}
    }
    return dataobject::NOREADERROR; /* What, me worry? */
}

long chimp::Write(FILE  *fp, long  id, int  level)
{
    struct listitem *li = (this)->GetItemList();
    int numitems = (this)->GetNumItems();
    int i;
    struct comment *cm;

    if (id != (this)->GetWriteID()) {
	/* New write operation */
	(this)->SetWriteID( id);
	if (level>0) {
	    fprintf(fp, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
	}
	for (cm = this->comment; cm; cm = cm->next) {
	    fprintf(fp, "%s", cm->line); /* newline already there */
	}
	for (i=0; i<numitems; ++i) {
	    /* write out single item */
	    WriteOutEvent(fp, (struct eventnode *)li[i].rock);
	}
	if (level > 0) {
	    fprintf(fp, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
	}
    }
    return((this)->GetID());
}

static void WriteOutEvent(FILE  *fp, struct eventnode  *en)
{
    switch(en->ds.calsys) {
	case CALSYS_GREGORIAN:
	    fprintf(fp, "%d %d %d %d %d %d %d %d #%s\n",
		    CALSYS_GREGORIAN, en->ds.sys.gd.year,
		    en->ds.sys.gd.month, en->ds.sys.gd.day,
		    en->ds.sys.gd.wkday, en->ds.sys.gd.wkdayselector,
		    en->ds.sys.gd.hour, en->ds.sys.gd.min,
		    en->event);
	    break;
	case CALSYS_HEBREW:
	    fprintf(fp, "%d %d %d %d #%s\n",
		    CALSYS_HEBREW, en->ds.sys.hd.year,
		    en->ds.sys.hd.month, en->ds.sys.hd.day,
		    en->event);
	    break;
	case CALSYS_ECCLESIASTICAL:
	    fprintf(fp, "%d %d %d %d %d %d #%s\n",
		    CALSYS_ECCLESIASTICAL, en->ds.sys.ed.year,
		    en->ds.sys.ed.landmark, en->ds.sys.ed.offset,
		    en->ds.sys.ed.hour, en->ds.sys.ed.min,
		    en->event);
	    break;
	default:
	    fprintf(fp, "%d #%s\n", en->ds.calsys, en->event);
	    break;
    }
}

void chimp::AddNew(struct eventnode  *en)
{
    (this)->AddItemToEnd( en->event, (chlist_itemfptr)ChimpCallBack, (long)en);
    (this)->NotifyObservers( 0);
}

