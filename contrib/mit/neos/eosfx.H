#ifndef _eosfx_H_
#define _eosfx_H_
/*
	$Disclaimer: 
*Permission to use, copy, modify, and distribute this software and its 
*documentation for any purpose and without fee is hereby granted, provided 
*that the above copyright notice appear in all copies and that both that 
*copyright notice and this permission notice appear in supporting 
*documentation, and that the name of IBM not be used in advertising or 
*publicity pertaining to distribution of the software without specific, 
*written prior permission. 
*                        
*THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
*TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
*MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
*HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
*DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
*DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
*OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
*WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
* $
*/


 

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *  For full copyright information see:'mit-copyright.h'     *
 *************************************************************/

#include <mit-copyright.h>

#include "eos_structs.h"

#include <ATK.H>
class eosfx : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry();


	static void PaperClear(Paper *p);
	static void PaperCopy(Paper *src, Paper *dest);
	static struct paperPositions * LocatePaper(struct paperPositions *list, long x, Paper *paper)  ;
	static void AddPaperText(struct paperPositions **list, Paperlist paper, long x, long len);
	static void DestroyPositions(struct paperPositions **list);
	static char * OpenCourse(char *course, FX **fxp)  ;
	static void Close(FX **fxp);
	static char * SendFile(char *course, char *filename, Paper *paper, boolean delete_c) ;
	static char * RetrieveFile (FX *fxp, Paper *paper, char *filename)  ;
	static char * Retrieve (FX *fxp, Paper *paper, FILE *fp)  ;
	static char * Move(FX *fxp, Paper *src, Paper *dest)  ;
	static char * Delete(FX *fxp, Paper *paper)  ;
	static void ListDestroy(Paperlist_res **plist) ;
	static char * List(FX *fxp, Paper *paper, Paperlist_res **ret)  ;
	static char * AclList(FX *fxp, char *name, stringlist_res **list)  ;
	static void AclDestroy(stringlist_res **list);
	static char * AclAdd(FX *fxp, char *aclname, char *name)  ;
	static char * AclDel(FX *fxp, char *aclname, char *name)  ;
	static char * Directory(FX *fxp, stringlist_res **list)  ;
	static char * LocalUnique(char *template_c)  ;
	static char * PathTail(char *path)  ;
	static char * SaveString (char *s)  ;
	static void PaperCopyContents (Paper *src, Paper *dest);
	static void PaperFreeContents(Paper *p);
};


#endif /* _eosfx_H_ */
