/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/* rastimg.c		

	Code for the rasterimage object

	Provides for file name storage with a pixelimage
	and for refcnt, WriteID, and ObjectID

*/
/*
    $Log: rasterimage.C,v $
 * Revision 1.3  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.2  1994/08/11  18:51:43  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.1  1993/05/23  18:37:52  gk5g
 * Initial revision
 *
// Revision 1.2  1993/04/27  18:33:26  rr2b
// Made Stabilize function more robust, move malloc out of
// strcpy arguments, and cast its return value appropriately.
// Fixed signatures for rasterimage::RemoveObserver and AddObserver.
// Added cast for return value of pixelimage::Clone.
//
// Revision 1.1  1993/04/27  01:04:24  rr2b
// Initial revision
//
 * Revision 2.7  1992/12/15  21:40:02  rr2b
 * more disclaimerization fixing
 *
 * Revision 2.6  1992/12/14  20:52:02  rr2b
 * disclaimerization
 *
 * Revision 2.5  1991/09/12  16:29:01  bobg
 * Update copyright notice and rcsid
 *
 * Revision 2.4  1989/02/17  16:58:45  ghoti
 * ifdef/endif,etc. label fixing - courtesy of Ness
 *
 * Revision 2.3  89/02/08  16:31:51  ghoti
 * change copyright notice
 * 
 * Revision 2.2  89/02/04  12:44:12  ghoti
 * first pass porting changes: filenames and references to them
 * 
 * Revision 2.1  88/09/27  16:50:20  ghoti
 * adjusting rcs #
 * 
 * Revision 1.2  88/09/15  16:39:03  ghoti
 * copyright fix
 * 
 * Revision 1.1  88/09/01  02:03:14  zs01
 * "initial
 * 
 * Revision 5.2  88/08/19  14:18:51  ghoti
 * Includes now use '<>' instead of '""'
 * 
 * Revision 5.1  88/07/15  16:08:55  mp33
 * Changed Clone method to do the right thing.
 * 
 * Revision 1.1  87/12/06  16:37:14  wjh
 * Initial revision
 * 
 * 10 Nov 1987 WJHansen. Created.
 */

#include <andrewos.h>
ATK_IMPL("rasterimage.H")
#include <stdio.h>

#include <rasterimage.H>


/* Stabilize(s)
	Copies 's' into newly malloced storage.
	XXX we need a home for this function 
*/
	
ATKdefineRegistry(rasterimage, pixelimage, NULL);
static char * Stabilize(char  *s);


static char *
Stabilize(char  *s)
{
    char *result=s?(char *)malloc(strlen(s)+1):NULL;
    if (result) {
	strcpy(result, s);
    }
    return result;
}


rasterimage::rasterimage()
		{
	this->filename = this->resolutionPath = NULL;
	this->refcnt = 0;
	(this)->SetObjectID( 0);
	(this)->SetWriteID( 0);
	THROWONFAILURE( TRUE);
}

rasterimage::~rasterimage()
		{
	if (this->filename) free(this->filename);
	if (this->resolutionPath) free(this->resolutionPath);
	this->filename = this->resolutionPath = NULL;
}

	class rasterimage *
rasterimage::Create(long  width , long  height)
		{
	class rasterimage *self = new rasterimage;
	(self)->Resize( width, height);
	return self;
}



	void
rasterimage::AddObserver(class observable  *observer)
		{
	(this)->pixelimage::AddObserver( observer);
	this->refcnt++;
}

	void
rasterimage::RemoveObserver(class observable  *observer)
		{
	(this)->pixelimage::RemoveObserver( observer);
	this->refcnt--;
	if (this->refcnt <= 0)
		(this)->Destroy();
}

	class pixelimage *
rasterimage::Clone()
	{
	    class pixelimage *pnew = (this)->pixelimage::Clone();
	    class rasterimage *new_c = (class rasterimage *)pnew;
	new_c->filename = this->filename;
	new_c->refcnt = 1;
	(new_c)->SetObjectID( 0);
	(new_c)->SetWriteID( 0);
	return new_c;	
}

/* rasterimage__FindFile(self, filename, path)
	Searches for a file of the given name.  Checks directories in this order
		the directory containing current document (??? how)
		the directories in the rasterpath (from preferences)
		the given path  (which may be NULL)
	records the file name and the path that was found to contain the file

	Returns an open stream for the file.

	XXX right now it just uses the filename
*/
	FILE *
rasterimage::FindFile(char  *filename , char  *path)
		{
	this->filename = Stabilize(filename);
	this->resolutionPath = Stabilize(path);
	return  fopen(filename, "r");
}

/* Defile(self)
	Removes saved filename and path 
*/
	void
rasterimage::Defile()
	{
	if (this->filename) free(this->filename);
	if (this->resolutionPath) free(this->resolutionPath);
	this->filename = this->resolutionPath = NULL;
}

