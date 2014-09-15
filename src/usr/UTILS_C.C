//
//								-----------------------------------------
//								| This file is:							|
//								|			2440 Library File			|
//								|							Spartan 117	|
//								-----------------------------------------
//
//-----------------------------------------------------------------------------------------------------------------------------
#define UTILS_C

#include "../includes/OS_CPU.H"
#include "../includes/OS_CFG.H"
#include "../includes/ucos_ii.h"
#include "../includes/usr_includes.h"
#include "../includes/usr_includes.h"
#include "../includes/dataformat.h"

__inline void LCDSet_RS(INT8U RS);
__inline void LCDSet_RW(INT8U RW);
__inline void LCDSet_E(INT8U E);
__inline void LCDSet_RS_RW_E(INT8U RS,INT8U RW,INT8U E);
__inline void LCDSet_DATA(INT8U DATA);
__inline void LCDSet_Param(INT8U RS,INT8U RW,INT8U E,INT8U DATA);
void LCD1602_delay(void);
void msgbox(INT8U MODE,INT8U DATA);
//-----------------------------------------------------------------------------------------------------------------------------
extern	OS_EVENT		*Mbox_UART0Rx;
extern	OS_EVENT		*Mbox_UART0Tx;
extern	OS_EVENT		*Mbox_LEDMAN;
extern	OS_EVENT		*Mbox_BEEPMAN;
extern	OS_EVENT		*Mbox_LCDMAN;

extern	OS_EVENT		*Sem_UART0Tx;
extern	OS_EVENT		*Sem_UART1Tx;

extern	PackageFormat	PackageRx0;
extern	PackageFormat	PackageTx0;

extern	INT32U			iOPcode;

//-----------------------------------------------------------------------------------------------------------------------------



void IRQ_BUTTON(void){
	INT32U EINTPEND;
	delay(5);
	EINTPEND = rEINTPEND;
	rEINTPEND = rEINTPEND;
	rSRCPND = rINTPND;
	rINTPND = rINTPND;
	
	if (EINTPEND & (B8)) {
		iOPcode = TYPE_KEY;
		OSMboxPost(Mbox_LEDMAN,(void*)TYPE_KEY);
		OSTimeDlyResume(15);
		OSMboxPost(Mbox_LCDMAN,(void*)"  Register Key");
	} 
	else if (EINTPEND & (B11)) {
		iOPcode = TYPE_QUERY;
		OSMboxPost(Mbox_LEDMAN,(void*)TYPE_QUERY);
		OSTimeDlyResume(15);
		OSMboxPost(Mbox_LCDMAN,(void*)"  Query Status");
	}
	else if (EINTPEND & (B13)) {
		iOPcode = TYPE_DEFAULT;
		OSMboxPost(Mbox_LEDMAN,(void*)TYPE_DEFAULT);
		OSMboxPost(Mbox_BEEPMAN,(void*)TYPE_BE_STOP);
		OSTimeDlyResume(15);
	}
	else if (EINTPEND & (B15)) {
		iOPcode = TYPE_INSERT;
		OSMboxPost(Mbox_LEDMAN,(void*)TYPE_INSERT);
		OSTimeDlyResume(15);
		OSMboxPost(Mbox_LCDMAN,(void*)"   New Record");
	}
	else if (EINTPEND & (B14)) {
		OSMboxPost(Mbox_BEEPMAN,(void*)TYPE_BE_FAIL);
		OSTimeDlyResume(15);
		OSMboxPost(Mbox_LCDMAN,(void*)"Invalid  Operate");
	}
	else if (EINTPEND & (B19)) {
		OSMboxPost(Mbox_BEEPMAN,(void*)TYPE_BE_FAIL);
		OSTimeDlyResume(15);
		OSMboxPost(Mbox_LCDMAN,(void*)"Invalid  Operate");
	}
	else{
	}
	return;
}

void IRQ_UART1(void){
	INT32U drop,id,cont;
	drop = drop;

	if(rSUBSRCPND&(B5)){//Exception
		delay(10);
		rSUBSRCPND = (B3)|(B4)|(B5);
		rSRCPND = rINTPND;
		rINTPND = rINTPND;
		
		drop = rUERSTAT1;
		flush1();
	}
	else if(rSUBSRCPND&(B3)){//RXD1	
		rSUBSRCPND = (B3);
		rSRCPND = rINTPND;
		rINTPND = rINTPND;
		
		while(!RxF1COUNT);
		delay(10);
		drop = rURXH1;
		id = 0;
		for(cont = 1000000000;cont > 0;cont = cont/10)
			id += ((rURXH1-0x30)*cont);
		flush1();
		OSMboxPost(Mbox_UART0Tx,(void*)id);	
	}
}

void IRQ_UART0(void){
	INT32U drop;
	drop = drop;

	if(rSUBSRCPND&(B2)){//Exception
		delay(10);
		rSUBSRCPND = (B0)|(B1)|(B2);
		rSRCPND = rINTPND;
		rINTPND = rINTPND;		

		drop = rUERSTAT0;
		flush0();

	}
	else if(rSUBSRCPND&(B0)){//RXD0
		rSUBSRCPND = (B0);
		rSRCPND = rINTPND;
		rINTPND = rINTPND;
		
		while(!RxF0COUNT);
		delay(10);
		recbin0((INT8U*)&PackageRx0,16);
		flush0();
		OSMboxPost(Mbox_UART0Rx,(void*)&PackageRx0);
	}
}

void IRQ_UNDEF(void){
	rSUBSRCPND = rSUBSRCPND;
	rSRCPND = rSRCPND;
	rINTPND = rINTPND;
	transbin0("This IRQ not support yet\n",20);
	return;
}

void init_led(void){
    rGPBCON = ((B10) | (B12) | (B14) | (B16) | (B1));
    rGPBDAT |= ((B5) | (B6) | (B7) | (B8));
    rGPBUP |= ((B5) | (B6) | (B7) | (B8));
	return;
}

void init_lcd(void){
	rGPJCON = ((B0) | (B2) | (B4) | (B6) | (B8) | (B10) | (B12) | (B14) | (B16) | (B18) | (B20));
	rGPJUP = 0x1FFF;
	//rGPJUP = 8191;
	rGPJDAT = 0;
}

void S3C_InitTimer_PWM(void){
	rTCFG0  = ((49<<8)|(49<<0));		//(预分频值+1 = 50)
	rTCFG1  = 0;		  				//(所有定时器都设定为中断模式、频值=2)
	rTCNTB4 = 5000;	 	 				//T4-100Hz
	rTCNTB0 = 250;						//T0_PWM-2KHz
	rTCMPB0 = 0;						//
	rTCON  = (B22|B21|B20|B3|B2|B1|B0);	//(定时器1&4自动重新装载，启动位与手动更新位已设，TOUT0电平反转，清除手动更新位后启动)
}

void S3C_StartTimer4(void){
	rTCON	&= ~B21;
}

void S3C_StartPWMBuzzer(void){
	rTCON	&= ~B1;
	rTCON	|= B3;
}

void S3C_StopPWMBuzzer(void){
	rTCON	&= ~B3;
}

void S3C_SetPWMBuzzer(INT16U TCNTB,INT16U TCMPB){
	rTCON	&= ~(B1|B3);
	rTCMPB0 = TCMPB;
	if (TCNTB!=0)
		rTCNTB0 = TCNTB;
	rTCON	|= B1;
	rTCON	&= ~B1;
	rTCON	|= B3;
}

void S3C_SysInit(void){
	int i;
	SwiHandle(4);	//SetInterruptREG
	S3C_InitTimer_PWM();
	init_led ();
	init_lcd();
	for (i = 0;i <32;++i){
		VICVectAddr[i]  = (INT32U)IRQ_UNDEF;		
	}
	VICVectAddr[5]  = (INT32U)IRQ_BUTTON;
	VICVectAddr[23] = (INT32U)IRQ_UART1;
	VICVectAddr[28] = (INT32U)IRQ_UART0;
}
//1602液晶驱动
__inline void LCDSet_RS(INT8U RS){
	if(RS)
		rGPJDAT |= B8;
	else
		rGPJDAT &= ~B8;
}

__inline void LCDSet_RW(INT8U RW){
	if(RW)
		rGPJDAT |= B9;
	else
		rGPJDAT &= ~B9;
}

__inline void LCDSet_E(INT8U E){
	if(E)
		rGPJDAT |= B10;
	else
		rGPJDAT &= ~B10;
}

__inline void LCDSet_RS_RW_E(INT8U RS,INT8U RW,INT8U E){
	INT32U GPJDAT = rGPJDAT;
	INT32U VAL = 0;
	if(RS)
		VAL |= B8;
	if(RW)
		VAL |= B9;
	if(E)
		VAL |= B10;
	GPJDAT &= ~1792;
	GPJDAT |= VAL;
	rGPJDAT = GPJDAT;
}

__inline void LCDSet_DATA(INT8U DATA){
	INT32U GPJDAT = rGPJDAT;
	GPJDAT &= ~255;
	GPJDAT |= DATA;
	rGPJDAT = GPJDAT;
}

__inline void LCDSet_Param(INT8U RS,INT8U RW,INT8U E,INT8U DATA){
	INT32U GPJDAT = rGPJDAT;
	INT32U VAL = DATA;
	if(RS)
		VAL |= B8;
	if(RW)
		VAL |= B9;
	if(E)
		VAL |= B10;
	GPJDAT &= ~2047;
	GPJDAT |= VAL;
	rGPJDAT = GPJDAT;
}

__inline INT8U LCD_IsBusy(){
	INT8U rtn;
	rGPJCON &= ~B14;		//Set GPJ7 to Input mode
	rGPJDAT &= ~B8;		//RS=0
	rGPJDAT |= B9;		//RW=1
	rtn = (rGPJDAT & B7) >> 7;//Read D7
	rGPJCON |= B14;			//Set GPJ7 to Output mode
	return rtn;
}

void LCD1602_delay(){
	int i, j;
	for(i = 1; i > 0; i--)
		for(j = 300; j > 0; j--);
	return;
}

void msgbox(INT8U MODE,INT8U DATA){//0:Instruction,1:Data
	while(LCD_IsBusy());
	if(MODE == 0){
		LCDSet_RS(0);
		LCDSet_RW(0);
		LCDSet_DATA(DATA);
	}else{
		LCDSet_RS(1);
		LCDSet_RW(0);
		LCDSet_DATA(DATA);
	}
	LCDSet_E(0);
	LCD1602_delay();
	LCDSet_E(1);
}

void LCDInit(){
	int i;
	for(i = 0;i < 3;++i){
		LCD1602_delay();
		LCDSet_RS(0);
		LCDSet_RW(0);
		LCDSet_DATA(0x38);
		LCDSet_E(0);
		LCD1602_delay();
		LCDSet_E(1);
	}
	
	msgbox(0,0x08);
	msgbox(0,0x01);
	msgbox(0,0x06);
	msgbox(0,0x0C);
}

void LCDChar(INT8U ADD,char CH){
	msgbox(0,ADD);
	msgbox(1,CH);
}

void LCDClear(){
	msgbox(0,1);
}

void LCDString(INT8U Address, char *Str) {
	INT8U i;
	INT8S cur = 0;
	if (Address > 0x50)
		return;

	msgbox(0, Address | B7);
	
	for(i = 0;((*(Str+cur) != '\n') && (*(Str+cur) != '\0') && (Address + i <= 0x10));++i){
		msgbox(1,*(Str + (cur++)));
	}
	
	if(*(Str+i) == '\0'){
		return;
	} else{
		if(*(Str+cur) == '\n')
			++cur;
		Address = 0X40;
		msgbox(0,Address | B7);
	}
	
	for(i = 0;((*(Str+cur) != '\n') && (*(Str+cur) != '\0') && (Address + i <= 0x50));++i){
		msgbox(1,*(Str + (cur++)));
	}
}

