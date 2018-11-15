/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"
#include "thread.H"

extern Scheduler* SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
    //the head of blocked queue
    this->head = new Queue();
    queue_size = 0;
}

void BlockingDisk::wait_until_ready(){
   while (!is_ready()) {
            this->blocked_enqueue(Thread::CurrentThread());
            SYSTEM_SCHEDULER->yield();//yield the CPU for other threads
   }
}

void BlockingDisk::blocked_enqueue(Thread *_thread) {
    head->enqueue(_thread);//add thread to queue
    queue_size += 1;
}
void BlockingDisk::blocked_resume() {
  if (queue_size > 0){
    Thread *thread = this->head->dequeue();
    SYSTEM_SCHEDULER->resume(thread); 
  }
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::read(_block_no, _buf);

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::write(_block_no, _buf);
}
