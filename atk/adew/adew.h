/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#ifndef adew_ReadShort

typedef void (*adew_cbfptr)(class ATK *, class value *, long r1, long r2);
struct adew_array {
    class dataobject **object;
    class view **view;
    class cel *cel;
    class atom *name;
    adew_cbfptr callback;
    long rock;
};
#define adew_InitializeApplication(SELF,ARRAY) { struct adew_array *aa ;\
for(aa = ARRAY; aa->object != NULL;aa++){\
    *(aa->object) = NULL;\
    *(aa->view) = NULL;\
    aa->cel = NULL;\
    }\
};
#define adew_FinalizeApplication(SELF,ARRAY) { struct adew_array *aa ;\
for(aa = ARRAY; aa->object != NULL;aa++){\
    if(aa->callback) ((class value *)*(aa->object))->RemoveCallBackObserver(SELF);\
    }\
};
#define adew_NoteDestroyed(SELF,OB,ARRAY){ struct adew_array *aa ;\
for(aa = ARRAY; aa->object != NULL;aa++){\
    if((class observable *)(*(aa->object)) == OB){\
	*(aa->object) = NULL; aa->cel = NULL;}\
    else if((class observable *)(*(aa->view)) == OB) *(aa->view) = NULL;\
    }\
};
#define adew_InitApplicationCel(SELF,CL,CLV,ARRAY,ICNT){ class cel *cel = NULL;\
    struct adew_array *aa ;class atom *nm;\
    if(CL) cel = CL;\
    else if(CLV) cel = (class cel *) (cv)->GetDataObject();\
    if(cel != NULL) { \
      nm = (cel)->GetRefAtom();\
      for(aa = ARRAY; aa->object != NULL;aa++){\
       if(aa->name == nm){\
        if(CLV){ \
          if(((class celview *)CLV)->GetTrueChild() == NULL) ((class celview *)CLV)->InitChildren();\
           *(aa->view) = ((class celview *)CLV)->GetTrueChild();\
	   (*(ICNT))--;}\
        if(*(aa->object) != (cel)->GetObject()){\
            *(aa->object) = (cel)->GetObject();\
            aa->cel = cel;\
            if(aa->callback)((class value *) *(aa->object))->AddCallBackObserver( SELF,aa->callback,aa->rock);\
	    }\
        break;\
         }\
      }\
    }\
};
#define adew_InitDataObjects(SELF,ARRAY){ class cel *ccl; class arbiter *aab;int dummy;\
aab = (class arbiter *)  (SELF)->view::dataobject;\
for(ccl = (aab)->GetFirst(); ccl != (class cel *)aab; ccl = (ccl)->GetNextChain()){\
    adew_InitApplicationCel(SELF,ccl,NULL,ARRAY,&dummy);}}


#define adew_ReadShort(A,F,CNT) if(CNT-- > 0) fscanf(F,"%d\n",&(A))
#define adew_ReadLong(A,F,CNT) if(CNT-- > 0) fscanf(F,"%ld\n",&(A))
#define adew_ReadString(A,F,CNT)  if(CNT-- > 0){int ii; char *c;\
	 fscanf(F,"#%d\n",&ii);c = (A);\
	while(ii-- > 0) {*c++ = getc(F);}\
	if((*c = getc(F))!= '\n') ungetc(*c,F); \
        *c = '\0';\
}
#define adew_ReadFloat(A,F,CNT)  if(CNT-- > 0) fscanf(F,"%f\n",&(A))
#define adew_ReadStringArray(A,F,CNT,SIZE) {int inc; for(inc = 0; inc < SIZE; inc++) adew_ReadString((A)[inc],F,CNT);}
#define adew_ReadLongArray(A,F,CNT,SIZE) {int inc; for(inc = 0; inc < SIZE; inc++) adew_ReadLong((A)[inc],F,CNT);}

#define adew_DiscardString(F) {int ii; \
	fscanf(F,"%d ",&ii);\
	while(ii-- > 0) getc(F);\
	getc(F);\
}
#define adew_ReadJunk(F) {char c; c = getc(F); if(c != '#'){ while(c != '\n') c = getc(F); }\
else adew_DiscardString(F) }
#define adew_ReadExtraData(F,CNT) { while (CNT-- > 0) adew_ReadJunk(F) }

#define adew_WriteShort(A,F) fprintf(F,"%d\n",(A))
#define adew_WriteLong(A,F) fprintf(F,"%ld\n",(A))
#define adew_WriteString(A,F)  fprintf(F,"#%d\n%s\n",strlen(A),(A))
#define adew_WriteFloat(A,F)  fprintf(F,"%f\n",(A))
#define adew_WriteStringArray(A,F,SIZE) {int inc; for(inc = 0; inc < SIZE; inc++) adew_WriteString((A)[inc],F);}
#define adew_WriteLongArray(A,F,SIZE) {int inc; for(inc = 0; inc < SIZE; inc++) adew_WriteLong((A)[inc],F);}

#define adew_InitShort(A) (A) = 0
#define adew_InitLong(A) (A) = 0L
#define adew_InitString(A) (*(A)) = '\0'
#define adew_InitFloat(A) (A) = 0.0
#define adew_InitStringArray(A,SIZE) {int inc; for(inc = 0; inc < SIZE; inc++) adew_InitString((A)[inc]);}
#define adew_InitLongArray(A,SIZE) {int inc; for(inc = 0; inc < SIZE; inc++) adew_InitLong((A)[inc]);}
#endif
