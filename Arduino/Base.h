#ifndef Base_h
#define Base_h

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
#define PW_MAX 10

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <SPI.h>
#include <Keypad.h>


extern byte isWaitInput;
extern boolean isOpen;

extern byte rowPins[KEYPAD_ROWS];
extern byte colPins[KEYPAD_COLS];

extern char pwCount;

extern char password[PW_MAX];

extern char keys[KEYPAD_ROWS][KEYPAD_COLS];

extern Keypad dKeypad;
extern Adafruit_SSD1306 display;
extern SoftwareSerial fingerSerial;
extern Adafruit_Fingerprint fingerSensor;


void DisplayWelcome();
void SystemInit();
void SetPW();
void ChangePW();
void ResetTempPW();
boolean CheckPW();
void ShowPWDisplay();
void ShowWaitInput();

void InitDisplay();
void DisplayString(String s);

void InitFP();
void SetFP();
boolean CheckFingerSensorStatus();
void CheckFP();
boolean VerifyFP();
void WaitRemoveFinger();
boolean EnrollFP(uint8_t id);

#endif