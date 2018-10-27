/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    
    region_num = 0;
    max_num= Machine::PAGE_SIZE/sizeof(region_descriptor);
    regions = (region_descriptor*)(Machine::PAGE_SIZE * (frame_pool->get_frames(1)));
    page_table -> register_pool(this);

    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    if(_size == 0)
        return 0;
    unsigned long start_addr = 0;
    if(region_num == 0)
        start_addr = base_address;
    else
        start_addr = regions[region_num-1].base_address + regions[region_num-1].size;
    regions[region_num].base_address = start_addr;
    regions[region_num].size = _size;
    region_num++;
    if(region_num > max_num)
        assert(false);
    Console::puts("Allocated region of memory.\n");
    return start_addr;
}

void VMPool::release(unsigned long _start_address) {
    int pos = -1;
    for(int i=0; i<region_num; i++){
        if(regions[i].base_address == _start_address){
            pos = i;
            break;
        }
    }
    //page size:4096
    for (unsigned int i = 0; i < regions[pos].size/4096; i++)
    {
        page_table->free_page(_start_address);
        _start_address = _start_address + 4096;
    }
    
    region_descriptor* old= regions;
    regions=  (region_descriptor*)(4096 * (frame_pool->get_frames(1)));
    unsigned int j=0;
    for(unsigned int i=0; i<region_num; i++){
        if(i != pos)
            regions[j] = old[i];
        j++;
    }
    frame_pool->release_frames((unsigned long)old/4096);
    // flush the TLB
    page_table->load();
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    for(unsigned long i=0; i < this->region_num; i++){
        region_descriptor cur = regions[i];
        if(_address >= cur.base_address && _address <= cur.base_address+cur.size)
            return 1;
    }
    Console::puts("Checked whether address is part of an allocated region.\n");
    return 0;
}

