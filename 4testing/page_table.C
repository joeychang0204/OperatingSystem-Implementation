#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    PageTable::kernel_mem_pool = _kernel_mem_pool;
    PageTable::process_mem_pool = _process_mem_pool;
    PageTable::shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    page_directory = (unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
    unsigned long* page_table = (unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
    unsigned long address = 0;
    
    // writable , present
    for(int i=0; i<1024; i++){
        page_table[i] = address | 3;
        address += PAGE_SIZE;
    }
    
    page_directory[0] = (unsigned long) page_table;
    page_directory[0] |= 3;
    
    for(int i=1; i<1023; i++){
        // writable, not present
        //not sure
        page_directory[i] = 2;
    }
    //point the last entry to the PD's beginning
    page_directory[1023] = (unsigned long)page_directory | 3;
    
    registered_num = 0;
    //100 is the maximum size of registered_vmpools
    for(int i=0; i<100; i++){
        registered_vmpools[i] = NULL;
    }
    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    current_page_table = this;
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    write_cr3((unsigned long)(current_page_table->page_directory));
    paging_enabled = 1;
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    //read the page fault address from CR2
    unsigned long address = read_cr2();
    //page_dir address : 1023 1023 X (original : X Y offset)
    unsigned long *page_dir_address = (unsigned long*)(((address>>22) <<2) | (0xFFFFF<<12));
    //page_table address : 1023 X Y (original : X Y offset)
    unsigned long *page_table_address = (unsigned long*)(((address>>12) <<2) |(0x3FF << 22));
    unsigned long err_code = _r->err_code;
    
    //if this is a page fault accessing something not valid
    if((err_code & 1) == 0){
        VMPool** vm_pools = current_page_table->registered_vmpools;
        //find which vm pool is the page fault address in
        int pos = -1;
        for(int i = 0; i < current_page_table->registered_num; i++){
            if(vm_pools[i] != NULL && vm_pools[i]->is_legitimate(address)){
                pos = i;
                break;
            }
        }
        if(vm_pools[pos] != NULL){
            //successfully found the vm pool
            if((*page_dir_address & 1) == 1){
                //page fault happenes at PT, get a frame for it
                *page_table_address = PageTable::process_mem_pool->get_frames(1);
                *page_table_address = (*page_table_address << 12) | 5;
            }
            else{
                //page fault happenes at page directory, need to get frame for PD and PT
                *page_dir_address = PageTable::process_mem_pool->get_frames(1);
                *page_dir_address = (*page_dir_address << 12) | 3;
                
                //setting the PDE to the PT's address
                address = (address>>22)<<22;
                for(int i=0; i<1024; i++){
                    unsigned long *tmp = (unsigned long*) (((address >> 12) << 2 ) | (0x03FF<<22));
                    *tmp = 4;
                    address = ((address>>12)+1)<<12;
                }
                
                *page_table_address = PageTable::process_mem_pool->get_frames(1);
                *page_table_address =  (*page_table_address << 12) | 3;
            }
            
        }
    }
    
  Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool *_pool) {
    //if we can still register new vm pool
    if(registered_num<100){
        this->registered_vmpools[this->registered_num] = _pool;
        this->registered_num++;
    }
    else
        Console::puts("Register failed\n");
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    
    unsigned long frame_num;
    unsigned long *PTE_pointer;
    PTE_pointer = (unsigned long *) (((_page_no >> 12) << 2 ) | (0x03FF<<22));
    frame_num = (*PTE_pointer) >> 12;
    *PTE_pointer = 0;
    process_mem_pool->release_frames(frame_num);
    Console::puts("Page freed\n");
    
}

