ATK_INTER
#ifndef _windowsystem_H_
#define _windowsystem_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/* Window system package.
 * Provides an abstraction for dealing with multiple window management systems
 * using a singe executable.
 */

#include <ATK.H>
class im;
class graphic;
class cursor;
class fontdest;
class colormap;
class color;
class iddimage;
class image;
class windowsystem : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        virtual class im * CreateIM()  ;
        virtual class graphic * CreateGraphic()  ;
        virtual class cursor * CreateCursor()  ;
        virtual class fontdesc * CreateFontdesc()  ;
	virtual struct offscrwin * CreateOffScreenWindow(char * host, long width, long height)  ;
	virtual class colormap * CreateColormap(class im *im)  ;
	virtual class color * CreateColor(char *name, unsigned int r, unsigned int g, unsigned int b)  ;
        virtual void FlushAllWindows();
	virtual boolean HandleFiles(long WaitTime, boolean beCheap)  ;
	virtual iddimage *CreateDDImage(image *i, colormap **map);
};
/** @} */
#endif /* _windowsystem_H_ */
