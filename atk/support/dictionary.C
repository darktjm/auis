/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("dictionary.H")
#include <dictionary.H>
#include <view.H>

#define INITIALSIZE 128
#define dictionary_DELETED NULL
#define EntryDeleted(A) (A->view == (class view *) dictionary_DELETED &&  A->id == (char *)dictionary_DELETED)
#define DeleteEntry(A)  A->view = (class view *)dictionary_DELETED ; A->id = (char *) dictionary_DELETED ;
struct dirtable {
    class view *view;
    char *id;
    char *object;
};
static struct dirtable *table, *last, *end;

ATKdefineRegistry(dictionary, ATK, dictionary::InitializeClass);

boolean dictionary::InitializeClass()
    {
    table = (struct dirtable *) malloc(INITIALSIZE * sizeof(struct dirtable));
    end = &(table[INITIALSIZE]);
    last = table;

    return TRUE;
}
void dictionary::Insert(class view  *view,char  *id ,char  *object)
{
	ATKinit;

    struct dirtable *dd,*freeref;
    freeref = NULL;
    for(dd = table; dd < last; dd++){
        if(dd->view == view && dd->id == id) break;
        if(freeref == NULL && EntryDeleted(dd)) freeref = dd;
    }
    if(dd == last) {
        if(freeref != NULL){
            dd = freeref;
        }
        else {
            if(last == end){
                int newsize = INITIALSIZE + (end - table) ;
		long diff = last - table; 
                table = (struct dirtable *) realloc(table,newsize * sizeof(struct dirtable));
		if(table == NULL){
			fprintf(stderr,"Out Of Memory in dict.c\n");
			return;
			} 
                end  = &(table[newsize]);
		last = table + diff;
		dd = last;
            }
            last++;
        }
    }
    dd->view = view;
    dd->id = id;
    dd->object = object;
}
char *dictionary::LookUp(class view  *view,char  *id)
{
	ATKinit;

    struct dirtable *dd;
    for(dd = table; dd < last; dd++)
        if(dd->view == view && dd->id == id) return(dd->object);
    return(NULL);
}
long dictionary::CountViews(char  *id)
{
	ATKinit;

    struct dirtable *dd;
    long i = 0;
    for(dd = table; dd < last; dd++){
        if(dd->id == id && dd->view != NULL){
            i++;
        }
    }
    return i;
}
long dictionary::ListViews(char  *id,class view  **viewarray,long  len)
{
	ATKinit;

    struct dirtable *dd;
    long i = 0;
    for(dd = table; dd < last; dd++){
        if(dd->id == id && dd->view != NULL ){
            viewarray[i++] = dd->view;
            if(i == len) break;
        }
    }
    return i;
}
long dictionary::CountRefs(class view  *view)
{
	ATKinit;

    struct dirtable *dd;
    long i = 0;
    for(dd = table; dd < last; dd++){
        if(dd->view == view ){
            i++;
        }
    }
    return i;
}
long dictionary::ListRefs(class view  *view,char  **refarray,long  len)
{
	ATKinit;

    struct dirtable *dd;
    long i = 0;
    for(dd = table; dd < last; dd++){
        if(dd->view == view){
            refarray[i++] = dd->id;
            if(i == len) break;
        }
    }
    return i;
}

void dictionary::Delete(class view  *view,char  *id)
{
	ATKinit;

    struct dirtable *dd;
    for(dd = table; dd < last; dd++)
        if(dd->view == view && dd->id == id){
            DeleteEntry(dd);
            return;
        }
}
