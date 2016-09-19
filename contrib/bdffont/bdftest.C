#include <andrewos.h>
#include <environ.H>
#include <bdffont.H>


int main(int  argc, char  **argv)
		{
	class bdffont *bdf;
	FILE *fin = fopen(argv[1], "r");
	FILE *fout = fopen("/tmp/f", "w");


	bdf = new bdffont;
	parse::SetDebug(TRUE);
	(bdf)->Read( fin, 0);
	(bdf)->Write( fout, 0, 0);
}
