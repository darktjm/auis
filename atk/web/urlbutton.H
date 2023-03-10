#ifndef _urlbutton_H_
#define _urlbutton_H_
/** \addtogroup libhtmltext
 * @{ */
/*
 * urlbutton
 *
 * Dataobject for an button that refers to a URL.
 *
   (c) Copyright IBM Corp 1995.  All rights reserved. 
 */

#include <sbutton.H>

class urlbutton : public sbutton {
  public:
    TRACED_CLASS(urlbutton);
    urlbutton();
    static boolean InitializeClass();

    // overrides
    long Read(FILE *file, long id);
    long Write(FILE *file, long writeid, int level);
    const char *ViewName()  ;
    boolean EnsureSize(int ind)  ;
    virtual void SetURL(char *url);
    virtual void SetText(char *txt);		/* Text for user to read (i.e the label) */

    // new interface
    inline char	*GetURL()	 { return (current_url); }
    inline const char *GetURLLabel() { return GetLabel(0); }

  private:
    char *current_url;
    boolean has_label;
};
/** @} */
#endif /* _urlbutton_H_ */
