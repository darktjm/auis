

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
#define MAXNAMELEN 255

typedef char *nametype;
bool_t xdr_nametype();


typedef struct namenode *namelist;
bool_t xdr_namelist();


struct namenode {
	nametype name;
	namelist next;
};
typedef struct namenode namenode;
bool_t xdr_namenode();


struct readdir_res {
	int errno;
	union {
		namelist list;
	} readdir_res_u;
};
typedef struct readdir_res readdir_res;
bool_t xdr_readdir_res();


#define DIRPROG ((u_long)76)
#define DIRVERS ((u_long)1)
#define READDIR ((u_long)1)
extern readdir_res *readdir_1();

