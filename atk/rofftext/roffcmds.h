/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* be2roff cmds
 *
 */

extern NO_DLL_EXPORT void DoMacro(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);

extern NO_DLL_EXPORT void rm_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void ds_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void as_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void nr_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void af_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void rr_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void so_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void ex_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void br_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void de_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void am_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void di_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void da_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void c0_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void c1_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void if_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void ie_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void el_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void PM_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void PA_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void tl_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void wh_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void sp_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void it_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void ft_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void in_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void ti_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void fi_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void nf_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void ns_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void rs_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void ce_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void sv_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void ig_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void tr_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void ev_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void Ct_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void Et_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void Hd_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void Gc_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void Bu_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void Tag_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void bp_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void Fn_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void Ce_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void Ps_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern NO_DLL_EXPORT void InsertTbl(class rofftext  *self,Trickle  t);
