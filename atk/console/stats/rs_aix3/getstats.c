/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <nlist.h>
#include <time.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/vminfo.h>
#include <sys/iostat.h>
#include <sys/mntctl.h>
#include <sys/vmount.h>
#include <sys/statfs.h>
#include <sys/sysconfig.h>
#include <sys/var.h>
#include <sys/proc.h>
#include <procinfo.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/tokuser.h>

#include <getstats.h>

extern int errno, sys_nerr;
extern char *sys_errlist[];

#define sendval(text) {printf text ;fflush(stdout);}

/* vmker struct is kernelstruct (undocumented) */
/* vmker seems to hold some kernels virtual memeory variables */
struct vmker {
  uint n0,n1,n2,n3,n4,n5,n6,n7,n8;
  uint totalmem;
  uint n10;
  uint freemem;
  uint n12,n13;
  uint totalvmem,freevmem;
};

int dk_cnt;  /* number of disks in system  */

#define TICK_D 60.0
#define	INIT_BUF_SIZE 2000

#define NUMBER_OF_KNSTRUCTS 4
#define NLIST_SYSINFO 0
#define NLIST_VMKER 1
#define NLIST_VMINFO 2
#define NLIST_IOSTAT 3
struct nlist kernelnames[] = {
    {"sysinfo", 0, 0, 0, 0, 0},
    {"vmker", 0, 0, 0, 0, 0},
    {"vmminfo", 0, 0, 0, 0, 0},
    {"iostat", 0, 0, 0, 0, 0},
    {NULL, 0, 0, 0, 0, 0},
    };
#define N_VALUE(index) (kernelnames[index].n_value)

int sleep_sec=5;
int dk_skip_interval;
double realtime(void);

uid_t this_uid;

struct procdata {
  uint usr;			/* # procs runing under this user */
  uint root;			/* # procs runing under root */
  uint total;			/* # procs running */
};

int max_sys_procs;		/* nbr procs allowed on sys */
int max_usr_procs;		/* nbr procs per user */

int get_sys_vm_info(struct sysinfo *si,
		    struct vmker *vmk,
		    struct vminfo *vm,
		    struct procdata *prc,
		    cio_query_blk_t *tr);

void getkmemdata(char *buf,	    /* buffer for kernel data*/
		 int bufsize,	    /* buffers size */
		 long n_value);	    /* seek address to kernels data */

void getprocdata(struct procdata *procs);
void gettokdata(cio_query_blk_t *qparms);

void print_sysinfo(time_t refresh_time,
		   struct sysinfo *si,
		   struct sysinfo *si2,
		   struct vmker *vmk,
		   struct vminfo *vm,
		   struct vminfo *vm2,
		   struct procdata *proc,
		   tok_query_stats_t *tr,
		   tok_query_stats_t *tr2);

int print_dkstat(void);

/*
 * Parse command line and start up the data collection command.
 */
static void Initialize(int argc, char *argv[])
{
    int dk_sec;
    struct var sysvars;

    /*
     * check and parse command line
     */
    if (argc != 4){
    sendval(("%d:%d\n",	PARSE_ERR_1, 0));	/* make sure we have the right # args */
	exit(-1);
    }
    this_uid = atoi(argv[1]);			/* get uid we're gonna watch */

    if ((sleep_sec = atoi(argv[2])) < 1){	/* get the collection interval */
	sendval(("%d:%d\n", PARSE_ERR_2, 0));
	exit(-1);
    }
    if ((dk_sec = atoi(argv[3])) < 1){	/* get the disk collection interval */
	sendval(("%d:%d\n", PARSE_ERR_2, 0));
	exit(-1);
    }
    dk_skip_interval = dk_sec / sleep_sec;		/* Close enough for us. */
    sysconfig(SYS_GETPARMS,&sysvars,sizeof(sysvars));
    max_sys_procs = sysvars.v_proc;	/* max system processes */
    max_usr_procs = sysvars.v_maxup;	/* max user processes */
}

int main(int argc, char *argv[])
{
    struct sysinfo si1,si2;
    struct vmker vmk;
    struct vminfo vm1,vm2;
    struct procdata proc;
    tok_query_stats_t tok_q1, tok_q2;
    cio_query_blk_t tok_query1, tok_query2;
/*    double time1,time2;*/
    time_t time1, time2;

    Initialize(argc, argv);

    if (knlist(kernelnames,NUMBER_OF_KNSTRUCTS,sizeof(struct nlist)) == -1){
	perror("knlist, entry not found");
    }
    /* a couple more variables to initialize */
    tok_query1.bufptr = (caddr_t)&tok_q1;
    tok_query2.bufptr = (caddr_t)&tok_q2;
    tok_query1.buflen = tok_query2.buflen = sizeof(tok_query_stats_t);
    tok_query1.clearall = tok_query2.clearall = 0;

    get_sys_vm_info(&si2,&vmk,&vm2,&proc,&tok_query2);
    time2 = time(0);
    sleep(1);

    while (1) {
	get_sys_vm_info(&si1,&vmk,&vm1,&proc,&tok_query1);
	time1 = time(0);
	print_sysinfo(time1-time2,&si1,&si2,&vmk,&vm1,&vm2,&proc,&tok_q1,&tok_q2);
	print_dkstat();
	sleep(sleep_sec);

	get_sys_vm_info(&si2,&vmk,&vm2,&proc,&tok_query2);
	time2 = time(0);
	print_sysinfo(time2-time1,&si2,&si1,&vmk,&vm2,&vm1,&proc,&tok_q2,&tok_q1);
	print_dkstat();
	sleep(sleep_sec);
    }
    exit(0);
}

double realtime()
{
  struct timeval tp;
  gettimeofday(&tp,0);
  return((double)tp.tv_sec+tp.tv_usec*1.0e-6);
}

/****************************************************************************/
/* get_sys_vm_info(struct sysinfo *si,struct vker *vmk, struct vminfo *vm); */

int get_sys_vm_info(struct sysinfo *si,
		    struct vmker *vmk,
		    struct vminfo *vm,
		    struct procdata *prc,
		    cio_query_blk_t *tr)
{
    /*
    ** Get the system info structure from the running kernel.
    ** and update loadavg-values 
    ** Get the kernel virtual memory vmker structure
    ** Get the kernel virtual memory info structure
    */
    getkmemdata((char *)si,sizeof(struct sysinfo),N_VALUE(NLIST_SYSINFO));
    getkmemdata((char *)vmk,sizeof(struct vmker),N_VALUE(NLIST_VMKER));
    getkmemdata((char *)vm,sizeof(struct vminfo),N_VALUE(NLIST_VMINFO));
    getprocdata(prc);
/*    gettokdata(tr); */
}

/*********************************************************************/
void getkmemdata(char *buf,	    /* buffer for kernel data*/
		 int bufsize,	    /* buffers size */
		 long n_value)	    /* seek address to kernels data */
{
    static int fd;
    static initted = 0;
    /*
    ** Do stuff we only need to do once per invocation, like opening
    ** the kmem file and fetching the parts of the symbol table.
    */
    if (!initted) {
	initted = 1;
	fd = open("/dev/kmem", O_RDONLY);
	if (fd < 0) {
	    perror("kmem");
	    exit(1);
	}
    }
    /*
    ** Get the structure from the running kernel.
    */
    lseek(fd, n_value, SEEK_SET);
    read(fd, buf, bufsize);
}

#if SY_AIX4 /* rs_aix41 */
/* get info from the process table */
extern int errno;
void getprocdata(struct procdata *procs)
{
  static struct procsinfo *procinfo = NULL;
  static int nbrslots = 100;	    /* guess initial proctable size */
  int i;
  struct procsinfo *proc_p;
  int pid_index = 0;
  int nbrprocs;
  if (!procinfo)		    /* first time through - allocate space for table */
    procinfo = (struct procsinfo *)malloc(sizeof(struct procsinfo) * nbrslots);
  nbrprocs = getprocs(procinfo,	    /* get copy of process table */
		      sizeof(struct procsinfo),
		      NULL,
		      sizeof(struct fdsinfo),
		      &pid_index,
		      nbrslots);
  while (nbrprocs == nbrslots) {    /* our table is full, may not have gotten everything - loop til we do */
    free(procinfo);
    nbrslots += 100;
    procinfo = (struct procsinfo *)malloc(sizeof(struct procsinfo) * nbrslots);
    pid_index = 0;
    nbrprocs = getprocs(procinfo,   /* get copy of process table */
			sizeof(struct procsinfo),
			NULL,
			sizeof(struct fdsinfo),
			&pid_index,
			nbrslots);
  }
  procs->usr = procs->root = procs->total = 0;

  for (i=nbrslots, proc_p = procinfo; i; --i, ++proc_p) {	    /* count up entries */
    if (proc_p->pi_state == SSLEEP ||
	proc_p->pi_state == SRUN ||
	proc_p->pi_state == SZOMB ||
	proc_p->pi_state == SACTIVE) {
      ++procs->total;				/* total # proc */
      if (proc_p->pi_uid == 0)
	++procs->root;				/* # root owned procs */
      else if (proc_p->pi_uid == this_uid)
	++procs->usr;				/* # procs owned by this user */
    }
  }
}

#else /* rs_aix32 */
/* get info from the process table */
void getprocdata(struct procdata *procs)
{
  static struct procinfo *procinfo = NULL;
  static int nbrslots = 1;
  int i;
  struct procinfo *proc_p;
  if (!procinfo)		    /* first time through - get size of proc table */
    procinfo = (struct procinfo *)malloc(sizeof(struct procinfo));
  else {			    /* mark status of each slot in buffer as "unused" */
    proc_p = procinfo;
    for (i=nbrslots; i; --i, ++proc_p)
      proc_p->pi_stat = 0;
  }
  while (getproc(procinfo, nbrslots, sizeof(struct procinfo)) == -1 && errno == ENOSPC) {
    nbrslots = *(int *)procinfo + 10;		/* current size + extra */
    procinfo = (struct procinfo *)realloc(procinfo, sizeof(struct procinfo) * nbrslots);
  }
  procs->usr = procs->root = procs->total = 0;
  for (i=nbrslots, proc_p = procinfo; i; --i, ++proc_p) {	    /* count up entries */
    if (proc_p->pi_stat == SSLEEP || proc_p->pi_stat == SRUN || proc_p->pi_stat == SZOMB) {
      ++procs->total;				/* total # proc */
      if (proc_p->pi_uid == 0)
	++procs->root;				/* # root owned procs */
      else if (proc_p->pi_uid == this_uid)
	++procs->usr;				/* # procs owned by this user */
    }
  }
}
#endif

/* Open and read TR info from the token ring device driver */
void gettokdata(cio_query_blk_t *qparms)
{
  static int f = -1;
  if (f == -1) {
    if ((f = open("/dev/tok0", O_RDONLY)) == -1) {
      if (errno == EBUSY) {
	fprintf(stderr, "Unable to monitor tokenring activity\n");
	f = -2;
      }
      else {
	perror("opening tok0");
	exit(1);
      }
    }
  }
  if (f < 0)	/* couldn't open tok0, so skip that check */
    return;
  if (ioctl(f, CIO_QUERY, qparms)) {
    perror("ioctl");
    exit(1); 
  }
}


  

#define SIDELTA(a) (si->a - si2->a)
#define VMDELTA(a) (vm->a - vm2->a)
#define TRDELTA(a) (tr->a - tr2->a)

void print_sysinfo(time_t refresh_time,
		   struct sysinfo *si,
		   struct sysinfo *si2,
		   struct vmker *vmk,
		   struct vminfo *vm,
		   struct vminfo *vm2,
		   struct procdata *proc,
		   tok_query_stats_t *tr,
		   tok_query_stats_t *tr2)
{
    long cpuTime, idleTime, userTime, kernalTime, waitTime;
    int swp_proc = 0;

    idleTime = SIDELTA(cpu[CPU_IDLE]);
    userTime = SIDELTA(cpu[CPU_USER]);
    kernalTime = SIDELTA(cpu[CPU_KERNEL]);
    waitTime = SIDELTA(cpu[CPU_WAIT]);
    cpuTime = idleTime + userTime + kernalTime + waitTime;

    /*
     * emit results
     */
    sendval(("%d:%d\n", LOADUSER, 100 * userTime / cpuTime));
    sendval(("%d:%d\n", LOADSYS, 100 * kernalTime / cpuTime));
    sendval(("%d:%d\n", LOADIO, 100 * waitTime / cpuTime));
    sendval(("%d:%d\n", LOADIDLE, 100 * idleTime / cpuTime));
    sendval(("%d:%d\n", LOADCPU, 100 - (100 * idleTime / cpuTime)));
    sendval(("%d:%d\n", VM, 100 - 100 * vmk->freevmem / vmk->totalvmem));
    if (refresh_time) {		    /* avoid divide by zero */
      sendval(("%d:%d\n", PAGEIN, VMDELTA(pgspgins)/refresh_time));
      sendval(("%d:%d\n", PAGEOUT, VMDELTA(pgspgouts)/refresh_time));
      sendval(("%d:%d\n", INTSSYS, SIDELTA(syscall)/refresh_time));
      sendval(("%d:%d\n", INTSSWAP, SIDELTA(pswitch)/refresh_time));
      sendval(("%d:%d\n", INTSIO, SIDELTA(devintrs)/refresh_time));
    }
    if(SIDELTA(runocc) > 0) {
	sendval(("%d:%d\n", QUEUERUN, (SIDELTA(runque)/SIDELTA(runocc))-1));
    }
    else {
	sendval(("%d:%d\n", QUEUERUN, 0));
    }
    if (SIDELTA(swpocc) == 0) swp_proc = 0;
    else swp_proc = (int)((double)SIDELTA(swpque)/(double)SIDELTA(swpocc));
    sendval(("%d:%d\n", QUEUEMEM, swp_proc));
    sendval(("%d:%d\n", MEMACTIVE,  vmk->totalvmem - vmk->freevmem));
    sendval(("%d:%d\n", MEMFREE,  vmk->freemem));
    sendval(("%d:%d\n", PAGEREPLACABLE, VMDELTA(pgrclm)));  /* not sure about this one */
    sendval(("%d:%d\n", PROCSUSER, 100 * proc->usr / max_usr_procs));
    sendval(("%d:%d\n", PROCSTOTAL, 100 * proc->total / max_sys_procs));
    sendval(("%d:%d\n", PROCSOTHER, proc->total - proc->root - proc->usr));
/*    sendval(("%d:%d\n", NDSTATIN, TRDELTA(cc.rx_frame_lcnt)));
    sendval(("%d:%d\n", NDSTATOUT, TRDELTA(cc.tx_frame_lcnt))); */

}

print_dkstat()
{
    char *buf = NULL;    /* buffer that will hold vmount structs */
    int status = 0;
    struct vmount *vmt = NULL;
    int id = DISK1, dkIndex = 0;
    int count;
    static int dk_skip_count = 9999;
    int bufsize = INIT_BUF_SIZE;

    /* We only do this every "dk_skip_count" time we are called because
     * it is very expensive.
     */
    if (dk_skip_count < dk_skip_interval) {
	dk_skip_count++;
	return;
    }
    dk_skip_count = 0;

    if((buf = (char*) calloc(bufsize, sizeof(char))) == NULL) {
	perror("calloc");
	exit(1);
    }

    while(1) {
	if((status = mntctl(MCTL_QUERY,	bufsize, buf)) < 0) {	/* failure */
	    perror("mntctl");
	    if (buf) free(buf);
	    return(-1);
	}
	else if(status == 0) {	/* Not enough room in buf for all vmounts structs */
	    bufsize = *(int*)buf;
	    if((buf = (char*) realloc(buf, bufsize * sizeof(char))) == NULL) {
		perror("realloc");
		exit(1);
	    }
	}
	else break; /* success */
    }

    vmt = (struct vmount*) buf;
    count = status;
    for (dkIndex = 0; dkIndex < count; dkIndex++) {
	struct statfs StatusBuf;
	char *mountOver = NULL;

	if (vmt->vmt_gfstype == MNT_JFS) {
	  mountOver = (char*) calloc(vmt2datasize(vmt,VMT_STUB) + 1, sizeof(char*));
	  strncpy(mountOver, vmt2dataptr(vmt, VMT_STUB), vmt2datasize(vmt,VMT_STUB));
	  if (statfs(mountOver, &StatusBuf) < 0) {
	    if (errno != EACCES) {
		perror("statfs");
		if (buf) free(buf);
		if (mountOver) free(mountOver);
		return(-1);
	    }
	  }
	  else {
		int pct = ((double)StatusBuf.f_bfree / (double)StatusBuf.f_blocks) * 100.0;
		sendval(("%d:%d:%s\n", id, 0, mountOver));
		sendval(("%d:%d\n", id, pct));
		id++;
	    }
	}
	vmt = (struct vmount *)((char*)vmt + vmt->vmt_length); 
	if(mountOver) free(mountOver);
    }
    if (buf) free(buf);
    return(0);
}


