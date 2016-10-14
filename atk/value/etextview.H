#ifndef _etextview_H_
#define _etextview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <textview.H>
class etextview: public textview {
public:
	TRACED_CLASS_NODEST(etextview);


     void ReceiveInputFocus();
     void LoseInputFocus();

    inline void SetClearOnRIF(boolean val) { this->ClearOnRIF = val; }
    inline void SetResetOnLIF(boolean val) { this->ResetOnLIF = val; }
    inline void SetValueview(class valueview *val) { this->valueview = val; }

    
	etextview();
    
	static boolean InitializeClass();


    class keystate *keystate;
    boolean ClearOnRIF,ResetOnLIF;
    class valueview *valueview;
};
/** @} */
#endif /* _etextview_H_ */
