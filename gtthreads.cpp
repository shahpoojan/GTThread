#include "gtthreads.h"
#include <queue>
#include <list>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define MAX_THREADS 100

using namespace std;

long quantum;
long total_threads = 0;
list<tcblk*> ready_queue;
list<tcblk*> wait_queue;
list<tcblk*> terminated_queue;
ucontext_t uctx_main;//, uctx_func1, uctx_func2;
ucontext_t wrapper_ctxt;
struct itimerval timer;
sigset_t set;

void internal_exit();

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)


void exit_wrapper(void)
{cout << "!!!!!" << endl;	
	//	swapcontext(&wrapper_ctxt, &uctx_main);
	cout << "In the wrapper" << endl;
	while(1)
	{
		cout << "Inside the wrapper" << endl;
		internal_exit();
	}

}


void sigalarm_handler(int sig)
{
	cout << "Alarm called" << endl;
	tcblk* head = ready_queue.front();
	ucontext_t *curr, *next;
	if(head->isMain)
		curr = &uctx_main;
	else
		curr = &head->ctxt;
	ready_queue.pop_front();

	ready_queue.push_back(head);

	if(ready_queue.front()->isMain)
	{
		cout << "Curr = " << head->tid << endl;
		cout << "isMain true" << endl;
		next = &uctx_main;
	}
	else
		next = &ready_queue.front()->ctxt;

	//	alarm(2);
	setitimer(ITIMER_REAL, &timer, NULL);

	cout << "Head id = " << head->tid << endl;
	cout << "Before swapping" << endl;
	swapcontext(curr, next);//&ready_queue.front()->ctxt);
}

void gtthread_init(long period)
{
	if (getcontext(&wrapper_ctxt) == -1)
		handle_error("getcontext");

	wrapper_ctxt.uc_stack.ss_sp = new char[16384];
	wrapper_ctxt.uc_stack.ss_size = 16384;
	wrapper_ctxt.uc_link = &uctx_main;
	makecontext(&wrapper_ctxt, (void(*)(void))&exit_wrapper, 0);


	// The set of signals to be blocked or unblocked
	sigfillset(&set);

	// Setup the signal handler structure
	struct sigaction new_action;
	new_action.sa_handler = sigalarm_handler;
	new_action.sa_flags = 0;

	// Block all the signals when the signal handler is running. This is to ensire that running signal unsafe functions in signals will not ciase any problems
	sigfillset(&new_action.sa_mask);

	// Setup the strucure to be used by the timer. Intialise it equal to the quantum
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec  = 2;
	timer.it_value.tv_usec = 0;

	quantum = period;
	tcblk *thread = new tcblk();
	thread->tid = total_threads++;
	thread->state = READY;
	//thread->ctxt = uctx_main;
	thread->isMain = 1;

	uctx_main.uc_stack.ss_sp = new char[16384];
	uctx_main.uc_stack.ss_size = 16384;


	thread->ctxt = uctx_main;
	ready_queue.push_back(thread);
	cout << "Thread created successfully" << endl;
	cout << "Current threads are: " << endl;

	// Assign the signal handler
	sigaction(SIGALRM, &new_action, NULL);
	setitimer(ITIMER_REAL, &timer, NULL);
}

int gtthread_create(gtthread_t *thread_t, void* (*start_routine)(void*), void *arg)
{
	// Block all the signals because there can be a race condition for the value of 'total_threads'. So, make it kind of atomic

	sigprocmask(SIG_BLOCK, &set, NULL);
	if(total_threads != MAX_THREADS)
	{

		tcblk *thread = new tcblk();
		*thread_t = total_threads;
		(*thread).tid = total_threads++;
		(*thread).state = READY;
		ready_queue.push_back(thread);

		sigprocmask(SIG_UNBLOCK, &set, NULL);	// Unblock the signals because rest of the code can't cause any race conditions
		cout << "Thread created successfully" << endl;
		cout << "Number of threads = " << total_threads << endl;
		if (getcontext(&thread->ctxt) == -1)
			handle_error("getcontext");
		(*thread).ctxt.uc_stack.ss_sp = new char[16384];//thread->c_stack;
		(*thread).ctxt.uc_stack.ss_size = 16384;//sizeof(thread->c_stack);
		(*thread).ctxt.uc_link = &wrapper_ctxt;
		(*thread).isMain = 0;
		makecontext(&thread->ctxt, (void(*)())start_routine, 1, arg);


		return thread->tid;
	}

	else
	{
		sigprocmask(SIG_UNBLOCK, &set, NULL);
		cout << "Reached maximum thread limit" << endl;
		return -1;
	}
}

int gtthread_join(gtthread_t thread, void **status)
{
	cout << "Entered join" << endl;
	int search_id = (int)thread;
	int returnValue;

	while(1)
	{
		//		cout << "Waiting " << thread << endl;
		int flag = 0;
		if(!terminated_queue.empty())
		{
			//list<gtthread_t*>::const_iterator prev;
			for (list<tcblk*>::iterator ci = terminated_queue.begin(); ci != terminated_queue.end(); ++ci)
			{
				if((*ci)->tid == search_id)
				{
					cout << "Found tid " << search_id << endl;
					cout << "Current size = " << terminated_queue.size() << endl;
					flag = 1;
					free((*ci)->ctxt.uc_stack.ss_sp);
					returnValue = (*ci)->retval;

					// Check if blocking is really required here or not
					sigprocmask(SIG_BLOCK, &set, NULL);
					terminated_queue.erase(ci);
					sigprocmask(SIG_UNBLOCK, &set, NULL);

					break;
				}
				//prev = ci;
			}
		}
		if(flag == 1)
			break;
		gtthread_yield();
	}

	return returnValue;
}

void gtthread_exit(void* retval)
{
	sigprocmask(SIG_BLOCK, &set, NULL);
	int *returnValue;

	if(retval != NULL)
		returnValue = (int*)retval;
	else
		returnValue = NULL;

	cout << "Exiting = " << ready_queue.size() << endl;
	tcblk* tcb = ready_queue.front();
	ready_queue.pop_front();
	cout << "Evicted " << tcb->tid << endl;

	cout << "Front = " << ready_queue.front()->tid;
	cout << "Back = " << ready_queue.back()->tid;

	if(returnValue != NULL)
		tcb->retval = *returnValue;
	else
		tcb->retval = 0;


	// Check if blocking is really required here or not
	//sigprocmask(SIG_BLOCK, &set, NULL);
	terminated_queue.push_back(tcb);
	//sigprocmask(SIG_UNBLOCK, &set, NULL);

	cout << "***Exited***" << endl << endl;

	if(ready_queue.front()->isMain != 1)
	{
		tcb->ctxt.uc_link = &ready_queue.front()->ctxt;
		cout << "Normal " << ready_queue.front()->tid << endl << endl;
	} 
	else
	{
		tcb->ctxt.uc_link = &uctx_main;
		cout << "It's main\n" << endl;
	}

	//free(&tcb->ctxt.uc_stack.ss_sp);
	makecontext(&tcb->ctxt, NULL, 0);
	setitimer(ITIMER_REAL, &timer, NULL);
	//sigprocmask(SIG_UNBLOCK, &set, NULL);
}

int gtthread_equal(gtthread_t t1, gtthread_t t2)
{
	return ((t1 == t2)?1:0);
}

gtthread_t gtthread_self()
{
	return ready_queue.front()->tid;
}

int gtthread_cancel(gtthread_t tid)
{
	for (list<tcblk*>::iterator ci = ready_queue.begin(); ci != ready_queue.end(); ++ci)
	{
		if((*ci)->tid == tid)
		{
			cout << "Found cancellation tid " << tid << endl;
			cout << "Current size = " << ready_queue.size() << endl;
			tcblk *blk = *ci;
			cout << "--------Block tid = " << blk->tid << endl;

			sigprocmask(SIG_BLOCK, &set, NULL);
			terminated_queue.push_back(blk);
			sigprocmask(SIG_UNBLOCK, &set, NULL);
			ready_queue.erase(ci);

			return 0;
		}
	}

	return -1;
}


void gtthread_yield()
{
	cout << "Yielding!!!!!!" << endl;
	setitimer(ITIMER_REAL, &timer, NULL);
	sigalarm_handler(0);
}

int  gtthread_mutex_init(gtthread_mutex_t *mutex)
{
	*mutex = 0;
}

int  gtthread_mutex_lock(gtthread_mutex_t *mutex)
{
	while(1)
	{
		sigprocmask(SIG_BLOCK, &set, NULL);
		if(*mutex == 0)
		{
			*mutex = 1;
			sigprocmask(SIG_UNBLOCK, &set, NULL);
			break;
		}
		sigprocmask(SIG_UNBLOCK, &set, NULL);
		gtthread_yield();	
	}

	return 0;
}

int  gtthread_mutex_unlock(gtthread_mutex_t *mutex)
{
	*mutex = 0;
}

void internal_exit()
{
	sigprocmask(SIG_BLOCK, &set, NULL);
	cout << "Exiting = " << ready_queue.size() << endl;
	tcblk* tcb = ready_queue.front();
	ready_queue.pop_front();
	cout << "Evicted " << tcb->tid << endl;

	cout << "Front = " << ready_queue.front()->tid;
	cout << "Back = " << ready_queue.back()->tid;


	terminated_queue.push_back(tcb);

	cout << "***Exited***" << endl << endl;

	setitimer(ITIMER_REAL, &timer, NULL);
	if(ready_queue.front()->isMain)
		swapcontext(&wrapper_ctxt, &uctx_main);
	else
		swapcontext(&wrapper_ctxt, &ready_queue.front()->ctxt);
}
