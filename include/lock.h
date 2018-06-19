#ifndef _LOCK_H
#define _LOCK_H

#include <string>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else //for linux
#define INFINITE 0x7FFFFFFF
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#endif
using namespace std;

/************************************************************************/
/*				         Locker Class                         */
/************************************************************************/
class Locker
{
public:
	Locker(void);
	~Locker(void);

public:
	void Lock();
	void Unlock();

private:
    #ifdef _WIN32
	CRITICAL_SECTION m_oCriticalSection;
	#else //for linux
	pthread_mutex_t mutex;
	#endif
};

/************************************************************************/
/*				         Single lock class                              */
/************************************************************************/
class ScopeLocker
{
public:
	ScopeLocker(Locker* pLocker);
	~ScopeLocker(void);

private:
	ScopeLocker(void);

private:
	Locker* m_pLocker;

};

#endif
