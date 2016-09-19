#ifndef _prochook_H_
#define _prochook_H_
ATK_INTER
/* Copyright 1995 Carnegie Mellon University All rights reserved.
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

#include <aaction.H>
#include <atom.H>

class ness_ProcHookType_Base : public aaction {
	void (*func)(ATK *o, const avalueflex &aux, const avalueflex &in, avalueflex &out);
	avalueflex auxargs;
	avalueflex intypes, outtypes;
  public:
	inline ness_ProcHookType_Base(void (*p)(ATK *o, const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a) : func(p), auxargs(a) {
    	}
    inline ness_ProcHookType_Base(void (*p)(ATK *o, const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a, const avalueflex &t, const avalueflex &r) :
    func(p),
    auxargs(a),
    intypes(t),
    outtypes(r) {
    	}
	inline virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out) {
		func(((ATK *)obj), auxargs, in, out);
	}
	inline virtual avalueflex &ArgTypes() {
		return intypes;
	}
	inline virtual avalueflex &RetTypes() {
		return outtypes;
	}
};

// DEFINE_AACTION_FUNC_CLASS(ness_ProcHookType_Base,ATK);
class ness_ProcHookType : public ness_ProcHookType_Base {
  public:
    inline ness_ProcHookType(void (*p)(ATK *o, const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a) : ness_ProcHookType_Base(p, a){ 
    	} 
    inline ness_ProcHookType(void (*p)(ATK *o, const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a, const avalueflex &t, const avalueflex &r) : ness_ProcHookType_Base(p, a, t, r) { 
    }
    boolean TypeCheck(const avalueflex &in);
    inline virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out) {
	if(!TypeCheck(in)) {
	    out.add("Type Mismatch", atom_def("error"));
	    fprintf(stderr, "Type mismatch in call to proctable ness function...\n");
	    return;
	}
	ness_ProcHookType_Base::operator()(obj,in,out);
    } 

};

NO_DLL_EXPORT extern ness_ProcHookType ness_ProcHook;
#endif
