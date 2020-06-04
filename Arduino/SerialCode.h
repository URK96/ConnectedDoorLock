#ifndef SC_h
#define SC_h

// Serial Status Code

#define SERIAL_WAIT 0x40
#define SERIAL_SENDEND 0x4F


// System Code


// Door Status Code

#define DOOR_CLOSE 0x00
#define DOOR_OPEN 0x01 // with PW_CORRECT


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

#endif