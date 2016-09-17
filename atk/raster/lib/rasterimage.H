ATK_INTER
#ifndef _rasterimage_H_
#define _rasterimage_H_
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
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
rasterimage.H

	rasterimage object

	is a subclass of pixelimage and adds a few fields utilized by the
	raster package

	Adds the file name finding operation

*/

#define  rasterimage_PROGRAMMERVERSION    1

/* status values for NotifyObservers */
#define rasterimage_DATACHANGED 1

#include <pixelimage.H>
class rasterimage : public  pixelimage {
public:
	virtual ATKregistryEntry *ATKregistry();




	 void AddObserver(/* struct rasterimage *self, */ class observable *observer);
	 void RemoveObserver(/* struct rasterimage *self, */ class observable *observer);
	 class pixelimage * Clone(/*struct rasterimage *self, */)  ;



	virtual FILE * FindFile(/* struct rasterimage *self, */ char *filename, char *path)  ;
		/* finds the file, searching in various places.  Opens it and
		 returns the stream.  Saves file name in filename. */
	virtual void Defile(/* struct rasterimage *self */);
		/* removes the filename information in preparation for
		 reading in a new file  (Should this be part of resize ? XXX) */



	inline void SetWriteID(/* self, */ long wid)	{ (this->writeID = (wid)); }
	inline long GetWriteID(/* self */)	{ return (this->writeID); }
	inline void SetObjectID(/* self, */ long obid)	{ (this->objectid = (obid)); }
	inline long GetObjectID(/* self*/ )	{ return (this->objectid); }
	inline long MarkPermanent(/* self */)	{ return (this->refcnt++); }
	inline char * GetFileName(/* self */)	{ return (this->filename); }
	inline char * GetFilePath(/* self */)	{ return (this->resolutionPath); }



	
	rasterimage();
	
	virtual ~rasterimage();
	static class rasterimage * Create(long width, long height)  ;
		/* creates a rasterimage object with 'bits' for a raster
			of the given width and height */



	long writeID;	/* the writeID when this pixelimage was most recently written */
	long objectid;	/* gives the objectid of the raster that mostly recently 
					wrote this pixelimage as a "bits"  */
	char *filename;		/* file raster is referenced from (or NULL) */
	char *resolutionPath;	/* path originally used to resolve filename */
	long refcnt;		/* XXX */

};


#endif /* _rasterimage_H_ */
