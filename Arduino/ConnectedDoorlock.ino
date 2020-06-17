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
  
    Serial.begin(9600); // With RPi Serial Way
    atmegaSerial.begin(9600);

    InitDisplay();
    InitFP();

    DisplayWelcome();
    SystemInit();
}

void loop()
{
    atmegaSerial.listen();

    delay(50);

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
                //delay(2000);
                break;
            case DOOR_CLOSE:
                isOpen = false;

                DisplayString("Door C");
                //delay(2000);
                break;
            case CAMERA_SCREENSHOT:
                Serial.write(CAMERA_SCREENSHOT);
                break;
            case CAMERA_RECORD_START:
                Serial.write(CAMERA_RECORD_START);
                break;
        }

        delay(1000);
    }
    else if (!isOpen)
    {
        ShowWaitInput();

        char key = dKeypad.getKey();
        
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

        fingerSerial.listen();
        
        if (fingerSensor.getImage() != FINGERPRINT_NOFINGER)
        {
            if (CheckFP())
            {
                atmegaSerial.listen();

                atmegaSerial.write(FP_PASS);
            }
        }
    }
}