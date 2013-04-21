/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main() {
    srpolicy(AGING);
    int * bs1 = (int *)(2048*NBPG + 1*256*NBPG);
    int j;
	//char *addr = (char*) 0x40000000; //1G
	char *addr = (char*) 0x40380000; //1G
	bsd_t bs = 1;

	int i = ((unsigned long) addr) >> 12;	// the ith page

	kprintf("\n\nHello World, Xinu lives\n\n");

	get_bs(bs, 200);


	char *addr1 = (char*) 0x80380000; //1G
	int i1 = ((unsigned long) addr1) >> 12;	// the ith page
    kprintf("content in bs addr(%x) %d\n",bs1,*bs1);
	if (xmmap(i1, (bsd_t)bs, 200) == SYSERR) {
		kprintf("xmmap call failed\n");
		return 0;
    }
    for(j=0;j<150;j++)
    {
        *addr1 = 17;
        addr1 += NBPG;
    }
	//kprintf("\n0x%08x: %d\n", addr1, *addr1);
	xmunmap(i1);
    kprintf("content in bs addr(%x) %d\n",bs1,*bs1);

	if (xmmap(i, (bsd_t)bs, 200) == SYSERR) {
		kprintf("xmmap call failed\n");
		return 0;
    }
    for(j=0;j<150;j++)
    {
        *addr = 44;
        addr += NBPG;
    }
    //addr = (char*) 0x40000000;
    //*addr = 5;
	xmunmap(i);
    for(j=0;j<150;j++)
        kprintf("content in bs addr(%x) %d\n",(bs1+NBPG),*(bs1+NBPG));

	/*for (i = 0; i < NFRAMES; i++) {
        kprintf("0x%08x: %d\n",addr,i);
		*addr =  1;
		addr += NBPG;	//increment by one page each time
	}

	addr = (char*) 0x40000000; //1G
	for (i = 0; i < NFRAMES; i++) {
		kprintf("0x%08x: %d\n", addr, *addr);
		addr += NBPG;       //increment by one page each time
	}*/

    //char * pdbaddr = (char*)proctab[currpid].pdbr;
    //char * ptaddr = (pd_t*)pdbaddr + (0x40000000 >> 22);
    //kprintf("page directory addr: %x  page directory entry: %x\n",pdbaddr,*ptaddr);
    //char * paddr = &ptaddr[(0x40000000 >> 12)&0x3ff];
    //kprintf("page table addr: %x  page table entry: %x\n",paddr,*paddr);

	//i = 0x40000000 >> 12; //1G
    //for(j=0; j<6; j++)
	    //xmunmap(i + j*200);
    //*addr = 'z';

	return 0;
}
