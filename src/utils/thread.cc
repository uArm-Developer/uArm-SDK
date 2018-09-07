/*
* \file uarm/thread.cc
# Software License Agreement (MIT License)
#
# Copyright (c) 2018, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>
*/
#include "uarm/thread.h"

Thread::Thread()
{
#ifdef _WIN32
	m_handle = NULL;
#else
	m_thread_t = 0;
#endif
}

Thread::~Thread()
{
	swift = NULL;
#ifdef _WIN32
	if (NULL != m_handle)
	{
		CloseHandle(m_handle);
	}
	m_handle = NULL;
#else
	m_thread_t = 0;
#endif
}

bool Thread::create()
{
	bool ret = false;
#ifdef _WIN32
	m_handle = (HANDLE)_beginthreadex(NULL, 0, thread_func, this, 0, NULL);
	if (NULL != m_handle)
	{
		ret = true;
	}
#else
	if (0 == pthread_create(&m_thread_t, NULL, thread_func, this))
	{
		ret = true;
	}
	else
	{
		m_thread_t = 0;
	}
#endif
	return ret;
}

void Thread::wait()
{
#ifdef _WIN32
	WaitForSingleObject(m_handle, INFINITE);
	if (NULL != m_handle)
	{
		CloseHandle(m_handle);
	}
	m_handle = NULL;
#else
	pthread_join(m_thread_t, NULL);
	m_thread_t = 0;
#endif // _WIN32
}

#ifdef _WIN32
unsigned __stdcall Thread::thread_func(void* arg)
#else
void* Thread::thread_func(void* arg)
#endif
{
	Thread *pthis = (Thread*)arg;
	pthis->run(pthis->swift);
	return NULL;
}