#if 0  /* just to shut up makedep; this file is not built */
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* **********************************************************************
*   This code is designed to read what might be priveledged (setuid) 
*   information regarding both Disk Statistics (% full) and a host of 
*   stats from /dev/kmem (including but not limited to, CPU, I/O, and VM)
*
*   When retriving the data - this program will print out to stdout
*   a string in the form of either "%d:%d\n" or "%d:%d:%s\n"
*   The latter case is for passing the name of where a disk is mounted
*   back to the parent program.
*
*   The parent program (Console, or any other program which wishes to get
*   at this information) is responsible for setting up a pipe, binding the
*   child's (this program) stdout to one end of a pipe, and parsing the
*   strings which are passed back.
*
*   The basic string format is an ID (int), a colon, a value (int), and
*   optionally another colon followed by a string.  The ID is coded from
*   the included file "getstats.h" - ID values 50 and over represent 
*   ERRORS as documented in the above mentioned inclued file.  When an 
*   ERROR or the optional string is passed, the value (second parameter)
*   can be safely ignored, and is usually set to 0.
*
*   The arguments to be passed to this program are the effective UID from
*   the parent program, a polling frequency (# of seconds) for checking
*   /dev/kmem (usually between 1 and 5, must be > 0), and a polling
*   frequency for checking how full the local disks are (generally higher
*   than the value for /dev/kmem, but could vary greatly).  Thus the call
*   is:
*
*   execvp("getstats", argv)
*
*   with argv as:
*
*   argv[0]="getstats";
*   argv[1]=~ ("%d", UID);
*   argv[2]=~ ("%d", kmempollfrequency);
*   argv[3]=~ ("%d", diskpollfrequency);
*   argv[4]=NULL;
*
********************************************************************** */
#ifdef AIX
/*
 * just be stupid and fake something for testing...
 */
#include <getstats.h>
#include <stdio.h>

#define min(a, b) (((a) <= (b)) ? (a) : (b))
#define sendval(text) {printf text ;fflush(stdout);}

main(argc, argv)
int argc;
char **argv;
{
    int GVMPollFreq = 0, DiskPollFreq = 0, UsersID = 0;
    int time1 = 0, time2 = 0, sleepTime = 0;

    int cpuload = 0;
    int userload = 0;

    if (argc < 4){
	sendval(("%d:%d\n", PARSE_ERR_1, 0));
	exit(-1);
    }
    *argv++;
    UsersID = atoi(*argv++);
    if ((GVMPollFreq = atoi(*argv++)) < 1){
	sendval(("%d:%d\n", PARSE_ERR_2, 0));
	exit(-1);
    }
    if ((DiskPollFreq = atoi(*argv)) < 1){
	sendval(("%d:%d\n", PARSE_ERR_2, 0));
	exit(-1);
    }
    while (1){	/* for lack of a better loop for now? */
	if (time1 < 1){
	    cpuload +=10;
	    if (cpuload >= 100) cpuload = 0;
	    sendval(("%d:%d\n", LOADCPU, cpuload));
	    time1 = GVMPollFreq;
	}
	if (time2 < 1){
	    userload +=5;
	    if (userload >= 100) userload = 0;
	    sendval(("%d:%d\n", LOADUSER, userload));
	    time2 = DiskPollFreq;
	}
	sleepTime = min(time1, time2);
	sleep(sleepTime);
	time1 -= sleepTime;
	time2 -= sleepTime;
    }
}




#else /* #ifdef AIX */

/* #include <sitevars.h> */ /* just to shut up makedep; this file is not built */

/* The following include defines sys_vax_20 for Vax release 2.0 op sys, */
/* since that particular OS needs some special treatment.  We would have */
/* used -Dsys_vax_20 except we want to use the same Makefile for each cpu... */

/* Wrap In_Imake around andrewos.h because we don't want any include files from this, only the op-sys info.  This is a hack that gets around the fact that andrewos.h now includes <sys/time.h> and so does <sys/user.h> below... Gawd! */

#define In_Imake
#include <andrewos.h>
#undef In_Imake

#include <stdio.h>
#include <sys/param.h>
#ifdef hpux
#include <sys/signal.h>
#endif /* hpux */
#include <sys/user.h> 
#include <sys/proc.h>
#include <sys/vm.h>
#include <sys/dk.h>
#include <sys/map.h>
#ifndef hpux /* included indirectly by sys.user.h above */
#include <nlist.h>/* search namelist in vmunix */
#endif /* hpux  */

#ifndef sys_vax_20
#include <sys/buf.h>
#endif /* sys_vax_20 */

#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef sys_vax_20
#ifndef sun
#ifdef hpux
#include <checklist.h>
#include <mntent.h>
#define MOUNTED MNT_MNTTAB
#define fstab checklist
#define FSTAB_RW CHECKLIST_RW
#define FSTAB_RO CHECKLIST_RO	 
#else /* hpux */
#include <fstab.h>
#include <mtab.h>
#endif /* hpux */
#ifndef hpux
#include <ufs/fs.h>
#endif /* hpux */

#ifndef VAX_ENV
#include <sys/fs.h>
#endif /* VAX_ENV */
#else /* sun */

#include <sys/socket.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <sun/ndio.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <ufs/fs.h>

#endif /* sun */
#endif /* sys_vax_20 */

#ifdef sys_vax_20
#include <sys/mount.h>
#include <andrewos.h> /* sys/types.h strings.h */
#include <ustat.h>
#endif /* sys_vax_20 */

#ifdef sys_vax_20
struct	fs_data *mountbuffer;
#define MSIZE (NMOUNT*sizeof(struct fs_data))
#endif /* sys_vax_20 */


#ifndef MAXUPRC    /* This is a constant in BSD, but */
    long maxuprc = 0;    /* we have to nlist in Ultrix  */
#else /* MAXUPRC     */
    long maxuprc = MAXUPRC;
#endif /* MAXUPRC     */

#define VMMON_DODECL
#include <getstats.h>
extern struct nlist RawStatistics[];


#ifndef sys_vax_20
#if ! (defined(sun) | defined(hpux))
struct mtab mtab[NMOUNT];
int	fi;
daddr_t	alloc();
char	*strcpy();
#endif /* ! (defined(sun) | defined(hpux)) */
#endif /* sys_vax_20 */

char	root[32];

int TotalTime;
int deficit;
int MemoryFile /* file descriptor for unix memory */ ;
int nproc;
int SwapMapAddress;
int nSwapMapEntries;
int SwapMapSize;
int nSwapBlks;
int CurrentSwapUsed;
int dmmax;
off_t procp;
struct mapent *SwapMap;
struct proc proc[8];/* 8 = a few, for fewer syscalls */
struct proc *mproc;
extern char *malloc();


struct{
   long time[CPUSTATES];
   long xfer[DK_NDRIVE];
   struct vmmeter Rate;
   struct vmtotal Total;
#ifdef sun
   struct ndstat  ndstat;
#endif /* sun */
   long  dk_xfer[DK_NDRIVE];
}s, s1;

#ifndef sys_vax_20
union {
    struct fs iu_fs;
    char dummy[SBSIZE];
} sb;
#endif /* sys_vax_20 */

#define	rate		s.Rate
#define	total		s.Total
#define sblock sb.iu_fs
#define sendval(text) {printf text ;fflush(stdout);}




GetGVMStats(UsersID)
int UsersID;
{
    int   i;
    long  t;
    struct mapent *sp;
    lseek(MemoryFile,(long) RawStatistics[X_CPTIME].n_value, 0);
    read(MemoryFile, s.time, sizeof(s.time));

#ifdef sun
    if (RawStatistics[X_NDSTAT].n_value != 0){
	lseek(MemoryFile,(long) RawStatistics[X_NDSTAT].n_value, 0);
	read(MemoryFile, &s.ndstat, sizeof(s.ndstat));
    }
#endif /* sun */
    if (RawStatistics[X_NDSTAT].n_value == 0){
	lseek(MemoryFile,(long) RawStatistics[X_DKXFER].n_value, 0);
	read(MemoryFile, s.dk_xfer, sizeof(s.dk_xfer));
    }
    lseek(MemoryFile,(long) RawStatistics[X_RATE].n_value, 0);
    read(MemoryFile, &rate, sizeof(rate));
    sp = SwapMap;
    lseek(MemoryFile, SwapMapAddress, 0);
    read(MemoryFile, sp, SwapMapSize);
    for (CurrentSwapUsed = nSwapBlks;sp->m_size;sp++){
	CurrentSwapUsed -= sp->m_size;
    }
    lseek(MemoryFile,(long) RawStatistics[X_TOTAL].n_value, 0);
    read(MemoryFile, &total, sizeof(total));
    lseek(MemoryFile,(long) RawStatistics[X_DEFICIT].n_value, 0);
    read(MemoryFile, &deficit, sizeof(deficit));
    for (i = 0; i < CPUSTATES; i++) {
	t = s.time[i];
	s.time[i] -= s1.time[i];
	s1.time[i] = t;
    }
    s.time[1] += s.time[0];
    TotalTime = 0;
    for (i = 1; i < CPUSTATES; i++)
	TotalTime += s.time[i];
    if (TotalTime == 0)
	TotalTime = 1;
    sendval(("%d:%d\n", LOADCPU, (s.time[1] + s.time[2]) * 100 / TotalTime));
#ifdef sun
    if (RawStatistics[X_NDSTAT].n_value != 0){
	int myval = 0;
	if (s1.ndstat.ns_rpacks == 0){
	    s1.ndstat = s.ndstat;
	}
	myval = s.ndstat.ns_rpacks - s1.ndstat.ns_rpacks + s.ndstat.ns_xpacks - s1.ndstat.ns_xpacks;
	if (myval > 100) myval = 100;
	sendval(("%d:%d\n", LOADIO, myval));
	s1.ndstat = s.ndstat;
    }
    else
#endif /* sun */
    {
	int myval = 0;
	{
	    int   i;
	    for (i = 1; i < DK_NDRIVE - 1; i++)
		s.dk_xfer[0] += s.dk_xfer[i];
	}
	if (s1.dk_xfer[0] == 0){
	    s1.dk_xfer[0] = s.dk_xfer[0];
	}
	myval = s.dk_xfer[0] - s1.dk_xfer[0];
	if (myval > 100) myval = 100;
	sendval(("%d:%d\n", LOADIO, myval));
	s1.dk_xfer[0] = s.dk_xfer[0];
    }
    sendval(("%d:%d\n", LOADUSER, s.time[1] * 100 / TotalTime));
    sendval(("%d:%d\n", LOADSYS, s.time[2] * 100 / TotalTime));
    sendval(("%d:%d\n", LOADIDLE, s.time[3] * 100 / TotalTime));
    sendval(("%d:%d\n", VM, nSwapBlks ? CurrentSwapUsed * 100 / nSwapBlks : -1));
    sendval(("%d:%d\n", PAGEIN, rate.v_pgpgin / 2));
    sendval(("%d:%d\n", PAGEOUT, rate.v_pgpgout / 2));
    sendval(("%d:%d\n", PAGEREPLACABLE, rate.v_scan));
    sendval(("%d:%d\n", PAGEDEFICIT, deficit));
    sendval(("%d:%d\n", MEMACTIVE, total.t_avm / 2));
    sendval(("%d:%d\n", MEMFREE, total.t_free / 2));
    sendval(("%d:%d\n", QUEUERUN, total.t_rq));
    sendval(("%d:%d\n", QUEUEBLOCK, total.t_dw + total.t_pw));
    sendval(("%d:%d\n", QUEUEMEM, total.t_sw));
    sendval(("%d:%d\n", INTSIO, rate.v_intr));
    sendval(("%d:%d\n", INTSSYS, rate.v_syscall));
    sendval(("%d:%d\n", INTSSWAP, rate.v_swtch));
    if (1) {/* DoPROCESSES */
	int i, j, userprocesses, totalprocesses, otherprocs;
	off_t tmpprocp;

	userprocesses = 0;
	totalprocesses = 1;
	otherprocs = 0;
	tmpprocp = procp;

	for (i = 0; i < nproc; i += 8) {
	    lseek(MemoryFile, (long) tmpprocp, 0);
	    j = nproc - i;
	    if (j > 8)
		j = 8;
	    j *= sizeof(struct proc);
	    if (read(MemoryFile, (char *) proc, j) != j) {
		sendval(("%d:%d\n", GVM_ERR_1, 0));
		exit(-1);
	    }
	    tmpprocp += j;
	    for (j = j / sizeof(struct proc) - 1; j >= 0; j--) {
		mproc = &proc[j];
		if (mproc->p_pid != 0) {
		    totalprocesses++;
		    if (UsersID == mproc->p_uid) {
			userprocesses ++;
		    } else if (mproc->p_uid) {
			otherprocs++;
		    }
		}
	    }
	}
	sendval(("%d:%d\n", PROCSUSER, maxuprc ? (userprocesses * 100) / maxuprc : -1));
	sendval(("%d:%d\n", PROCSTOTAL, nproc ? totalprocesses * 100 / nproc : -1));
	sendval(("%d:%d\n", PROCSOTHER, otherprocs));
    }
#ifdef sun
    if (s1.ndstat.ns_rpacks == 0) s1.ndstat = s.ndstat;
    sendval(("%d:%d\n", NDSTATIN, s.ndstat.ns_rpacks - s1.ndstat.ns_rpacks));
    sendval(("%d:%d\n", NDSTATOUT, s.ndstat.ns_xpacks - s1.ndstat.ns_xpacks));
    sendval(("%d:%d\n", NDSTATERR, s.ndstat.ns_utimo - s1.ndstat.ns_utimo
	    + s.ndstat.ns_stimo - s1.ndstat.ns_stimo));
    s1.ndstat = s.ndstat;
#endif /* sun */
}





InitGVMStats()
{
   time_t bootime;
   int code = 0;

   /* 
    set up Unix interface: scan name list for current system
    addresses and then open a file which is the memory image
    of the system.
    */
   code = nlist(_SITE_VMUNIX, RawStatistics);
   if (code == -1){
       sendval(("%d:%d\n", GVM_ERR_2, 0));
       exit(-1);
   }
   else{
       if (RawStatistics[0].n_type == 0){
	   sendval(("%d:%d\n", GVM_ERR_3, 0));
	   exit(-1);
       }
   }
   MemoryFile = open(_SITE_DEV_KMEM, 0);
   if (MemoryFile < 0){
       sendval(("%d:%d\n", GVM_ERR_4, 0));
       exit(-1);
   }
   lseek(MemoryFile,(long) RawStatistics[X_BOOTIME].n_value, 0);
   read(MemoryFile, &bootime, sizeof(bootime));
   lseek(MemoryFile, (long) RawStatistics[X_PROC].n_value, 0);
   read(MemoryFile,(char *) &procp, sizeof(procp));
   lseek(MemoryFile, (long) RawStatistics[X_NPROC].n_value, 0);
   read(MemoryFile,(char *) &nproc, sizeof(nproc));
#ifdef sun
   /* 
    * Even if there is an nd driver in the system, we may not actually be
    * a client.  If the nd driver has never been used, ignore it.
    */
   if (RawStatistics[X_NDSTAT].n_value){
       lseek(MemoryFile,(long) RawStatistics[X_NDSTAT].n_value, 0);
       read(MemoryFile, &s.ndstat, sizeof(s.ndstat));
       if (s.ndstat.ns_xpacks == 0 || s.ndstat.ns_rpacks == 0)
	   RawStatistics[X_NDSTAT].n_value = 0;
   }
#endif /* sun */
#ifndef MAXUPRC
    lseek(MemoryFile, (long) RawStatistics[X_MAXUPRC].n_value, 0);
    read(MemoryFile,(char *) &maxuprc, sizeof(maxuprc));
#endif /* MAXUPRC */
    lseek(MemoryFile,(long) RawStatistics[X_SWAPMAP].n_value, 0);
    read(MemoryFile, &SwapMapAddress, sizeof(SwapMapAddress));
    SwapMapAddress += sizeof(struct map);
    lseek(MemoryFile,(long) RawStatistics[X_NSWAPMAP].n_value, 0);
    read(MemoryFile, &nSwapMapEntries, sizeof(nSwapMapEntries));
    SwapMapSize = (--nSwapMapEntries)*sizeof(struct mapent);
    lseek(MemoryFile,(long) RawStatistics[X_NSWAPBLKS].n_value, 0);
    read(MemoryFile, &nSwapBlks, sizeof(nSwapBlks));
    lseek(MemoryFile,(long) RawStatistics[X_DMMAX].n_value, 0);
    read(MemoryFile, &dmmax, sizeof(dmmax));
    nSwapBlks -= dmmax/2;  /* See vm_sw.c--initialization of swap space */
    SwapMap = (struct mapent *) malloc(SwapMapSize);
}


/* the DeviceTable keeps a list of all the devices (and their mounted directory file name) that we should watch */

extern int getmnt();

GetDiskStats(Init)
int Init;
{
    int i = 0;
#ifdef sys_vax_20
    int ret = 0;
    struct	fs_data *fd;
    int loc = 0;
#else /* sys_vax_20 */
#if (defined(sun) | defined(hpux))
    struct stat statb;
    char tmpname[1024];
    FILE *mtabp;
    struct mntent *mnt;
#else /* (defined(sun) | defined(hpux)) */
    struct fstab *fsp;
#endif /* (defined(sun) | defined(hpux)) */
#endif /* sys_vax_20 */


#ifdef sys_vax_20
    if(Init){
	mountbuffer = (struct fs_data *) malloc(MSIZE);
    }
    ret = getmnt(&loc, mountbuffer, MSIZE, NOSTAT_MANY, NULL);
    if (ret < 0) {
	sendval(("%d:%d\n", DISK_ERR_6, 0));
	exit(1);
    }
    i = DISK1;
    for (fd=mountbuffer; fd < &mountbuffer[ret]; fd++) {
	print_df(i++, fd, Init);
    }
#else /* sys_vax_20 */
#if (defined(sun) | defined(hpux))
    sync();
    if ((mtabp = setmntent(MOUNTED, "r")) == NULL) {
	sendval(("%d:%d\n", DISK_ERR_1, 0));
	exit(1);
    }
    i = DISK1 - 1; /* figuratively 0 */
    while ((mnt = getmntent(mtabp)) != NULL) {
	i++;
	if (strcmp(mnt->mnt_type, MNTTYPE_IGNORE) == 0 ||
	    strcmp(mnt->mnt_type, MNTTYPE_SWAP) == 0)
	    continue;
#ifdef sun
	if (strcmp(mnt->mnt_type, MNTTYPE_42) == 0 &&
	    (stat(mnt->mnt_fsname, &statb) >= 0) &&
	    (((statb.st_mode & S_IFMT) == S_IFBLK) ||
	     ((statb.st_mode & S_IFMT) == S_IFCHR))) {
	    (void) strcpy(tmpname, mnt->mnt_fsname);
	    dfree1(i, tmpname, 0, Init);
	} else {
	    dfree2(i, mnt->mnt_dir, mnt, Init);
	}
#endif /* sun */
    }
    (void) endmntent(mtabp);
#else /* (defined(sun) | defined(hpux)) */
    i = open(_SITE_MTAB, 0);
    if (i >= 0) {
	(void) read(i, (char *)mtab, sizeof (mtab));
	(void) close(i);
    }
    sync();
    if (setfsent() == 0){
	sendval(("%d:%d\n", DISK_ERR_5, 0));
	exit(1); 
    }
    i = DISK1 - 1; /* figuratively 0 */
    while (fsp = getfsent()) {
	i++;
#ifdef hpux
	if (strcmp(fsp->fs_type, FSTAB_RW) &&
	    strcmp(fsp->fs_type, FSTAB_RO))
	    continue;
#else /* hpux */
	if (strcmp(fsp->fs_type, FSTAB_RW) &&
	    strcmp(fsp->fs_type, FSTAB_RO) &&
	    strcmp(fsp->fs_type, FSTAB_RQ))
	    continue;
#endif /* hpux */
	if (root[0] == 0){
	    (void) strcpy(root, fsp->fs_spec);
	}
	dfree1(i, fsp->fs_spec, 1, Init);
    }
    endfsent();
#endif /* (defined(sun) | defined(hpux)) */
#endif /* sys_vax_20 */
}

#ifndef sys_vax_20
int bread(fi, bno, buf, cnt)
int fi;
daddr_t bno;
char *buf;
int cnt;
{
    (void) lseek(fi, (long)(bno * DEV_BSIZE), 0);
#ifndef sun
    if (read(fi, buf, cnt) != cnt)
#else /* sun */
	if (read(fi, buf, (unsigned) cnt) < 0)
#endif /* sun */
	{
	    /* probably a dismounted disk if errno == EIO */
	    if (errno != EIO) { 
		sendval(("%d:%d\n", DISK_ERR_5, 0));
	    }
	    return (0);
	}
    return (1);
}
#endif /* sys_vax_20 */

/*
 * Given a name like /dev/rrp0h, returns the mounted path, like /usr.
 */
#ifndef sys_vax_20
char *mpath(file)
char *file;
{
#if defined(sun) | defined(hpux)
    FILE *mntp;
    struct mntent *mnt;
#else /* defined(sun) | defined(hpux) */
    struct mtab *mp;
#endif /* defined(sun) | defined(hpux) */


#if defined(sun) | defined(hpux)
    if ((mntp = setmntent(MOUNTED, "r")) == 0) {
	sendval(("%d:%d\n", DISK_ERR_1, 0));
	exit(1);
    }
    while ((mnt = getmntent(mntp)) != 0) {
	if (strcmp(file, mnt->mnt_fsname) == 0) {
	    (void) endmntent(mntp);
	    return (mnt->mnt_dir);
	}
    }
    (void) endmntent(mntp);
#else /* defined(sun) | defined(hpux) */
    if (eq(file, root)){
	return ("/");
    }
    for (mp = mtab; mp < mtab + NMOUNT; mp++){
	if (eq(file, mp->m_dname)){
	    return (mp->m_path);
	}
    }
#endif /* defined(sun) | defined(hpux) */
    return "";
}
#endif /* sys_vax_20 */

#ifndef sys_vax_20
#ifndef sun
eq(f1, f2)
char *f1, *f2;
{
    if (strncmp(f1, "/dev/", 5) == 0)
	f1 += 5;
    if (strncmp(f2, "/dev/", 5) == 0)
	f2 += 5;
    if (!strcmp(f1, f2))
	return (1);
    if (*f1 == 'r' && !strcmp(f1+1, f2))
	return (1);
    if (*f2 == 'r' && !strcmp(f1, f2+1))
	return (1);
    if (*f1 == 'r' && *f2 == 'r' && strcmp(f1+1, f2+1) == 0)
	return (1);
    return (0);
}
#endif /* sun */
#endif /* sys_vax_20 */

int round(num)
double num;
{
    int inum = (int) num;
    return(((num - inum) >= 0.5) ? (inum + 1) : inum);
}

#ifdef sys_vax_20
print_df(id, fd, Init)
int id;
struct fs_data *fd;
int Init;
{
	int used;
	used = fd->fd_btot - fd->fd_bfree;
	if (Init){
	    sendval(("%d:%d:%s\n", id, 0, fd->fd_path));
	}
	else{
	    sendval(("%d:%d\n", id, fd->fd_btot == 0 ? 0 :
		     round(used / (double)(fd->fd_btot - (fd->fd_bfree - fd->fd_bfreen)) * 100.0)));
	}
}
#endif /* sys_vax_20 */


#ifndef sys_vax_20
dfree1(id, file, infsent, Init)
int id;
char *file;
int infsent;
int Init;
{
    long totalblks, availblks, free, used;
    int fi;
#ifndef sun
    struct stat stbuf;
    struct fstab *fsp;

    if (stat(file, &stbuf) == 0 &&
	 (stbuf.st_mode&S_IFMT) != S_IFCHR &&
	 (stbuf.st_mode&S_IFMT) != S_IFBLK) {
	if (infsent) { 
	    sendval(("%d:%d\n", DISK_ERR_3, 0));
	    return;
	}
	setfsent();
	while (fsp = getfsent()) {
	    struct stat stb;

	    if (stat(fsp->fs_spec, &stb) == 0 &&
		stb.st_rdev == stbuf.st_dev) {
		file = fsp->fs_spec;
		endfsent();
		goto found;
	    }
	}
	endfsent();
	sendval(("%d:%d\n", DISK_ERR_4, 0));
	return;
    }
    found:
      fi = open(file, 0);
#else /* sun */
    fi = open(file, 0);
#endif /* sun */
    if (fi < 0){
	return;
    }
    if (bread(fi, SBLOCK, (char *)&sblock, SBSIZE) == 0) {
	(void) close(fi);
	return;
    }
    totalblks = sblock.fs_dsize;
    free = sblock.fs_cstotal.cs_nbfree * sblock.fs_frag +
      sblock.fs_cstotal.cs_nffree;
    used = totalblks - free;
    availblks = totalblks * (100 - sblock.fs_minfree) / 100;
    if(Init){
	sendval(("%d:%d:%s\n", id, 0, mpath(file)));
    }
    else{
	sendval(("%d:%d\n", id, availblks == 0 ? 0 : round((double) used / (double) availblks * 100.0)));
    }
    (void) close(fi);
}
#endif /* sys_vax_20 */


#ifdef sun
dfree2(id, file, mnt, Init)
int id;
char *file;
struct mntent *mnt;
int Init;
{
    struct statfs fs;
    long totalblks, avail, free, used, reserved;

    if (statfs(file, &fs) < 0) {
	return;
    }
    totalblks = fs.f_blocks;
    free = fs.f_bfree;
    used = totalblks - free;
    avail = fs.f_bavail;
    reserved = free - avail;
    if (avail < 0)
	avail = 0;
    totalblks -= reserved;
    if(Init){
	sendval(("%d:%d:%s\n",id, 0, mnt->mnt_dir));
    }
    else{
	sendval(("%d:%d\n", id, round((double) used / (double) totalblks * 100.0)));
    }
}
#endif /* sun */

#define min(a, b) (((a) <= (b)) ? (a) : (b))


main(argc, argv)
int argc;
char **argv;
{
    int GVMPollFreq = 0, DiskPollFreq = 0, UsersID = 0;
    int time1 = 0, time2 = 0, sleepTime = 0;

    if (argc < 4){
	sendval(("%d:%d\n", PARSE_ERR_1, 0));
	exit(-1);
    }
    *argv++;
    UsersID = atoi(*argv++);
    if ((GVMPollFreq = atoi(*argv++)) < 1){
	sendval(("%d:%d\n", PARSE_ERR_2, 0));
	exit(-1);
    }
    if ((DiskPollFreq = atoi(*argv)) < 1){
	sendval(("%d:%d\n", PARSE_ERR_2, 0));
	exit(-1);
    }
    InitGVMStats();
    GetDiskStats(1);
    while (1){	/* for lack of a better loop for now? */
	if (time1 < 1){
	    GetGVMStats(UsersID);
	    time1 = GVMPollFreq;
	}
	if (time2 < 1){
	    GetDiskStats(0);
	    time2 = DiskPollFreq;
	}
	sleepTime = min(time1, time2);
	sleep(sleepTime);
	time1 -= sleepTime;
	time2 -= sleepTime;
    }
}

#endif /* #ifdef AIX */


#endif
