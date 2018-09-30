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
    page_directory = (unsigned long*)(kernel_mem_pool->get_frames(1)*PAGE_SIZE);
    unsigned long* page_table = (unsigned long*)(kernel_mem_pool->get_frames(1)*PAGE_SIZE);
    unsigned long address = 0;
    
    for(int i=0; i<1024; i++){
        page_table[i] = address | 3;
        address += PAGE_SIZE;
    }
    
    page_directory[0] = (unsigned long) page_table;
    page_directory[0] |= 3;
    
    for(unsigned int i=1; i<1024; i++){
        page_directory[i] = 0 | 2;
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
    write_cr3((unsigned long)current_page_table->page_directory);
    paging_enabled = 1;
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    unsigned long address = read_cr2();
    unsigned long page_dir_address = address >> 22;
    unsigned long page_table_address = address >> 12;
    
    unsigned long* page_dir = (unsigned long*) read_cr3();
    unsigned long err_code = _r->err_code;
    unsigned long* new_page_table = (unsigned long*)((page_dir[page_dir_address]>>12)<<12);
    
    
    
    if((err_code & 1) == 0){
        if(page_dir[address >> 22] & 1 == 1){
            new_page_table[page_table_address & 0x03FF] = PageTable::process_mem_pool->get_frames(1) * PAGE_SIZE | 3;
        }
        else{
            page_dir[page_dir_address] = (unsigned long)(kernel_mem_pool->get_frames(1) * PAGE_SIZE | 3);
            for(int i=0; i<1024; i++){
                new_page_table[i] = 4;
            }
            new_page_table[page_table_address & 0x03FF] = PageTable::process_mem_pool->get_frames(1) * PAGE_SIZE | 3;
        }
    }
  Console::puts("handled page fault\n");
}

