/* Copyright 1993 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
 */

extern boolean ConfirmReadOnly(class textview *self);
extern long textview_LookCmd(ATK *self, long c);
extern void InitializeMod();
extern int charType(char  c);
extern class keymap *textview_InitEmacsKeyMap();
extern class keymap *textview_InitViInputModeKeyMap();
extern class keymap *textview_InitViCommandModeKeyMap();
extern void textview_NOOPCmd (class textview *self);
extern class keymap *textview_InitEmacsKeyMap(struct ATKregistryEntry *classInfo,class menulist **normalMenus);
extern class keymap *textview_InitViCommandModeKeyMap(struct ATKregistryEntry *classInfo,class menulist **Menus);
extern class keymap *textview_InitViInputModeKeyMap(struct ATKregistryEntry *classInfo,class menulist **Menus);
extern int textview_GetNextNonSpacePos(class textview *self,int pos);
extern void textview_AddSpaces(class textview *self,long pos,long startPos,long len);
extern void textview_SelfInsertCmd(class textview *self,char a);
extern void textview_DeleteWordCmd (class textview *self);
extern void textview_ExtendBackwardCmd(class textview *self);
extern void textview_ExtendForwardCmd(class textview *self);
extern void textview_ExtendBackwardLineCmd(class textview *self);
extern void textview_ExtendForwardLineCmd(class textview *self);
extern void textview_ExtendBackwardScreenCmd(class textview *self);
extern void textview_ExtendForwardScreenCmd(class textview *self);
extern void textview_ExtendEndCmd(class textview *self);
extern void textview_ExtendBeginCmd(class textview *self);
extern void textview_ForeKillWordCmd (class textview *self);
extern void textview_OpenLineCmd(class textview *self);
extern void textview_JoinCmd(class textview *self);
extern void textview_YankLineCmd(class textview *self);
extern boolean textview_objecttest(class textview *self,const char *name,const char *desiredname);
extern void textview_InsertInsetCmd (class textview *self,long rock);
extern void textview_InsertFile(class textview *self);
extern void textview_YankCmd(class textview *self);
extern void textview_PutAfterCmd(class textview *self);
extern void textview_PutBeforeCmd(class textview *self);
extern void textview_BackwardsRotatePasteCmd(class textview *self);
extern void textview_RotatePasteCmd(class textview *self);
extern void textview_InsertNLCmd(class textview *self);
extern void textview_ZapRegionCmd(class textview *self);
extern void textview_KillLineCmd(class textview *self);
extern void textview_MITKillLineCmd(class textview *self);
extern void textview_AppendNextCut(class textview *self);
extern void textview_CopyRegionCmd (class textview *self);
extern void textview_GetToCol (class textview *self,int col);
extern void textview_InsertSoftNewLineCmd(class textview *self);
extern void textview_MyLfCmd(class textview *self);
extern void textview_MySoftLfCmd(class textview *self);
extern void textview_DeleteCmd (class textview *self);
extern void textview_ViDeleteCmd (class textview *self);
extern void textview_KillWhiteSpaceCmd(class textview *self);
extern int textview_GetSpace(class textview *self,int pos);
extern void textview_UnindentCmd(class textview *self);
extern void textview_IndentCmd(class textview *self);
extern void textview_ExchCmd(class textview *self);
extern void textview_RuboutCmd (class textview *self);
extern void textview_TwiddleCmd (class textview *self);
extern void textview_RuboutWordCmd (class textview *self);
extern void textview_BackKillWordCmd (class textview *self);
extern void textview_YankWordCmd(class textview *self);
extern void textview_DeleteEndOfWordCmd(class textview *self);
extern void textview_YankEndOfWordCmd(class textview *self);
extern void textview_DeleteBackwardWordCmd(class textview *self);
extern void textview_YankBackwardWordCmd(class textview *self);
extern void textview_DeleteWSWordCmd(class textview *self);
extern void textview_YankWSWordCmd(class textview *self);
extern void textview_DeleteBackwardWSWordCmd(class textview *self);
extern void textview_YankBackwardWSWordCmd(class textview *self);
extern void textview_DeleteEndOfWSWordCmd(class textview *self);
extern void textview_YankEndOfWSWordCmd(class textview *self);
extern void textview_ViDeleteLineCmd(class textview *self);
extern void textview_ViYankLineCmd(class textview *self);
extern void textview_OpenLineBeforeCmd(class textview *self);
extern void textview_OpenLineAfterCmd(class textview *self);
extern void textview_InsertAtBeginningCmd(class textview *self);
extern void textview_InsertAtEndCmd(class textview *self);
extern void textview_ChangeRestOfLineCmd(class textview *self);
extern void textview_ChangeLineCmd(class textview *self);
extern void textview_ChangeWordCmd(class textview *self);
extern void textview_ChangeSelectionCmd(class textview *self);
extern void textview_ReplaceCharCmd(class textview *self);
extern void textview_SubstituteCharCmd(class textview *self);
extern void textview_DigitCmd(class textview *self,char c);
extern void textview_UppercaseWord(class textview *self,long key);
extern void textview_LowercaseWord(class textview *self,long key);
extern void textview_CapitalizeWord(class textview *self,long key);
extern void textview_ToggleCase(class textview *self,long key);
extern void textview_QuoteCmd(class textview *self);
extern void textview_PrintFile(class textview *self);
extern void textview_PreviewCmd(class textview *self);
extern void textview_SetPrinterCmd(class textview *self);
extern void textview_ToggleViModeCmd(class textview *self);
extern void textview_ViCommandCmd(class textview *self,long key);
extern void textview_ToggleEditorCmd(class textview *self);
extern void textview_GrabReference(class textview *self,long key);
extern void textview_PlaceReference(class textview *self,long key);
extern void textview_CheckSpelling(class textview *self);
extern void textview_ToggleReadOnly(class textview *self);
extern void textview_InsertPageBreak (class textview *self);
extern void textview_NextPage (class textview *self);
extern void textview_LastPage (class textview *self);
extern void textview_InsertFootnote(class textview *self);
extern void textview_OpenFootnotes(class textview *self);
extern void textview_CloseFootnotes(class textview *self);
extern void textview_WriteFootnotes(class textview *self);
extern void textview_EndOfWordCmd (class textview *self);
extern void textview_ForwardWordCmd (class textview *self);
extern void textview_BackwardWordCmd (class textview *self);
extern void textview_LineToTopCmd(class textview *self);
extern void textview_ForwardParaCmd(class textview *self);
extern void textview_BackwardParaCmd(class textview *self);
extern void textview_GotoParagraphCmd(class textview *self);
extern void textview_WhatParagraphCmd (class textview *v);
extern void textview_ViGlitchUpCmd(class textview *self);
extern void textview_ViGlitchDownCmd(class textview *self);
extern void textview_DownCmd(class textview *self);
extern void textview_UpCmd(class textview *self);
extern void textview_GlitchUpCmd(class textview *self);
extern void textview_GlitchDownCmd(class textview *self);
extern void textview_NextScreenCmd(class textview *self);
extern void textview_NextScreenMoveCmd(class textview *self);
extern void textview_PrevScreenCmd(class textview *self);
extern void textview_PrevScreenMoveCmd(class textview *self);
extern void textview_StartOfParaCmd (class textview *self);
extern void textview_EndOfParaCmd (class textview *self);
extern void textview_SelectRegionCmd(class textview *self);
extern void textview_CtrlAtCmd(class textview *self);
extern void textview_BackwardCmd(class textview *self);
extern void textview_ForwardCmd(class textview *self);
extern void textview_PreviousLineCmd (class textview *self /**/);
extern void textview_NextLineCmd (class textview *self /**/);
extern void textview_EndOfTextCmd(class textview *self);
extern void textview_BeginningOfTextCmd(class textview *self);
extern void textview_EndOfLineCmd (class textview *self);
extern void textview_BeginningOfLineCmd(class textview *self);
extern void textview_EndOfWSWordCmd(class textview *self);
extern void textview_ForwardWSWordCmd(class textview *self);
extern void textview_BackwardWSWordCmd(class textview *self);
extern void textview_ForwardBeginWordCmd(class textview *self);
extern void textview_ForwardBeginWSWordCmd(class textview *self);
extern void textview_BeginningOfFirstWordCmd(class textview *self);
extern void textview_BeginningOfPreviousLineCmd(class textview *self);
extern void textview_BeginningOfNextLineCmd(class textview *self);
extern void textview_CursorToLine(class textview *self,long line);
extern void textview_CursorToTop(class textview *self,long key);
extern void textview_CursorToCenter(class textview *self,long key);
extern void textview_CursorToBottom(class textview *self,long key);
extern void textview_GoToLineCmd(class textview *self);
extern int textview_SearchCmd(class textview *self, char *arg);
extern int textview_RSearchCmd(class textview *self, char *arg);
extern void textview_SearchAgain(class textview *self);
extern void textview_SearchAgainOppositeCmd(class textview *self);
extern void textview_QueryReplaceCmd(class textview *self);
extern void textview_BalanceCmd(class textview *self);
extern void textview_PlainerCmd(class textview *self,char *type);
extern void textview_PlainestCmd(class textview *self);
extern void textview_ExposeStyleEditor(class textview *self);
extern void textview_ShowStylesCmd(class textview *self);
extern void textview_ChangeTemplate(class textview *self);
extern void textview_ToggleExposeStyles(class textview *self);
extern void textview_ToggleColorStyles(class textview *self);
extern void textview_DisplayInsertEnvironment(class textview *self);
extern void textview_PlainInsertEnvCmd(class textview *self);
extern void textview_UpInsertEnvironmentCmd(class textview *self);
extern void textview_DownInsertEnvironmentCmd(class textview *self);
extern void textview_LeftInsertEnvironmentCmd(class textview *self);
extern void textview_RightInsertEnvCmd(class textview *self);
extern void textview_InsertEnvironment(class textview *self,const char *sName);
extern void textview_ToggleLineDisplay(textview *self, char *arg);
/* Horizontal scrolling */
extern int textview_get_hshift_interval(textview *self);
extern void textview_ShiftLeftCmd(textview *self, char *arg);
extern void textview_ShiftRightCmd(textview *self, char *arg);
extern void textview_ShiftHomeCmd(textview *self, char *arg);
