typedef struct stone {
	int col, row, note;
		/* note - if non-zero, display a value
		  positive: display integer;  
		  negative display character (-note) or special character:
			  given as the negative of ^ + % @ # *
		*/
	char color; 	/*  W - white,  B - black,  / - empty  */
	long accnum;	/* assigned sequentially on update
			useful for updating views */
} ELTTYPE ;
