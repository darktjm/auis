/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* label.c		

	Code for the label data object

*/

/* sys/types.h in AIX PS2 defines "struct label", causing a type name clash.
   Avoid this by temporarily redefining "label" to be something else in the preprocessor. */
#define label gezornenplatz
#include <andrewos.h> /* strings.h */
ATK_IMPL("label.H")
#undef label

#include <fontdesc.H>
#include <label.H>

/**/

ATKdefineRegistryNoInit(label, dataobject);

label::label()
		{
	/* here we give initial values to any fields that need them */
	this->text = NULL;
	this->font = fontdesc::Create("andy", fontdesc_Bold, 10);
	this->flags = 0;
	(this)->SetText( "LABEL");
	THROWONFAILURE( TRUE);
}

	label::~label()
		{
	/* free any storage allocated to self */
	if (this->text) free(this->text);
}

	void
label::SetFlags( int  flags )
                {
        this->flags = flags;
}



	long
label::Read( FILE   *file, long   id			/* !0 if data stream, 0 if direct from file*/ )
			{
	char fontfamily[50];
	long style, size;
	char text[257], *tail;
	char s[255 + 2];
	long c;

	if ((c=getc(file)) == '\n') {}		/* COMPATIBILITY KLUDGE */
	else if (c == '\\') fgets(s, 255+2, file);	/* skip header */
	else ungetc(c, file);

	/* reads a label from -file-.  See file format in label.ch */
	/* This routine reads the \enddata, if any. Its syntax is not checked */

	fscanf(file, " %s %ld %ld ", fontfamily, &style, &size);
	(this)->SetFont( fontfamily, style, size);

	*text = '\0';
	while (TRUE) {
		/* read the lines of the data stream */
		if ((fgets(s, 255 + 2, file)) == 0) 
			/* EOF or error */
			break;
		if (*s == '\\') 
			/* \enddata */
			break;
		if (strlen(text) + strlen(s) <= 255)
			strcat(text, s);		
	}
	tail = text + strlen(text) - 1;		/* point at last character */
	if (*tail == '\n')
		*tail = '\0';				/* delete newline*/
	(this)->SetText( text);
	(this)->NotifyObservers( label_DATACHANGED);
	return dataobject::NOREADERROR;
}
	  
	long
label::Write( FILE   *file, long   writeID, int   level )
		 		{
	char head[50];
	const char *fontfamily;
	long style, size;
	long id = (this)->GetID();
	if (this->GetWriteID() != writeID) {
		/* new instance of write, do it */
		this->SetWriteID(writeID);
		sprintf(head, "data{%s, %ld}\n", (this)->GetTypeName(), id);
		fprintf(file, "\\begin%s", head);

		fontfamily = (this)->GetFont( &style, &size);
		fprintf(file, " %s %ld %ld\n", fontfamily, style, size);
	
		fprintf(file, "%s\n", (this)->GetText());

		fprintf(file, "\\end%s", head);
	}
	return id;
}

	void
label::SetText(const char  *text)
		{
	char *s, *t;
	int length = strlen(text);
	if (length > 255) length = 255;
	if (this->text) free(this->text);
	this->text = NULL;
	s=(char *)malloc(length+1);
	if (s==NULL) {
	    fprintf(stderr, "WARNING: couldn't allocate space for label text.\n");
	    return;
	}
	s =  strncpy(s, text, length);
	s[length] = '\0';
	t = s;
	while ((t= strchr(t, '\\')))	/* delete backslashes */
		strcpy(t, t+1);
	t = s;
	while ((t= strchr(t, '{')))	/* delete left brackets */
		strcpy(t, t+1);
	t = s;
	while ((t= strchr(t, '}')))	/* delete right brackets */
		strcpy(t, t+1);
	if (*s) this->text = s;
	(this)->NotifyObservers( label_DATACHANGED);
}
	void
label::SetFont(const char  *fontfamily, long  style , long  size)
			{
	this->font = fontdesc::Create(fontfamily, style, size);
	(this)->NotifyObservers( label_DATACHANGED);
}
	char *
label::GetText()
	{
	return this->text;
}
	const char *
label::GetFont(long  *style , long  *size)
		{
	*style = (this->font)->GetFontStyle();
	*size = (this->font)->GetFontSize();
	return (this->font)->GetFontFamily();
}

