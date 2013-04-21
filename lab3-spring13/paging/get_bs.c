#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id 
    if(bs_id < 0 || bs_id > MAX_ID)
        return SYSERR;
    if(npages == 0 || npages > 200)
        return SYSERR;
    if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
    {
        bsm_tab[bs_id].bs_status = BSM_MAPPED;
        bsm_tab[bs_id].bs_pid = currpid;
        bsm_tab[bs_id].bs_vpno = 0;
        bsm_tab[bs_id].bs_npages = npages;
        
    }
    else if(bsm_tab[bs_id].bs_status == BSM_MAPPED && bsm_tab[bs_id].bs_pid == currpid)
    {
        if(256 - bsm_tab[bs_id].bs_npages >= npages)
            bsm_tab[bs_id].bs_npages += npages;
    }
    else
        return SYSERR;
    */
    if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
        return OK;
    else
        return SYSERR;
    

}


