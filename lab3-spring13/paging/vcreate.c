/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	kprintf("-----------------vcreate--------------------!\n");
    int i;
    struct pentry * pptr; 
    int pid = create(procaddr,ssize,priority,name,nargs,args);
    kprintf("debug\n");
    pptr = &proctab[pid];
    //find an empty backing store as virual heap
    int iffind = 0;
    for(i=0; i<=MAX_ID; i++)
    {
        if(bsm_tab[i].bs_status == BSM_UNMAPPED)
        {
            bsm_tab[i].bs_status = BSM_MAPPED;
            bsm_tab[i].bs_pid = pid;
            bsm_tab[i].bs_vpno = (4096*NBPG) >> 12;
            bsm_tab[i].bs_npages = hsize;
            iffind = 1;
            break;
        }
    }
    if(iffind==0)
        return SYSERR;
    // initilize memlist for the process
    //pptr->vmemlist = (struct mblock*)(2048*NBPG + (i+1)*256*NBPG)-1;
	kprintf("vmemlist addr: %x--!\n",&pptr->vmemlist);
    pptr->vmemlist.mnext = (struct mblock*)(2048*NBPG + i*256*NBPG);
	kprintf("vmemlist first element addr: %x--!\n",pptr->vmemlist.mnext);
    //pptr->vmemlist->mnext = (struct mblock *)roundmb(4096*NBPG);
    pptr->vmemlist.mnext->mnext = (struct mblock*)NULL;
    pptr->vmemlist.mnext->mlen = (int) truncew(hsize*NBPG);
    pptr->vhpnpages = hsize;
    pptr->vhpno = (4096*NBPG) >> 12;
    pptr->store = i;
	kprintf("-------------vcreate finishe-----------------!\n");
	return pid;

    //----------------------finished----------------------------

    
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
