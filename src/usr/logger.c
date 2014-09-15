#define USER_LOGGER_C

#include "../includes/includes.h"
#include "../includes/dataformat.h"
#include "../includes/usr_includes.h"

#define FREQ 150



OS_EVENT *Mbox_UART0Rx;
OS_EVENT *Mbox_UART0Tx;
OS_EVENT *Mbox_LEDMAN;
OS_EVENT *Mbox_BEEPMAN;
OS_EVENT *Mbox_LCDMAN;

OS_EVENT *Sem_UART0Tx;
OS_EVENT *Sem_UART1Tx;

PackageFormat PackageRx0;
PackageFormat PackageTx0;

OS_STK Stk_TaskStart[100];
OS_STK Stk_Task_UART0Rx[100];
OS_STK Stk_Task_UART0TX[100];
OS_STK Stk_Task_LEDMAN[100];
OS_STK Stk_Task_BEEPMAN[100];
OS_STK Stk_Task_LCDMAN[100];

INT32U iOPcode;

void TaskStart(void *pdata);
void Task_UART0Rx(void *pdata);
void Task_UART0TX(void *pdata);
void Task_LEDMAN(void *pdata);
void Task_BEEPMAN(void *pdata);
void Task_LCDMAN(void *pdata);

char vBuffer[32];


void  Main (void){
	INT16U freq;
	S3C_SysInit();

	OSInit();	
	
	LCDInit();
	LCDString(0x00,"     Bitman\n  Spartan 117");
	delay(10);
	
	for(freq = 10;freq < 300;freq += 10){
		S3C_SetPWMBuzzer(freq,freq/2);
		delay(1);
	}
	
	
	LCDClear();
	delay(8);
	LCDString(0,"   Bitman Lab\n   Log System.");
	S3C_SetPWMBuzzer(650,10);
	S3C_StartPWMBuzzer();
	delay(3);
	S3C_StopPWMBuzzer();
	delay(1);
	S3C_StartPWMBuzzer();
	delay(3);
	S3C_StopPWMBuzzer();
	delay(10);
	
	
	
	
	
	Sem_UART0Tx = OSSemCreate(1);
	Sem_UART1Tx = OSSemCreate(1);

	Mbox_UART0Rx = OSMboxCreate((void *)0);
	Mbox_UART0Tx = OSMboxCreate((void *)0);
	Mbox_LEDMAN  = OSMboxCreate((void *)0);
	Mbox_BEEPMAN = OSMboxCreate((void *)0);
	Mbox_LCDMAN  = OSMboxCreate((void *)0);
	
	
	iOPcode = TYPE_DEFAULT;


	OSTaskCreate(TaskStart,(void *)0,&Stk_TaskStart[99],10);
	OSStart();
}


void TaskStart(void *pdata){
	rSUBSRCPND = rSUBSRCPND;
	rSRCPND = rSRCPND;
	rINTPND = rINTPND;
	pdata = pdata;
	
	S3C_StartTimer4();
	
	SwiHandle(1);
	SwiHandle(3);	//Enable Interrupt
	
	rGPBDAT = (15<<5);

	OSStatInit();
	
	OSTaskCreate(Task_UART0TX,(void *)0,&Stk_Task_UART0TX[99],11);
	OSTaskCreate(Task_UART0Rx,(void *)0,&Stk_Task_UART0Rx[99],12);
	OSTaskCreate(Task_LEDMAN,(void *)0,&Stk_Task_LEDMAN[99],13);
	OSTaskCreate(Task_BEEPMAN,(void *)0,&Stk_Task_BEEPMAN[99],14);
	OSTaskCreate(Task_LCDMAN,(void *)0,&Stk_Task_LCDMAN[99],15);
	
	while(1){
		OSTaskSuspend(OSPrioCur);
	}

}

void itos_dec(INT32U num,char* dst,INT32U bite){
	INT32S i;
	INT32U j;
	j = 0xA;
	for(i = bite-1;i >= 0x0;--i){
		dst[i] = (num % j) + 0x30;
		num = num / 0xA;
	}
	dst[bite] = '\0';
}

void Task_UART0TX(void *pdata){
	INT8U err;
	INT32U Recv = 0;
	
	while(1){
		Recv = (INT32U)OSMboxPend(Mbox_UART0Tx,0,&err);
		PackageTx0.iSave = 0x0;
		PackageTx0.iType = iOPcode;
		PackageTx0.iData = Recv;
		PackageTx0.iMagicNumber = MAGIC;
		sendbin0((INT8U*)&PackageTx0,sizeof(PackageFormat));
		itos_dec(Recv,vBuffer,10);
		OSTimeDlyResume(15);
		OSMboxPost(Mbox_LCDMAN,(void*)vBuffer);
	}
}

//Original
//void Task_UART0TX(void *pdata){
//	INT8U err;
//	INT32U Recv = 0;
//	
//	while(1){
//		Recv = (INT32U)OSMboxPend(Mbox_UART0Tx,0,&err);
//		PackageTx0.iSave = 0x0;
//		PackageTx0.iType = iOPcode;
//		PackageTx0.iData = Recv;
//		PackageTx0.iMagicNumber = MAGIC;
//		sendbin0((INT8U*)&PackageTx0,sizeof(PackageFormat));
//	}
//}


void Task_UART0Rx(void *pdata){
	INT8U err;
	INT32U Recv = 0;
	Recv = Recv;
	
	while(1){
		Recv = (INT32U)OSMboxPend(Mbox_UART0Rx,0,&err);
		OSMboxPost(Mbox_UART0Tx,(void*)PackageRx0.iData);
		OSMboxPost(Mbox_BEEPMAN,(void*)PackageRx0.iType);
	}
}


void Task_LEDMAN(void *pdata){
	INT8U err;
	INT32U Recv = 0;
	Recv = Recv;
	while(1){
		Recv = (INT32U)OSMboxPend(Mbox_LEDMAN,0,&err);
		switch (iOPcode){
			case TYPE_KEY:
				rGPBDAT = (7<<5);
				break;
			case TYPE_QUERY:
				rGPBDAT = (11<<5);
				break;
			case TYPE_DEFAULT:
				rGPBDAT = (15<<5);
				break;
			case TYPE_INSERT:
				rGPBDAT	= (13<<5);
				break;
			default:
				break;
		}
	}
}


void Task_BEEPMAN(void *pdata){
	INT8U err;
	INT32U Recv = 0;
	
	while(1){
		Recv = (INT32U)OSMboxPend(Mbox_BEEPMAN,0,&err);
		//S3C_StartPWMBuzzer();
		switch (Recv){
			case TYPE_BE_BEEP:
				S3C_SetPWMBuzzer(34464,0);	
				S3C_StartPWMBuzzer();
				break;
			case TYPE_BE_STOP:
				S3C_StopPWMBuzzer();
				break;
			case TYPE_BE_SUCCESS_IN:
				S3C_SetPWMBuzzer(190,60);
				S3C_StartPWMBuzzer();
				delay(3);
				S3C_StopPWMBuzzer();
				delay(1);
				S3C_StartPWMBuzzer();
				delay(3);
				S3C_StopPWMBuzzer();
				break;
			case TYPE_BE_SUCCESS_OUT:
				S3C_SetPWMBuzzer(270,200);	
				S3C_StartPWMBuzzer();
				delay(15);
				S3C_StopPWMBuzzer();
				break;
			case TYPE_BE_IN:
				S3C_SetPWMBuzzer(520,200);	
				S3C_StartPWMBuzzer();
				delay(15);
				S3C_SetPWMBuzzer(190,60);
				S3C_StartPWMBuzzer();
				delay(3);
				S3C_StopPWMBuzzer();
				delay(1);
				S3C_StartPWMBuzzer();
				delay(3);
				S3C_StopPWMBuzzer();
				break;
			case TYPE_BE_NOT_IN:
				S3C_SetPWMBuzzer(520,200);	
				S3C_StartPWMBuzzer();
				delay(15);
				S3C_StopPWMBuzzer();
				break;
			case TYPE_BE_SPECIAL:
				break;
			default:
				S3C_SetPWMBuzzer(390,80);
				S3C_StartPWMBuzzer();
				delay(15);
				S3C_StopPWMBuzzer();
				break;
		}
		//S3C_StopPWMBuzzer();
	}	
}

void Task_LCDMAN(void *pdata){
	char* src;
	INT8U err;
	while(1){
		src = (char*)OSMboxPend(Mbox_LCDMAN,0,&err);
		LCDClear();
		//LCDString(0,"ID:");
		LCDString(0,src);
		OSTimeDlyHMSM(0,0,3,0);
		LCDClear();
		LCDString(0,"   Bitman Lab\n   Log System.");
	}
}

