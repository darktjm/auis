

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
#ifndef ANSITEXT_H
#define ANSITEXT_H

#define ansitext_BaseFactor		(2.0)
#define ansitext_AscenderFactor		(1.0)
#define ansitext_DescenderFactor	(1.0)

#define ansitext_CapitalFactor \
	(ansitext_BaseFactor + ansitext_AscenderFactor)
#define ansitext_DescentRatio \
	(ansitext_DescenderFactor / ansitext_CapitalFactor)

#define ansitext_ComputeAscent(fontsize)  (fontsize)
#define ansitext_ComputeDescent(fontsize) ((fontsize) * ansitext_DescentRatio)

#define ansitext_ComputeDelta(fontsize, spacing) \
	((((spacing) / 4.0) - (1.0 / 6.0)) * (fontsize))

#define ansitext_SlantFontSlope		(2.5)

#endif
