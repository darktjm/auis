int MSDebugging=0;
const char ProgramName[]="testdate";

main(argc,argv)
int	argc;
char	**argv;
{
    if(strcmp(*++argv,"-u")==0)
	puts(date64_unparse(*++argv));
    else
	puts(date64_parse(*argv));
}
