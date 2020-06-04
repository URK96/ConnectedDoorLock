#include <stdio.h>
#include <WiringPi.h>
#include <WiringPiSerial.h>

int fd;
char data;

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
        }
    }
}