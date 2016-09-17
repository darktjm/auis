/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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


#define cursor_width 18
#define cursor_height 13
#define cursor_x_hot 16
#define cursor_y_hot 6
static char cursor_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3c, 0x00,
   0x00, 0x78, 0x00, 0xfe, 0xff, 0x00, 0xfe, 0xff, 0x01, 0xfe, 0xff, 0x00,
   0x00, 0x78, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x0c, 0x00,
   0x00, 0x00, 0x00};

#define cursormask_width 18
#define cursormask_height 13
#define cursormask_x_hot 16
#define cursormask_y_hot 6

static char cursormask_bits[] = {
   0x00, 0x0e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x7e, 0x00,
   0xff, 0xff, 0x00, 0xff, 0xff, 0x01, 0xff, 0xff, 0x03, 0xff, 0xff, 0x01,
   0xff, 0xff, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1e, 0x00,
   0x00, 0x0e, 0x00};
