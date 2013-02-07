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
	int ret = *(int*)arg;
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
	int ret = *(int*)arg;
	gtthread_exit(&ret);
}

void* thread_func2(void* arg)
{
	int ret = *(int*)arg;
	gtthread_exit(&ret);
//        while(1)
  //             cout << "Called 3" << endl;
}

int main()
{

	gtthread_mutex_init(&lock1);
	gtthread_init(3000);
	gtthread_t t1, t2, t3, t4;
	int arg1 = 9;
	int arg2 = 7;
	int arg3 = 5;
	int arg4 = 4;

	int *status1 = new int();
	int *status2 = new int();
	int *status3 = new int();
	int *status4 = new int();
	gtthread_create(&t1, &thread_func1, (void*)&arg1);
	gtthread_create(&t2, &thread_func2, (void*)&arg2);
	gtthread_create(&t3, &thread_func3, (void*)&arg3);
        gtthread_create(&t4, &thread_func2, (void*)&arg4);

	cout << "Ret = " << gtthread_join(t2,(void**)&status1) << endl;
	cout << "Ret = " << gtthread_join(t3,(void**)&status2) << endl;
	cout << "Ret = " << gtthread_join(t1,(void**)&status3) << endl;;
	cout << "Ret = " << gtthread_join(t4,(void**)&status4) << endl;

	if(status3!=NULL)
	{
	cout << *status1 << endl;
	cout << *status2 << endl;
	cout << *status3 << endl;
	cout << *status4 << endl;
	}
	
	//while(1);
	int i=0;
	/*while(i<7)
	{
		i++;
		cout << "This is main" << endl;
		sleep(1);
	}*/

	cout << "Main returning" << endl;

	return 0;
}
