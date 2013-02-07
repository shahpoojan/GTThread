#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>

#define READY 0
#define WAITING 1
#define BLOCKED 2
#define RUNNING 3
#define KILLED 4

typedef long gtthread_t;
typedef long gtthread_mutex_t;

struct tcblk{
	long tid;
	int state;
	//char c_stack[16384];
	ucontext_t ctxt;
	int isMain;
	int retval;
};


// Basic thread functions
void gtthread_init(long period);
int gtthread_create(gtthread_t *thread, void*(*start_routine)(void*), void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);


// Thread synchronization functions
int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);

