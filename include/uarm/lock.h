/*
* \file uarm/lock.h
# Software License Agreement (MIT License)
#
# Copyright (c) 2018, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>
*/
#ifndef LOCK_H_
#define LOCK_H_


#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

#ifdef WIN32
class Mutex
{
	CRITICAL_SECTION m_cs;
public:
	Mutex() { InitializeCriticalSection(&m_cs); }
	virtual ~Mutex() { DeleteCriticalSection(&m_cs); }

	void lock() { EnterCriticalSection(&m_cs); }
	void unlock() { LeaveCriticalSection(&m_cs); }
};

#else
class Mutex
{
	pthread_mutex_t m_mutex;
public:
	Mutex() { pthread_mutex_init(&m_mutex, NULL); }
	~Mutex() { pthread_mutex_destroy(&m_mutex); }

	void lock() { pthread_mutex_lock(&m_mutex); }
	void unlock() { pthread_mutex_unlock(&m_mutex); }
};
#endif

#endif // LOCK_H_