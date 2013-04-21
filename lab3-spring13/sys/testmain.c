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

void process() 
        { 
            kprintf("page directory base address: %x\n",proctab[currpid].pdbr);
            int *x; 
            int *y; 
            int *z; 
            int temp; 
            x = vgetmem(1024);  /* allocates some memory in the virtual heap which is in virtual memory */ 
            kprintf("base address: %x\n",x);
            *x = 100; 
            x++; 
            *x = 200; 
            temp = *x;  /* You are reading back from virtual heap to check if the previous write was successful */
            kprintf("[%x] = %d\n",x,*x);
            kprintf("[%x] = %d\n",(x-1),*(x-1));
            y = vgetmem(1024);  /* allocates some memory in the virtual heap which is in virtual memory */ 
            kprintf("base address: %x\n",y);
            *y = 300; 
            y++; 
            *y = 400; 
            temp = *y;  /* You are reading back from virtual heap to check if the previous write was successful */
            kprintf("[%x] = %d\n",y,*y);
            kprintf("[%x] = %d\n",(y-1),*(y-1));
            vfreemem(--x, 1024); /* frees the allocation in the virtual heap */ 
            z = vgetmem(1024);  /* allocates some memory in the virtual heap which is in virtual memory */ 
            kprintf("base address: %x\n",z);
            *z = 500; 
            z++; 
            *z = 600; 
            temp = *z;  /* You are reading back from virtual heap to check if the previous write was successful */
            kprintf("[%x] = %d\n",z,*z);
            kprintf("[%x] = %d\n",(z-1),*(z-1));
            vfreemem(--y, 1024); /* frees the allocation in the virtual heap */ 
            vfreemem(--z, 1024); /* frees the allocation in the virtual heap */ 
        }

int main() {
	char *addr = (char*) 0x40000000; //1G
	bsd_t bs = 1;

	int i = ((unsigned long) addr) >> 12;	// the ith page

	kprintf("\n\nHello World, Xinu lives\n\n");

    int pid = vcreate(process,MINSTK,100,20,"test");
    resume(pid);

    kprintf("finished\n");

	return 0;
}
