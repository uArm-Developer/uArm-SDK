#ifndef THREAD_H_
#define THREAD_H_

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

class Thread {
public:
	Thread();
	virtual ~Thread();

	bool create();
	void wait();
	void *swift;
	void (*run)(void *arg);

#ifdef WIN32
	static unsigned __stdcall thread_func(void* arg);
#else
	static void* thread_func(void* arg);
#endif
protected:

#ifdef WIN32
	HANDLE m_handle;
#else
	pthread_t m_thread_t;
#endif
};

#endif // THREAD_H_