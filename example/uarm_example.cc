/*
* \file example/uarm_example.cc
# Software License Agreement (MIT License)
#
# Copyright (c) 2018, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>
*/
#include <string>
#include <iostream>
#include <cstdio>

#include "uarm/uarm.h"

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;

/*
using uarm::Swift;
using uarm::Timeout;
using uarm::PortInfo;
using uarm::list_ports;
*/

using namespace uarm;

void enumerate_ports()
{
	vector<PortInfo> devices_found = list_ports();

	vector<PortInfo>::iterator iter = devices_found.begin();

	while (iter != devices_found.end())
	{
		PortInfo device = *iter++;
		cout << "port: " << device.port << ", desc: " << device.description << ", hardware_id: " << device.hardware_id << endl;
	}
}

void async_callback(int ret) {
	cout << "async callback: " << ret << endl;
}

void async_callback_str(std::string ret) {
	cout << "async callback string: " << ret << endl;
}

void power_callback(bool power)
{
	cout << "report power: " << power << endl;
}

void pos_report_callback(std::vector<float> pos)
{
	cout << "report ";
	for (uint8_t i = 0; i < pos.size(); i++) {
		cout << "pos[" << std::to_string(i) << "]=" << pos[i];
		if (i != pos.size() - 1)
			cout << ", ";
		else
			cout << endl;
	}
}
void key0_report_callback(int value)
{
	cout << "report key0: " << value << endl;
}
void key1_report_callback(int value)
{
	cout << "report key1: " << value << endl;
}
void limit_switch_callback(bool status)
{
	cout << "report limit switch: " << status << endl;
}

void test_register_callback(Swift *swift) {
	swift->register_power_callback(power_callback);
	swift->register_report_position_callback(pos_report_callback);
	swift->register_key0_callback(key0_report_callback);
	swift->register_key1_callback(key1_report_callback);
	swift->register_limit_switch_callback(limit_switch_callback);

	while (!swift->power_status) {
		int value = swift->get_power_status();
		cout << "get_power_status: " << value << endl;
		if (!swift->power_status) {
			sleep_milliseconds(500);
		}
	}

	int ret = swift->set_report_position(1);
	cout << "set_report_position: " << ret << endl;
	ret = swift->set_report_keys(true);
	cout << "set_report_keys: " << ret << endl;;
}

void test_send_cmd(Swift *swift) {
	std::string ret = swift->send_cmd_sync("G0 X200 Y0 Z90 F100000", 10);
	cout << "send_cmd_sync ret: " << ret << endl;
	ret = swift->send_cmd_async("G0 X200 Y100 Z90 F100000", 10, async_callback_str);
	// cout << "send_cmd_async ret: " << ret << endl;
	ret = swift->send_cmd_sync("G0 X200 Y0 Z90 F100000", 10);
	cout << "send_cmd_sync ret: " << ret << endl;
}

void test_attack_detach(Swift *swift) {
	int ret = swift->set_servo_detach(0);
	cout << "set_servo_detach(0) ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_servo_detach(1);
	cout << "set_servo_detach(1) ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_servo_detach(2);
	cout << "set_servo_detach(2) ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_servo_attach(0);
	cout << "set_servo_attach(0) ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_servo_attach(1);
	cout << "set_servo_attach(1) ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_servo_attach(2);
	cout << "set_servo_attach(2) ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_servo_detach(-1);
	cout << "set_servo_detach(ALL) ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_servo_attach(-1);
	cout << "set_servo_attach(ALL) ret: " << ret << endl;
}

void test_move(Swift *swift) {
	swift->reset();
	swift->flush_cmd();
	int ret = swift->set_position(200, -100, 80, 10000, false, false, 2, async_callback);
	ret = swift->set_position(250, 100, 120, 10000, false, true, 10);
	cout << "set_position ret: " << ret << endl;
	swift->set_polar(200, 0, 80, 10000, false, false, 10, async_callback);
	swift->set_polar(250, 30, 120, 10000, false, true, 10);
	cout << "set_polar ret: " << ret << endl;
	ret = swift->set_servo_angle(0, 120, 10000, false, 10, async_callback);
	ret = swift->set_servo_angle(1, 120, 10000, true);
	cout << "set_servo_angle ret: " << ret << endl;
	swift->flush_cmd();
	
	ret = swift->set_position(200, 0, 150, 5000, false, false, 30, async_callback);
	swift->flush_cmd();
	//cout << "set_position ret: " << ret << endl;
	ret = swift->set_position(200, 100, 80, 5000, false, false, 30, async_callback);
	swift->flush_cmd();
	//cout << "set_position ret: " << ret << endl;
	ret = swift->set_position(200, -100, 80, 5000, false, false, 30, async_callback);
	swift->flush_cmd();
	//cout << "set_position ret: " << ret << endl;
	ret = swift->set_position(200, 100, 80, 5000, false, false, 30, async_callback);
	swift->flush_cmd();
	//cout << "set_position ret: " << ret << endl;
	ret = swift->set_position(200, 0, 150, 5000, false, true, 10, async_callback);
	cout << "set_position ret: " << ret << endl;
}

void test_pump_gripper(Swift *swift) {
	int ret = swift->set_pump(true);
	cout << "set_pump(true) ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_pump(false);
	cout << "set_pump(false) ret: " << ret << endl;
	ret = swift->set_gripper(true);
	cout << "set_gripper(true) ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_gripper(false);
	cout << "set_gripper(false) ret: " << ret << endl;
}

void test_buzzer(Swift *swift) {
	int ret = swift->set_buzzer(1000, 2, true);
	cout << "set_buzzer ret: " << ret << endl;
	sleep_milliseconds(2000);
	ret = swift->set_buzzer(1500, 2, false, 2, async_callback);
}

void async_callback_pos(std::vector<float> pos) {
	cout << "async_callback_pos: ";
	for (uint8_t i = 0; i < pos.size(); i++) {
		cout << "pos[" << std::to_string(i) << "]=" << pos[i];
		if (i != pos.size() - 1)
			cout << ", ";
		else
			cout << endl;
	}
}

void async_callback_polar(std::vector<float> pos) {
	cout << "async_callback_polar: ";
	for (uint8_t i = 0; i < pos.size(); i++) {
		cout << "pos[" << std::to_string(i) << "]=" << pos[i];
		if (i != pos.size() - 1)
			cout << ", ";
		else
			cout << endl;
	}
}

void async_callback_angles(std::vector<float> pos) {
	cout << "async_callback_angles: ";
	for (uint8_t i = 0; i < pos.size(); i++) {
		cout << "angle[" << std::to_string(i) << "]=" << pos[i];
		if (i != pos.size() - 1)
			cout << ", ";
		else
			cout << endl;
	}
}

void async_callback_get(int value) {
	cout << "async_callback_get: " << value << endl;
}

void test_get(Swift *swift) {
	swift->get_position(false, 2, async_callback_pos);
	std::vector<float> pos = swift->get_position(true, 2);
	cout << "get_position: ";
	for (uint8_t i = 0; i < pos.size(); i++) {
		cout << "pos[" << std::to_string(i) << "]=" << pos[i];
		if (i != pos.size() - 1)
			cout << ", ";
		else
			cout << endl;
	}

	swift->get_polar(false, 2, async_callback_polar);
	pos = swift->get_polar(true, 2);
	cout << "get_polar: ";
	for (uint8_t i = 0; i < pos.size(); i++) {
		cout << "polar[" << std::to_string(i) << "]=" << pos[i];
		if (i != pos.size() - 1)
			cout << ", ";
		else
			cout << endl;
	}

	swift->get_servo_angle(false, 2, async_callback_angles);
	pos = swift->get_servo_angle(true, 2);
	cout << "get_servo_angle: ";
	for (uint8_t i = 0; i < pos.size(); i++) {
		cout << "angle[" << std::to_string(i) << "]=" << pos[i];
		if (i != pos.size() - 1)
			cout << ", ";
		else
			cout << endl;
	}

	swift->get_power_status(false, 2, async_callback_get);
	int value = swift->get_power_status();
	cout << "get_power_status: " << value << endl;

	swift->get_mode(false, 2, async_callback_get);
	value = swift->get_mode();
	cout << "get_mode: " << value << endl;

	swift->get_servo_attach(0, false, 2, async_callback_get);
	value = swift->get_servo_attach(0);
	cout << "get_servo_attach: " << value << endl;

	swift->get_limit_switch(false, 2, async_callback_get);
	value = swift->get_limit_switch();
	cout << "get_limit_switch: " << value << endl;

	swift->get_gripper_catch(false, 2, async_callback_get);
	value = swift->get_gripper_catch();
	cout << "get_gripper_catch: " << value << endl;

	swift->get_pump_status(false, 2, async_callback_get);
	value = swift->get_pump_status();
	cout << "get_pump_status: " << value << endl;
	
	std::string *info = swift->get_device_info();
	cout << "device_type: " << info[0] << endl;
	cout << "hardware_version: " << info[1] << endl;
	cout << "firmware_version: " << info[2] << endl;
	cout << "api_version: " << info[3] << endl;
	cout << "device_unique: " << info[4] << endl;
}

int main(int argc, char **argv) {
	/*	
	serial::Serial ser;
	ser.setPort("COM12");
	ser.setBaudrate(115200);
	ser.setTimeout(serial::Timeout::simpleTimeout(1000));
	ser.open();
	*/
	if (argc < 2) {
		// Print usage
		cout << "Usage: test_uarm [-e | {<serial port address>} [register | cmd | attach | detach | move | pump | gripper | buzzer | all]]" << endl;
		return -1;
	}
	std::string arg1(argv[1]);
	std::string port = "";
	std::string type;
	if (argc == 2) {
		if (arg1 == "-e") {
			enumerate_ports();
			return 0;
		}
		else {
			type = arg1;
		}
	}
	else if (argc >= 3) {
		std::string arg2(argv[2]);
		port = arg1;
		type = arg2;
	}

	Swift swift(port);
	if (!swift.connected) {
		return -2;
	}
	// sleep_milliseconds(1000);
	if (swift.get_power_status() <= 0) {
		sleep_milliseconds(500);
	}

	if (type == "all" || type == "register") {
		test_register_callback(&swift);
	}

	while (!swift.power_status) {
		int value = swift.get_power_status();
		cout << "get_power_status: " << value << endl;
		if (!swift.power_status) {
			sleep_milliseconds(500);
		}
	}
	if (type == "all" || type == "cmd") {
		test_send_cmd(&swift);
		sleep_milliseconds(2000);
	}
	if (type == "all" || type == "attach" || type == "detach") {
		test_attack_detach(&swift);
		sleep_milliseconds(2000);
	}
	if (type == "all" || type == "pump" || type == "gripper") {
		test_pump_gripper(&swift);
		sleep_milliseconds(2000);
	}
	if (type == "all" || type == "move") {
		test_move(&swift);
		sleep_milliseconds(2000);
	}
	if (type == "all" || type == "buzzer") {
		test_buzzer(&swift);
		sleep_milliseconds(2000);
	}
	if (type == "all" || type == "get") {
		test_get(&swift);
		sleep_milliseconds(2000);
	}

	while (1) {
		sleep_milliseconds(500);
	}
	return 0;
}
