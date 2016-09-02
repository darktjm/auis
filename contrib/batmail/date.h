#define date64_CREATION    "000000"

#define date64_Cmp(d1,d2) strncmp(d1,d2,AMS_DATESIZE)
#define date64_IsCreation(d) (*(d)=='\0' || strcmp(d,date64_CREATION)==0)

char	*date64_Parse(), *date64_Unparse();
