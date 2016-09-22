/* user code begins here for HeaderInfo */

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

 
/* user code ends here for HeaderInfo */
#include <andrewos.h>
ATK_IMPL("pcontrol.H")

#include <proctable.H>
#include <view.H>
#include <arbiterview.H>
#include <pcontrol.H>
#include <celview.H>
#include <text.H>
#include <controlV.H>
#include <cel.H>
#include <lsetview.H>
#include <lset.H>
#include <value.H>
#include <textview.H>
/* user code begins here for includes */
#include <attribs.h>
#include <sys/ioctl.h>
#include <message.H>
#include <completion.H>
#include <im.H>
#define NUM 4
#define MAXNAMES 40
#define WHOLE 0
#define HALF 1
#define QUARTER 2
#define EIGHTH 3
#define getspeed(self) ((self->duration) ? (8 << ((3 - (self->duration)->GetValue()))) : self->lastnoteval)

#if defined (sys_rt_r3) || defined (sys_rt_aos4)

#include  <machineio/speakerio.h>
#endif /* defined (sys_rt_r3) || defined (sys_rt_aos4) */
struct nh {
    char *note;
    float freq;
};
static float arr[61];

ATKdefineRegistry(pcontrol, ATK, pcontrol::InitializeClass);
static void setbl(struct spk_blk  *b,float  freq);
static void play(char  *buf,int  Speed);
static class pcontrol *FindSelf(class view  *v);
static void replayCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void kbCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void volumeCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void clearCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void modeCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void ntCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void speedCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void ReadCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void noteoutCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void undoCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void restCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void SaveCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2);
static void initself(class pcontrol  *self,class view  *v);
static void pcontrol_start(class view  *v,long  dat);


static void setbl(struct spk_blk  *b,float  freq)
{
#if defined (sys_rt_r3) || defined (sys_rt_aos4)
    if (freq < 23) {
	b->freqhigh=0;
	b->freqlow=SPKOLOMIN;
    } else if (freq < 46) {
	b->freqhigh=64;
	b->freqlow = (char) ((6000.0 /(float) freq) - 9.31);
    } else if (freq < 91) {
	b->freqhigh=32;
	b->freqlow = (char) ((12000.0 /(float) freq) - 9.37);
    } else if (freq < 182) {
	b->freqhigh=16;
	b->freqlow = (char) ((24000.0 /(float) freq) - 9.48);
    } else if (freq < 363) {
	b->freqhigh=8;
	b->freqlow = (char) ((48000.0 /(float) freq) - 9.71);
    } else if (freq < 725) {
	b->freqhigh=4;
	b->freqlow = (char) ((96000.0 /(float) freq) - 10.18);
    } else if (freq < 1433) {
	b->freqhigh=2;
	b->freqlow = (char) ((192000.0 /(float) freq) - 11.10);
    } else if (freq < 12020) {
	b->freqhigh=1;
	b->freqlow = (char) ((384000.0 /(float) freq) - 12.95);
    } else {
	b->freqhigh=0;
	b->freqlow=SPKOLOMIN;
    }
#endif /* defined (sys_rt_r3) || defined (sys_rt_aos4) */
}
static int sp = 0;
static void play(char  *buf,int  Speed)
{
#if defined (sys_rt_r3) || defined (sys_rt_aos4)
    char *note;
    int ::v,d;
    float f;
    struct spk_blk  b;
    sscanf(buf,"%d,%d,%f",&::v,&d,&f);
    b.volume = ::v;
    b.duration = d + (d * Speed / 50) ;
    if((note = strrchr(buf,',')) == NULL){
	printf("bad format\n");
	return;
    }
    note++;
    setbl(&b,f);
    write(sp,&b,sizeof(b));
    b.volume = 0;	
    b.duration = 1;
    write(sp,&b,sizeof(b));
#endif /* defined (sys_rt_r3) || defined (sys_rt_aos4) */
}

#define openspk() ((sp = open("/dev/speaker",1)) > 0)
#define closespk() close(sp)

static int masks[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096 };
static const char * const names[] = { "","C","C#","D","D#","E","F","F#","G","G#","A","A#","B","C" };
/* user code ends here for includes */

static class pcontrol *firstpcontrol;
static class pcontrol *FindSelf(class view  *v)
{
	class pcontrol *self,*last = NULL;
	for(self= firstpcontrol; self != NULL; self = self->next){
		if(self->v == v) return self;
		last = self;
		}
	self = new pcontrol;
	initself(self,v);
	if(last == NULL) firstpcontrol = self;
	else last->next = self;
	return self;
}
static void replayCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for replayCallBack */
    char buf[256],*cp;
    int i,len;
    int Speed =  (self->speed)->GetValue();
    if(self->score == NULL || !openspk()) return;
    len = (self->score)->GetLength();
    for(i = 0, cp = buf; i < len; i++){
	*cp = (self->score)->GetChar(i);
	if(*cp == '\n'){
	    *cp = '\0';
	    play (buf,Speed);
	    cp = buf;
	}
	else cp++;
    }
    closespk();
/* user code ends here for replayCallBack */
}
static void kbCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for kbCallBack */
    char buf[64];
    long w,i,v;
    int Speed =  (self->speed)->GetValue();
    if(self->score == NULL) return;
    w = (val)->GetValue();
    if(w == 0 || !openspk()) return;
    for(i = 1; i < 13; i++){
	if(masks[i] & w){
	    v = i + 2 + (r1 * 12);
	    sprintf(buf,"%ld,%ld,%f %s",(self->volume)->GetValue(),
		    getspeed(self),arr[v],
		    names[i]);
	    play(buf,Speed);
	    if((self->mode)->GetValue()){
		self->lastlen = (self->score)->GetLength();
		strcat(buf,"\n");
		(self->score)->InsertCharacters(self->lastlen,buf,strlen(buf));
		(self->score)->NotifyObservers(0);
	    }
	}
    }
    (val)->SetValue(0);
    closespk();
/* user code ends here for kbCallBack */
}
static void volumeCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for volumeCallBack */
/* user code ends here for volumeCallBack */
}
static void clearCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for clearCallBack */
    (self->score)->Clear();
    (self->score)->NotifyObservers(0);
/* user code ends here for clearCallBack */
}
static void modeCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for modeCallBack */
/* user code ends here for modeCallBack */
}
static void ntCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for ntCallBack */
    static int len[] = { 96,64,50,32,24,16,12,8,6,4,3,2};
    static char s[2];
    self->lastnoteval = len[r1];
    s[0] = '@' + r1;
    s[1] = '\0';
    (self->noteout)->SetString(s);
/* user code ends here for ntCallBack */
}
static void speedCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for speedCallBack */
/* user code ends here for speedCallBack */
}
static void ReadCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for ReadCallBack */

    char filename[1024];
    FILE *file;

    if (im::GetDirectory(filename) != NULL) /* get the current directory */
	strcat(filename, "/");
    if (completion::GetFilename(self->v, "file to read: ", filename, filename, sizeof(filename), FALSE, TRUE) == -1 )
	return;
    if((file = fopen(filename,"r")) != NULL){
	(self->score)->Clear();
	(self->score)->Read(file,0);
	(self->score)->NotifyObservers(0);
	message::DisplayString(self->v,0,"Done");
    }
    else{
	message::DisplayString(self->v,0,"Can't write file");
    }
/* user code ends here for ReadCallBack */
}
static void noteoutCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for noteoutCallBack */
/* user code ends here for noteoutCallBack */
}
static void undoCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for undoCallBack */
    int i;
    self->lastlen = (self->score)->GetLength();
    for(i = self->lastlen -1 ;--i > 0 ;){
	if((self->score)->GetChar(i) == '\n'){
	    i++;
	    (self->score)->DeleteCharacters(i,self->lastlen - i);
	    (self->score)->NotifyObservers(0);
	    break;
	}
    }
/* user code ends here for undoCallBack */
}
static void restCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for restCallBack */
    char buf[64];
    sprintf(buf,"0,%ld,0.0 rest",
	     getspeed(self));
/*    play(buf, value_GetValue(self->speed)); */
    if((self->mode)->GetValue()){
	self->lastlen = (self->score)->GetLength();
	strcat(buf,"\n");
	(self->score)->InsertCharacters(self->lastlen,buf,strlen(buf));
	(self->score)->NotifyObservers(0);
    }
/* user code ends here for restCallBack */
}
static void SaveCallBack(class pcontrol  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for SaveCallBack */

    char filename[1024];
    FILE *file;

    if (im::GetDirectory(filename) != NULL) /* get the current directory */
	strcat(filename, "/");
    if (completion::GetFilename(self->v, "file to save: ", filename, filename, sizeof(filename), FALSE, FALSE) == -1 )
	return;
    if((file = fopen(filename,"w")) != NULL){
	(self->score)->Write(file,im::GetWriteID(),0);
	fclose(file);
	message::DisplayString(self->v,0,"Wrote song");
    }
    else{
	message::DisplayString(self->v,0,"Can't write file");
    }
/* user code ends here for SaveCallBack */
}
static void initself(class pcontrol  *self,class view  *v)
{
	self->v = v;
	self->kb_2View = (class pianoV *)arbiterview::GetNamedView(v,"kb-2");
	self->kb_2 = (class value *)arbiterview::GetNamedObject(v,"kb-2");
	if(self->kb_2) (self->kb_2)->AddCallBackObserver( self,(value_fptr)kbCallBack,2);
	self->kb_3View = (class pianoV *)arbiterview::GetNamedView(v,"kb-3");
	self->kb_3 = (class value *)arbiterview::GetNamedObject(v,"kb-3");
	if(self->kb_3) (self->kb_3)->AddCallBackObserver( self,(value_fptr)kbCallBack,3);
	self->replayView = (class buttonV *)arbiterview::GetNamedView(v,"replay");
	self->replay = (class value *)arbiterview::GetNamedObject(v,"replay");
	if(self->replay) (self->replay)->AddCallBackObserver( self,(value_fptr)replayCallBack,0);
	self->volumeView = (class fourwayV *)arbiterview::GetNamedView(v,"volume");
	self->volume = (class value *)arbiterview::GetNamedObject(v,"volume");
	if(self->volume) (self->volume)->AddCallBackObserver( self,(value_fptr)volumeCallBack,0);
	self->nt_10View = (class buttonV *)arbiterview::GetNamedView(v,"nt-10");
	self->nt_10 = (class value *)arbiterview::GetNamedObject(v,"nt-10");
	if(self->nt_10) (self->nt_10)->AddCallBackObserver( self,(value_fptr)ntCallBack,10);
	self->clearView = (class buttonV *)arbiterview::GetNamedView(v,"clear");
	self->clear = (class value *)arbiterview::GetNamedObject(v,"clear");
	if(self->clear) (self->clear)->AddCallBackObserver( self,(value_fptr)clearCallBack,0);
	self->nt_11View = (class buttonV *)arbiterview::GetNamedView(v,"nt-11");
	self->nt_11 = (class value *)arbiterview::GetNamedObject(v,"nt-11");
	if(self->nt_11) (self->nt_11)->AddCallBackObserver( self,(value_fptr)ntCallBack,11);
	self->nt_0View = (class buttonV *)arbiterview::GetNamedView(v,"nt-0");
	self->nt_0 = (class value *)arbiterview::GetNamedObject(v,"nt-0");
	if(self->nt_0) (self->nt_0)->AddCallBackObserver( self,(value_fptr)ntCallBack,0);
	self->nt_1View = (class buttonV *)arbiterview::GetNamedView(v,"nt-1");
	self->nt_1 = (class value *)arbiterview::GetNamedObject(v,"nt-1");
	if(self->nt_1) (self->nt_1)->AddCallBackObserver( self,(value_fptr)ntCallBack,1);
	self->modeView = (class onoffV *)arbiterview::GetNamedView(v,"mode");
	self->mode = (class value *)arbiterview::GetNamedObject(v,"mode");
	if(self->mode) (self->mode)->AddCallBackObserver( self,(value_fptr)modeCallBack,0);
	self->nt_2View = (class buttonV *)arbiterview::GetNamedView(v,"nt-2");
	self->nt_2 = (class value *)arbiterview::GetNamedObject(v,"nt-2");
	if(self->nt_2) (self->nt_2)->AddCallBackObserver( self,(value_fptr)ntCallBack,2);
	self->nt_3View = (class buttonV *)arbiterview::GetNamedView(v,"nt-3");
	self->nt_3 = (class value *)arbiterview::GetNamedObject(v,"nt-3");
	if(self->nt_3) (self->nt_3)->AddCallBackObserver( self,(value_fptr)ntCallBack,3);
	self->nt_4View = (class buttonV *)arbiterview::GetNamedView(v,"nt-4");
	self->nt_4 = (class value *)arbiterview::GetNamedObject(v,"nt-4");
	if(self->nt_4) (self->nt_4)->AddCallBackObserver( self,(value_fptr)ntCallBack,4);
	self->nt_5View = (class buttonV *)arbiterview::GetNamedView(v,"nt-5");
	self->nt_5 = (class value *)arbiterview::GetNamedObject(v,"nt-5");
	if(self->nt_5) (self->nt_5)->AddCallBackObserver( self,(value_fptr)ntCallBack,5);
	self->nt_6View = (class buttonV *)arbiterview::GetNamedView(v,"nt-6");
	self->nt_6 = (class value *)arbiterview::GetNamedObject(v,"nt-6");
	if(self->nt_6) (self->nt_6)->AddCallBackObserver( self,(value_fptr)ntCallBack,6);
	self->nt_7View = (class buttonV *)arbiterview::GetNamedView(v,"nt-7");
	self->nt_7 = (class value *)arbiterview::GetNamedObject(v,"nt-7");
	if(self->nt_7) (self->nt_7)->AddCallBackObserver( self,(value_fptr)ntCallBack,7);
	self->nt_8View = (class buttonV *)arbiterview::GetNamedView(v,"nt-8");
	self->nt_8 = (class value *)arbiterview::GetNamedObject(v,"nt-8");
	if(self->nt_8) (self->nt_8)->AddCallBackObserver( self,(value_fptr)ntCallBack,8);
	self->nt_9View = (class buttonV *)arbiterview::GetNamedView(v,"nt-9");
	self->nt_9 = (class value *)arbiterview::GetNamedObject(v,"nt-9");
	if(self->nt_9) (self->nt_9)->AddCallBackObserver( self,(value_fptr)ntCallBack,9);
	self->speedView = (class sliderV *)arbiterview::GetNamedView(v,"speed");
	self->speed = (class value *)arbiterview::GetNamedObject(v,"speed");
	if(self->speed) (self->speed)->AddCallBackObserver( self,(value_fptr)speedCallBack,0);
	self->ReadView = (class buttonV *)arbiterview::GetNamedView(v,"Read");
	self->Read = (class value *)arbiterview::GetNamedObject(v,"Read");
	if(self->Read) (self->Read)->AddCallBackObserver( self,(value_fptr)ReadCallBack,0);
	self->noteoutView = (class stringV *)arbiterview::GetNamedView(v,"noteout");
	self->noteout = (class value *)arbiterview::GetNamedObject(v,"noteout");
	if(self->noteout) (self->noteout)->AddCallBackObserver( self,(value_fptr)noteoutCallBack,0);
	self->undoView = (class buttonV *)arbiterview::GetNamedView(v,"undo");
	self->undo = (class value *)arbiterview::GetNamedObject(v,"undo");
	if(self->undo) (self->undo)->AddCallBackObserver( self,(value_fptr)undoCallBack,0);
	self->scoreView = (class textview *)arbiterview::GetNamedView(v,"score");
	self->score = (class text *)arbiterview::GetNamedObject(v,"score");
	self->ab1View = (class arbiterview *)arbiterview::GetNamedView(v,"ab1");
	self->ab1 = (class arbiter *)arbiterview::GetNamedObject(v,"ab1");
	self->ab2View = (class lsetview *)arbiterview::GetNamedView(v,"ab2");
	self->ab2 = (class lset *)arbiterview::GetNamedObject(v,"ab2");
	self->restView = (class buttonV *)arbiterview::GetNamedView(v,"rest");
	self->rest = (class value *)arbiterview::GetNamedObject(v,"rest");
	if(self->rest) (self->rest)->AddCallBackObserver( self,(value_fptr)restCallBack,0);
	self->kb_0View = (class pianoV *)arbiterview::GetNamedView(v,"kb-0");
	self->kb_0 = (class value *)arbiterview::GetNamedObject(v,"kb-0");
	if(self->kb_0) (self->kb_0)->AddCallBackObserver( self,(value_fptr)kbCallBack,0);
	self->SaveView = (class buttonV *)arbiterview::GetNamedView(v,"Save");
	self->Save = (class value *)arbiterview::GetNamedObject(v,"Save");
	if(self->Save) (self->Save)->AddCallBackObserver( self,(value_fptr)SaveCallBack,0);
	self->kb_1View = (class pianoV *)arbiterview::GetNamedView(v,"kb-1");
	self->kb_1 = (class value *)arbiterview::GetNamedObject(v,"kb-1");
	if(self->kb_1) (self->kb_1)->AddCallBackObserver( self,(value_fptr)kbCallBack,1);
}
static void pcontrol_start(class view  *v,long  dat)
 {
class pcontrol *self;
if((self = FindSelf(v)) == NULL) return;
/* user code begins here for pcontrol_start */
arbiterview::SetIgnoreUpdates(v, TRUE);
self->duration = (class value *)arbiterview::GetNamedObject(v,"duration");
/* user code ends here for pcontrol_start */
}
boolean pcontrol::InitializeClass()
{
struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");
firstpcontrol = NULL;
proctable::DefineProc("pcontrol-start",(proctable_fptr)pcontrol_start, viewtype,NULL,"pcontrol start");
/* user code begins here for InitializeClass */
#if defined (sys_rt_r3) || defined (sys_rt_aos4)
    int i;
/*
    if((sp = open("/dev/speaker",1)) < 0) {
	printf("can't open speaker\n");
	return FALSE;
    }
    ioctl(sp,TIOCNXCL,NULL);
*/
    arr[60] = 1760.0 * 4.0;
    for(i = 59; i >= 0; i--){
	arr[i] = arr[i + 1] - (arr[i + 1] / 17.817);
	/*	  printf("\"%s\",%4.2f,\n",notes[12 - (i %12)],arr[i]); */
    }
#endif /* defined (sys_rt_r3) || defined (sys_rt_aos4) */
#if !( defined (sys_rt_r3) || defined (sys_rt_aos4) )
    return FALSE;
#else /* defined (sys_rt_r3) || defined (sys_rt_aos4) */
/* user code ends here for InitializeClass */
return TRUE;
#endif /* defined (sys_rt_r3) || defined (sys_rt_aos4) */
}
pcontrol::pcontrol()
{
	ATKinit;

this->kb_2 = NULL;
this->kb_2View = NULL;
this->kb_3 = NULL;
this->kb_3View = NULL;
this->replay = NULL;
this->replayView = NULL;
this->volume = NULL;
this->volumeView = NULL;
this->nt_10 = NULL;
this->nt_10View = NULL;
this->clear = NULL;
this->clearView = NULL;
this->nt_11 = NULL;
this->nt_11View = NULL;
this->nt_0 = NULL;
this->nt_0View = NULL;
this->nt_1 = NULL;
this->nt_1View = NULL;
this->mode = NULL;
this->modeView = NULL;
this->nt_2 = NULL;
this->nt_2View = NULL;
this->nt_3 = NULL;
this->nt_3View = NULL;
this->nt_4 = NULL;
this->nt_4View = NULL;
this->nt_5 = NULL;
this->nt_5View = NULL;
this->nt_6 = NULL;
this->nt_6View = NULL;
this->nt_7 = NULL;
this->nt_7View = NULL;
this->nt_8 = NULL;
this->nt_8View = NULL;
this->nt_9 = NULL;
this->nt_9View = NULL;
this->speed = NULL;
this->speedView = NULL;
this->Read = NULL;
this->ReadView = NULL;
this->noteout = NULL;
this->noteoutView = NULL;
this->undo = NULL;
this->undoView = NULL;
this->score = NULL;
this->scoreView = NULL;
this->ab1 = NULL;
this->ab1View = NULL;
this->ab2 = NULL;
this->ab2View = NULL;
this->rest = NULL;
this->restView = NULL;
this->kb_0 = NULL;
this->kb_0View = NULL;
this->Save = NULL;
this->SaveView = NULL;
this->kb_1 = NULL;
this->kb_1View = NULL;
this->v = NULL;
this->next = NULL;
/* user code begins here for InitializeObject */
this->lastlen = 0;
this->lastnoteval = 16;
this->duration = NULL;
/* user code ends here for InitializeObject */
THROWONFAILURE( TRUE);}
/* user code begins here for Other Functions */
/* user code ends here for Other Functions */
