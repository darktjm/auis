#include <andrewos.h>
#include <table.H>
#include <text.H>
#include <viewref.H>
#include <environment.H>
#include <view.H>

#ifndef TEXTVIEWREFCHAR
#define TEXTVIEWREFCHAR ((unsigned char)255)
#endif

static void DestroyCell(class table  *T, struct cell  *oldcell)
{
    struct viewlist *vl;

    switch (oldcell->celltype) {
	case table_TextCell:
	    free(oldcell->interior.TextCell.textstring);
	    break;
	case table_ValCell:
	    free(oldcell->interior.ValCell.formula);
	    break;
	case table_ImbeddedObject:
	   
	    if (oldcell->interior.ImbeddedObject.data) {
		(oldcell->interior.ImbeddedObject.data)->Destroy();
		oldcell->interior.ImbeddedObject.data = NULL;
	    }
	    while (vl = oldcell->interior.ImbeddedObject.views) {
		    (vl->child)->UnlinkTree();
		    (vl->child)->Destroy();
		/* end of misuse */
		oldcell->interior.ImbeddedObject.views = vl->next;
		    free((char *) vl);
	    }
	    
	    break;
    }
    oldcell->format = GENERALFORMAT;
    oldcell->lock = 0;
    oldcell->precision = table_DefaultPrecision;
    oldcell->celltype = table_EmptyCell;
    oldcell->lastcalc = ++(T->timeStamper);
}

static void TableImbedObject (table *self, dataobject *obj, Chunk  chunk)
{
    int r, c;
    struct cell *cell;

    (self)->StampEverything();
    for (r = max (chunk->TopRow, 0); r <= chunk->BotRow; r++) {
	for (c = max (chunk->LeftCol, 0); c <= chunk->RightCol; c++) {
	    cell = (self)->GetCell( r, c);
	    if (!(self)->IsJoinedToAnother( r, c) && !(cell->lock)) {
		DestroyCell(self, cell);
		if (obj) {
		    cell->celltype = table_ImbeddedObject;
		    cell->interior.ImbeddedObject.data = obj;
		    cell->interior.ImbeddedObject.views = NULL;
		    cell->lastcalc = self->cellChanged;
		    obj->Reference();
		    (obj)->AddObserver(self);
		}
	    }
	}
    }
    (self)->NotifyObservers( 0);
    (self)->SetModified();
}

dataobject *HTMLTextFixupText(text *src) {
    long count=0;
    dataobject *result=src;
    table *tbl=NULL;
    long row=0, col=0;
    long pos;
    boolean needcol=TRUE;
    boolean needrow=TRUE;
    for(pos=0;pos<src->GetLength();pos++) {
        switch(src->GetChar(pos)) {
                // ignore whitespace before the first inset.
            case ' ':
            case '\t':
                if(count>0) needcol=TRUE;
                break;
            case '\n':
                if(count>0) needrow=TRUE;
                break;
            case TEXTVIEWREFCHAR:
                if(needcol) col++;
                if(needrow) {
                    row++;
                    col=1;
                }
                needcol=needrow=FALSE;
                {
                    ATK_CLASS(hidden);
                environment *env=(environment *)src-> rootEnvironment-> GetInnerMost(pos);
                if(env->type!=environment_View) continue;
                viewref *ref=env->data.viewref;
                if(ref==NULL || ref->dataObject==NULL  || ref->dataObject->IsType(class_hidden)) continue;
                if(tbl==NULL) tbl=new table;
                tbl->ChangeSize(row, col);
                if(count==0) {
                    result=ref->dataObject;
                }
                count++;
                struct chunk chunk;
                chunk.LeftCol=col;
                chunk.RightCol=col;
                chunk.TopRow=row;
                chunk.BotRow=row;
                TableImbedObject(tbl, ref->dataObject, &chunk);
                }
                needcol=TRUE;
                break;
            default:
                if(tbl) tbl->Destroy();
                return src;
        }
    }
    if(tbl==NULL) return src;
    if(count>1) return tbl;
    tbl->Destroy();
    if(count==0) return src;
    return result?result:src;
}


dataobject *NewFixupText(dataobject *obj) {
    ATK_CLASS(text);
    ATK_CLASS(table);
    ATK_CLASS(hidden);
    if(!obj->IsType(class_text)) return obj;
    text *t=(text *)obj;
    long pos=0, len=t->GetLength();
    long others=0;
    long tables=0;
    table *tbl=NULL;
    while(pos<len) {
        switch(t->GetChar(pos)) {
            case ' ':
            case '\t':
            case '\n':
                break;
            case TEXTVIEWREFCHAR: {
                environment *env=(environment *)t-> rootEnvironment-> GetInnerMost(pos);
                
                if(env->type!=environment_View) break;
                
                viewref *ref=env->data.viewref;
                
                if(ref==NULL || ref->dataObject==NULL  || ref->dataObject->IsType(class_hidden)) break;

                if(ref->dataObject->IsType(class_table)) {
                    tbl=(table *)ref->dataObject;
                    tables++;
                } else others++;
                }
                break;
            default:
                // skip out we found something that wasn't whitespace or a view.
                others++;
                pos=len;
                break;
        }
        pos++;
    }
    if(tables!=1 || others!=0) return obj;
    tbl->Reference();
    return tbl;
}
