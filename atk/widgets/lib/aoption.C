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

#include <andrewos.h>

#include <alist.H>
#include <aoption.H>
#include <text.H>
#include <style.H>
#include <environment.H>
#include <aoptionv.H>
#include <flex.H>

static boolean Init() {
    return AOptionMenuv::Init();
}

ATKdefineRegistry(AOptionMenu,AButton,Init);

AOptionMenu::AOptionMenu() : buttonTextAct(&ButtonText, this) {
    ATKinit;
#ifdef AOPTION_GIVES_ALIST_INTERFACE
    TSLOTINIT(selectedItemCount,1L);
    TSLOTINIT(selectedItems,(void *)&item);
    item.ind=1;
    item.pos=0;
    item.len=0;
#endif
    text.SetFormula(&buttonTextAct);
    SLOTINIT(source, NULLATK);
    SLOTINIT(selected,0);
}


void AOptionMenu::Insert(long ind, const char *str) {
    class text *txt=Source();
    if(txt==NULL) return;
    long pos=txt->GetPosForLine(ind);
    long len=strlen(str);
    txt->AlwaysInsertCharacters(pos, (char *)str, len);
    txt->AlwaysInsertCharacters(pos+len, "\n", 1);
}

void AOptionMenu::Remove(long ind) {
    class text *txt=Source();
    if(txt==NULL) return;
    long pos=txt->GetPosForLine(ind);
    long epos=txt->Index(pos, '\n', txt->GetLength()-pos);
    if(epos<0) epos=txt->GetLength();
    txt->AlwaysDeleteCharacters(pos, epos-pos+1);
}

const char *AOptionMenu::Item(long ind) {
    class text *txt=Source();
    long pos=txt->GetPosForLine(ind);
    long epos=txt->Index(pos, '\n', txt->GetLength()-pos);
    if(epos<0) epos=txt->GetLength();
    static flex buf;
    buf.Remove((size_t)0,buf.GetN());
    buf.Insert((size_t)0, (size_t)(epos-pos));
    size_t dummy;
    char *p=buf.GetBuf((size_t)0, (size_t)(epos-pos+1), &dummy);
    txt->CopySubString(pos, epos-pos, p, FALSE);
    p[epos+pos]='\0';
    return p;
}

void AOptionMenu::ButtonText(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    AOptionMenu *ao=(AOptionMenu *)aux[0].ATKObject();
    class text *t=ao->Source();
    long line=ao->Selected();
    if(t) {
        long lpos=t->GetPosForLine(line);
        long epos=t->GetEndOfLine(lpos);
        static flex buf;
        buf.Remove((size_t)0, buf.GetN());
        char *cbuf=buf.Insert((size_t)0, (size_t)(epos-lpos+1));
        if(cbuf==NULL) return;
        t->CopySubString(lpos, epos-lpos, cbuf, FALSE);
        cbuf[epos-lpos]='\0';
        out.add(cbuf);
    } else out.add("");
}

void AOptionMenu::NotifyObservers(long change) {
    class text *t=Source();
    if(t) t->NotifyObservers(change);
}
