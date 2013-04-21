/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include<proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
  kprintf("---------ISR----------\n");

  unsigned int pfaddr = read_cr2();// faulted address 
  //Check that pfaddr is a legal address ????

  unsigned int pdbaddr = read_cr3()&(~0xfff); //page directory base address
  
  unsigned int p = pfaddr >> 22; // upper ten bits of faulted address
  unsigned int q = (pfaddr >> 12)&0x3ff; // middle ten bits of faulted address
  unsigned int offset = pfaddr & 0xfff; // last twelve bits

  pd_t * pt = (pd_t *)pdbaddr + p ; //pth page table 
  //kprintf("%x\n",*pt);
  if(pt->pd_pres == 0)  // if page table is not present
  {
      int frm_num = get_frm(1,FR_TBL,pfaddr>>12);
      //kprintf("%d\n",frm_num);

      if(frm_num == SYSERR)
          return SYSERR;
      pt->pd_base = (0x00400000 + frm_num*4096) >> 12;
      pt->pd_pres = 1;
      write_cr3(pdbaddr);
        kprintf("faulted addr: %x xth page table: %x, content: %x\n",pfaddr,pt,p);
      return OK;
  }
  else 
  {
      pt_t * pt1 = (pt_t *) (pt->pd_base << 12) + q;  // qth page 
        kprintf("faulted addr: %x xth page table: %x, content: %x\n",pfaddr,pt1,*pt1);
      if(pt1 -> pt_pres == 0)// if page is not present, we need to bring the mapped bs to memory
      {
          int i;
          for(i=0; i<=MAX_ID; i++)
          {
              //bs_map_t * bs = &bsm_tab[i];
              //kprintf("-----%d\n",bsm_tab[i].bs_status);
              //kprintf("-----%x\n",bsm_tab[i].bs_vpno+bsm_tab[i].bs_npages);
              //kprintf("-----%d\n\n",bsm_tab[i].bs_pid);
              // enter only when frame is mapped and pid is currpid and virtual page is within that range
              //while(bs != NULL)
              //{
                if(bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_pid == currpid && (pfaddr >> 12) >= bsm_tab[i].bs_vpno && ((pfaddr >> 12) < bsm_tab[i].bs_vpno + bsm_tab[i].bs_npages ))
                {
                      int frm_num = get_frm(1,FR_PAGE,pfaddr>>12);
                      //kprintf("%d\n",frm_num);
                      if(frm_num == SYSERR)
                          return SYSERR;
                      pt1->pt_base = (0x00400000 + frm_num*4096) >> 12;
                      pt1 -> pt_pres = 1;
                      //kprintf("%x\n",*pt1);
                      read_bs((char*)(pt1->pt_base<<12),i,(pfaddr>>12)-bsm_tab[i].bs_vpno);
                      write_cr3(pdbaddr);
                      //void * bsaddr = 0x00800000 + i*256*4096 + ((pfaddr >> 12) - bsm_tab[i].bs_vpno)*4096;
                      //memcpy((void*)(pt1->pt_base << 12), bsaddr, 4096);
                      return OK;

                }
                //bs = bs->next;
              //}
          }
          //no mapping was found, return syserr
          return SYSERR;
      }
      else
          return OK;
  }
  // check that a is a legal address

  //return OK;
}


