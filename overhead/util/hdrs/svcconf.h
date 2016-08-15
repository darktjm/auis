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


 

/* ************************************************************ *\
	svcconf.h
	Definitions for cell-based services.
\* ************************************************************ */

extern const char *ThisDomain;
extern int ThisDomainLen;
extern const char *ViceFile;

extern int AMS_ViceIsRunning;

extern int AMS_DeliverySystem;

extern int AMS_UseWP;

extern int AMS_OnAIX;
extern int AMS_LocalMailSystemExists, AMS_DefaultToAFSCellMail, AMS_ThisDomainAuthFromWS;

extern const char *CellCommonPrefix, *CellCommonRWSuffix, *CellCommonSuffix,
	*CellCommonWPDirSuffix;
extern const char *WorkstationCell, *WorkstationName;
