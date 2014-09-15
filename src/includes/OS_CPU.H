/*
*********************************************************************************************************
*                                               uC/OS-II
*                                        The Real-Time Kernel
*
*                         (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                          All Rights Reserved
*
*                                       80x86/80x88 Specific code
*                                          LARGE MEMORY MODEL
*
*                                          Borland C/C++ V4.51
*
* File         : OS_CPU.H
* By           : Jean J. Labrosse
*********************************************************************************************************
*/

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

__swi(0x00) void SwiHandle(int Handle);		//0:IRQoff,1:IRQon,2:FIQoff,3:FIQon,4:SetInterruptREG
extern void OSTickISR(void);

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */
typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSW = 32 bits) */

#define BYTE           INT8S                     /* Define data types for backward compatibility ...   */
#define UBYTE          INT8U                     /* ... to uC/OS V1.xx.  Not actually needed for ...   */
#define WORD           INT16S                    /* ... uC/OS-II.                                      */
#define UWORD          INT16U
#define LONG           INT32S
#define ULONG          INT32U

/* 
*********************************************************************************************************
*                              Intel 80x86 (Real-Mode, Large Model)
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In other words, if 
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section.
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to 
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*********************************************************************************************************
*/

#define  OS_CRITICAL_METHOD    1

#if      OS_CRITICAL_METHOD == 1
#define  OS_ENTER_CRITICAL()  {SwiHandle(0);SwiHandle(2);}                    /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()   {SwiHandle(1);SwiHandle(3);}                    /* Enable  interrupts                        */
#endif

#if      OS_CRITICAL_METHOD == 2
#define  OS_ENTER_CRITICAL()  asm {PUSHF; CLI}            /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()   asm  POPF                   /* Enable  interrupts                        */
#endif

#if      OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()  (cpu_sr = OSCPUSaveSR())    /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()   (OSCPURestoreSR(cpu_sr))    /* Enable  interrupts                        */
#endif

/*
*********************************************************************************************************
*                           Intel 80x86 (Real-Mode, Large Model) Miscellaneous
*********************************************************************************************************
*/

#define  OS_STK_GROWTH        1                       /* Stack grows from HIGH to LOW memory on 80x86  */

#define  uCOS                 0x05                    /* Interrupt vector # used for context switch    */

#define  OS_TASK_SW()         SwiHandle(uCOS)

/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

//OS_CPU_EXT  INT8U  OSTickDOSCtr;       /* Counter used to invoke DOS's tick handler every 'n' ticks    */
OS_CPU_EXT	OS_STK 		OSIntStk[100];

/*
*********************************************************************************************************
*                                              PROTOTYPES
*********************************************************************************************************
*/

//void       OSTaskStkInit_FPE_x86(OS_STK **pptos, OS_STK **ppbos, INT32U *psize);

#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
OS_CPU_SR  OSCPUSaveSR(void);
void       OSCPURestoreSR(OS_CPU_SR cpu_sr);
#endif

#define rSRCPND		(*(volatile unsigned long *)0x4A000000)
#define rINTPND		(*(volatile unsigned long *)0x4A000010)
#define rINTOFFSET	(*(volatile unsigned long *)0x4A000014)
#define rSUBSRCPND	(*(volatile unsigned long *)0x4A000018)
#define rUERSTAT0	(*(volatile unsigned long *)0x50000014)
#define	rUFSTAT0	(*(volatile unsigned long *)0x50000018)
#define	rUTXH0		(*(volatile unsigned char *)0x50000020)
#define	rURXH0		(*(volatile unsigned char *)0x50000024)
#define rUERSTAT1	(*(volatile unsigned long *)0x50004014)
#define rUFSTAT1	(*(volatile unsigned long *)0x50004018)
#define rUTXH1		(*(volatile unsigned char *)0x50004020)
#define rURXH1		(*(volatile unsigned char *)0x50004024)
#define rTCFG0		(*(volatile unsigned long *)0x51000000)
#define rTCFG1		(*(volatile unsigned long *)0x51000004)
#define rTCON		(*(volatile unsigned long *)0x51000008)
#define rTCNTB0		(*(volatile unsigned long *)0x5100000C)
#define rTCMPB0		(*(volatile unsigned long *)0x51000010)
#define rTCNTB4		(*(volatile unsigned long *)0x5100003C)
#define rGPBCON 	(*(volatile unsigned long *)0x56000010)
#define rGPBDAT		(*(volatile unsigned long *)0x56000014)
#define rGPBUP		(*(volatile unsigned long *)0x56000018)
#define rGPJCON		(*(volatile unsigned long *)0x560000d0)
#define rGPJDAT		(*(volatile unsigned long *)0x560000d4)
#define rGPJUP		(*(volatile unsigned long *)0x560000d8)
#define rEINTPEND  	(*(volatile unsigned long *)0x560000A8)

#define TxF1FULL	((rUFSTAT1>>14)&(1))
#define TxF1COUNT	((rUFSTAT1>>8)&(63))
#define	RxF1FULL	((rUFSTAT1>>6)&(1))
#define RxF1COUNT	((rUFSTAT1)&(63))
#define TxF0FULL	((rUFSTAT0>>14)&(1))
#define TxF0COUNT	((rUFSTAT0>>8)&(63))
#define	RxF0FULL	((rUFSTAT0>>6)&(1))
#define RxF0COUNT	((rUFSTAT0)&(63))

#define	B0			(0x00000001)
#define	B1			(0x00000002)
#define	B2			(0x00000004)
#define	B3			(0x00000008)
#define	B4			(0x00000010)
#define	B5			(0x00000020)
#define	B6			(0x00000040)
#define	B7			(0x00000080)
#define	B8			(0x00000100)
#define	B9			(0x00000200)
#define	B10			(0x00000400)
#define	B11			(0x00000800)
#define	B12			(0x00001000)
#define	B13			(0x00002000)
#define	B14			(0x00004000)
#define	B15			(0x00008000)
#define	B16			(0x00010000)
#define	B17			(0x00020000)
#define	B18			(0x00040000)
#define	B19			(0x00080000)
#define	B20			(0x00100000)
#define	B21			(0x00200000)
#define	B22			(0x00400000)
#define	B23			(0x00800000)
#define	B24			(0x01000000)
#define	B25			(0x02000000)
#define	B26			(0x04000000)
#define	B27			(0x08000000)
#define	B28			(0x10000000)
#define	B29			(0x20000000)
#define	B30			(0x40000000)
#define	B31			(0x80000000)



#define LEDMASK 0x61F
#define LED1MASK 0x7DF
#define LED2MASK 0x7BF
#define	LED3MASK 0x77F
#define	LED4MASK 0x6FF
