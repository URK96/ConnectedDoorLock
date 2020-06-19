#include "Base.h"
#include "SerialCode.h"

/*#include <Adafruit_Fingerprint.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <SPI.h>*/

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
  
    Serial.begin(9600); // Arduino -- RPi serial way
    atmegaSerial.begin(9600);
    btSerial.begin(9600);

    InitDisplay();
    InitFP();

    DisplayWelcome();
    SystemInit();
}

void loop()
{
    atmegaSerial.listen();

    delay(50);
    
    // Receive ATmega128 signal
    if (atmegaSerial.available())
    {
        int code = atmegaSerial.read();

        display.println(code);
        display.display();

        switch (code)
        {
            case DOOR_OPEN:
                isOpen = true;

                DisplayString("Door O");
                Serial.write(DOOR_OPEN);
                BuzzerOpen();
                break;
            case DOOR_CLOSE:
                isOpen = false;

                DisplayString("Door C");
                Serial.write(DOOR_CLOSE);
                BuzzerClose();
                break;
            case DOOR_CHECKCLOSE:
                BuzzerCheckClose();
                break;
            case CAMERA_SCREENSHOT:
                Serial.write(CAMERA_SCREENSHOT);
                break;
            case CAMERA_RECORD_START:
                Serial.write(CAMERA_RECORD_START);
                break;
        }

        delay(500);
    }
    // Receive RPi 4 signal
    else if (Serial.available())
    {
        int code = Serial.read();

        display.println(code);
        display.display();

        switch (code)
        {
            case RFID_PASS:
                atmegaSerial.write(RFID_PASS);
                break;
        }

        delay(500);
    }
    else if (!isOpen)
    {
        ShowWaitInput();

        char key = dKeypad.getKey();
        
        // Process 16x16 keypad signal
        if (key != NO_KEY)
        {
            if ((pwCount < 10) && (key >= '0') && (key <= '9'))
            {
                tPassword[pwCount++] = key;
            }
            else if (key == '*')
            {
                if (CheckPW())
                {
                    atmegaSerial.listen();
                    atmegaSerial.write(PW_CORRECT);
                    isOpen = true;
                }
            }
            else if (key == '#')
            {
                ResetTempPW();
            }
            else if (key == 'A')
            {
                DisplayString("Change PW");
                delay(2000);
                ChangePW();
            }
            else if (key == 'B')
            {
                DisplayString("Add FP");
                delay(2000);

                fingerSerial.listen();
                SetFP();
            }
            else if (key == 'C')
            {
                DisplayString("Del FP?");
                delay(2000);
                display.println("");
                display.println("1 : Y, 2: N");

                display.display();

                while ((key = dKeypad.getKey()) == NO_KEY);

                if (key == '1')
                {
                    fingerSensor.emptyDatabase();
                }
            }
        }

        btSerial.listen();

        delay(50);

        // Receive bluetooth signal
        if (btSerial.available())
        {
            ResetTempPW();

            while (btSerial.available())
            {
                char data = btSerial.read();

                if ((data >= '0') && (data <= '9'))
                {
                    tPassword[pwCount++] = data;
                }
                else if (data == '*')
                {
                    break;
                }
            }

            if (CheckPW())
            {
                atmegaSerial.write(BT_PASS);
                isOpen = true;
            }
        }

        fingerSerial.listen();
        
        // Receive fingerprint sensor signal
        if (fingerSensor.getImage() != FINGERPRINT_NOFINGER)
        {
            if (CheckFP())
            {
                atmegaSerial.listen();

                atmegaSerial.write(FP_PASS);
                isOpen = true;
            }
        }
    }
}