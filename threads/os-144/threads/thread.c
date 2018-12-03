#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"
// global variables declarations

struct thread* ready_queue;
struct thread* current;
void* stack_list[THREAD_MAX_THREADS];

/* This is the thread control block */
struct thread {
    ucontext_t mycontext;
    struct thread* next;
    Tid myid;
    //int ready_flag;
};


/***********Helper Functions*********************/

/* thread starts by calling thread_stub. The arguments to thread_stub are the
 * thread_main() function, and one argument to the thread_main() function. */
void
thread_stub(void (*thread_main)(void *), void *arg)
{
	Tid ret;

	thread_main(arg); // call thread_main() function with arg
	ret = thread_exit(THREAD_SELF);
	// we should only get here if we are the last thread. 
	assert(ret == THREAD_NONE);
	// all threads are done, so process should exit
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

/************************** ready queue linked list functions**********************************/

void remove_first()
{           

    if(ready_queue == NULL)
    {
        printf("ready_queue empty.\n");
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
    Tid num = 1;
    struct thread * curr = ready_queue;
    while((num <= THREAD_MAX_THREADS)&&(curr!=NULL))
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
    //first->ready_flag = 1;
    getcontext(&current->mycontext);
    current->next = NULL;
    ready_queue = NULL;
    //void* first_stack = malloc(THREAD_MIN_STACK);
    //stack_list[0] = first_stack;
    Tid i;
    for(i = 1; i < THREAD_MAX_THREADS; i++)
        stack_list[i] = NULL;
        
    
}

Tid
thread_id()
{
	//TBD();
    if(current != NULL)
        return current->myid;
    return THREAD_INVALID;
}

Tid
thread_create(void (*fn) (void *), void *parg)
{
	//TBD();
    //interrupts_off();
    //iterate through the stack_array to find an appropriate index
    Tid iter;
    for (iter = 1; iter< THREAD_MAX_THREADS;iter++)
    {
        if( stack_list[iter]==NULL)
        {
            struct thread* new_thread = malloc(sizeof(struct thread));
            void* new_stack = malloc(THREAD_MIN_STACK);
            //check if there's memory
            if((new_thread == NULL)|| (new_stack == NULL))
            {   //interrupts_on();
                return THREAD_NOMEMORY;
            }
            new_thread->myid = iter;
            new_thread->next = NULL;
            getcontext(&new_thread->mycontext);
            stack_list[iter] = new_stack;
            //printf("   %d",(int)iter);
            //set the corresponding registers
            new_thread->mycontext.uc_mcontext.gregs[REG_RIP] = (long int) thread_stub;
            new_thread->mycontext.uc_mcontext.gregs[REG_RDI] = (long int) (*fn);
            new_thread->mycontext.uc_mcontext.gregs[REG_RSI] = (long int) parg;
            new_thread->mycontext.uc_mcontext.gregs[REG_RSP] = (long long) new_stack + THREAD_MIN_STACK;
            new_thread->mycontext.uc_stack.ss_sp=new_stack;
            //put new_thread into ready queue
            insert_at_end(new_thread);
           // interrupts_on();
            return iter;
        }
    }
    if (iter == THREAD_MAX_THREADS)
    {
       // interrupts_on();
        return THREAD_NOMORE;
    }
      //  interrupts_on();
	return THREAD_FAILED;
}

Tid
thread_yield(Tid want_tid)
{
    
    //disable interrupt
    interrupts_off();

     Tid thread_id;
    if(want_tid==THREAD_ANY)
    {
        if(ready_queue==NULL)
        {
            interrupts_on();
            return THREAD_NONE;
        }
       
        thread_id=ready_queue->myid;
        
    }
    else if(want_tid==THREAD_SELF)
    {
        thread_id=current->myid;
        interrupts_on();
        return thread_id;
    }
    else
    {
        thread_id=want_tid;
    }
    if(thread_id >= THREAD_MAX_THREADS ||thread_id<0)
    {
        return THREAD_INVALID;
    }

    if(current->myid==thread_id)
    {
        interrupts_on();
        return thread_id;
    }
    //When the thread is valid we give it the right thread run
    
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
            interrupts_on();
            return THREAD_INVALID;
        }
    }
    

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
      interrupts_on();
      return thread_id;
   
    interrupts_on();
    return THREAD_FAILED;
}

Tid
thread_exit(Tid tid)
{
  //disable interrupt
    interrupts_off();

     Tid thread_id;
    if(tid==THREAD_ANY)
    {
        if(ready_queue==NULL)
        {
            interrupts_on();
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
        interrupts_on();
        return THREAD_INVALID;
    }
   //handle when current thread wants to exit
    if(current->myid==thread_id)
    {
        struct thread* next = ready_queue;
        //if(thread_id == 1023)
          //printf("  %d    ",(int)thread_id);
        if(ready_queue == NULL)
        {
            //printf("only thread\n");
            interrupts_on();
            return THREAD_NONE;
        }
        else
        {
            int ready_flag= 0;
            struct thread *temp = current;
            getcontext(&current->mycontext);

            if(ready_flag==0)
            {
                current = next;
                remove_by_index(next->myid);
                free(temp->mycontext.uc_stack.ss_sp);
                //printf(" index: %d\n", (int)temp->myid);
                stack_list[temp->myid]=NULL;
                
                //free(temp);
                temp=NULL;
               // current->next = NULL;

                ready_flag=1;
                setcontext(&(current->mycontext));  
            }

                
               interrupts_on();
               return thread_id;
        }
            
    }
     //when destroy another thread other than the current one
    else
    {
        struct thread* del = find_index(thread_id);
        if(del != NULL)
        {
        remove_by_index(thread_id);
        free(del->mycontext.uc_stack.ss_sp);
        stack_list[thread_id]=NULL;
        free(del);
        del = NULL;
        interrupts_on();
        return thread_id;
        }
        else
        {
            interrupts_on();
            return THREAD_INVALID;
        }
        
    }
     
	return THREAD_FAILED;
}

/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* This is the wait queue structure */
struct wait_queue {
	/* ... Fill this in ... */
};

struct wait_queue *
wait_queue_create()
{
	struct wait_queue *wq;

	wq = malloc(sizeof(struct wait_queue));
	assert(wq);

	TBD();

	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	TBD();
	free(wq);
}

Tid
thread_sleep(struct wait_queue *queue)
{
	TBD();
	return THREAD_FAILED;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
	TBD();
	return 0;
}

struct lock {
	/* ... Fill this in ... */
};

struct lock *
lock_create()
{
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);

	TBD();

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	TBD();

	free(lock);
}

void
lock_acquire(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

void
lock_release(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

struct cv {
	/* ... Fill this in ... */
};

struct cv *
cv_create()
{
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);

	TBD();

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	TBD();

	free(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}
