

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
char patternnames[ZRPATTERNS_NUM][16] = {
    "PATTERNS:",
    "(invert)",
    "black", 
    "grey 7/8", 
    "grey 3/4", 
    "grey 5/8", 
    "grey 1/2", 
    "grey 3/8", 
    "grey 1/4", 
    "grey 1/8", 
    "white",
    "vert.lines", 
    "horiz.lines", 
    "diag.lines", 
    "checks", 
    "big checks"
};

unsigned char patternlist[ZRPATTERNS_NUM][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 

    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},

    {0xee, 0xff, 0xbb, 0xff, 0xee, 0xff, 0xbb, 0xff}, 
    {0xee, 0xbb, 0xee, 0xbb, 0xee, 0xbb, 0xee, 0xbb}, 
    {0xee, 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55}, 
    {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55}, 
    {0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11}, 
    {0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88}, 
    {0x22, 0x00, 0x88, 0x00, 0x22, 0x00, 0x88, 0x00}, 
 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 

    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33}, 
    {0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00}, 
    {0x33, 0x66, 0xcc, 0x99, 0x33, 0x66, 0xcc, 0x99}, 
    {0x33, 0x33, 0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc}, 
    {0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0}
};


char brushnames[ZRBRUSHES_NUM][16] = {
    "BRUSHES:",
    "point",
    "spot 2",
    "spot 3",
    "spot 4",
    "spot 5",
    "spot 6",
    "spot 7",
    "spot 8",
    "cross",
    "vert.line",
    "horiz.line",
    "slant",
    "short slant",
};

unsigned char brushlist[ZRBRUSHES_NUM][16] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 

    {0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x08, 0x1c, 0x08, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00},
    {0x00, 0x1c, 0x3e, 0x3e, 0x3e, 0x1c, 0x00, 0x00},
    {0x00, 0x3c, 0x7e, 0x7e, 0x7e, 0x7e, 0x3c, 0x00},
    {0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c, 0x00},
    {0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c},

    {0x00, 0x10, 0x10, 0x10, 0xfe, 0x10, 0x10, 0x10},
    {0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00},
    {0x00, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03},
    {0x00, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x00, 0x00}

};
