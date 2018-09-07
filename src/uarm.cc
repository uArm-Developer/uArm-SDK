/*
* \file uarm/uarm.cc
# Software License Agreement (MIT License)
#
# Copyright (c) 2018, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>
*/
#include <iostream>
#include <functional>
#include <string.h>
#include "uarm/uarm.h"

using uarm::Swift;
using uarm::PortInfo;
using uarm::list_ports;
using uarm::Timeout;
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::exception;


static void recv_thread_func(void *arg) 
{
	cout << "recv thread start" << endl;
	Swift *swift = (Swift *)arg;
	// cout << "swift: " << swift << ", ser: " << &swift->ser << ", open: " << swift->ser.isOpen() << endl;
	while (swift->connected) {
		try {
			if (swift->ser.available() > 0) {
				string result = swift->ser.readline();
				result = rstrip(result, " \r\n");
				if (result.length() > 0) {
					// cout << result << endl;
					if (result[0] == '$') {
						// response
						size_t pos = result.find(" ");
						size_t size = result.size();
						if (pos != std::string::npos) {
							string x = result.substr(0, pos);
							result = result.substr(pos + 1, size);
							int cnt = std::stoi(x.substr(1));
							// cout << "cnt: " << cnt << ", result: " << result << endl;
							if (swift->callbacks[cnt] != NULL) {
								// cout << "callback" << endl;
								std::unique_lock<std::mutex> locker(swift->mutex);
								std::function<void(std::string)> callback = swift->callbacks[cnt];
								swift->callbacks[cnt] = NULL;
								swift->cmd_pend_count -= 1;
								swift->cmd_cond.notify_all();
								locker.unlock();
								swift->timer.AsyncWait(0, callback, result);
								// swift->callbacks[cnt](result);
							}
							else {
								// cout << "set result" << endl;
								swift->result[cnt] = result;
								//cout << swift->result[cnt] << "  " << *swift->result[cnt] << endl;
							}
						}
						/*
						vector<string> tmpList = split(result, " ");
						for (int i = 0; i < tmpList.size(); i++) {
							cout << "split: " << tmpList[i] << ", length: " << tmpList[i].length() << endl;
						}
						*/
					}
					else if (result[0] == '@') {
						// report
						// cout << result << endl;
						std::vector<std::string> tmpList = split(result, " ");
						if (tmpList.size() > 1) {
							if (tmpList[0] == "@3" && tmpList.size() >= 4) {
								// report pos
								if (swift->pos_report_callback != NULL) {
									std::vector<float> pos;
									for (uint8_t i = 1; i < tmpList.size(); i++) {
										pos.push_back(std::stof(tmpList[i].substr(1)));
									}
									swift->timer.AsyncWait(0,swift->pos_report_callback, pos);
									// swift->pos_report_callback(pos);
								}
							}
							else if (tmpList[0] == "@4" && tmpList.size() >= 3) {
								// resport keys
								if (swift->key0_report_callback != NULL || swift->key1_report_callback != NULL) {
									if (tmpList[1] == "B0" && swift->key0_report_callback != NULL) {
										// key0
										swift->timer.AsyncWait(0, swift->key0_report_callback, std::stoi(tmpList[2].substr(1)));
										// swift->key0_report_callback(std::stoi(tmpList[2].substr(1)));
									}
									else if (tmpList[1] == "B1" && swift->key1_report_callback != NULL) {
										// key1
										swift->timer.AsyncWait(0, swift->key1_report_callback, std::stoi(tmpList[2].substr(1)));
										// swift->key1_report_callback(std::stoi(tmpList[2].substr(1)));
									}
								}
							}
							else if (tmpList[0] == "@5" && tmpList.size() >= 2) {
								// report power
								if (tmpList[1].length() >= 2 && tmpList[1][1] == '0') {
									swift->power_status = false;
									if (swift->power_callback != NULL) {
										swift->timer.AsyncWait(0, swift->power_callback, false);
										// swift->power_callback(false);
									}
								}
								else if (tmpList[1].length() >= 2 && tmpList[1][1] == '1') {
									swift->power_status = true;
									if (swift->power_callback != NULL) {
										swift->timer.AsyncWait(0, swift->power_callback, true);
										// swift->power_callback(true);
									}
								}
							}
							else if (tmpList[0] == "@6" && tmpList.size() >= 3) {
								// report limit switch
								if (swift->limit_switch_callback != NULL) {
									if (tmpList[2].length() >= 2 && tmpList[2][1] == '0') {
										swift->timer.AsyncWait(0, swift->limit_switch_callback, true);
										// swift->limit_switch_callback(false);
									}
									else if (tmpList[2].length() >= 2 && tmpList[2][1] == '1') {
										swift->timer.AsyncWait(0, swift->limit_switch_callback, true);
										// swift->limit_switch_callback(true);
									}
								}
							}
						}

						/*
						for (int i = 0; i < tmpList.size(); i++) {
							cout << "split: " << tmpList[i] << ", length: " << tmpList[i].length() << endl;
						}
						*/
					}
				}
			}
		}
		catch (serial::PortNotOpenedException &e) {
			swift->connected = false;
			cerr << "PortNotOpenedException: " << e.what() << endl;
		}
		catch (serial::SerialException &e) {
			swift->connected = false;
			cerr << "SerialException: " << e.what() << endl;
		}
		catch (serial::IOException &e) {
			swift->connected = false;
			cerr << "IOException: " << e.what() << endl;
		}
		catch (exception &e) {
			// swift->connected = false;
			cerr << "Exception: " << e.what() << endl;
		}
		sleep_milliseconds(10);
	}
	cout << "recv thread exit" << endl;
}

Swift::Swift(const string &_port, uint32_t _baudrate, Timeout timeout, int _cmd_pend_size)
{
	connected = false;
	power_status = false;
	cnt = 1;
	cmd_pend_count = 0;
	cmd_pend_size = _cmd_pend_size;
	power_callback = NULL;
	port = _port;
	baudrate = _baudrate;
	mode = -1;
	for (size_t i = 0; i < 5; i++) {
		device_info[i] = "";
	}
	pos_report_callback = NULL;
	key0_report_callback = NULL;
	key1_report_callback = NULL;
	limit_switch_callback = NULL;
	//result = { NULL };
	for (int i = 0; i < MAX_CNT; i++) {
		result[i] = "";
		callbacks[i] = NULL;
	}
	connect(port, baudrate, timeout);
}

Swift::~Swift()
{
	disconnect();
	for (int i = 0; i < MAX_CNT; i++) {
		if (result[i] != "") {
			delete &result[i];
			result[i] = "";
		}
		callbacks[i] = NULL;
	}
	for (size_t i = 0; i < 5; i++) {
		device_info[i] = "";
	}
}

int Swift::connect(const string &_port, uint32_t _baudrate, Timeout timeout)
{
	if (_port != "" && _port != port) {
		port = _port;
	}
	baudrate = _baudrate;

	if (port == "") {
		std::vector<PortInfo> devices_found = list_ports();
		std::vector<PortInfo>::iterator iter = devices_found.begin();

		while (iter != devices_found.end())
		{
			PortInfo device = *iter++;
			size_t pos = device.hardware_id.find("VID_");
			size_t pos2 = device.hardware_id.find("PID_");
			if (pos != std::string::npos) {
				std::string pidvid = device.hardware_id.substr(pos + 4, 4) + ":" + device.hardware_id.substr(pos2 + 4, 4);
				// cout << "pidvid:" << pidvid << endl;
				if (pidvid == "2341:0042") {
					port = device.port;
					break;
				}
			}
		}
	}
	ser.setPort(port);
	ser.setBaudrate(baudrate);
	ser.setTimeout(timeout);
	try {
		ser.open();
		ser.setRTS(true);
		ser.setDTR(true);
		// ser.setBreak(true);
		//ser.setFlowcontrol(serial::flowcontrol_t::flowcontrol_software);
		if (ser.isOpen()) {
			connected = true;
			cmd_pend_count = 0;
			cout << "connect success" << endl;
			// cout << "swift: " << this << ", ser: " << &ser << ", open: " << ser.isOpen() << endl;
			// r_thread.swift = (void *)this;
			// r_thread.run = recv_thread_func;
			// r_thread.create();
			// std::thread t(recv_thread_func, this);
			r_thread = std::thread(recv_thread_func, this);
			return 0;
		}
		else {
			cout << "connect failed" << endl;
			return -1;
		}
	}
	catch (exception &e) {
		connected = false;
		cerr << "connect failed: " << e.what() << endl;
		return -1;
	}
}

void Swift::disconnect()
{
	// cout << "disconnect" << endl;
	if (ser.isOpen()) {
		ser.close();
		cout << "disconnect" << endl;
	}
	connected = false;
}

inline std::string Swift::_send_cmd(const std::string &cmd, bool wait, float timeout, std::function<void(std::string)> callback)
{
	if (connected) {
		std::unique_lock<std::mutex> locker(mutex);
		if (cnt >= MAX_CNT) {
			cnt = 1;
		}
		int _cnt = cnt;
		cnt += 1;
		cmd_cond.wait(locker, [this] {return cmd_pend_count < cmd_pend_size; });
		cmd_pend_count += 1;
		cmd_cond.notify_all();
		locker.unlock();
		result[_cnt] = "";
		std::string data = "#" + std::to_string(_cnt) + cmd + "\n";
		if (!wait) {
			callbacks[_cnt] = callback;
			if (callback != NULL) {
				std::function<void()> _timeout_callback = [callback, _cnt, this]() {
					if (callbacks[_cnt] != NULL) {
						std::unique_lock<std::mutex> locker(mutex);
						callbacks[_cnt] = NULL;
						cmd_pend_count -= 1;
						cmd_cond.notify_all();
						locker.unlock();
						callback("TIMEOUT");
					}
				};
				timer.AsyncWait((int)(timeout * 1000), _timeout_callback);
			}
			else {
				std::unique_lock<std::mutex> locker(mutex);
				callbacks[_cnt] = NULL;
				cmd_pend_count -= 1;
				cmd_cond.notify_all();
				locker.unlock();
			}
		}

		try {
			size_t ret = ser.write(data);
			//if (ret != data.length()) {
			// 	return "Exception";
			//}
			/*
			if (wait) {
				cout << "send sync cmd: " << data;
			}
			else {
				cout << "send async cmd: " << data;
			}
			*/
		}
		catch (serial::PortNotOpenedException &e) {
			connected = false;
			cerr << "PortNotOpenedException: " << e.what() << endl;
			return "Exception";
		}
		catch (serial::SerialException &e) {
			connected = false;
			cerr << "SerialException: " << e.what() << endl;
			return "Exception";
		}
		catch (serial::IOException &e) {
			connected = false;
			cerr << "IOException: " << e.what() << endl;
			return "Exception";
		}
		if (wait) {
			long long start_time = get_system_time();
			while (get_system_time() - start_time < timeout * 1000) {
				if (&result[_cnt] != NULL && &result[_cnt] != 0 && result[_cnt] != "") {
					// cout << "cnt: " << _cnt << ", result: " << result[_cnt] << endl;
					std::unique_lock<std::mutex> locker(mutex);
					cmd_pend_count -= 1;
					cmd_cond.notify_all();
					locker.unlock();
					return result[_cnt];
				}
				sleep_milliseconds(10);
			}
			std::unique_lock<std::mutex> locker(mutex);
			cmd_pend_count -= 1;
			cmd_cond.notify_all();
			locker.unlock();
			return "TIMEOUT";
		}
		else {
			return "ok";
		}
	}
	else {
		return "NotConnected";
	}
}

std::string Swift::send_cmd_sync(const std::string &cmd, float timeout)
{
	return _send_cmd(cmd, true, timeout);
}

std::string Swift::send_cmd_async(const std::string &cmd, float timeout, void(*callback)(std::string))
{
	return _send_cmd(cmd, false, timeout, (std::function<void(std::string)>)callback);
}

std::string Swift::send_cmd_async(const std::string &cmd, float timeout, std::function<void(std::string)> callback)
{
	return _send_cmd(cmd, false, timeout, callback);
}

bool Swift::register_power_callback(void(*callback)(bool))
{
	if (callback != NULL) {
		power_callback = callback;
		return true;
	}
	return false;
}
void Swift::release_power_callback()
{
	power_callback = NULL;
}

bool Swift::register_report_position_callback(void(*callback)(std::vector<float>))
{
	if (callback != NULL) {
		pos_report_callback = callback;
		return true;
	}
	return false;
}
void Swift::release_report_position_callback()
{
	pos_report_callback = NULL;
}

bool Swift::register_key0_callback(void(*callback)(int))
{
	if (callback != NULL) {
		key0_report_callback = callback;
		return true;
	}
	return false;
}
void Swift::release_key0_callback()
{
	key0_report_callback = NULL;
}

bool Swift::register_key1_callback(void(*callback)(int))
{
	if (callback != NULL) {
		key1_report_callback = callback;
		return true;
	}
	return false;
}
void Swift::release_key1_callback()
{
	key1_report_callback = NULL;
}

bool Swift::register_limit_switch_callback(void(*callback)(bool))
{
	if (callback != NULL) {
		limit_switch_callback = callback;
		return true;
	}
	return false;
}
void Swift::release_limit_switch_callback()
{
	limit_switch_callback = NULL;
}

inline int Swift::_handle_set_int(std::string cmd, bool wait, float timeout, void(*callback)(int)) {
	int value = -1;
	if (wait) {
		std::string ret = send_cmd_sync(cmd, timeout);
		std::vector<std::string> tmpList = split(ret, " ");
		// cout << "ret: " << ret << " size:" << tmpList.size() << endl;
		if (tmpList.size() >= 1) {
			if (tmpList[0] == "ok")
				value = 0;
			else if (tmpList[0] == "NotConnected")
				value = -1;
			else if (tmpList[0] == "TIMEOUT")
				value = -2;
			else if (tmpList[0] == "Exception")
				value = -3;
			else if (tmpList[0] == "E20")
				value = -4;
			else if (tmpList[0] == "E21")
				value = -5;
			else if (tmpList[0] == "E22")
				value = -6;
			else if (tmpList[0] == "E23")
				value = -7;
			else if (tmpList[0] == "E24")
				value = -8;
			else if (tmpList[0] == "E25")
				value = -9;
			else if (tmpList[0] == "E26")
				value = -10;
		}
		return value;
	}
	else {
		if (callback != NULL) {
			std::function<void(std::string)> _callback = [callback, this](std::string result) {
				std::vector<std::string> tmpList = split(result, " ");
				int v = -1;
				// cout << "ret: " << result << " size:" << tmpList.size() << endl;
				if (tmpList.size() >= 1) {
					if (tmpList[0] == "ok")
						v = 0;
					else if (tmpList[0] == "NotConnected")
						v = -1;
					else if (tmpList[0] == "TIMEOUT")
						v = -2;
					else if (tmpList[0] == "Exception")
						v = -3;
					else if (tmpList[0] == "E20")
						v = -4;
					else if (tmpList[0] == "E21")
						v = -5;
					else if (tmpList[0] == "E22")
						v = -6;
					else if (tmpList[0] == "E23")
						v = -7;
					else if (tmpList[0] == "E24")
						v = -8;
					else if (tmpList[0] == "E25")
						v = -9;
					else if (tmpList[0] == "E26")
						v = -10;
				}
				if (callback != NULL) {
					callback(v);
				}
			};
			/*
			int _cnt = cnt;
			std::function<void()> _timeout_callback = [callback, _cnt, this]() {
				if (callbacks[_cnt] != NULL) {
					callback(-2);
					callbacks[_cnt] = NULL;
				}
			};
			timer.AsyncWait((int)(timeout * 1000), _timeout_callback);
			*/
			std::string ret = send_cmd_async(cmd, timeout, _callback);
		}
		else {
			std::string ret = send_cmd_async(cmd, timeout, NULL);
		}
		return value;
	}
}

inline std::vector<float> Swift::_handle_get_float_vector(std::string cmd, bool wait, float timeout, void(*callback)(std::vector<float>))
{
	std::vector<float> pos;
	if (wait) {
		std::string ret = send_cmd_sync(cmd, timeout);
		std::vector<std::string> tmpList = split(ret, " ");
		// cout << "ret: " << ret << endl;
		if (tmpList.size() > 1) {
			if (tmpList[0] == "ok" && tmpList.size() >= 4) {
				for (uint8_t i = 1; i < tmpList.size(); i++) {
					pos.push_back(std::stof(tmpList[i].substr(1)));
				}
			}
		}
		return pos;
	}
	else {
		if (callback != NULL) {
			std::function<void(std::string)> _callback = [callback](std::string result) {
				std::vector<std::string> tmpList = split(result, " ");
				// cout << "result: " << result << endl;
				if (tmpList.size() > 1) {
					if (tmpList[0] == "ok" && tmpList.size() >= 4) {
						if (callback != NULL) {
							std::vector<float> p;
							for (uint8_t i = 1; i < tmpList.size(); i++) {
								p.push_back(std::stof(tmpList[i].substr(1)));
							}
							callback(p);
						}
					}
				}
			};
			std::string ret = send_cmd_async(cmd, timeout, _callback);
		}
		else {
			std::string ret = send_cmd_async(cmd, timeout, NULL);
		}
		return pos;
	}
}

inline int Swift::_handle_get_int(std::string cmd, bool wait, float timeout, void(*callback)(int), std::string type)
{
	int value = -1;
	if (wait) {
		std::string ret = send_cmd_sync(cmd, timeout);
		std::vector<std::string> tmpList = split(ret, " ");
		// cout << "ret: " << ret << endl;
		if (tmpList.size() > 1) {
			if (tmpList[0] == "ok" && tmpList.size() >= 2) {
				value = std::stoi(tmpList[1].substr(1));
			}
		}
		if (value != -1) {
			if (type == "mode") {
				mode = value;
			}
			else if (type == "power") {
				power_status = (value > 0) ? true : false;
			}
		}
		return value;
	}
	else {
		if (callback != NULL) {
			std::function<void(std::string)> _callback = [callback, type, this](std::string result) {
				std::vector<std::string> tmpList = split(result, " ");
				int v = -1;
				// cout << "result: " << result << endl;
				if (tmpList.size() > 1) {
					if (tmpList[0] == "ok" && tmpList.size() >= 2) {
						v = std::stoi(tmpList[1].substr(1));
						if (v != -1) {
							if (type == "mode") {
								mode = v;
							}
							else if (type == "power") {
								power_status = (v > 0) ? true : false;
							}
						}
					}
				}
				if (callback != NULL) {
					callback(v);
				}
			};
			std::string ret = send_cmd_async(cmd, timeout, _callback);
		}
		else {
			std::string ret = send_cmd_async(cmd, timeout, NULL);
		}
		return value;
	}
}

inline std::string Swift::_handle_get_string(std::string cmd, bool wait, float timeout, void(*callback)(std::string), std::string type)
{
	std::string value = "";
	if (wait) {
		std::string ret = send_cmd_sync(cmd, timeout);
		std::vector<std::string> tmpList = split(ret, " ");
		// cout << "ret: " << ret << endl;
		if (tmpList.size() > 1) {
			if (tmpList[0] == "ok" && tmpList.size() >= 2) {
				value = tmpList[1].substr(1);
				if (type == "device_type") {
					device_info[0] = value;
				}
				else if (type == "hardware_version") {
					device_info[1] = value;
				}
				else if (type == "firmware_version") {
					device_info[2] = value;
				}
				else if (type == "api_version") {
					device_info[3] = value;
				}
				else if (type == "device_unique") {
					device_info[4] = value;
				}
			}
		}
		return (value != "") ? value : ret;
	}
	else {
		if (callback != NULL) {
			std::function<void(std::string)> _callback = [callback, type, this](std::string result) {
				std::vector<std::string> tmpList = split(result, " ");
				std::string v = "";
				// cout << "result: " << result << endl;
				if (tmpList.size() > 1) {
					if (tmpList[0] == "ok" && tmpList.size() >= 2) {
						v = tmpList[1].substr(1);
						if (v != "") {
							if (type == "device_type") {
								device_info[0] = v;
							}
							else if (type == "hardware_version") {
								device_info[1] = v;
							}
							else if (type == "firmware_version") {
								device_info[2] = v;
							}
							else if (type == "api_version") {
								device_info[3] = v;
							}
							else if (type == "device_unique") {
								device_info[4] = v;
							}
						}
					}
				}
				if (callback != NULL) {
					if (v != "")
						callback(v);
					else
						callback(result);
				}
			};
			std::string ret = send_cmd_async(cmd, timeout, _callback);
		}
		else {
			std::string ret = send_cmd_async(cmd, timeout, NULL);
		}
		return value;
	}
}

int Swift::set_report_position(float interval, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2120 V" + std::to_string(interval);
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_report_keys(bool on, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2213 V" + std::to_string(on ? 0 : 1);
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_servo_attach(int servo_id, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "";
	if (servo_id < 0) {
		cmd += "M17";
	}
	else {
		cmd += "M2201 N" + std::to_string(servo_id);
	}
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_servo_detach(int servo_id, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "";
	if (servo_id < 0) {
		cmd += "M2019";
	}
	else {
		cmd += "M2202 N" + std::to_string(servo_id);
	}
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_mode(int mode, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2400 S" + std::to_string(mode);
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_position(float x, float y, float z, long speed, bool relative, bool wait, float timeout, void(*callback)(int), std::string cmd)
{
	std::string data = "";
	if (relative) {
		data += "G2204 X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z) + " F" + std::to_string(speed);
	}
	else {
		data += cmd + " X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z) + " F" + std::to_string(speed);
	}
	return _handle_set_int(data, wait, timeout, callback);
}

int Swift::set_polar(float stretch, float rotation, float height, long speed, bool relative, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "";
	if (relative) {
		cmd += "G2205 S" + std::to_string(stretch) + " R" + std::to_string(rotation) + " H" + std::to_string(height) + " F" + std::to_string(speed);
	}
	else {
		cmd += "G2201 S" + std::to_string(stretch) + " R" + std::to_string(rotation) + " H" + std::to_string(height) + " F" + std::to_string(speed);
	}
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_servo_angle(int servo_id, float angle, long speed, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "G2202 N" + std::to_string(servo_id) + " V" + std::to_string(angle) + " F" + std::to_string(speed);
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_wrist(float angle, long speed, bool wait, float timeout, void(*callback)(int))
{
	return set_servo_angle(3, angle, speed, wait, timeout, callback);
}

int Swift::set_buzzer(int frequency, float duration, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2210 F" + std::to_string(frequency) + " T" + std::to_string(duration * 1000);
	if (wait) {
		int ret = _handle_set_int(cmd, true, timeout, callback);
		if (ret == 0) {
			sleep_milliseconds((unsigned long)duration * 1000);
		}
		return ret;
	}
	else {
		return _handle_set_int(cmd, false, timeout, callback);
	}
}

int Swift::set_pump(bool on, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2231 V" + std::to_string(on ? 1 : 0);
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_gripper(bool _catch, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2232 V" + std::to_string(_catch ? 1 : 0);
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_digital_output(int pin, int value, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2240 N" + std::to_string(pin) + " V" + std::to_string(value);
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_digital_direction(int pin, int value, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2241 N" + std::to_string(pin) + " V" + std::to_string(value);
	return _handle_set_int(cmd, wait, timeout, callback);
}

int Swift::set_acceleration(float acc, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M204 A" + std::to_string(acc);
	return _handle_set_int(cmd, wait, timeout, callback);
}

void Swift::reset(long speed, float timeout, float x, float y, float z)
{
	set_servo_attach(-1);
	set_wrist(90);
	set_pump(false);
	set_gripper(false);
	set_position(x, y, z, speed, false, true, timeout);
}

std::vector<float> Swift::get_position(bool wait, float timeout, void(*callback)(std::vector<float>))
{
	std::string cmd = "P2220";
	return _handle_get_float_vector(cmd, wait, timeout, callback);
}

std::vector<float> Swift::get_polar(bool wait, float timeout, void(*callback)(std::vector<float>))
{
	std::string cmd = "P2221";
	return _handle_get_float_vector(cmd, wait, timeout, callback);
}

std::vector<float> Swift::get_servo_angle(bool wait, float timeout, void(*callback)(std::vector<float>))
{
	std::string cmd = "P2200";
	return _handle_get_float_vector(cmd, wait, timeout, callback);
}

int Swift::get_mode(bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "P2400";
	return _handle_get_int(cmd, wait, timeout, callback, "mode");
}
int Swift::get_servo_attach(int servo_id, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2203 N" + std::to_string(servo_id);
	return _handle_get_int(cmd, wait, timeout, callback);
}
int Swift::get_limit_switch(bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "P2233";
	return _handle_get_int(cmd, wait, timeout, callback);
}
int Swift::get_gripper_catch(bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "P2232";
	return _handle_get_int(cmd, wait, timeout, callback);
}
int Swift::get_pump_status(bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "P2231";
	return _handle_get_int(cmd, wait, timeout, callback);
}
int Swift::get_power_status(bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "P2234";
	return _handle_get_int(cmd, wait, timeout, callback, "power");
}

int Swift::get_digital(int pin, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "P2240 N" + std::to_string(pin);
	return _handle_get_int(cmd, wait, timeout, callback);
}

int Swift::get_analog(int pin, bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "P2241 N" + std::to_string(pin);
	return _handle_get_int(cmd, wait, timeout, callback);
}

int Swift::get_is_moving(bool wait, float timeout, void(*callback)(int))
{
	std::string cmd = "M2200";
	return _handle_get_int(cmd, wait, timeout, callback);
}

std::string* Swift::get_device_info(float timeout)
{
	float t;
	if (timeout >= 8) {
		t = timeout / 5;
	}
	else {
		t = timeout;
	}
	std::vector<std::string> result;
	std::string cmd = "P2201";
	if (device_info[0] == "") {
		_handle_get_string(cmd, true, t, NULL, "device_type");
	}
	if (device_info[1] == "") {
		cmd = "P2202";
		_handle_get_string(cmd, true, t, NULL, "hardware_version");
	}
	if (device_info[2] == "") {
		cmd = "P2203";
		_handle_get_string(cmd, true, t, NULL, "firmware_version");
	}
	if (device_info[3] == "") {
		cmd = "P2204";
		_handle_get_string(cmd, true, t, NULL, "api_version");
	}
	if (device_info[4] == "") {
		cmd = "P2205";
		_handle_get_string(cmd, true, t, NULL, "device_unique");
	}
	return device_info;
}

void Swift::flush_cmd()
{
	std::unique_lock<std::mutex> locker(mutex);
	cmd_cond.wait(locker, [this] {return cmd_pend_count == 0; });
	locker.unlock();
}