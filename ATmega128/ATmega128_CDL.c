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

int sendCode = -1;

/*
**************************************************************************************************************
*                                           FUNCTION PROTOTYPES
**************************************************************************************************************
*/

void StartTask(void *data); // Init task

void CommandTask(void *data); // Command receive task
void DoorTask(void *data); // Door control task
void DDSTask(void *data); // Door distance sensor monitoring task
void CDSTask(void *data); // Camera distance sensor monitoring task

void SetServoAngle(INT8 angle);

void PortInit();
void TimerInit();
void USARTInit(INT16U ubrr);
void USART0Transmit(INT8 data);
INT8 USART0Receive();
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

    OSTaskCreate(StartTask, (void *)0, (void *)&TaskStartStk[TASK_STK_SIZE - 1], 0);

    OSStart();                                          /* Start multitasking                               */

    return 0;
}

/*
**************************************************************************************************************
*                                              STARTUP TASK
**************************************************************************************************************
*/

void StartTask(void *data)
{
    INT8U err;

    int code;

    data = data;                                        /* Prevent compiler warning                           */

    OS_ENTER_CRITICAL();
    TimerInit();
    OS_EXIT_CRITICAL();

    PortInit();
    USARTInit(MYUBRR);
    OSStatInit();                                       /* Initialize uC/OS-II's statistics                 */

    OSTaskCreate(CommandTask, 0, (void*)&TaskStk[0][TASK_STK_SIZE - 1], 2);
    OSTaskCreate(DoorTask, 0, (void*)&TaskStk[1][TASK_STK_SIZE - 1], 3);
    OSTaskCreate(DDSTask, 0, (void*)&TaskStk[3][TASK_STK_SIZE - 1], 5);
    OSTaskCreate(CDSTask, 0, (void*)&TaskStk[2][TASK_STK_SIZE - 1], 4);

    PORTA = 0x01;

    OSTaskSuspend(2);
    OSTaskSuspend(3);
    OSTaskSuspend(4);
    OSTaskSuspend(5);

    while (1)
    {
        code = USART0Receive();
        
        if (code == SERIAL_WAIT)
        {
            break;
        }

        OSTimeDlyHMSM(0, 0, 0, 200);
    }

    USART0Transmit(SERIAL_WAIT);

    SetServoAngle(120);

    OSTaskResume(2);
    OSTaskResume(3);
    OSTaskResume(4);

    for(;;)
    {
        OSTimeDlyHMSM(0, 0, 2, 0);
    }
}

/*
**************************************************************************************************************
*                                                  TASKS
**************************************************************************************************************
*/

void CommandTask(void *data)
{
    INT8U err;

    int code;

    while (1)
    {
        PORTA |= 0x02;

        code = USART0Receive();

        switch (code)
        {
            case PW_CORRECT:
            case BT_PASS:
            case FP_PASS:
            case RFID_PASS:
                OSFlagPost(osFlag, 0x01, OS_FLAG_SET, &err);
                break;
        }

        OSTimeDlyHMSM(0, 0, 0, 500);
    }
}

void DoorTask(void *data)
{
    INT8U err;

    while (1)
    {
        OSFlagPend(osFlag, 0x01, OS_FLAG_WAIT_SET_ALL, 0, &err);

        OSTaskSuspend(2);       

        PORTA = 0x03;

        USART0Transmit(DOOR_OPEN);
        SetServoAngle(10);
        OSTimeDlyHMSM(0, 0, 0, 500);
        OSTaskResume(5);

        while (1)
        {
            PORTA = 0x03;

            OSFlagPend(osFlag, 0x02, OS_FLAG_WAIT_SET_ALL, 0, &err);
            USART0Transmit(DOOR_CHECKCLOSE);

            PORTA = 0x04;

            OSTimeDlyHMSM(0, 0, 1, 0);

            if ((OSFlagQuery(osFlag, &err) & 0x02) == 0x02)
            {
                break;
            }
        }

        USART0Transmit(DOOR_CLOSE);
        OSFlagPost(osFlag, 0x01, OS_FLAG_CLR, &err);
        SetServoAngle(120);

        OSTimeDlyHMSM(0, 0, 0, 500);

        OSTaskResume(2);
        OSTaskSuspend(5);

        OSFlagPost(osFlag, 0x02, OS_FLAG_CLR, &err);
    }
}

void DDSTask(void *data)
{
    INT8U err;

    while (1)
    {
        if ((PINF & 0x01) == 0x01)
        {
            OSFlagPost(osFlag, 0x02, OS_FLAG_CLR, &err);
        }
        else
        {
            OSFlagPost(osFlag, 0x02, OS_FLAG_SET, &err);
        }

        OSTimeDlyHMSM(0, 0, 0, 200);
    }
}

void CDSTask(void *data)
{
    INT8 count;

    while (1)
    {
        if ((PINF & 0x02) != 0x02)
        {
            PORTA |= 0x80;

            for (count = 0; count <= 2; ++count)
            {
                if ((PINF & 0x02) == 0x02)
                {
                    goto EXITLOOP;
                }

                OSTimeDlyHMSM(0, 0, 1, 0);
            }

            PORTA |= 0x40;

            OSTaskSuspend(2);

            USART0Transmit(CAMERA_RECORD_START);
            //sendCode = CAMERA_RECORD_START;

            OSTimeDlyHMSM(0, 0, 0, 500);

            OSTaskResume(2);

            PORTA &= ~0x40;
        }
        else
        {
            PORTA &= ~0x80;
        }
        

EXITLOOP:
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
    OCR1AL = (i & 0xFF);
}

/*void BuzzerOpen()
{
    int k;

    BuzzerLoop(1.1);
    BuzzerLoop(0.9);
    BuzzerLoop(0.7);

	_delay_ms(500);
}

void BuzzerCheckClose()
{
    int k;

    BuzzerLoop(0.7);
    BuzzerLoop(0.7);

	_delay_ms(500);
}

void BuzzerClose()
{
	int k;

    BuzzerLoop(0.7);
    BuzzerLoop(0.9);
    BuzzerLoop(1.1);

	_delay_ms(500);
}

void BuzzerLoop(float delay)
{
    INT8U k;

    for (k = 0; k < 120; ++k)
	{
		PORTD = 0x01;

		_delay_ms(delay);
		
        PORTD = 0x00;
		
        _delay_ms(delay);
	}
}*/

/*
**************************************************************************************************************
*                                                  SUPPORT ROUTINES
**************************************************************************************************************
*/

void PortInit()
{
    DDRA = 0xFF;
    DDRC = 0X00;
    DDRB = 0xFF; // PB5 => Door Servo Motor 
    DDRF = 0x00; // PF0 => Door DS, PF1 => Camera DS
    DDRG = 0x03; // PG0 => CDS Event, PG1 => Door Status 
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
    OCR1AL = (300 & 0xFF);
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
    if (!(UCSR0A & (1 << RXC0)))
    {
        return -1;
    }

    return UDR0;
}