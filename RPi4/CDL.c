#define PW_MAX 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#include "SerialCode.h"
#include "MFRC522.h"

/*typedef enum
{
    false = 0,
    true = 1
}bool;*/

int fd;
char data;
bool isDoorOpen = false;

byte rfidCardUID[] = { 0x69, 0xDF, 0x0C, 0xB4 };

MFRC522 mfrc;

PI_THREAD(RFIDScan)
{
    bool isPass = false;

    while (1)
    {
        if (!isDoorOpen)
        {
            if (!mfrc.PICC_IsNewCardPresent() || !mfrc.PICC_ReadCardSerial())
            {
                delay(500);

                continue;
            }

            isPass = true;

            for (byte i = 0; i < mfrc.uid.size; ++i)
            {
                if (mfrc.uid.uidByte[i] != rfidCardUID[i])
                {
                    isPass = false;

                    break;
                }
            }

            if (isPass)
            {
                fprintf(stdout, "RFID correct! Send door open signal.\n");
                serialPutchar(fd, RFID_PASS);

                isDoorOpen = true;
            }
            else
            {
                fprintf(stdout, "RFID not correct!\n");
            }
        }

        delay(500);
    }
}

void RunScreenShotCmd()
{
    time_t t;
    struct tm *tp;
    
    char cmd[100] = "raspistill -w 1920 -h 1080 -rot 180 -o ";
    char subCmd[50] = " && ./FUpload ";
    char ext[5] = ".jpg";
    char fileName[50] = "";
    
    time(&t);
	
	tp = localtime(&t);

    sprintf(fileName, "%d%d%d_%d%d%d%s", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, ext);
    strcat(cmd, fileName);
    strcat(subCmd, fileName);
    strcat(cmd, subCmd);

    fprintf(stdout, "%s\n", cmd);
    
    while (system(NULL) == 0);
    
    system(cmd);
}

void RunRecordCmd()
{
    time_t t;
    struct tm *tp;
    
    char cmd[100] = "raspivid -w 1280 -h 720 -rot 180 -t 5000 -o ";
    char subCmd[50] = " && ./FUpload ";
    char ext[6] = ".h264";
    char fileName[50] = "";
    
    time(&t);
	
	tp = localtime(&t);

    sprintf(fileName, "%d%d%d_%d%d%d%s", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, ext);
    strcat(cmd, fileName);
    strcat(subCmd, fileName);
    strcat(cmd, subCmd);
    
    fprintf(stdout, "%s\n", cmd);
    
    while (system(NULL) == 0);
    
    system(cmd);
}

void RunCommand()
{
    char *cmd;
    
    fprintf(stdout, "Command Code : %d\n", data);

    switch (data)
    {
        case SERIAL_WAIT:
            serialPutchar(fd, SERIAL_WAIT);
            break;
        case CAMERA_SCREENSHOT:
            fprintf(stdout, "Run screenshot and upload\n");
            RunScreenShotCmd();
            break;
        case CAMERA_RECORD_START:
            fprintf(stdout, "Run record video and upload\n");
            RunRecordCmd();
            break;
        case DOOR_OPEN:
            isDoorOpen = true;
        
            fprintf(stdout, "Receive door open signal\n");
            break;
        case DOOR_CLOSE:
            isDoorOpen = false;
            
            fprintf(stdout, "Receive door close signal\n");
            break;
    }
}

int main()
{
    if (wiringPiSetup() == -1)
    {
        fprintf(stderr, "Unable to start wiringPi!\n");

        exit(1);
    }

    if ((fd = serialOpen("/dev/ttyUSB0", 9600)) < 0)
    {
        fprintf(stderr, "Cannot serial open!\n");

        exit(1);
    }
    
    mfrc.PCD_Init();
    
    if (piThreadCreate(RFIDScan) != 0)
    {
        fprintf(stderr, "Cannot start RFID scan thread!\n");
    }

    while (1)
    {
        if (serialDataAvail(fd) > 0)
        {
            data = serialGetchar(fd);

            if (data >= 0)
            {
                RunCommand();
            }
        }
    }
}
