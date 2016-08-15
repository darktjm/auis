/* C++ified by hand !@#%&@#$ */

#ifndef _msshrprotos_H_
#define _msshrprotos_H_ 1

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

/* C++ prototypes for functions in libmsshr.a */

/* utils.c */
char *StripWhiteEnds(char *string);
const char *SkipWhiteEnds(const char *string, int *rlen);
void bone(char *buf, int len);
void ReduceWhiteSpace(char *string);
char *multrindex(char *s, char *t);
int BuildNickName(char *FullName, char *NickName);
int lc2strncmp(const char *s1, const char *s2, int len);
void LowerStringInPlace(char *string, int len);
char *NextAddress(char *add);
char *cvEng(int foo, int Capitalized, int MaxToSpellOut);

/* tildes.c */
char *FindUserDir(char *user, char *cellname);

/* findroot.c */
void FindTreeRoot(char *DirName, char *RootName, int ReallyWantParent);

/* brkdown.c */
void BreakDownContentTypeField(char *HeadBuf, char *fmt, int fmtsz, char *vers, int verssz, char *resources, int resourcessz);
char **BreakDownResourcesIntoArray(char *reslist);

ENDCPLUSPLUSPROTOS
 
#endif /* _msshrprotos_H_ */
