/*
 File: scheduler.C
 
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

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
Queue::Queue(){
    thread = NULL;
    next = NULL;
}

Queue::Queue(Thread* t){
    thread = t;
    next = NULL;
}

Queue::Queue(Queue& q){
    thread = q.thread;
    next = q.next;
}

void Queue::enqueue(Thread* t){
    if(thread == NULL)
        thread = t;
    else{
        if(next == NULL)
            next = new Queue(t);
        else
            next->enqueue(t);
    }
}

Thread* Queue::dequeue(){
    if(thread == NULL)
        return NULL;
    Thread* cur = thread;
    if(next != NULL){
        thread = next->thread;
        next = next->next;
    }
    else
        thread = NULL;
    return cur;
}

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
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() { 
  size = 0;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
    if(size > 0){
        size -= 1;
        //find the next thread in the ready queue, dispatch to it
        Thread* next_thread = ready_queue.dequeue();
        Thread::dispatch_to(next_thread);
    }
    else{
            //assert(false);
    }
}

void Scheduler::resume(Thread * _thread) {
    size += 1;
    ready_queue.enqueue(_thread);
}

void Scheduler::add(Thread * _thread) {
    size += 1;
    ready_queue.enqueue(_thread);
}

void Scheduler::terminate(Thread * _thread) {
    for(int i = 0; i < size; i++){
        Thread* cur = ready_queue.dequeue();
        if(cur->ThreadId() == _thread->ThreadId())
            size -= 1;
        else
            ready_queue.enqueue(cur);
    }
}
