#ifndef SC_h
#define SC_h

// Serial Status Code

#define SERIAL_WAIT 0x40
#define SERIAL_SENDEND 0x4F


// System Code

#define DOOR_DS_ON 0x30
#define DOOR_DS_OFF 0x31
#define CAMERA_DS_ON 0x32
#define CAMERA_DS_OFF 0x33
#define CAMERA_SCREENSHOT 0x34
#define CAMERA_RECORD_START 0x35
#define CAMERA_RECORD_STOP 0x36


// Door Status Code

#define DOOR_CLOSE 0x00
#define DOOR_OPEN 0x01 // with PW_CORRECT
#define DOOR_CHECKCLOSE 0x02


// PW Serial Code

#define REQ_PW 0x10
#define SAVE_PW 0x11
#define CHECK_PW 0x12
#define PW_INCORRECT 0x13
#define PW_CORRECT 0x14
#define EXIST_PW 0x1E
#define NO_PW 0x1F


// FP, BT, RFID Code

#define FP_PASS 0x20
#define FP_FAIL 0x21
#define RFID_PASS 0x22
#define RFID_FAIL 0x23
#define BT_PASS 0x24
#define BT_FAIL 0x25

#endif