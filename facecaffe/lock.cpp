#include "lock.h"

Locker::Locker(void)
{
    #ifdef _WIN32
	::InitializeCriticalSection(&m_oCriticalSection);
	#else //for linux
	if(0 != pthread_mutex_init(&mutex, NULL))
	{
        perror("Init mutex failed!");
	}
	#endif
}

Locker::~Locker(void)
{
    #ifdef _WIN32
	::DeleteCriticalSection(&m_oCriticalSection);
	#else //for linux
	if(0 != pthread_mutex_destroy(&mutex))
	{
        perror("Destory mutex failed!");
	}
	#endif
}

void Locker::Lock()
{
    #ifdef _WIN32
	::EnterCriticalSection(&m_oCriticalSection);
	#else //for linux
	if(0 != pthread_mutex_lock(&mutex))
	{
        perror("Lock mutex failed!");
	}
	#endif
}

void Locker::Unlock()
{
    #ifdef _WIN32
	::LeaveCriticalSection(&m_oCriticalSection);
	#else //for linux
	if(0 != pthread_mutex_unlock(&mutex))
	{
        perror("Unlock mutex failed!");
	}
	#endif
}

ScopeLocker::ScopeLocker(void)
{
}

ScopeLocker::ScopeLocker( Locker* pLocker )
{
	if (NULL != pLocker)
	{
		m_pLocker = pLocker;
		m_pLocker->Lock();
	}
}

ScopeLocker::~ScopeLocker(void)
{
	if (NULL != m_pLocker)
	{
		m_pLocker->Unlock();
	}
}