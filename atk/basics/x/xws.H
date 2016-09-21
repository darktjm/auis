ATK_INTER
#ifndef _xws_H_
#define _xws_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/* X implementation of the windowsystem class.
 */

#include <windowsystem.H>
class iddimage;
class cursor;
class fontdesc;
class graphic;
class im;
class xws : public  windowsystem {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        virtual class cursor * CreateCursor()  ;
        virtual class fontdesc * CreateFontdesc()  ;
        virtual class graphic * CreateGraphic()  ;
        virtual class im * CreateIM()  ;
	virtual struct offscrwin * CreateOffScreenWindow(char * host, long width, long height)  ;
        virtual void FlushAllWindows();
        virtual boolean HandleFiles(long WaitTime, boolean beCheap)  ;
	virtual iddimage *CreateDDImage(image *i, colormap **map);

};
/** @} */
#endif /* _xws_H_ */
