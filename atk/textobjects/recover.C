/* File recover.C created by Robert Andrew Ryan at 11:12:46 on Mon Nov  7 1994. */

/* ********************************************************************** *\
 *    Copyright Carnegie Mellon University, 1993 - All rights reserved    *
\* ********************************************************************** */

/* A program to attempt automatic recovery of data in unknown insets in any ATK document. */

#include <andrewos.h>
#include <text.H>
#include <search.H>

static const char umatch[]="\\begindata{unknown,";
static const char uematch[]="\\enddata{unknown,";
static const char bmatch[]="\\begindata{";

static void FatalInternal(int err) {
    fprintf(stderr, "Internal error in updating object id.\nThe output file is corrupted and incomplete.\n");
    exit(err);
}

int main (int argc, char *argv[])
{
    if(argc>1) {
	fprintf(stderr, "usage: recover<original-file>corrected-file\nThis program attempts to recover a document which contained insets unknown\nby a previous version of AUIS used to modify the document.\nThe original file and the corrected file should have the same extension.\nReading the corrected file into an AUIS application may crash the application.\nIt is recommended that immediately after running recover you\attempt to edit the document in a new process.");
	exit(0);
    }
    class text *t=new text;
    if(t==NULL) {
	fprintf(stderr, "Out of memory!\n");
	exit(-1);
    }
    if(t->Read(stdin,0)!=dataobject::NOREADERROR) {
	fprintf(stderr, "Couldn't read original file.\n");
	exit(-2);
    }

    class search pat, pat2;
    const char *err1= pat.CompilePattern("\\\\begindata{unknown,[0-9]*}\n");
    const char *err2= pat2.CompilePattern("\\\\enddata{unknown,[0-9]*}\n");
    if(err1) {
	fprintf(stderr, "Internal error in pattern match for unknown begindata.\nError was:%s",err1);
	exit(-4);
    }
    if(err2) {
	fprintf(stderr, "Internal error in pattern match for unknown enddata.\nError was:%s",err2);
	exit(-4);
    }

    long pos=0;
    while(pos<t->GetLength()) {
	pos=pat.MatchPattern(t, pos);
	if(pos<0) break;

	long len=pat.GetMatchLength();
	if(t->Strncmp(pos, umatch, sizeof(umatch)-1)!=0 || 
t->Strncmp(pos+len, bmatch, sizeof(bmatch)-1)!=0) {
	    pos+=len;
	    continue;
	}
	
	static char *buf=NULL;
	static int bufsize=0;

	if(buf==NULL) {
	    buf=(char *)malloc(len+1);
	    bufsize=len+1;
	} else if(bufsize<len+1) {
	    buf=(char *)realloc(buf, len+1);
	    bufsize=len+1;
	}
	if(buf==NULL) {
	    FatalInternal(-11);
	}
	t->CopySubString(pos+sizeof(umatch)-1, len-sizeof(umatch)+1, buf, FALSE);

	char *p=strchr(buf, '}');
	if(p) *p='\0';
	else {
	    FatalInternal(-5);
	}
	t->AlwaysDeleteCharacters(pos, len);

	long pos2=t->Index(pos, '}', t->GetLength()-pos);

	if(pos2<0) {
	    FatalInternal(-7);
	}

	char *ebuf=(char *)malloc(pos2-pos+2); // 2 is pos2-pos would be enough for the first char +1 for the pos2 char +1 for the NUL

	if(ebuf==NULL) {
	    FatalInternal(-12);
	}
	strcpy(ebuf, "\\end");
	t->CopySubString(pos+6, pos2-pos+1-6, ebuf+4, FALSE);

	char *pstr=search::GetQuotedSearchString(ebuf, NULL, 0);
	if(pstr==NULL) {
	    FatalInternal(-13);
	}
	class search pat3;
	const char *err3=pat3.CompilePattern(pstr);
	if(err3) {
	    fprintf(stderr, "Fatal internal errror.\nError was:%s\n", err3);
	    exit(-14);
	}

	boolean foundend=FALSE;
	long pos3=pos2;
	while(pos3<t->GetLength()) {
	    pos3=pat3.MatchPattern(t, pos3);
	    if(pos3<0) {
		FatalInternal(-15);
	    }
	    if(t->Strncmp(pos3, ebuf, pos2-pos+1-6+4)==0) {
		foundend=TRUE;
		break;
	    }
	    pos3+=pos2-pos+1;
	}

	pos=t->Index(pos, ',', t->GetLength()-pos);
	if(pos<0) {
	    FatalInternal(-6);
	}

	len=pos2-pos-1;
	pos++;
	if(len<0) {
	    FatalInternal(-8);
	}

	if(foundend) {
	    long pos4;
	    pos3=t->Index(pos3, ',', t->GetLength()-pos3);
	    if(pos3<0) FatalInternal(-16);
	    pos4=t->Index(pos3, '}', t->GetLength()-pos3);
	    if(pos4<0) FatalInternal(-17);
	    t->AlwaysReplaceCharacters(pos3+1, pos4-pos3-1, buf, strlen(buf));
	} else FatalInternal(-18);

	free(ebuf);
	free(pstr);
	
	t->AlwaysReplaceCharacters(pos, len, buf, strlen(buf));

	
	pos=pos2;
	foundend=FALSE;
	while(pos2<t->GetLength()) {
	    pos2=pat2.MatchPattern(t, pos2);
	    if(pos2<0) break;
	    len=pat2.GetMatchLength();
	    if(t->Strncmp(pos2, uematch, sizeof(uematch)-1)!=0) {
		pos2+=len;
		continue;
	    }
	    if(t->Strncmp(pos2+sizeof(uematch)-1, buf, strlen(buf))!=0) continue;
	    foundend=TRUE;
	    t->AlwaysDeleteCharacters(pos2, len);
	    
	    break;
	}
	if(foundend==FALSE) {
	    FatalInternal(-9);
	}
    }
    
    t->WriteSubString(0, t->GetLength(), stdout,FALSE);
    if(ferror(stdout)!=0 || fclose(stdout)!=0) {
	fprintf(stderr, "Couldn't write the updated file,\n");
	exit(-10);
    }
    return 0;
}

