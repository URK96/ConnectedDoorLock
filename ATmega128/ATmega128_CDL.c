/*
*************************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*************************************************************************************************************
*/

#include "includes.h"
#include "SerialCode.h"
#include <util/delay.h>

/*
**************************************************************************************************************
*                                               CONSTANTS
**************************************************************************************************************
*/

#define TASK_STK_SIZE OS_TASK_DEF_STK_SIZE   /* Size of each task's stacks (# of bytes)                  */
#define N_TASKS 12                  /* Number of identical tasks                                */

#define MYUBRR 9600
#define INT8 char
#define PW_MAX 10

/*************************************************************************************************************
*                                               VARIABLES
**************************************************************************************************************
*/

OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];   /* Tasks stacks                                           */
OS_STK TaskStartStk[TASK_STK_SIZE];

OS_FLAG_GRP *osFlag;

INT8U pw[10] = { 0 };
INT8U pwCount = 0;

/*
**************************************************************************************************************
*                                           FUNCTION PROTOTYPES
**************************************************************************************************************
*/

void DataTask(void *data);

void DoorTask(void *data);
void DSMTask(void *data);

void BuzzerOpen();
void BuzzerClose();
void RequestPW();
void ReceivePW();
void SendPW(INT8U code);

void PortInit();
void TimerInit();
void USARTInit(INT16U ubrr);
void USART0Transmit(INT8S data);
INT8 USART0Receive();
//void USARTTransmitString(INT8 *str);

void LCDInit();
void LCDClear();
void LCDString(INT8U x, INT8U y, INT8 *str);
void LCDChar(INT8U x, INT8U y, INT8 data);
/*
**************************************************************************************************************
*                                                MAIN
**************************************************************************************************************
*/

int main()
{
    INT8U err;

    OSInit();

    osFlag = OSFlagCreate(0, &err);

    OSTaskCreate(DataTask,(void *)0, (void *)&TaskStartStk[TASK_STK_SIZE - 1], 0);

    OSStart();                                          /* Start multitasking                               */

    return 0;
}

/*
**************************************************************************************************************
*                                              STARTUP TASK
**************************************************************************************************************
*/

void DataTask(void *data)
{
    int code;

    data = data;                                        /* Prevent compiler warning                           */

    OS_ENTER_CRITICAL();
    TimerInit();
    OS_EXIT_CRITICAL();

    PortInit();
    USARTInit(MYUBRR);
    OSStatInit();                                       /* Initialize uC/OS-II's statistics                 */

    OSTaskCreate(DoorTask, 0, (void*)&TaskStk[0][TASK_STK_SIZE - 1], 2);
    OSTaskCreate(DSMTask, 0, (void*)&TaskStk[1][TASK_STK_SIZE - 1], 4)

    for(;;)
    {
        code = USART0Receive();

        switch (code)
        {
            case FP_PASS:
                SetDoorOpen();
                break;
        }
    }
}

/*
**************************************************************************************************************
*                                                  TASKS
**************************************************************************************************************
*/

void DoorTask(void *data)
{
    INT8U err;

    while (1)
    {
        OSFlagPend(osFlag, 0x01, OS_FLAG_WAIT_SET_ALL, 0, &err);

        USART0Transmit(DOOR_OPEN);
        SetServoAngle(90);
        BuzzerOpen();
        OSTimeDlyHMSM(0, 0, 0, 500);

        OSFlagPend(osFlag, 0x02, OS_FLAG_WAIT_SET_ALL, 0, &err);

        USART0Transmit(DOOR_CLOSE);
        OSFlagPost(osFlag, 0x01, OS_FLAG_CLR, &err);
        SetServoAngle(0);
        BuzzerClose();

        OSFlagPost(osFlag, 0x02, OS_FLAG_CLR, &err);

        OSTimeDlyHMSM(0, 0, 0, 500);
    }
}

void DSMTask(void *data)
{
    while (1)
    {
        if ((PINF & 0x01) == 0x01)
        {
            OSFlagPost(osFlag, 0x02, OS_FLAG_SET, &err);
        }
        else
        {
            OSFlagPost(osFlag, 0x02, OS_FLAG_CLR, &err);
        }
        
        if ((PINF & 0x02) == 0x02)
        {
            
        }
        else
        {
            /* code */
        }

        OSTimeDlyHMSM(0, 0, 0, 300);
    }
}

/*
**************************************************************************************************************
*                                                  SUPPORT ROUTINES
**************************************************************************************************************
*/

void SetServoAngle(INT8 angle)
{
    unsigned int i = 375 - 200 * angle / 90;

    OCR1AH = (i >> 8);
    OCR1AL = (i & 0xFF;
}

void SetDoorOpen()
{
    INT8U err;

    OSFlagPost(osFlag, 0x01, OS_FLAG_SET, &err);
}

void BuzzerOpen()
{
    int k;

	for (k = 0; k < 120; ++k)
	{
		PORTD = 0x01;
		_delay_ms(1.1);
		PORTD = 0x00;
		_delay_ms(1.1);
	}

	for (k = 0; k < 120; ++k)
	{
		PORTD = 0x01;
		_delay_ms(0.9);
		PORTD = 0x00;
		_delay_ms(0.9);
	}

	for (k = 0; k < 120; ++k)
	{
		PORTD = 0x01;
		_delay_ms(0.7);
		PORTD = 0x00;
		_delay_ms(0.7);
	}

	_delay_ms(500);
}

void BuzzerClose()
{
	int k;
	
	for (k = 0; k < 120; ++k)
	{
		PORTD = 0x01;
		_delay_ms(0.7);
		PORTD = 0x00;
		_delay_ms(0.7);
	}
	
	for (k = 0; k < 120; ++k)
	{
		PORTD = 0x01;
		_delay_ms(0.9);
		PORTD = 0x00;
		_delay_ms(0.9);
	}
	
	for (k = 0; k < 120; ++k)
	{
		PORTD = 0x01;
		_delay_ms(1.1);
		PORTD = 0x00;
		_delay_ms(1.1);
	}

	_delay_ms(500);
}

/*
**************************************************************************************************************
*                                                  SUPPORT ROUTINES
**************************************************************************************************************
*/

void PortInit()
{
    DDRC = 0X00;
    DDRB = 0xFF; // Door Servo Motor (PB5)
    DDRD = 0xFF; 
    DDRF = 0x00; // PF0 => Door DS, PF1 => Camera DS
}

void TimerInit()
{
    TCCR0 = (1 << CS02) | (1 << CS01) | (1 << CS00);
    TCCR1A = 0x82;
    TCCR1B = 0x1B;
    TCCR1C = 0x00;
    ICR1H = (2499 >> 8);
    ICR1L = 2499 & 0xFF;
    OCR1AH = (300 >> 8);
    OCR1AL = (300 & 0xFF;
    TIMSK = (1 << TOIE0);
    TCNT0 = 256 - CPU_CLOCK_HZ/OS_TICKS_PER_SEC/1024;
}

void USARTInit(INT16U ubrr) 
{
    UBRR0H = 0x00;   //(INT8U)(ubrr >> 8);
    UBRR0L = 0x67;   //(INT8U)ubrr;
    UCSR0A = 0x00;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART0Transmit(INT8 data)
{
    OS_ENTER_CRITICAL();

    while(!(UCSR0A & (1 << UDRE0)));

    UDR0 = data;

    OS_EXIT_CRITICAL();
}

INT8 USART0Receive()
{
    while(!(UCSR0A & (1 << RXC0)));
    {
        OSTimeDlyHMSM(0, 0, 0, 1);
    }

    return UDR0;
}

/*void USARTTransmitString(INT8 *str)
{
    while (*str != '\0')
    {
        USART0Transmit(*str++);
    }
}*/

void LCDInit()
{
    INT8U err;

    USARTTransmitString("$I\r");
}

void LCDClear()
{
    INT8U err;

    USARTTransmitString("$C\r");
}

void LCDString(INT8U x, INT8U y, INT8 *str)
{
    INT8U err;

    USART0Transmit('$');
    USART0Transmit('G');
    USART0Transmit(',');
    USART0Transmit(x + '0');
    USART0Transmit(',');

    if(y < 10)
    {
        USART0Transmit(y + '0');
    }
    else
    {
        USART0Transmit(y / 10 + '0');
        USART0Transmit(y % 10 + '0');
    }

    USART0Transmit('\r');
    USART0Transmit('$');
    USART0Transmit('T');
    USART0Transmit(',');
    USARTTransmitString(str);
    USART0Transmit('\r');
}

void LCDChar(INT8U x, INT8U y, INT8 data)
{
    INT8U err;

    USART0Transmit('$');
    USART0Transmit('G');
    USART0Transmit(',');
    USART0Transmit(x + '0');
    USART0Transmit(',');

    if(y < 10)
    {
        USART0Transmit(y + '0');
    }
    else
    {
        USART0Transmit(y / 10 + '0');
        USART0Transmit(y % 10 + '0');
    }

    USART0Transmit('\r');
    USART0Transmit('$');
    USART0Transmit('T');
    USART0Transmit(',');
    USART0Transmit(data);
    USART0Transmit('\r');
}