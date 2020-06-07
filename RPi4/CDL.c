#define PW_MAX 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WiringPi.h>
#include <WiringPiSerial.h>

#include "SerialCode.h"
#include "XORCrypt.h"

int fd;
char data;

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
            SavePW();
            break;
        case CHECK_PW:
            CheckPW();
            break;
        case CAMERA_SCREENSHOT:
            break;
        case CAMERA_RECORD_START:
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

            RunCommand();
        }
    }
}