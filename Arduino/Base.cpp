#include "Base.h"
#include "SerialCode.h"

byte isWaitInput = 0;
boolean isOpen = false;

byte rowPins[KEYPAD_ROWS] = { 5, 6, 15, 14 };
byte colPins[KEYPAD_COLS] = { 17, 16, 8, 7 };

char pwCount = 0;

char password[PW_MAX] = { 0 };
char tPassword[PW_MAX] = { 0 };

char keys[KEYPAD_ROWS][KEYPAD_COLS] = 
{
    { 'A', 'B', 'C', 'D' },
    { '3', '6', '9', '#' },
    { '2', '5', '8', '0' },
    { '1', '4', '7', '*' }
};

Keypad dKeypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SoftwareSerial fingerSerial = SoftwareSerial(2, 3);
SoftwareSerial atmegaSerial = SoftwareSerial(10, 9);
SoftwareSerial btSerial = SoftwareSerial(11, 12);
Adafruit_Fingerprint fingerSensor = Adafruit_Fingerprint(&fingerSerial);

// System Function

void DisplayWelcome()
{
    display.clearDisplay();

    display.setCursor(0, 0);
    display.println("* CDL System *\n");
    display.println("ver. 0.1");

    display.display();

    delay(3000);
}

void SystemInit()
{
    char key;
    int code;

    DisplayString("Wait other system...");

    Serial.write(SERIAL_WAIT);

    while (!Serial.available());

    atmegaSerial.listen();

    display.println();
    display.println("ATmega128");

    display.display();

    atmegaSerial.write(SERIAL_WAIT);

    while (!atmegaSerial.available());

    display.println();
    display.println(atmegaSerial.read());

    display.display();

    delay(1000);

    SetPW();

    display.clearDisplay();

    display.setCursor(0, 0);
    display.println("First Set O");

    display.display();

    delay(2000);
}

void SetPW()
{
    char key, i;

    ShowPWDisplay();

    while (1)
    {
        key = dKeypad.getKey();
        
        if (key != NO_KEY)
        {
            if ((pwCount < 10) && (key >= '0') && (key <= '9'))
            {
                password[pwCount++] = key;

                ShowPWDisplay();
            }
            else if (key == '*')
            {
                if (pwCount >= 1)
                {
                    DisplayString("Set PW O");
                    delay(2000);

                    break;
                }
                else
                {
                    DisplayString("Empty!");
                    delay(2000);
                    ShowPWDisplay();
                }
            }
        }
    }

    pwCount = 0;
}

void ChangePW()
{
    char key, count = 0;

    ResetTempPW();

    DisplayString("Input PW");

    while (1)
    {
        key = dKeypad.getKey();

        if (count >= 5)
        {
            return;
        }

        if (key != NO_KEY)
        {
            if ((pwCount < 10) && (key >= '0') && (key <= '9'))
            {
                tPassword[pwCount++] = key;

                ShowPWDisplay();
            }
            else if (key == '*')
            {
                if (CheckPW())
                {
                    break;
                }
                else
                {
                    DisplayString("Try again");
                    delay(2000);
                    ResetTempPW();

                    count += 1;
                } 
            }
        }
    }

    ResetTempPW();
    SetPW();
}

void ResetTempPW()
{
    for (char i = 0; i < 10; ++i)
    {
        tPassword[i] = 0;
    }

    pwCount = 0;
}

boolean CheckPW()
{
    byte isCorrect = 1;

    for (char i = 0; i < 10; ++i)
    {
        if (password[i] != tPassword[i])
        {
            isCorrect = 0;

            break;
        }
    }

    if (isCorrect)
    {
        DisplayString("PW O");
        delay(2000);
    }
    else
    {
        DisplayString("PW X");
        delay(2000);
    }

    pwCount = 0;

    ResetTempPW();

    return (isCorrect == 1);
}

void ShowPWDisplay()
{
    display.clearDisplay();

    display.setCursor(0, 0);
    display.println("Set PW");
    display.println("");

    for (char i = 0; i < pwCount; ++i)
    {
        display.print("*");
    }

    display.display();
}

void ShowWaitInput()
{
    display.clearDisplay();

    display.setCursor(1, 0);
    display.println("Wait...");
    display.println("");

    for (char i = 0; i < pwCount; ++i)
    {
        display.print("*");
    }

    display.display();

    isWaitInput = 1;
}



// Display Function

void InitDisplay()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        while (1);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
}

void DisplayString(String s)
{
    display.clearDisplay();

    display.setCursor(0, 0);
    display.println(s);

    display.display(); 
}



// FingerPrint Function

void InitFP()
{
    fingerSensor.begin(57600);
}

void SetFP()
{
    if (!CheckFingerSensorStatus())
    {
        DisplayString("FP not working!");
        delay(2000);
        ShowWaitInput();

        return;
    }

    fingerSensor.getTemplateCount();

    if (EnrollFP(fingerSensor.templateCount + 1))
    {
        DisplayString("FP set O");
    }
    else
    {
        DisplayString("FP set X");
    }

    delay(2000);
}

boolean CheckFingerSensorStatus()
{
    return fingerSensor.verifyPassword();
}

boolean CheckFP()
{
    if (VerifyFP())
    {
        DisplayString("FP O");
        delay(2000);

        return true;
    }
    else
    {
        DisplayString("FP X");
        delay(2000);

        return false;
    }
}

boolean VerifyFP()
{
    uint8_t p = fingerSensor.getImage();

    if (p != FINGERPRINT_OK)
    {
        return false;
    }

    p = fingerSensor.image2Tz();

    if (p != FINGERPRINT_OK)  
    {
        return false;
    }

    p = fingerSensor.fingerFastSearch();

    if (p != FINGERPRINT_OK)  
    {
        return false;
    }
    
    return true; 
}

void WaitRemoveFinger()
{
    int p = 0;
  
    DisplayString("Remove finger");

    while ((p = fingerSensor.getImage()) != FINGERPRINT_NOFINGER);

    delay(1000);
}

boolean EnrollFP(uint8_t id)
{
    int p;
    char count;

CHECK:
    p = -1;
    count = 0;

    DisplayString("Waiting...");

    while ((p = fingerSensor.getImage()) != FINGERPRINT_OK) 
    {
        switch (p) 
        {
            case FINGERPRINT_OK:
                display.print("S");
                display.display();
                break;
            case FINGERPRINT_NOFINGER:
                if (!(count % 10))
                {
                    display.print(".");
                    display.display();
                }
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
            case FINGERPRINT_IMAGEFAIL:
            default:
                display.println("F");
                display.display();
                delay(1000);
                goto CHECK;
                break;
        }

        delay(100);
        count += 1;

        if (count >= 100)
        {
            return false;
        }
    }

    delay(1000);

    DisplayString("Convert...");
    p = fingerSensor.image2Tz(1);

    switch (p) 
    {
        case FINGERPRINT_OK:
            display.print("S");
            display.display();
            break;
        case FINGERPRINT_IMAGEMESS:
        case FINGERPRINT_PACKETRECIEVEERR:
        case FINGERPRINT_FEATUREFAIL:
        case FINGERPRINT_INVALIDIMAGE:
        default:
            display.print("F!");
            display.display();
            delay(1000);
            return false;
    }

    delay(1000);

    WaitRemoveFinger();

CHECK2:
    p = -1;
    count = 0;

    DisplayString("Place same finger");

    while ((p = fingerSensor.getImage()) != FINGERPRINT_OK) 
    {
        switch (p) 
        {
            case FINGERPRINT_OK:
                display.print("S");
                display.display();
                break;
            case FINGERPRINT_NOFINGER:
                if (!(count % 10))
                {
                    display.print(".");
                    display.display();
                }
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
            case FINGERPRINT_IMAGEFAIL:
            default:
                display.print("F");
                display.display();
                delay(1000);
                goto CHECK2;
                break;
        }

        delay(100);

        count += 1;

        if (count >= 100)
        {
            return false;
        }
    }

    delay(1000);

    DisplayString("Convert...");

    p = fingerSensor.image2Tz(2);

    switch (p) 
    {
        case FINGERPRINT_OK:
            display.print("S");
            display.display();
            break;
        case FINGERPRINT_IMAGEMESS:
        case FINGERPRINT_PACKETRECIEVEERR:
        case FINGERPRINT_FEATUREFAIL:
        case FINGERPRINT_INVALIDIMAGE:
        default:
            display.print("F");
            display.display();
            delay(1000);
            return false;
    }

    delay(1000);

    DisplayString("Creating model");  
    display.print("ID No. ");
    display.println(id);
    
    display.display();

    p = fingerSensor.createModel();

    switch (p)
    {
        case FINGERPRINT_OK:
            display.print("S");
            display.display();
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
        case FINGERPRINT_ENROLLMISMATCH:
        default:
            display.print("F");
            display.display();
            delay(1000);
            return false;
    }

    delay(1000);

    DisplayString("Storing model");
    display.print("ID No. ");
    display.println(id);
    
    display.display();

    p = fingerSensor.storeModel(id);

    switch (p)
    {
        case FINGERPRINT_OK:
            display.print("S");
            display.display();
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
        case FINGERPRINT_BADLOCATION:
        case FINGERPRINT_FLASHERR:
        default:
            display.print("F");
            display.display();
            delay(1000);
            return false;
    }

    delay(1000);

    WaitRemoveFinger();

    return true;
}


// Buzzer Function

void BuzzerOpen()
{
    int note[] = { 2093, 2637, 3136 };

    for (int i = 0; i < 3; ++i)
    {
        tone(BUZZER_PIN, note[i], 500);
    }
}

void BuzzerCheckClose()
{
    int note = 2637;

    tone(BUZZER_PIN, note, 500);
    tone(BUZZER_PIN, note, 500);
}

void BuzzerClose()
{
    int note[] = { 3136, 2637, 2093 };

    for (int i = 0; i < 3; ++i)
    {
        tone(BUZZER_PIN, note[i], 500);
    }
}

void BuzzerLoop(float tDelay)
{
    int k;

    for (k = 0; k < 120; ++k)
	{
		digitalWrite(BUZZER_PIN, HIGH);

		delay(tDelay);
		
        digitalWrite(BUZZER_PIN, LOW);
		
        delay(tDelay);
	}
}
