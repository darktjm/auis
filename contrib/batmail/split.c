#include <com.h>

char *strchr(),  *strrchr();

	char *
split(s,c)
	char *s,c;
{
    char *p=strchr(s,c);
    
    if(p!=NULL)
	*p++='\0';

    return p;
}

	char *
rsplit(s,c)
	char *s,c;
{
    char *p=strrchr(s,c);

    if(p!=NULL)
	*p++='\0';

    return p;
}
