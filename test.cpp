#include "gtthreads.h"
#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>


using namespace std;

gtthread_mutex_t lock1;

void* thread_func1(void* arg)
{
	int *p = (int*)arg;
	cout << "Argument = " << *p << endl;
	int i=0;


//	gtthread_mutex_lock(&lock1);
	while(i<5)
	{	
		i++;
//		if(i == 3)
//			gtthread_cancel(3);
		cout << "Called 1" << endl;
		sleep(1);
	}
//	gtthread_mutex_unlock(&lock1);
	int ret = 1;
//	gtthread_exit(&ret);
}

void* thread_func3(void* arg)
{
	//cout <<

	int i = 0;

//	gtthread_mutex_lock(&lock1);
	while(i<9)
	{
		i++;
		cout << "Called 3" << endl;
//		if(i == 5)
//			gtthread_yield();
		sleep(1);
	}

//	gtthread_mutex_unlock(&lock1);
	int ret = 3;
//	gtthread_exit(&ret);
}

void* thread_func2(void* arg)
{
	int ret = 2;
	gtthread_exit(&ret);
//        while(1)
  //              cout << "Called 3" << endl;
}

int main()
{

	gtthread_mutex_init(&lock1);
	gtthread_init(20);
	gtthread_t t1, t2, t3, t4;
	int arg = 9;
	gtthread_create(&t1, &thread_func1, (void*)&arg);
	gtthread_create(&t2, &thread_func2, NULL);
	gtthread_create(&t3, &thread_func3, NULL);
        //gtthread_create(&t4, &thread_func2, NULL);

	cout << "Thread ID = " << t4 << endl;
	cout << "Ret = " << gtthread_join(t2,NULL) << endl;
	cout << "Ret = " << gtthread_join(t3,NULL) << endl;
	cout << "Ret = " << gtthread_join(t1,NULL) << endl;;
	//gtthread_join(t4,NULL);

	//while(1);
	int i=0;
	while(i<7)
	{
		i++;
		cout << "This is main" << endl;
		sleep(1);
	}

	cout << "Thread ID = " << t1 << endl;
	cout << "Thread ID = " << t2 << endl;
	cout << "Thread ID = " << t3 << endl;
	cout << "Main returning" << endl;

	return 0;
}
