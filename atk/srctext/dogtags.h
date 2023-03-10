/* File dogtags.h created by R S Kemmetmueller
   (c) Copyright IBM Corp.  (This work is unpublished).  All rights reserved.

   dogtags, an automatic 'stamper' for files as they're loaded in. */

/* INSTRUCTIONS:  Just #include this file, and link your .do with dogtags.o.  Then, after a successful call to text_Read(), call dogtags_substitute(struct text *) to roll through your file looking for special dogtags and substituting the appropriate information.  (Dogtags are delimited by <@ and @>, and have no respect for strings or language syntax, by design).  See source code in dogtags.c, and the dogtags.help help file for a list of recognized dogtag names, and available features such as optional right-justification and centering of dogtag contents. */
/* tjm - FIXME: this should probably be converted to object and exported */
extern NO_DLL_EXPORT void dogtags_substitute(class text *self);
extern NO_DLL_EXPORT void dogtags_substituteregion(class text *self, long *posa, long *lena /* *posa and *lena passed by reference; *lena will probably be changing! */);
