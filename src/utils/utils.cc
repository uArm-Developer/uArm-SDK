/*
* \file uarm/utils.cc
# Software License Agreement (MIT License)
#
# Copyright (c) 2018, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>
*/
#include <iostream>
#include <sys/timeb.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "uarm/utils.h"

#define LEFTSTRIP 0
#define RIGHTSTRIP 1
#define BOTHSTRIP 2

void sleep_milliseconds(unsigned long milliseconds) {
#ifdef _WIN32
	Sleep(milliseconds); // 100 ms
#else
	usleep(milliseconds * 1000); // 100 ms
#endif
}

long long get_system_time()
{
	struct timeb t;
	ftime(&t);
	return 1000 * t.time + t.millitm;
}

static std::string do_strip(const std::string &str, int striptype, const std::string &chars)
{
	std::string::size_type strlen = str.size();
	std::string::size_type charslen = chars.size();
	std::string::size_type i, j;

	//默认情况下，去除空白符
	if (0 == charslen)
	{
		i = 0;
		//去掉左边空白字符
		if (striptype != RIGHTSTRIP)
		{
			while (i < strlen&&::isspace(str[i]))
			{
				i++;
			}
		}
		j = strlen;
		//去掉右边空白字符
		if (striptype != LEFTSTRIP)
		{
			j--;
			while (j >= i&&::isspace(str[j]))
			{
				j--;
			}
			j++;
		}
		return str.substr(i, j - i);
	}
	else
	{
		//把删除序列转为c字符串
		const char*sep = chars.c_str();
		i = 0;
		if (striptype != RIGHTSTRIP)
		{
			//memchr函数：从sep指向的内存区域的前charslen个字节查找str[i]
			while (i < strlen&&memchr(sep, str[i], charslen))
			{
				i++;
			}
		}
		j = strlen;
		if (striptype != LEFTSTRIP)
		{
			j--;
			while (j > 0 && j >= i&&memchr(sep, str[j], charslen))
			{
				j--;
			}
			if (j != 0)
				j++;
		}
		//如果无需要删除的字符
		if (0 == i&& j == strlen)
		{
			return str;
		}
		else
		{
			return str.substr(i, j - i);
		}
	}
}

std::string strip(const std::string & str, const std::string & chars)
{
	return do_strip(str, BOTHSTRIP, chars);
}

std::string lstrip(const std::string & str, const std::string & chars)
{
	return do_strip(str, LEFTSTRIP, chars);
}

std::string rstrip(const std::string & str, const std::string & chars)
{
	return do_strip(str, RIGHTSTRIP, chars);
}

std::vector<std::string> split(const std::string &str, const std::string &pattern)
{
	std::vector<std::string> resVec;

	if ("" == str)
	{
		return resVec;
	}
	//方便截取最后一段数据
	std::string strs = str + pattern;

	size_t pos = strs.find(pattern);
	size_t size = strs.size();

	while (pos != std::string::npos)
	{
		std::string x = strs.substr(0, pos);
		resVec.push_back(x);
		strs = strs.substr(pos + 1, size);
		pos = strs.find(pattern);
	}

	return resVec;
}