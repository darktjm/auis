#ifndef _pushbutton_H_
#define _pushbutton_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblink
 * @{ */
/*
  Ooops.  I screwed up.

  It is the insets responsibility to read through their \enddata's.
  The old version of this code didn't.  This one does.
  Problem is, I changed the datastream representation that tickles this
  bug in the old version of this code.

  This means that if I were to release this code, with the new
  datastream, now, I will cause old code to break (badly) when reading
  documents created with the new code.

  So, for patch level 8, which the world will get (particularly CMU
  campus), the nifty color stuff has no UI, and the old datastream is
  written.  (But both the old and the new datastreams will be supported.)

  For patch level 9, undefine "PL8", and the UI to the new features will
  be available, and the new datastream will be written.  This means that
  people running PL8 will be able to read documents created using PL9 
  features.

  NOTE: users who save PL9-created documents under PL8-code will
  keep information, since the PL8 code will write the new datastream,
  if it reads an inset with the new datastream!

  So, remember inset designers, slurp up those \enddata's!
*/
/* #define PL8 */ /* enable color */

/*
  Appearances for pushbutton:

  Style  Look
    0    plain, in line text
    1    a boxed rectangle
    2    a motif-ish 3-Dish button
    3    A mac-ish rounded rectangle box
    4    a single rectangle
*/
#define pushbutton_PLAIN 0
#define pushbutton_BOXEDRECT 1
#define pushbutton_THREEDEE 2
#define pushbutton_ROUNDRECT 3
#define pushbutton_PLAINBOX 4
#define pushbutton_MOTIF 5

#include <dataobject.H>
class pushbutton: public  dataobject {
public:
	TRACED_CLASS(pushbutton);

  
    
	static boolean InitializeClass();

    
	pushbutton();
    
  
     long Read (FILE *fp, long id)  ;
     long Write (FILE *fp, long id, int level)  ;
  
    virtual void SetText(const char *txt);
    virtual void SetStyle(int stylecode);
    virtual void SetButtonFont(class fontdesc *f);
    virtual void SetFGColor(char *name, int red, int green, int blue); /* if name is NULL, use RGB */
    virtual void SetBGColor(char *name, int red, int green, int blue);
    virtual char * GetFGColor(unsigned char *rgb_vect)  ;
    virtual char * GetBGColor(unsigned char *rgb_vect)  ;
    virtual void ParseRGB(char *rgb_string, unsigned char *rgb_vect);
  

    inline const char * GetText() { return (this->text); }
    inline const char * GetSafeText() { return this->text?this->text:"Push Me"; }
    inline int GetStyle() { return (this->style); }
    inline class fontdesc * GetButtonFont() { return (this->myfontdesc); }
  
    char *text;
    int style;
    class fontdesc *myfontdesc;
    char *foreground_name, *background_name;
    unsigned char foreground_color[3], background_color[3];
#ifdef PL8
    int new_DS;
#endif /* PL8 */

};
/** @} */
#endif /* _pushbutton_H_ */
