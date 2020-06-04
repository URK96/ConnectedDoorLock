#include "Base.h"
#include "SerialCode.h"


byte isWaitInput = 0;
boolean isOpen = false;

byte rowPins[KEYPAD_ROWS] = { 7, 6, 5, 4 };
byte colPins[KEYPAD_COLS] = { 11, 10, 9, 8 };

char pwCount = 0;

char password[PW_MAX] = { 0 };

char keys[KEYPAD_ROWS][KEYPAD_COLS] = 
{
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
};

Keypad dKeypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SoftwareSerial fingerSerial = SoftwareSerial(2, 3);
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

    Serial.write(REQ_PW);

    while (!Serial.available());

    code = Serial.read();

    if (code == NO_PW)
    {
        SetPW();
    }

    /*display.clearDisplay();

    display.setCursor(0, 0);
    display.println("Set PW");
    display.println("");
    display.println("Press any key");

    display.display();

    while ((key = dKeypad.getKey()) == NO_KEY);

    SetPW();*/

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

    DisplayString("Save PW...");

    Serial.write(SAVE_PW);

    for (i = 0; i < pwCount; ++i)
    {
        while (!Serial.available());

        Serial.write(password[pwCount]);
    }

    Serial.write(SERIAL_SENDEND);

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
                password[pwCount++] = key;

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
        password[i] = 0;
    }

    pwCount = 0;
}

boolean CheckPW()
{
    byte isCorrect = 1;
    int code;
    char i;

    Serial.write(CHECK_PW);

    while (!Serial.available());

    for (i = 0; i < pwCount; ++i)
    {
        Serial.write(password[i]);
    }

    Serial.write(SERIAL_SENDEND);

    while (!Serial.available());

    code = Serial.read();

    if (code == PW_CORRECT)
    {
        DisplayString("PW O!");
        delay(2000);

        return true;
    }
    else if (code == PW_INCORRECT)
    {
        DisplayString("PW X!");
        delay(2000);

        return false;
    }
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
    //fingerSensor.emptyDatabase();
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

void CheckFP()
{
    if (VerifyFP())
    {
        DisplayString("FP O");
        Serial.write(FP_PASS);
        delay(2000);
    }
    else
    {
        DisplayString("FP X");
        delay(2000);
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