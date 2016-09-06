/* These need cleanup (probably not all used here, and the rest
 * should be static member functions or methods of the class.
 * They should definitely not be transferred to the C files, even though
 * C++ linkage protocol includes type names.
 */
extern long eqview_Extender(class eqview  *self, char  *s, char  **stringp, long  code , long  size);
extern char *eqview_Extendable(class eqview  *self, struct formula  *f , struct formula  *leftf , struct formula  *rightf, enum eqstyle  eqstyle, char  ext1 , char  ext2, long  hang		/* whether to hang (root) or center (paren) */, long  one_part_extender	/* whether extender has only one part */);
extern struct formula *eqview_Format(class eqview  *self, class eq  *eqptr, struct formula  *leftf , struct formula  *f , struct formula  *rightf, enum eqstyle  eqstyle);
extern void eqview_Default(class eqview  *self, char  c);
extern void eqview_WriteC(class eqview  *self);
extern void eqview_WriteEqn(class eqview  *self);
extern void eqview_WriteTroff(class eqview  *self);
extern void eqview_WriteDvi(class eqview  *self);
extern long eqview_MoveRight(class eqview  *self, class eq  *eqptr, long  i , long  x);
extern void eqview_MoveForward(class eqview  *self);
extern void eqview_MoveBackward(class eqview  *self);
extern void eqview_MoveToBeginning(class eqview  *self);
extern void eqview_MoveToEnd(class eqview  *self);
extern void eqview_DeleteBackward(class eqview  *self);
extern void eqview_DeleteForward(class eqview  *self);
extern void eqview_CR(class eqview  *self);
extern void eqview_MoveUp(class eqview  *self);
extern void eqview_MoveDown(class eqview  *self);
extern void eqview_Bar(class eqview  *self);
extern void eqview_Dot(class eqview  *self);
extern void eqview_Prime(class eqview  *self);
#ifdef notdef
extern void eqview_Open(class eqview  *self, char  c);
#endif /* notdef */
extern void eqview_Close(class eqview  *self, char  c);
extern void eqview_Cut(class eqview  *self);
extern void eqview_Copy(class eqview  *self);
extern void eqview_Paste(class eqview  *self);
extern void eqview_Exit();
extern void eqview_DumpAndWrite(class eqview  *self);
extern void eqview_doc();
extern void eqview_DoSpecial(class eqview  *self, char  *s);
extern void eqview_Special(class eqview  *self, char  c);
extern void eqview_SuperScript(class eqview  *self);
extern void eqview_SubScript(class eqview  *self);
extern void eqview_AboveScript(class eqview  *self);
extern void eqview_BelowScript(class eqview  *self);
extern void eqview_String(class eqview  *self, char  *s);
extern void eqview_Root(class eqview  *self);
extern void eqview_Fraction(class eqview  *self);
extern void eqview_lbrace(class eqview  *self);
extern class keymap *eqview_InitKeyMap(struct ATKregistryEntry   *classInfo, class menulist  **eqviewMenus , class menulist  **eqviewCutMenus);
