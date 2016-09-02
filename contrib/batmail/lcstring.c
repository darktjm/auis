#include <ctype.h>

#define lowerit(c) (isupper(c)?tolower(c):(c))

int	lcstrcmp(str1,str2)
char	*str1,*str2;
{
    while(*str1 && *str2){
	char	c1=lowerit(*str1),c2=lowerit(*str2);

	if(c1!=c2)
	    return c1-c2;

	str1++;
	str2++;
    }

    return *str1-*str2;
}
