#ifndef _metextview_H_
#define _metextview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <textview.H>
class metextview: public textview {
public:
	TRACED_CLASS_NODEST(metextview);


     void ReceiveInputFocus();
     void LoseInputFocus();
     void ObservedChanged (class observable *changed, long value);

    inline void SetClearOnRIF(boolean val) { this->ClearOnRIF = val; }
    inline void SetResetOnLIF(boolean val) { this->ResetOnLIF = val; }
    inline void SetValueview(class valueview *val) { this->valueview = val; }

    
	metextview();
    
	static boolean InitializeClass();


    class keystate *keystate;
    boolean ClearOnRIF,ResetOnLIF;
    class valueview *valueview;
};
/** @} */
#endif /* _metextview_H_ */
