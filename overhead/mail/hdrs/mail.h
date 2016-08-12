#ifndef _MAIL_H_
#define _MAIL_H_
/* C++ified by magic !@#%&@#$ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

/*
	Include file for /usr/andrew/lib/libmail.a.
*/
#include <parseadd.h>

extern char *ams_genid(int  IsFileName);	/* Generates a world-unique identifier; Boolean parameter specifies whether or not to generate a long name */
extern char *convlongto64(long  num, int  pad);	/* converts a long and padding to 6 bytes base 64 */
extern unsigned long conv64tolong(register char  *xnum); /* reverses the above conversion, basically */
extern char *arpadate();	/* Returns pointer to current ASCII date string in RFC821 format */
extern char *EX_Messages[];	/* Text messages for sysexits.h */
extern int EX_Nerr;	/* Number of error messages in above table */
extern char *UnixSysExits(int	 exitNumber);	/* Pass it a sysexits value and it returns a static description */

/* From vmail.c: */
extern char VM_text[];	/* loaded with readable message for all vmail ops */
extern int VM_errordetail;	/* loaded with a more specific error code from vmail ops */
extern int VM_open(char  *User , char  *Mailbox , char  *ReturnPath , char  *For , char  *Authenticated , char  *PgmName);	/* pass User, Mailbox, ReturnPath, For, and Auth (all char*) */
extern int VM_write(char  *s, int  n);	/* pass addr of characters and number of chars */
extern int VM_printf(char  *fmt,/*  void * a0, void * a1, void * a2, void * a3, void * a4, void * a5, void * a6, void * a7, void * a8, void  * a9 */ ...);	/* pass format string and parameters */
extern int VM_close();	/* no parameters necessary */
extern int VM_SetTiming(int  param);	/* pass Boolean for whether to do timing--flag cleared after VM_close */
/* Values for VM_errordetail */
#define vm_ViceDown 1
#define vm_NotADir 2
#define vm_NotOnVice 3
#define vm_InternalError 4
#define vm_NoSuchUser 5
#define vm_AlreadyOpen 6
#define vm_NotOpen 7
#define vm_BadParameters 8
#define vm_OverQuota 9
#define vm_OutOfRetries 10
#define vm_OutOfMemory 11
#define vm_DirMayHaveMoved 12
#define vm_SystemErrorOffset 100	/* added to errno values */


/* extern char *LocalMailHostname;	Name of this host for mail purposes */
extern int  IsLocalAlias(char *name);	/* Says whether a name is a nickname for the local mail hostname */

extern void CanonicalizePersonName(char  *LocalPart);	/* Interprets its char* argument as a person's
			   name and canonicalizes it, handling embedded spaces and dots.
			   Overwrites its argument.
			   The process turns ``a.b'' into ``a b'' and ``a..b'' into ``a. b''. */

enum MailHostQuality {mailhost_good, mailhost_bad, mailhost_indeterminate};

extern enum MailHostQuality ValidateMailHostName(char  *InName , char  *OutName, int  OutNameLen , int  TolerableDelay);
extern enum MailHostQuality ValidateDomainMail(char  *InName , char  *OutName , int  OutNameLen , char  *FwdName, int  FwdNameLen , int  TolerableDelay);

/* Stuff for interpretation of local addresses (spec in la.spec) */
struct MailDom {	/* pointed to by parsed addresses; result of valhost; managed in locaddr */
	struct MailDom	*Next, *Prev;
	int		Refs;		/* Reference count */
	char		*Orig, *Final;
	enum MailHostQuality	Qual;
	unsigned long int	DomainAddress;
	unsigned int	NumFwds;
	unsigned short int	*FwdPrefs;
	char		**Fwds;
};

#define latype_Remote	0	/* addr->MD->Qual might say if good or bad host name */
#define latype_LocalID	1
#define latype_LocalName	2
#define latype_DistList	3
#define latype_DirInsert	4
#define latype_FSMembers	5
/* and more to follow.. */

#define laerr_NoError	0	/* no problem */
#define laerr_OutOfMemory	1	/* a malloc() failed */
#define laerr_SyntaxError	2	/* the local part had bad quoting syntax */
#define laerr_UnrecSpecial	3	/* local part ``+foo+bar'' for unrecognized ``foo'' */
#define laerr_WPerror	4	/* la_Resolve returns this_c sometimes */
#define laerr_BadSecond	5	/* local address is foo+bar, where bar has white space. */

extern int la_Kind(PARSED_ADDRESS  *Addr, int  *outType, char  **outPrime , char  **outSecond);	/* la_Kind(Addr, outType, outPrime, outSecond)
			PARSED_ADDRESS *Addr;
			int *outType; -- returns one of the latype_XXX codes
			char **outPrime, **outSecond;	*/
extern int la_KindDomain(PARSED_ADDRESS  *Addr, int  *outType, char  **outPrime , char  **outSecond , char  *Domain);	/* la_KindDomain(Addr, outType, outPrime, outSecond, Domain)
			PARSED_ADDRESS *Addr;
			int *outType; -- returns one of the latype_XXX codes
			char **outPrime, **outSecond; char *Domain	*/
extern char *la_ErrorString(int  errcode);	/* given a laerr_XXX code, tell you what the code means */

#if 0
extern int  la_Resolve(PARSED_ADDRESS *Addr, char *PrimePart, wp_SearchToken *outSearchToken, int *outMinFound, int MaxQuality, int *outMatchQuality, wp_PrimeKey *outPrimeKey);	/* la_Resolve(Addr, PrimePart, outSearchToken, outMinFound,
				MaxQuality, outMatchQuality, outPrimeKey)
			PARSED_ADDRESS *Addr; char *PrimePart;
			wp_SearchToken *outSearchToken;
			int *outMinFound, MaxQuality, *outMatchQuality;
			wp_PrimeKey *outPrimeKey;	*/
#endif
extern int BracketField(char  *Hdr , char  *FieldName, char  **pBegin , char  **pEnd , char  **pLineBegin);	/* Searches for the contents of the named field. */

extern int IsOK822Atom(int  ch);	/* TRUE iff the char argument could be part of an RFC822 Atom */
#define is822Atom 1	/* Return codes from NextWord */
#define is822QuotedString 2
#define is822Special 3
#define is822End 4
extern int Next822Word(char  **Line , char  *LineEnd , char  *RsltBuf , int  sizeRsltBuf);	/* next word */
extern char *Next822LPart(char  *Line , char  *LineEnd , char  *RsltBuf , int  sizeRsltBuf);	/* next local-part */
extern char *Next822Phrase(char  *Line , char  *LineEnd , char  *RsltBuf , int  sizeRsltBuf);	/* next phrase */
extern char *Quote822LPart(char  *Cleartext);	/* quote a local-part */
extern char *Quote822Phrase(char  *Cleartext);	/* quote a phrase */

extern int CheckAMSConfiguration();
extern int CheckAMSDelivery(char  *someDomain);	/* Hand it a cell name.  Returns >0 if that cell is running AMS delivery, <0 if they're not, 0 if it can't tell. */
extern int CheckAMSNameSep(char  *someDomain);	/* Hand it a cell name; it determines what that cell uses as its space-substitution in validated mail names.  Returns -1 if no, 0 if you can't tell, and >0 if it does.  If the value is >0, it's the separator character itself. */
extern char *CheckAMSMBName(char  *someDomain);	/* Hand it a cell name.  Returns what that cell uses as the name of its mail in-box directory, that accounts use to receive mail.  Returns NULL if it can't tell. */
extern char *CheckAMSPMName(char  *someDomain);	/* Hand it a cell name.  Returns what that cell uses as the username of its distinguished delivery agent.  Returns NULL if it can't tell. */
extern char *CheckAMSWPIAddr(char  *someDomain);	/* Hand it a cell name.  Returns what that cell uses as the address for WPI update requests.  Returns NULL if it can't tell. */
extern int CheckAMSFmtOK(char  *someDomain); /* Hand it a domain name.  Returns >0 if the domain accepts ATK-formatted mail, <0 if it doesn't, and 0 if we can't tell. */
extern int CheckAMSUUCPSupp(char  *someDomain); /* Hand it a domain name.  Returns >0 if the domain thinks a!b is a remote address, <0 if it doesn't, and 0 if we can't tell. */
extern int CheckAMSUseridPlusWorks(char  *someDomain); /* Hand it a domain name.  Returns >0 if the domain handles the a+ and a+b special local addresses, <0 if it doesn't, and 0 if we can't tell. */
#define	vld_WPValid	0x1	/* AMS_WPValidation: 1 */
#define	vld_PasswdValid	0x2	/* AMS_PasswdValidation: 1 */
#define	vld_LocalDBValid	0x4	/* AMS_LocalDatabaseValidation: 1 */
#define	vld_AliasesValid    0x8	/* AMS_AliasesValidation: 1 */
extern int CheckAMSValidationMask(char  *someDomain); /* Hand it a cell name.  Returns a mask for how that cell does its name validation.  Returns an int <0 if it can't find out. */
/* Structure to describe a site's default MSPATH. */
struct cell_msPath {
    char    *Abbrev;	/* e.g. "local" or "external" */
    char    *RootDir;	/* the root directory of that mspath element. */
    char    *Decorate;	/* any square-bracketed decoration on the mspath element */
    char    *RootCell;	/* the AFS cell in which this root sits */
    int	Validated:1;	/* whether this one has been checked */
    int	CkMailbox:1;	/* whether we check this one's mailbox */
};
extern int CheckAMSDfMSPath(char  *someDomain, struct cell_msPath  **valP);	/* Given a cell name as its first parameter, it returns a count of cell_msPaths returned as an array via its second parameter.  Returns 0 if there are none.  Returns <0 if the domain name isn't running AMS delivery or there's some problem in finding the list. */

/* Functions to describe a user's authentication in multiple cells */
extern char AMSHome_errmsg[];	/* Text describing the result of the FindAMSHomeCell call. */
extern void ForgetAMSHome();	/* Forget the AMS home cell and everything else. */
extern int FindAMSHomeCell(struct CellAuth  **ppCellAuth);	/* int FindAMSHomeCell(ppCellAuth)
				struct CellAuth **ppCellAuth;
		Returns a pointer to what the AMS thinks its home cell should be, if there is one.
		Return 1 if there's no such cell, or 2 if there's no authentication at all. */

extern int SetAMSHomeCell(struct CellAuth  *cellAuth);	/* int SetAMSHomeCell(cellAuth)
				struct CellAuth *cellAuth;
		Declare that the given cellAuth should be marked as being the AMS home cell.
		Can fail if the cell isn't the workstation's cell and the given cell isn't running AMS delivery. */


/**** from fwdvalid.c ****/
/*  fwdvalid_msgbuf:
    ----------------
    The char array where ValidateFwdAddr will put diagnostics.
    */
extern char fwdvalid_msgbuf[];


/* fwdvalid_SetTildeUser:
   ----------------------
   Takes a string, and uses that to resolve addresses of the form
   "+dir-insert+~/..." in later calls to ValidateFwdAddr. */
extern void fwdvalid_SetTildeUser(char  *s);

/* ValidateFwdAddr:
   ----------------
   Takes a string (NewAddr), and checks it for validity as a (forwarding)
   mail address.  Returns 0 if successful.  The "canonicalized"
   form of NewAddr will be returned in FixedAddr (malloc'd).  Diagnostics
   can be found in fwdvalid_msgbuf.  */
extern int ValidateFwdAddr( char  *NewAddr ,  char  **FixedAddr);

ENDCPLUSPLUSPROTOS
 
#endif
