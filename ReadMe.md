# uArm-C++-SDK
----------

## Overview
- This module encapsulates the operations for uArm. It provides baisc Movement on C++.
- The library only supports uArm Swift/SwiftPro
- The library is based on [serial](https://github.com/wjwwood/serial)

## Caution
- Temporarily only supports Swift / SwiftPro.
- Make sure you move the device head to a safe position and completely quit uArm Studio before running the tests.


## Doc
- [Swift/SwiftPro](doc/swift_api.md)

### Install

Get the code:

    git clone https://github.com/uArm-Developer/uArm-SDK.git

#### Linux/MacOS 
- Build (only in Linux and MacOS):
	```
	make
	```

- Install (only in Linux and MacOS):
	```
	sudo make install
	```

- Run Test (only in Linux and MacOS)
    ```
	./test_uarm all
	```

#### Windows
- please refer to [visual_studio example](visual_studio/)


## Example:
- [Swift/SwiftPro](example/)

**Import**
```
#include "uarm/uarm.h"
using namespace uarm;	
```

**Create instance**
```
Swift swift();
Swift swift(port);
```

**API list**
- Please refer to the [documentation](doc/swift_api.md) or the appendix for details and parameters of the API.

```
// connect and disconnect
swift.connect(...);
swift.disconnect(...);

// send cmd
swift.send_cmd_sync(...);
swift.send_cmd_async(...);

// register and release event callback
swift.register_power_callback(...);
swift.release_power_callback();
swift.register_report_position_callback(...);
swift.release_report_position_callback();
swift.register_key0_callback(...);
swift.release_key0_callback();
swift.register_key1_callback(...);
swift.release_key1_callback();
swift.register_limit_switch_callback(...);
swift.release_limit_switch_callback();
swift.set_report_position(...);
swift.set_report_keys(...);

// move
swift.set_position(...);
swift.set_polar(...);
swift.set_servo_angle(...);
swift.set_wrist(...);
swift.reset(...);

// set
swift.set_servo_attach(...);
swift.set_servo_detach(...);
swift.set_mode(...);
swift.set_buzzer(...);
swift.set_pump(...);
swift.set_gripper(...);
swift.set_digital_output(...);
swift.set_digital_direction(...);
swift.set_acceleration(...);

// get
swift.get_position(...);
swift.get_polar(...);
swift.get_servo_angle(...);
swift.get_device_info(...);
swift.get_power_status(...);
swift.get_mode(...);
swift.get_servo_attach(...);
swift.get_limit_switch(...);
swift.get_gripper_catch(...);
swift.get_pump_status(...);
swift.get_digital(...);
swift.get_analog(...);
swift.get_is_moving(...);

// flush cmd
swift.flush_cmd(...);
```

-------------------------------
# Appendix (Swift API) #
namespace uarm {
using serial::Timeout;
using serial::PortInfo;
using serial::list_ports;

## class  Swift ##

### class constructor ###

/\*!
- @param port: a std::string containing the address of the serial port, which would be something like 'COM1' on Windows and '/dev/ttyACM0' on Linux.
- @param baudrate: an unsigned 32-bit integer that represents the baudrate
- @param timeout: A Timeout struct that defines the timeout

\*/
```
Swift(const std::string &port = "", uint32_t baudrate = 115200, Timeout timeout = Timeout::simpleTimeout(1000), int cmd_pend_size = 2);
```
--------------------------------------

### property ###

```
bool connected; // connect status
serial::Serial ser; // serial instance
bool power_status; // power status
std::string port; // port name
uint32_t baudrate; // baudrate
uint8_t mode;
std::string device_info[5];
	// device_info[0]: device_type
	// device_info[1]: hardware_version
	// device_info[2]: firmware_version
	// device_info[3]: api_version
	// device_info[4]: device_unique
```
----------------------------------------

### method ###

/\*! Connect to uArm
- @param port: a std::string containing the address of the serial port, which would be something like 'COM1' on Windows and '/dev/ttyS0' on  Linux.
- @param baudrate: An unsigned 32-bit integer that represents the baudrate
- @param timeout: A Timeout struct that defines the timeout
- return: 0 if connect success else -1

\*/
```
int connect(const std::string &port = "", uint32_t baudrate = 115200, Timeout timeout = Timeout::simpleTimeout(1000));
```	

/\*! Disconnect to uArm
\*/
```
void disconnect();
```

/\*! Sync send cmd to uArm
- @param cmd: a std::string
- @param timeout: A float number that maximum seconds to waiting the execute result return
- return: the execute result or TIMEOUT or NotConnected or Exception

\*/
```
std::string send_cmd_sync(const std::string &cmd, float timeout = default_timeout_2);
```	

/\*! Async send cmd to uArm
- @param cmd: a std::string
- @param callback: a function pointer to handle the execute result
- return: OK or TIMEOUT or NotConnected or Exception

\*/
```
std::string send_cmd_async(const std::string &cmd, 
	float timeout = default_timeout_2, 
	std::function<void (std::string)> callback = NULL);
std::string send_cmd_async(const std::string &cmd, 
	float timeout = default_timeout_2, 
	void(*callback)(std::string) = NULL);
```

/\*! Register power event callback
- @param callback: a function pointer
- return: true/false

\*/
```
bool register_power_callback(void(*callback)(bool));
```

/\*! Release power event callback
\*/
```
void release_power_callback();
```

/\*! Register position report event callback
- @param callback: a function pointer
- return: true/false

\*/
```
bool register_report_position_callback(void(*callback)(std::vector<float>));
```

/\*! Release position report event callback
\*/
```
void release_report_position_callback();
```

/\*! Register key0 pressed event callback
- @param callback: a function pointer
- return: true/false

\*/
```
bool register_key0_callback(void(*callback)(int));
```

/\*! Release key0 pressed event callback
\*/
```
void release_key0_callback();
```

/\*! Register key1 pressed event callback
- @param callback: a function pointer
- return: true/false

\*/
```
bool register_key1_callback(void(*callback)(int));
```

/\*! Release key1 pressed event callback
\*/
```
void release_key1_callback();
```

/\*! Register limit switch event callback
- @param callback: a function pointer
- return: true/false

\*/
```
bool register_limit_switch_callback(void(*callback)(bool));
```

/\*! Release limit switch event callback
\*/
```
void release_limit_switch_callback();
```

/\*! Report position in (interval) seconds
- @param interval: seconds, disable report if interval is 0
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: 
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_report_position(float interval, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Report the buttons event
- @param on: true/false, disable report if on is false
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_report_keys(bool on, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Attach the servo with the servo_id
- @param servo_id: -1 ~ 3, -1 represents all servo
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_servo_attach(int servo_id, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Detach the servo with the servo_id
- @param servo_id: -1 ~ 3, -1 represents all servo
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_servo_detach(int servo_id, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Set the mode, only support SwiftPro
- @param mode: mode, 0: general mode, 1: laser mode, 2: 3D Print mode, 3: pen/gripper mode
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_mode(int mode, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Set the position
- @param x: (mm) location X
- @param y: (mm) location Y
- @param x: (mm) location Z
- @param speed: (mm/min) speed of move
- @param relative: relative move or not, default is false
- @param wait: true/false, default is false
- @param timeout: timeout, default is 10s
- @param callback: callback, default is None, only available if wait is true
- @param cmd: "G0" or "G1", default is "G0"
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_position(float x, float y, float z, long speed = default_speed, bool relative = false, bool wait = false, float timeout = default_timeout_10, void(*callback)(int) = NULL, std::string cmd="G0");
```

/\*! Set the polar coordinate
- @param stretch: (mm)
- @param rotation: (degree), 0 ~ 180
- @param height: (degree)
- @param speed: (mm/min) speed of move
- @param relative: relative move or not, default is false
- @param wait: true/false, default is false
- @param timeout: timeout, default is 10s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_polar(float stretch, float rotation, float height, long speed = default_speed, bool relative = false, bool wait = false, float timeout = default_timeout_10, void(*callback)(int) = NULL);
```

/\*! Set the servo angle
- @param servo_id: servo_id, 0 ~ 3
- @param angle: (degree), 0 ~ 180
- @param speed: (mm/min) speed of move
- @param relative: relative move or not, default is false
- @param wait: true/false, default is false
- @param timeout: timeout, default is 10s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_servo_angle(int servo_id, float angle, long speed = default_speed, bool wait = false, float timeout = default_timeout_10, void(*callback)(int) = NULL);
```

/\*! Set the wrist angle (SERVO HAND)
- @param angle: (degree), 0 ~ 180
- @param speed: (mm/min) speed of move
- @param relative: relative move or not, default is false
- @param wait: true/false, default is false
- @param timeout: timeout, default is 10s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_wrist(float angle, long speed = default_speed, bool wait = false, float timeout = default_timeout_10, void(*callback)(int) = NULL);
```

/\*!  Control the buzzer
- @param frequency: frequency, default is 1000
- @param duration: duration, default is 2s
- @param wait: true/false, default is false
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_buzzer(int frequency = 1000, float duration = 2, bool wait = false, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Control the pump
- @param on: true/false
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_pump(bool on, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Control the gripper
- @param on: true/false
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_gripper(bool _catch, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Set digital output value
- @param pin: IO pin
- @param value: 0: output low level, 1: output high level
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_digital_output(int pin, int value, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Set digital direction
- @param pin: IO pin
- @param value: 0: input, 1: output
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_digital_direction(int pin, int value, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Set the acceleration, only support firmware version > 4.0
- @param acc: acc value
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return:
	- 0: ok
	- -1: not connect
	- -2: timeout
	- -3: serial exception
	- -4: command not exist
	- -5: params error
	- -6: address over
	- -7: command buffer full
	- -8: power is not connect
	- -9: operatation failed
	- -10: servo is detach, can not execute the cmd

\*/
```
int set_acceleration(float acc, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Reset
- @param speed:
- @param timeout: timeout, default is 10s
- @param x: reset position x, default is 200
- @param y: reset position x, default is 0
- @param z: reset position x, default is 150

*/
```
void reset(long speed = default_speed, float timeout = default_timeout_10, float x = 200, float y = 0, float z = 150);
```

/\*! Get position
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: std::vector<float> object, like [x, y, z, r] or [] 

\*/
```
std::vector<float> get_position(bool wait = true, float timeout = default_timeout_2, void(*callback)(std::vector<float>) = NULL);
```

/\*! Get polar
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: std::vector<float> object, like [stretch, rotation, height] or []

\*/
```
std::vector<float> get_polar(bool wait = true, float timeout = default_timeout_2, void(*callback)(std::vector<float>) = NULL);
```

/\*! Get servo angle
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: std::vector<float> object, like [angle-0, angle-1, angle-2] or []

\*/
```
std::vector<float> get_servo_angle(bool wait = true, float timeout = default_timeout_2, void(*callback)(std::vector<float>) = NULL);
```

/\*! Get device info
- @param timeout: timeout, default is 10s
- return: device_info
	- device_info[0]: device_type
	- device_info[1]: hardware_version
	- device_info[2]: firmware_version
	- device_info[3]: api_version
	- device_info[4]: device_unique

\*/
```
std::string* get_device_info(float timeout = default_timeout_10);
```

/\*! Get power status
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: 0: power off, 1: power on, -1: failed

\*/
```
int get_power_status(bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Get mode
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: mode, 0: general mode, 1: laser mode, 2: 3D Print mode, 3: pen/gripper mode, -1: failed

\*/
```
int get_mode(bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Get servo attach status
- @param servo_id:
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: 0: attach, 1: detach, -1: failed

\*/
```
int get_servo_attach(int servo_id, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Get limit switch
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: 0: limit switch on, 1: limit switch off, -1: failed

\*/
```
int get_limit_switch(bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Get gripper status
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: 0: stop, 1: working, 2: catch thing, -1: failed

\*/
```
int get_gripper_catch(bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Get pump status
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: 0: stop, 1: working, 2: pump thing, -1: failed

\*/
```
int get_pump_status(bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Get the digital value from specific pin
- @param pin: specific pin
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: 0: low level, 1: high level, -1: failed

\*/
```
int get_digital(int pin, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Get the analog value from specific pin
- @param pin: specific pin
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: analog value if return !== -1 else failed

\*/
```
int get_analog(int pin, bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Check uArm is moving or not
- @param wait: true/false, default is true
- @param timeout: timeout, default is 2s
- @param callback: callback, default is None, only available if wait is true
- return: 0: not move, 1: moving, -1: failed

\*/
```
int get_is_moving(bool wait = true, float timeout = default_timeout_2, void(*callback)(int) = NULL);
```

/\*! Wait until all command return or timeout
\*/
```
void flush_cmd();
```
};
} // namespace uarm



