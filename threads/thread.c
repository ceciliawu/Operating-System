#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"
// global variables declarations

struct thread* ready_queue;
struct thread* current;
struct thread* exit_queue;
int thread_valid[THREAD_MAX_THREADS];


/* This is the thread control block */
struct thread {
    ucontext_t mycontext;
    struct thread* next;
    Tid myid;
    int state;//0 for exit
    //int ready_flag;
};


/***********Helper Functions*********************/

/* thread starts by calling thread_stub. The arguments to thread_stub are the
 * thread_main() function, and one argument to the thread_main() function. */
void
thread_stub(void (*thread_main)(void *), void *arg)
{
    int interrupt_state = interrupts_set(1);
	Tid ret;

	thread_main(arg); // call thread_main() function with arg
	ret = thread_exit(THREAD_SELF);
	// we should only get here if we are the last thread. 
	assert(ret == THREAD_NONE);
	// all threads are done, so process should exit
        interrupts_set(interrupt_state);
	exit(0);
}

//function to insert a new thread at the end of ready queue
void 
insert_at_end (struct thread* new_ready)
{
    //if the ready_queue is empty
    if(ready_queue == NULL)
    {
        ready_queue = new_ready;
        return;
    }
    
    struct thread* curr;
    curr = ready_queue;
    if(curr != ready_queue)
        printf("ERROR\n");
    while(curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new_ready;
    return;
        
}

void 
insert_at_end_exit (struct thread* new_exit)
{
    //if the ready_queue is empty
    if(exit_queue == NULL)
    {
        exit_queue = new_exit;
        return;
    }
    
    struct thread* curr;
    curr = exit_queue;
    while(curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new_exit;
    return;        
}
/************************** ready queue linked list functions**********************************/

void remove_first()
{           

    if(ready_queue == NULL)
    {
        return;
    }
        ready_queue = ready_queue->next;

}

void remove_last()
{
    struct thread * head = ready_queue;
    if(head==NULL)
        return;
    struct thread * curr = head;
    struct thread * previous = NULL;
    while(curr->next != NULL)
    {
        previous = curr;
        curr = curr->next;
    }
    if(previous == NULL)
    {
        ready_queue=NULL;
    }
    else
    {
        previous->next=NULL;
    }
    return;
}

void remove_by_index(Tid index)
{
    Tid num = 1;
    if(ready_queue == NULL)
        return;
    struct thread * head = ready_queue;
    struct thread * curr = head;
    struct thread * previous = NULL;
    while((num<THREAD_MAX_THREADS)&&(curr!=NULL))
    {  
        if (curr->myid==index)
        {
            //remove the element
            if(curr == ready_queue)
            {
                remove_first();
            }
            else if (curr->next == NULL)
            {
                remove_last();
            }
            else
            {
                previous->next = curr->next;
            }
            
            return;
        }
        previous = curr;
        curr = curr->next;
        num++;
    }
	return;
    return;
}

struct thread* find_index(Tid index)
{
    struct thread * curr = ready_queue;
    while(curr!=NULL)
    {
        if(curr->myid == index)
            return curr;
        curr = curr->next;
    }
    return NULL;
}


/**********************************************************************************/
void
thread_init(void)
{
    current = malloc(sizeof(struct thread));
    current->myid = 0;
    getcontext(&current->mycontext);
    current->next = NULL;
    current->state = 1;
    ready_queue = NULL;
    Tid i;
    for(i = 1; i < THREAD_MAX_THREADS; i++)
    {
        thread_valid[i] = 1;
    }
        
    
}

Tid
thread_id()
{
	//TBD();    if(current != NULL)
        return current->myid;
    return THREAD_INVALID;
}

Tid
thread_create(void (*fn) (void *), void *parg)
{
    int interrupt_state = interrupts_set(0);
    //iterate through the stack_array to find an appropriate index
    Tid iter;
    for (iter = 1; iter< THREAD_MAX_THREADS;iter++)
    {
        if( thread_valid[iter]==1)
        {
            struct thread* new_thread = malloc(sizeof(struct thread));
            void* new_stack = malloc(THREAD_MIN_STACK+8);
            //check if there's memory
            if((new_thread == NULL)|| (new_stack == NULL))
            {   
                interrupts_set(interrupt_state);
                return THREAD_NOMEMORY;
            }
            new_thread->myid = iter;
            new_thread->next = NULL;
            getcontext(&new_thread->mycontext);
            thread_valid[iter] = 0;
            //set the corresponding registers
            new_thread->mycontext.uc_mcontext.gregs[REG_RIP] = (long int) thread_stub;
            new_thread->mycontext.uc_mcontext.gregs[REG_RDI] = (long int) (*fn);
            new_thread->mycontext.uc_mcontext.gregs[REG_RSI] = (long int) parg;
            new_thread->mycontext.uc_mcontext.gregs[REG_RSP] = (long long) new_stack + THREAD_MIN_STACK+8;
            new_thread->mycontext.uc_stack.ss_sp=new_stack;
            new_thread->state = 1;
            //put new_thread into ready queue
            insert_at_end(new_thread);
            interrupts_set(interrupt_state);
            return iter;
        }
    }
    if (iter == THREAD_MAX_THREADS)
    {
        interrupts_set(interrupt_state);
        return THREAD_NOMORE;
    }
        interrupts_set(interrupt_state);
	return THREAD_FAILED;
}

Tid
thread_yield(Tid want_tid)
{
    //disable interrupt
    int interrupt_state = interrupts_set(0);
    //check if current is in exit state
    if(current->state == 0)
    {
        if(ready_queue == NULL)
        {
            interrupts_set(interrupt_state);
            return THREAD_NONE;
        }
        insert_at_end_exit(current);
        int ready_flag= 0;
        getcontext(&current->mycontext);
        if(ready_flag == 0)
        {        
            current = ready_queue;
            ready_queue = ready_queue->next;
            current->next = NULL;
            ready_flag = 1;
            setcontext(&(current->mycontext));  
        }
    }
    

    //we first clear all the exit queues

     while (exit_queue != NULL)
     {
         struct thread* temp_del = exit_queue;
         exit_queue = exit_queue->next;
         free(temp_del->mycontext.uc_stack.ss_sp);
         thread_valid[temp_del->myid]=1;
         free(temp_del);
         temp_del = NULL;
     }
     exit_queue = NULL;
     Tid thread_id;
    if(want_tid==THREAD_ANY)
    {
        if(ready_queue==NULL)
        {
            interrupts_set(interrupt_state);
            return THREAD_NONE;
        }
       
        thread_id=ready_queue->myid;
        
    }
    else if(want_tid==THREAD_SELF)
    {
        thread_id=current->myid;
        interrupts_set(interrupt_state);
        return thread_id;
    }
    else
    {
        thread_id=want_tid;
    }
    if(thread_id >= THREAD_MAX_THREADS ||thread_id<0)
    {
        interrupts_set(interrupt_state);
        return THREAD_INVALID;
    }

    if(current->myid==thread_id)
    {
        interrupts_set(interrupt_state);
        return thread_id;
    }
     

    
    //when just need the first thread in the ready queue
    struct thread* next;
    if(want_tid == THREAD_ANY){
        next = ready_queue;
    }
    //when need to find the want_tid
    else 
    {
        next = find_index(want_tid);
        if(next == NULL)
        {
            interrupts_set(interrupt_state);
            return THREAD_INVALID;
        }
    }
    
    // do the actual yield
    if(thread_id != next->myid)
        printf("ERRRORRR\n");
    int ready_flag= 0;
        getcontext(&current->mycontext);
        if(ready_flag == 0)
        {        
            insert_at_end(current);
            current = next;
            remove_by_index(thread_id);
            current->next = NULL;
            ready_flag = 1;
            setcontext(&(current->mycontext));        
        }
      interrupts_set(interrupt_state);
      return thread_id;
   
    interrupts_set(interrupt_state);
    return THREAD_FAILED;
}

Tid
thread_exit(Tid tid)
{
  //disable interrupt
    int interrupt_state = interrupts_set(0);
     Tid thread_id;
    if(tid==THREAD_ANY)
    {
        if(ready_queue==NULL)
        {
            interrupts_set(interrupt_state);
            return THREAD_NONE;
        }
       
        thread_id=ready_queue->myid;      
    }
    else if(tid==THREAD_SELF)
    {
        thread_id=current->myid;
    }
    else
    {
        thread_id=tid;
    }
    if(thread_id >= THREAD_MAX_THREADS ||thread_id<0)
    {
        interrupts_set(interrupt_state);
        return THREAD_INVALID;
    }
   //handle when current thread wants to exit
    if(current->myid==thread_id)
    {
        if(ready_queue == NULL)
        {
            interrupts_set(interrupt_state);
            return THREAD_NONE;
        }
        else
        {      
            current->state = 0;
            insert_at_end_exit(current);
            int ready_flag= 0;
            getcontext(&current->mycontext);
            if(ready_flag == 0)
            {        
                current = ready_queue;
                ready_queue = ready_queue->next;
                current->next = NULL;
                ready_flag = 1;
                setcontext(&(current->mycontext));        
            }
            interrupts_set(interrupt_state);
            return thread_id;
        }
            
    }
     //when destroy another thread other than the current one
    else
    {
        struct thread* del = find_index(thread_id);
        if(del != NULL)
        {
            del->state= 0;
            interrupts_set(interrupt_state);
            return thread_id;
        }
        else
        {
            interrupts_set(interrupt_state);
            return THREAD_INVALID;
        }
    }
    interrupts_set(interrupt_state);
    return THREAD_FAILED;
}

/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* This is the wait queue structure */
struct wait_queue {
	/* ... Fill this in ... */
    struct thread* wait_head;
};

struct wait_queue *
wait_queue_create()
{
	struct wait_queue *wq;

	wq = malloc(sizeof(struct wait_queue));
	assert(wq);
        wq->wait_head = NULL;
	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
    while (wq->wait_head != NULL)
    {
        struct thread* temp = wq->wait_head;
        wq->wait_head = wq->wait_head->next;
        insert_at_end_exit(temp);
    }
    
	free(wq);
}

/******************************helper functions********************************************/
void 
insert_at_end_wait (struct thread* new_wait, struct wait_queue* dest_wait_queue)
{
    //if the ready_queue is empty
    new_wait->next = NULL;
    if(dest_wait_queue->wait_head == NULL)
    {
        dest_wait_queue->wait_head = new_wait;
        return;
    }
    
    struct thread* curr;
    curr = dest_wait_queue->wait_head;
    while(curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new_wait;
    return;        
}
/***************************************************************************************/
Tid
thread_sleep(struct wait_queue *queue)
{
    int interrupt_state = interrupts_set(0);
    // check if the pass in wait queue is valid
    if (queue == NULL)
    {
        interrupts_set(interrupt_state);
        return THREAD_INVALID;
    }
    // check if there is thread ready to run
    if (ready_queue == NULL)
    {
        interrupts_set(interrupt_state);
        return THREAD_NONE;
    }
    // the ready_queue is not NULL and wait queue is valid
    //put it into the wait_queue
    insert_at_end_wait(current, queue);
    //run another thread
    if(ready_queue == 0)
    {
        interrupts_set(interrupt_state);
        return THREAD_NONE;
    }
    else
    {
    int ready_flag= 0;
    Tid return_id = ready_queue->myid;
    getcontext(&current->mycontext);
    if(ready_flag == 0)
    {        
        current = ready_queue;       
        ready_queue = ready_queue->next;
        current->next = NULL;
        ready_flag = 1;
        setcontext(&(current->mycontext));  
    }
    interrupts_set(interrupt_state);
    return return_id;
    }
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
    int interrupt_state = interrupts_set(0);
    if (queue == NULL)
    {
        interrupts_set(interrupt_state);
        return 0;
    }
    else if (queue->wait_head == NULL)
    {
        interrupts_set(interrupt_state);
        return 0;
    }
       
    //wake up one thread only
    if(all == 0)
    {
        struct thread* temp = queue->wait_head;
        queue->wait_head = queue->wait_head->next;
        temp->next = NULL;
        insert_at_end(temp);
        interrupts_set(interrupt_state);
        return 1;
    }
    //wake up all threads
    else
    {
        int count = 0;
        //struct thread* curr = queue->wait_head;
        while(queue->wait_head != NULL)
        {
            struct thread* temp = queue->wait_head;
            queue->wait_head = queue->wait_head->next;
            temp->next = NULL;
            insert_at_end(temp);
            count ++;           
        }
        interrupts_set(interrupt_state);
        return count;
        
    }
}

struct lock {
	/* ... Fill this in ... */
    int available;
    struct wait_queue* wq;
};

struct lock *
lock_create()
{
        int interrupt_state = interrupts_set(0);
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);
	lock->available = 1;
        lock->wq = wait_queue_create();
        interrupts_set(interrupt_state);
	return lock;
}

void
lock_destroy(struct lock *lock)
{
    int interrupt_state = interrupts_set(0);
	assert(lock != NULL);
        if(lock->available == 1)
        {
        wait_queue_destroy(lock->wq);
        free(lock);
        }
        interrupts_set(interrupt_state);
}

void
lock_acquire(struct lock *lock)
{
    int interrupt_state = interrupts_set(0);
	assert(lock != NULL);

	while (lock->available == 0)
        {
            thread_sleep(lock->wq);
        }
        lock->available = 0;
        interrupts_set(interrupt_state);
}

void
lock_release(struct lock *lock)
{
        int interrupt_state = interrupts_set(0);
	assert(lock != NULL);
        thread_wakeup(lock->wq,1);
        lock->available = 1;
        interrupts_set(interrupt_state);

	
}

struct cv {
	/* ... Fill this in ... */
    struct wait_queue *wq;
};

struct cv *
cv_create()
{
    int interrupt_state = interrupts_set(0);
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);
        cv->wq = wait_queue_create();

        interrupts_set(interrupt_state);
	return cv;
}

void
cv_destroy(struct cv *cv)
{
       int interrupt_state = interrupts_set(0);
	assert(cv != NULL);

	wait_queue_destroy(cv->wq);

	free(cv);
        interrupts_set(interrupt_state);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
       int interrupt_state = interrupts_set(0);
	assert(cv != NULL);
	assert(lock != NULL);
        lock_release (lock);
        thread_sleep(cv->wq);
        lock_acquire(lock);
        
        
        interrupts_set(interrupt_state);
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
       int interrupt_state = interrupts_set(0);
	assert(cv != NULL);
	assert(lock != NULL);
        thread_wakeup(cv->wq,0);
	interrupts_set(interrupt_state);
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
       int interrupt_state = interrupts_set(0);
	assert(cv != NULL);
	assert(lock != NULL);
        thread_wakeup(cv->wq,1);
	interrupts_set(interrupt_state);
}
