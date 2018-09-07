/*
* \file uarm/utils.h
# Software License Agreement (MIT License)
#
# Copyright (c) 2018, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>
*/
#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <vector>
#include <string.h>

long long get_system_time();
void sleep_milliseconds(unsigned long milliseconds);
std::string strip(const std::string & str, const std::string & chars = " ");
std::string lstrip(const std::string & str, const std::string & chars = " ");
std::string rstrip(const std::string & str, const std::string & chars = " ");
std::vector<std::string> split(const std::string &str, const std::string &pattern);

#endif // UTILS_H_