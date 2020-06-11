#define PW_MAX 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <WiringPi.h>
#include <WiringPiSerial.h>

#include "SerialCode.h"
#include "XORCrypt.h"

int fd;
char data;

char* CreateScreenShotCmd()
{
    time_t t;
    struct tm *pt;
    
    char cmd[100] = "raspistill -w 1920 -h 1080 -o ";
    char subCmd[50] = " && ./FUpload ";
    char ext[5] = ".jpg";
    char fileName[50] = "";

    sprintf(fileName, "%d%d%d_%d%d%d%s", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, ext);
    strcat(cmd, fileName);
    strcat(subCmd, fileName);
    strcat(cmd, subCmd);

    return cmd;
}

char* CreateRecordCmd()
{
    time_t t;
    struct tm *pt;
    
    char cmd[100] = "raspivid -w 1280 -h 720 -t 5000 -o ";
    char subCmd[50] = " && ./FUpload ";
    char ext[5] = ".jpg";
    char fileName[50] = "";

    sprintf(fileName, "%d%d%d_%d%d%d%s", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, ext);
    strcat(cmd, fileName);
    strcat(subCmd, fileName);
    strcat(cmd, subCmd);

    return cmd;
}

void SavePW()
{
    char tdata, count = 0, i;
    char inputPW[PW_MAX];
    FILE *fp;

    while ((tdata = serialGetchar(fd)) != SERIAL_SENDEND)
    {
        inputPW[count++] = tdata;
    }

    XOREncrypt(inputPW, count);

    if (access("cdl_pw", 0) == 0)
    {
        remove("cdl_pw");
    }

    fp = fopen("cdl_pw", "w+");

    for (i = 0; i < count; ++i)
    {
        fprintf(fprintf, "%d\n", inputPW[i]);
    }

    fclose(fp);
}

bool CheckPW()
{
    char tdata, count = 0, i = 0;
    int fdata;
    char inputPW[PW_MAX], *decryptPW;
    bool result = true;
    FILE *fp;

    while ((tdata = serialGetchar(fd)) != SERIAL_SENDEND)
    {
        inputPW[count++] = tdata;
    }

    decryptPW = malloc(sizeof(uint8_t) * count);

    if (access("cdl_pw", 0) == 0)
    {
        fp = fopen("cdl_pw", "r");

        while (fscanf(fp, "%d", &fdata) != EOF)
        {
            decryptPW[i++] = fdata;
        }

        XORDecrypt(decryptPW, count);
    }
    else
    {
        return false;
    }

    for (i = 0; i < count; ++i)
    {
        if (inputPW[i] != decryptPW[i])
        {
            result = false;
        }
    }

    fclose(fp);
    free(decryptPW);

    return result;
}

void RunCommand()
{
    fprintf(stdout, "Command Code : %d\n", data);

    switch (data)
    {
        case REQ_PW:
            if (access("cdl_pw", 0) == 0)
            {
                serialPutchar(fd, EXIST_PW);
            }
            else
            {
                serialPutchar(fd, NO_PW);
            }
            break;
        case SAVE_PW:
            fprintf(stdout, "Run save password\n");
            SavePW();
            break;
        case CHECK_PW:
            fprintf(stdout, "Run check password\n");
            CheckPW();
            break;
        case CAMERA_SCREENSHOT:
            fprintf(stdout, "Run screenshot and upload\n");

            while (system(NULL) == 0);

            system(CreateScreenShotCmd());
            break;
        case CAMERA_RECORD_START:
            fprintf(stdout, "Run record video and upload\n");
            
            while (system(NULL) == 0);

            system(CreateRecordCmd());
            break;
        case CAMERA_RECORD_STOP:
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

    while (1)
    {
        if (serialDataAvail(fd))
        {
            data = serialGetchar(fd);

            if (data >= 0)
            {
                RunCommand();
            }
        }
    }
}