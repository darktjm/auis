/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* be2roff cmds
 *
 */

extern void DoMacro(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);

extern void rm_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void ds_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void as_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void nr_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void af_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void rr_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void so_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void ex_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void br_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void de_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void am_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void di_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void da_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void c0_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void c1_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void if_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void ie_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void el_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void PM_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void PA_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void tl_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void wh_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void sp_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void it_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void ft_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void in_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void ti_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void fi_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void nf_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void ns_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void rs_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void ce_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void sv_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void ig_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void tr_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void ev_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void Ct_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void Et_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void Hd_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void Gc_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void Bu_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void Tag_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void bp_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void Fn_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void Ce_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void Ps_cmd(struct rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
extern void InsertTbl(class rofftext  *self,Trickle  t);
