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
  
    Serial.begin(9600); // With ATmega128 Serial Way

    InitDisplay();
    InitFP();

    DisplayWelcome();
    SystemInit();
}

void loop()
{
    if (Serial.available())
    {
        int code = Serial.read();

        if (code == DOOR_OPEN)
        {
            isOpen = true;

            DisplayString("Door O");
            delay(2000);
        }
        else if (code == DOOR_CLOSE)
        {
            isOpen = false;

            DisplayString("Door C");
            delay(2000);
        }
    }
    else if (!isOpen)
    {
        ShowWaitInput();

        char key = dKeypad.getKey();
        
        if (key != NO_KEY)
        {
            if ((pwCount < 10) && (key >= '0') && (key <= '9'))
            {
                password[pwCount++] = key;
            }
            else if (key == '*')
            {
                CheckPW();
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
        else if (fingerSensor.getImage() != FINGERPRINT_NOFINGER)
        {
            CheckFP();
        }
    }
}