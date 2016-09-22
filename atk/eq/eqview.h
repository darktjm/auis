/* These need cleanup (probably not all used here, and the rest
 * should be static member functions or methods of the class.
 * They should definitely not be transferred to the C files, even though
 * C++ linkage protocol includes type names.
 */
extern NO_DLL_EXPORT struct formula *eqview_Format(class eqview  *self, class eq  *eqptr, struct formula  *leftf , struct formula  *f , struct formula  *rightf, enum eqstyle  eqstyle);
extern NO_DLL_EXPORT class keymap *eqview_InitKeyMap(struct ATKregistryEntry   *classInfo, class menulist  **eqviewMenus , class menulist  **eqviewCutMenus);
