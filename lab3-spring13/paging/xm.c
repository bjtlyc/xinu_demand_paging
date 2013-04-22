/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  /* sanity check ! */

  if ( (virtpage < 4096) || ( source < 0 ) || ( source > MAX_ID) ||(npages < 1) || ( npages >200)){
	kprintf("xmmap call error: parameter error! \n");
	return SYSERR;
  }


  /*--------------------------modified---------------------------*/
  bs_map_t * next, * p;

    if(bsm_tab[source].bs_status == BSM_UNMAPPED)
    {
        kprintf("xmmap - %x, bsnum: %d, npages: %d\n",virtpage,source,npages);
        bsm_tab[source].bs_status = BSM_MAPPED;
        bsm_tab[source].bs_pid = currpid;
        bsm_tab[source].bs_vpno = virtpage;
        bsm_tab[source].bs_npages = npages;
        bsm_tab[source].next = NULL;
    }
    else
    {
        /*p = &bsm_tab[source];
        next = bsm_tab[source].next;
        while(next!=NULL)
        {
            p = p->next;
            next = next->next;
        }
        bs_map_t * new_map = getmem(sizeof(bs_map_t)); 
        new_map->bs_status = BSM_MAPPED;
        new_map->bs_pid = currpid;
        new_map->bs_vpno = virtpage;
        new_map->bs_npages = npages;
        new_map->next = NULL;
        p->next = new_map;*/
        return SYSERR;
    }
    /* not sure about it
    else if(bsm_tab[source].bs_status == BSM_MAPPED && bsm_tab[source].bs_pid == currpid)
    {
        if(256 - bsm_tab[source].bs_npages >= npages)
            bsm_tab[source].bs_npages += npages;
    }*/
    
    return OK;

}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage )
{
  /* sanity check ! */
  if ( (virtpage < 4096) ){ 
	kprintf("xmummap call error: virtpage (%d) invalid! \n", virtpage);
	return SYSERR;
  }

  int i;
  kprintf("xmunmap implemented!");

  for(i=0;i<=MAX_ID;i++)
  {
      if(bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_vpno <= virtpage && (bsm_tab[i].bs_vpno+bsm_tab[i].bs_npages) > virtpage)
      {
          bsm_tab[i].bs_status = BSM_UNMAPPED;
          unsigned int pdbaddr = proctab[currpid].pdbr;
          pd_t * pdep = (pd_t*)pdbaddr + (bsm_tab[i].bs_vpno>>10);  
          int page=0;
          //write the page that have accessed back to the backing store
          if(pdep->pd_pres != 0)
          {
                pt_t * ptbp = (pt_t*)(pdep->pd_base << 12);
                pt_t * ptep = &ptbp[bsm_tab[i].bs_vpno&0x3ff];
                while((ptep-ptbp) < 1024 && page < bsm_tab[i].bs_npages)
                {
                    if(ptep->pt_pres == 1)
                    {
                        write_bs((char*)(ptep->pt_base<<12),i,page);
                        ptep->pt_pres = 0;
                        int index = ((ptep->pt_base<<12)-1024*NBPG)/NBPG;
                        frm_tab[index].fr_status = FRM_UNMAPPED;
                    }
                    page++;
                    ptep++;
                }
          }
          else
              page = 1024 - (bsm_tab[i].bs_vpno&0x3ff);
          //if the mapping virtual address is in different page table
          if(page < bsm_tab[i].bs_npages)
          {
             pdep++;
             if(pdep->pd_pres != 0)
             {
                pt_t * ptbp = (pt_t*)(pdep->pd_base << 12);
                pt_t * ptep = &ptbp[0];
                while((ptep-ptbp) < 1024 && page < bsm_tab[i].bs_npages)
                {
                    if(ptep->pt_pres == 1)
                    {
                        write_bs((char*)(ptep->pt_base<<12),i,page);
                        ptep->pt_pres = 0;
                        int index = ((ptep->pt_base<<12)-1024*NBPG)/NBPG;
                        frm_tab[index].fr_status = FRM_UNMAPPED;
                    }
                    page++;
                    ptep++;
                }
             }
          }
          return OK;
      }
  }
  return SYSERR;
}

