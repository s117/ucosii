#define SYSTEM_CODE_C

#include "../includes/OS_CPU.H"
#include "../includes/OS_CFG.H"
#include "../includes/ucos_ii.h"
#include "../includes/usr_includes.h"

extern	OS_EVENT		*Sem_UART0Tx;
extern	OS_EVENT		*Sem_UART1Tx;

void HandleIRQ(int IntOffset){
	void (*p)(void);
	p = (void(*)(void))VICVectAddr[rINTOFFSET];
	p();
	OSIntExit();
}

__inline void delay(int msec){
	int i, j;
	for(i = 1000; i > 0; i--)
		for(j = msec*10; j > 0; j--);
	return;
}



//----UART0-COMPUTER-----------------------------------------------------------------------------------------------------------
int getstream0(INT8U* serial_in){
	int i = 0;
	serial_in[0] = '\0';
	while(!RxF0COUNT);
#ifndef debug
	delay(10);
#endif
	while(RxF0COUNT)
		serial_in[i++] = rURXH0;
	serial_in[i] = '\0';
	return i;
}

void transbin0(INT8U* dest,int size){
	int i = 0;
	for (i = 0;i < size;++i){
		while(TxF0FULL);
		rUTXH0 = dest[i];
	}
	return;
}

INT8U sendbin0(INT8U* dest,int size){
	INT8U err;
	OSSemPend(Sem_UART0Tx,0,&err);
	transbin0(dest,size);
	OSSemPost(Sem_UART0Tx);
	return err;
}

void recbin0(INT8U* dest,int size){
	int i = 0;
	while(!RxF0COUNT);
#ifndef debug
	delay(10);
#endif
	for (i = 0;i < size;++i){
		while(!RxF0COUNT);
		dest[i] = rURXH0;
	}
	return;
}

void flush0(void){
	unsigned int drop;
	drop = drop;	//Prevent compiler's warning
	while(RxF0COUNT)
		drop = rURXH0;
	return;
}


//----UART1-RFID---------------------------------------------------------------------------------------------------------------
int getstream1(INT8U* serial_in){
	int i = 0;
	serial_in[0] = '\0';
	while(!RxF1COUNT);
#ifndef debug
	delay(10);
#endif
	while(RxF1COUNT)
		serial_in[i++] = rURXH1;
	serial_in[i] = '\0';
	return i;
}

void transbin1(INT8U* dest,int size){
	int i = 0;
	for (i = 0;i < size;++i){
		while(TxF1FULL);
		rUTXH1 = dest[i];
	}
	return;
}

INT8U sendbin1(INT8U* dest,int size){
	INT8U err;
	OSSemPend(Sem_UART1Tx,0,&err);
	transbin1(dest,size);
	OSSemPost(Sem_UART1Tx);
	return err;
}

void recbin1(INT8U* dest,int size){
	int i = 0;
	while(!RxF1COUNT);
#ifndef debug
	delay(10);
#endif
	for (i = 0;i < size;++i){
		while(!RxF1COUNT);
		dest[i] = rURXH1;
	}
	return;
}

void flush1(void){
	unsigned int drop;
	drop = drop;	//Prevent compiler's warning
	while(RxF1COUNT)
		drop = rURXH1;
	return;
}
