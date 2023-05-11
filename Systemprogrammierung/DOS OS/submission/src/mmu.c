#include "../include/mmu.h"

/* Memory start address: Startadresse unseres 12 bit Adressraumes */
static void *mem_start_addr;

/* page table base register = Seitentabellenbasisregister */
static addr_t *ptbr;

/* +------------------------------------+ *
 * | Hier ist Platz für Hilfsfunktionen | *
 * +------------------------------------+ */



/* -------------------------------------- */

void mmu_init(void* mem)
{
	mem_start_addr = mem;
	ptbr = mem;
}

int switch_process(int proc_id)
{
    if (proc_id >= 0 && proc_id < PROC_NUM  ) {
        ptbr = (proc_id * PT_AMOUNT) + ((addr_t*) mem_start_addr);
        return 0;
    } else {
        return 1;
    }
}

addr_t mmu_translate(addr_t va, req_type req)
{
    pte * pteConvert = (pte *)((va / 256) + ptbr);
    if (pteConvert->info / 8 != 1){
        return MY_NULL;
    }
    if ((req & pteConvert->offset) == 0 ){
        return MY_NULL;
    }
    if (req == READ || req==EXECUTE ){
        pteConvert->info |=  0X4 ;
    }
    else{
        pteConvert->info |= 0X6 ;
    }
    return ((pteConvert->page_num * 256) + (va % PAGE_SIZE ));
}

addr_t mmu_check_request(request r)
{
    if (switch_process(r.p_num) == 1 ){
        return MY_NULL;
    }
    return mmu_translate(r.addr,r.type);
}
