/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int page_replace_policy;
void insertq();
fr_map_t * getnext();
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
    int i,j;
  kprintf("----init_frm----\n");
  for(i=0; i<NFRAMES; i++)
  {
      frm_tab[i].fr_status = FRM_UNMAPPED;
      frm_tab[i].next = NULL;
      frm_tab[i].fr_loadtime = 0;
  }
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
//SYSCALL get_frm(int* avail)
SYSCALL get_frm(int fnum,int type,unsigned int vpno)
{
  //kprintf("----get_frm----\n");
    int i,j;
    int countnum = fnum;
// search in the frm_tab to find a unmapped frame
  for(i=0;i<NFRAMES; i++)
  {
      if(frm_tab[i].fr_status == FRM_UNMAPPED)
      {
          if(countnum == fnum)
              j = i ;
          countnum--;
          if(countnum == 0)
          {
              int k;
              for(k=j; k<=i; k++)
              {
                  frm_tab[k].fr_status = FRM_MAPPED;
                  frm_tab[k].fr_type = type;
                  frm_tab[k].fr_pid = currpid;
                  frm_tab[k].fr_vpno = vpno;
                  frm_tab[k].next = NULL;
                  if(type == FR_PAGE)
                    insertq(k);
              }
              return j;
          }
      }
      else
          countnum = fnum;
  }

    kprintf("replacement happenes\n");
    //get the first element from queue accroding to replacement policy
    fr_map_t * rp = getnext();
    if(rp == NULL)
        return SYSERR;
    else
        fifoq.next = rp->next;

    for(i=0; i<=MAX_ID; i++)
    {
        if(bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_pid == rp->fr_pid && rp->fr_vpno >= bsm_tab[i].bs_vpno && (rp->fr_vpno < bsm_tab[i].bs_vpno + bsm_tab[i].bs_npages ))
        {
            write_bs((char*)(0x400000+(rp-frm_tab)*NBPG),i,rp->fr_vpno-bsm_tab[i].bs_vpno);
            //kprintf("bsm_vpno: %x, fr_vpno: %x\n",rp->fr_vpno,bsm_tab[i].bs_vpno);
            unsigned int * phyaddr = (unsigned int*)(0x800000 + (i<<20) + (rp->fr_vpno-bsm_tab[i].bs_vpno)*NBPG);
            //kprintf("%x\n",i);
            int * maddr = (int*)(0x400000 + (rp-frm_tab)*NBPG);
            kprintf("write back from %x, %d(%d) to backing store %x\n",maddr,*phyaddr,*maddr,phyaddr);
            frm_tab[rp-frm_tab].fr_status = FRM_MAPPED;
            frm_tab[rp-frm_tab].fr_type = type;
            frm_tab[rp-frm_tab].fr_pid = currpid;
            frm_tab[rp-frm_tab].fr_vpno = vpno; 
            frm_tab[rp-frm_tab].fr_loadtime = 128; 
            frm_tab[rp-frm_tab].next = NULL; 
            insertq(rp-frm_tab);
            return rp-frm_tab;
        }
    }
  
  return SYSERR;
  //return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

  if(frm_tab[i].fr_status == FRM_UNMAPPED)
      return OK;
  else
  {
      frm_tab[i].fr_status = FRM_UNMAPPED;
  }

  return OK;
}

/*-------------------------------------------------------------------------
 * create page structure for a new process, create the page table for shared address 
 *-------------------------------------------------------------------------
 */
unsigned int create_ps()
{
    kprintf("--------------create page structure-------------\n");

    int i,j;
    unsigned int pdbaddr = 0x00400000 + get_frm(1,FR_DIR,0)*NBPG;
    pd_t * pdp = (pd_t*)pdbaddr; 

    for(i=0;i<4;i++)
    {
        unsigned int pdtaddr = 0x00400000 + get_frm(1,FR_TBL,0)*NBPG;
        pt_t * ptp = (pt_t *)pdtaddr; 

        (pdp+i)->pd_pres = 1;
        (pdp+i)->pd_write = 1;
        (pdp+i)->pd_base = pdtaddr >> 12;

        for(j=0;j<1024;j++)
        {
            (ptp+j)->pt_pres = 1;
            (ptp+j)->pt_write = 1;
            (ptp+j)->pt_base =(i*1024*NBPG + j*NBPG) >> 12;
        }
    }
    return pdbaddr;
}

unsigned int init_ps()
{
    kprintf("--------------init page structure-------------\n");

    int i,j;
    unsigned int pdbaddr = 0x00400000 + get_frm(1,FR_DIR,0)*NBPG;
    pd_t * pdp = (pd_t*)pdbaddr; 

    for(i=0;i<4;i++)
    {
        unsigned int nullpdbaddr = proctab[0].pdbr;
        pd_t * nullpd = (pd_t *)nullpdbaddr; 

        (pdp+i)->pd_pres = 1;
        (pdp+i)->pd_write = 1;
        (pdp+i)->pd_base = (nullpd+i)->pd_base;
    }
    return pdbaddr;
}

void insertq(int k)
{
    lastfifoq->next = &frm_tab[k];
    lastfifoq = &frm_tab[k];
}

fr_map_t * getnext()
{
    fr_map_t *p,*q,*leastp;
    int count=256;

    //replacement for FIFO
    if(page_replace_policy == FIFO)
    {
        q = fifoq.next; 
        fifoq.next = q->next;
        return q;
    }
    //replacement for AGING 
    else if( page_replace_policy == AGING) 
    {
        //kprintf("fifoq: %d\n",fifoq.next);
        if(fifoq.next == NULL)
            return NULL;
        // reset all age accroding to acc bit of the pd_t
        p=fifoq.next;
        q=&fifoq;
        int i=0;
        while(p!=NULL)
        {
            p->fr_loadtime = p->fr_loadtime / 2;
            pd_t * pdeaddr = (pd_t *)((pd_t*)proctab[p->fr_pid].pdbr + (p->fr_vpno >>10));
            pt_t * pte = (pt_t *)(pdeaddr->pd_base << 12) + (p->fr_vpno&0x3ff);
            kprintf("%d,%x,%x\n",i++,p,pdeaddr);
            if(pte->pt_acc == 1)
                p->fr_loadtime += 128;
            if(p->fr_loadtime < count)
            {
                leastp = q;
                count = p->fr_loadtime;
            }
            //reset pt_acc to 0, means didn't accessed
            pte->pt_acc = 0;
            p=p->next;
            q=q->next;
        }
        p = leastp->next;
        leastp->next = p->next;
        return p;

    }
}
