/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

	kprintf("----------------get memory--------------\n");
	STATWORD ps;    
	struct	mblock	*p, *q, *leftover;

	disable(ps);
	if (nbytes==0 || proctab[currpid].vmemlist.mnext== (struct mblock *) NULL) {
		restore(ps);
		return( (WORD *)SYSERR);
	}
	nbytes = (unsigned int) roundmb(nbytes);
	for (q= &proctab[currpid].vmemlist,p=proctab[currpid].vmemlist.mnext ;
	     p != (struct mblock *) NULL ;
	     q=p,p=p->mnext)
		if ( p->mlen == nbytes) {
			q->mnext = p->mnext;
			restore(ps);
			return 4096*NBPG + ( (WORD *)p - (WORD*)(2048*NBPG+proctab[currpid].store*256*NBPG));
		} else if ( p->mlen > nbytes ) {
			leftover = (struct mblock *)( (unsigned)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
            kprintf("alloc: %x,leftover addr: %x, len: %d\n",p,leftover,leftover->mlen);
			restore(ps);
			return 4096*NBPG + ( (WORD *)p - (WORD*)(2048*NBPG+proctab[currpid].store*256*NBPG));
		}
	restore(ps);
	return( SYSERR );
}


