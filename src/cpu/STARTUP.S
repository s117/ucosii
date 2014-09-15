	PRESERVE8

	IMPORT Main
	IMPORT HandleIRQ
	IMPORT OSCtxSw
	IMPORT OSTickISR
	IMPORT OSTCBCur
	IMPORT OSIntNesting
	IMPORT OSIntStk

;-----------------------------------------------------------------------------------------------------------------------------
BK_IRQ			RLIST {R0-R12,LR}
RE_IRQ			RLIST {R0-R12,PC}
BK_FIQ			RLIST {R0-R7,LR}
BK_APCS			RLIST {R4-R11}
  
MAX_SWINUM		EQU 7

_STACKBASEADDR	EQU 0x32000000
rBWSCON			EQU 0x48000000
rSRCPND			EQU 0X4A000000
rINTMOD			EQU 0X4A000004
rINTMSK			EQU 0X4A000008
rINTPND			EQU 0x4A000010
rINTOFFSET		EQU 0x4A000014
rSUBSRCPND		EQU 0x4A000018
rINTSUBMSK		EQU 0X4A00001C
rLOCKTIME		EQU 0x4C000000
rMPLLCON		EQU 0x4C000004
rUPLLCON		EQU 0x4C000008
rCLKDIVN		EQU 0x4C000014
rULCON0			EQU 0x50000000
rUCON0			EQU 0x50000004
rUFCON0			EQU 0x50000008
rUMCON0			EQU 0x5000000C
rUBRDIV0		EQU 0x50000028
rUTXH0			EQU 0x50000023
rURXH0			EQU 0x50000027
rULCON1			EQU 0x50004000
rUCON1			EQU 0x50004004
rUFCON1			EQU 0x50004008
rUMCON1			EQU 0x5000400C
rUBRDIV1		EQU 0x50004028
rUTXH1			EQU 0x50004023
rURXH1			EQU 0x50004027
rWTCON			EQU 0x53000000
rGPBCON			EQU 0x56000010
rGPBDAT			EQU 0x56000014
rGPGCON			EQU 0x56000060
rGPGUP			EQU 0x56000068
rGPHCON			EQU 0x56000070
rGPHDAT			EQU 0x56000074
rGPHUP			EQU 0x56000078
rEINTMASK		EQU 0x560000A4
rEINTPEND		EQU 0x560000A8


B1_BWCON		EQU 2_0010
B2_BWCON		EQU 2_0001
B3_BWCON		EQU 2_0001
B4_BWCON		EQU 2_0001
B5_BWCON		EQU 2_0001
B6_BWCON		EQU 2_0010
B7_BWCON		EQU 2_0010


U_MDIV			EQU 0x38	  ;M/P/S DIV FOR UPLL,for more see chapter"PLL VALUE TABLE" in 2440datasheet
U_PDIV			EQU	0x2		  ;UPLL = 48.00MHz
U_SDIV			EQU	0x2

M_MDIV			EQU 0x7f	  ;M/P/S DIV FOR MPLL,for more see chapter"PLL VALUE TABLE" in 2440datasheet
M_PDIV			EQU	0x2		  ;MPLL = 405.00MHz
M_SDIV			EQU	0x1


USRMODE			EQU 0x10
FIQMODE			EQU 0x11
IRQMODE			EQU 0x12
SVCMODE			EQU 0x13
ABTMODE			EQU 0x17
UNDMODE			EQU 0x1B
SYSMODE			EQU 0x1F


IRQMSK			EQU 0x80
FIQMSK			EQU 0x40
NOINT			EQU 0xC0


StackUse		EQU (_STACKBASEADDR-0X3800)
StackSvc		EQU (_STACKBASEADDR-0x2800)
StackUnd		EQU (_STACKBASEADDR-0x2400)
StackAbt		EQU (_STACKBASEADDR-0x2000)
StackIrq		EQU (_STACKBASEADDR-0x1000)
StackFiq		EQU (_STACKBASEADDR-0x0)


OS_IntStkSIZE	EQU 100
OS_StkItemSIZE	EQU 4
;-----------------------------------------------------------------------------------------------------------------------------
	CODE32
	AREA	Init,CODE,READONLY,ALIGN=2
	ENTRY
	;VICTOR TABLE
    B ResetInit
	B HandlerUndef
	B HandlerSWI
	B HandlerPabort
	B HandlerDabort
	B .
	B HandlerIRQ
	B HandlerFIQ
	;VICTOR TABLE END
;-----------------------------------------------------------------------------------------------------------------------------
ResetInit
	LDR R0,=rWTCON ;Disable WDT
	LDR R1,=0x0
	STR R1,[R0]

	LDR R0,=rINTMSK	;Mask IRQ
	LDR R1,=0xFFFFFFFF
	STR R1,[R0]
	LDR R0,=rINTSUBMSK
	LDR R1,=0x0000FFFF
	STR R1,[R0]

;Start set new clock
	LDR R0,=rLOCKTIME
	LDR R1,=0xFFFFFFFF ;See Chapter"LOCKTIME REGISTER"In 2400datasheet
	STR R1,[R0]
			  
	LDR R0,=rCLKDIVN
	MOV R1,#5	;FCLK:HCLK:PCLK = 1:4:8,For more see Chapter"CLKDIVN REGISTER" In 2400datasheet
	STR R1,[R0]

	LDR R0,=rUPLLCON
	LDR R1,=((U_MDIV<<12)+(U_PDIV<<4)+U_SDIV)
	STR R1,[R0]
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP

	LDR R0,=rMPLLCON
	LDR R1,=((M_MDIV<<12)+(M_PDIV<<4)+M_SDIV)
	STR R1,[R0]
						
	MRC	p15,0,r0,c1,c0,0  ;Set Async Bus Mode
	ORR r0,r0,#0xC0000000
	MCR p15,0,r0,c1,c0,0

;New clock seted
	BL StackInit	   ;Initial stack
	LDR R0,=BUSINIT	   ;Initial Memory
	LDR R1,=rBWSCON
	LDMIA R0!, {R2-R8}				;MARK START LDMIA/STMIA
	STMIA R1!, {R2-R8}
	LDMIA R0!, {R2-R7}
	STMIA R1!, {R2-R7}				;MARK END

	LDR R0,=rGPHCON   ;Initial Serial Port:enable 
	LDR R1,=64170
	STR R1,[R0]
	LDR R0,=rGPHUP
	LDR R1,=0x0
	;Port:0
	STR R1,[R0]
	LDR R0,=rULCON0
	LDR R1,=0x03	  ;8n1
	STR R1,[R0]
	LDR R0,=rUCON0
	LDR R1,=69   	  ;用PCLK作为输入工作时钟，中断方式传送，关闭数据接收超时，数据错误产生中断
	STR R1,[R0]
	LDR R0,=rUFCON0
	LDR R1,=193		  ;启用FIFO，发送FIFO触发深度48字节，接收FIFO触发深度1字节
	STR R1,[R0]
	LDR R0,=rUMCON0
	LDR R1,=0x00	  ;AFC Disable
	STR R1,[R0]
	LDR R0,=rUBRDIV0  
	LDR R1,=26		  ;115200bps
	STR R1,[R0]
	;Port1
	LDR R0,=rULCON1																									
	LDR R1,=0x03	  ;8n1
	STR R1,[R0]
	LDR R0,=rUCON1
	LDR R1,=69		  ;用PCLK作为输入工作时钟，中断方式传送，关闭数据接收超时，数据错误产生中断
	STR R1,[R0]
	LDR R0,=rUFCON1	  
	LDR R1,=193		  ;启用FIFO，发送FIFO触发深度48字节，接收FIFO触发深度1字节
	STR R1,[R0]
	LDR R0,=rUMCON1
	LDR R1,=0x00      ;AFC Disable
	STR R1,[R0]
	LDR R0,=rUBRDIV1
	LDR R1,=325		  ;9600bps
	STR R1,[R0]
;×￠:1?óúUBRDIVn:?úPCLK=67.5Mhz:36-115200bps  435-9600bps,?úPCLK=50Mhz:26-115200bps  325-9600bps

;	LDR R0,=rGPBCON	  ;Initial LED&BEEP
;	LDR R1,=0x155555
;	STR R1,[R0]     


	B Main
;ResetInit END
;-----------------------------------------------------------------------------------------------------------------------------
StackInit
	MOV R0,LR

	MSR CPSR_c,#(SVCMODE:OR:IRQMSK:OR:FIQMSK)
	LDR SP,=StackSvc
	MSR CPSR_c,#(UNDMODE:OR:IRQMSK:OR:IRQMSK)
	LDR SP,=StackUnd
	MSR CPSR_c,#(ABTMODE:OR:IRQMSK:OR:FIQMSK)
	LDR SP,=StackAbt
	MSR CPSR_c,#(IRQMODE:OR:IRQMSK:OR:FIQMSK)
	LDR SP,=StackIrq
	MSR CPSR_c,#(FIQMODE:OR:IRQMSK:OR:FIQMSK)
	LDR SP,=StackFiq
	MSR CPSR_c,#(SYSMODE:OR:IRQMSK:OR:FIQMSK)
	LDR SP,=StackUse

	MOVS PC,R0 ;MOVS?

;-----------------------------------------------------------------------------------------------------------------------------
HandlerUndef
	B ResetInit
;-----------------------------------------------------------------------------------------------------------------------------	
HandlerSWI ;0:IRQoff,1:IRQon,2:FIQoff,3:FIQon,4:SetInterruptREG,5:OSCtxSw,6:OSSetCPSR(R1=CPSR_VALUE)
	CMP R0,#MAX_SWINUM
	LDRLO PC,[PC,R0,LSL#2]
	MOVS PC,LR

SWIFUNC_SWI
	DCD IRQDisable
	DCD IRQEnable
	DCD FIQDisable
	DCD FIQEnable
	DCD SETIRQReg
	DCD OSCtxSw
	DCD OSSetCPSR
IRQDisable
	MRS R0,SPSR
	ORR R0,#IRQMSK
	MSR SPSR_c,R0
	MOVS PC,LR
IRQEnable
	MRS R0,SPSR
	BIC R0,#IRQMSK
	MSR SPSR_c,R0
	MOVS PC,LR
FIQDisable
	MRS R0,SPSR
	ORR R0,#FIQMSK
	MSR SPSR_c,R0
	MOVS PC,LR
FIQEnable
	MRS R0,SPSR
	BIC R0,#FIQMSK
	MSR SPSR_c,R0
	MOVS PC,LR
SETIRQReg
	STMDB SP!,{R0,R1}	  ;STMFD
	LDR R0,=rGPGCON
	LDR R1,=8431746		  ;设置K1~K6为EINT
	STR R1,[R0]
	LDR R0,=rGPGUP		  ;注释掉这个设置可以正常使用
	LDR R1,=0			  ;启用端口上拉，稳定信号，记得消抖10ms
	STR R1,[R0]
	LDR R0,=rEINTMASK
	LDR R1,=(~(1<<8))&(~(1<<11))&(~(1<<13))&(~(1<<14))&(~(1<<15))&(~(1<<19))	  ;开启外部按键中断
	STR R1,[R0]
	LDR R0,=rEINTPEND
	MOV R1,R0			  ;清除外部中断挂起寄存器
	STR R1,[R0]				
	LDR R0,=rINTMOD
	LDR R1,=0		      ;全部使用IRQ处理
	STR R1,[R0]
	LDR R0,=rINTMSK
	LDR R1,=0xEF7FBFDF    ;UART0&1/EINT8_23/TIMER_4 打开
	STR R1,[R0]
	LDR R0,=rINTSUBMSK
	LDR R1,=32722	      ;INT_ERR1 INT_RXD1 INT_ERR0 INT_RXD0 打开
	STR R1,[R0]
	LDMIA SP!,{R0,R1}	  ;LDMFD
	MOVS PC,LR
OSSetCPSR
	MSR SPSR_cxsf,R0
	MOVS PC,LR
	
;HandlerSWI END
;-----------------------------------------------------------------------------------------------------------------------------
HandlerPabort
	b HandlerPabort
;-----------------------------------------------------------------------------------------------------------------------------
HandlerDabort
	b HandlerDabort
;-----------------------------------------------------------------------------------------------------------------------------
HandlerIRQ
    SUB LR,LR,#4

	MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE
	STMDB R13!,{R0-R12,LR}	;Store normal register
	MSR CPSR_c,#2_11010010	;Switch to IRQ mode WITH INTRUPT DISABLE
	MRS R0,SPSR				;CPSR=>R0
	MOV R1,LR				;PC=>R1
	MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE
	STMDB R13!,{R0,R1}		;Store CPSR&PC
	
	LDR R0,=OSIntNesting
	LDRB R1,[R0]			;OSIntNesting=>R1
	ADD R1,R1,#1
	STRB R1,[R0]			;++OSIntNesting
	
	CMP R1,#1				;Test whether change stack or not(for interrupt nesting)
	BNE GoISRNow
	LDR R0,=OSTCBCur
	LDR R0,[R0]
	STR R13,[R0]			;Save SP_usr ONLY WHEN first interrupt entering
	LDR R13,=OSIntStk
	ADD R13,R13,#(OS_IntStkSIZE*OS_StkItemSIZE)		;Switch to INTMODE's own stack
GoISRNow
	LDR R0,=rINTOFFSET
	LDR R0,[R0]
	LDR LR,=INTRETURN		;LR_irq = Return Address
	LDR PC,[PC,R0,LSL#2]
	NOP						;For align
VECTOR_IRQ
	DCD HandleIRQ		;00
	DCD HandleIRQ		;01
	DCD HandleIRQ		;02
	DCD HandleIRQ		;03
	DCD HandleIRQ		;04
	DCD HandleIRQ		;05
	DCD HandleIRQ		;06
	DCD HandleIRQ		;07
	DCD HandleIRQ		;08
	DCD HandleIRQ		;09
	DCD HandleIRQ		;10
	DCD HandleIRQ		;11
	DCD HandleIRQ		;12
	DCD HandleIRQ		;13
	DCD	OSTickISR		;14
	DCD HandleIRQ		;15
	DCD HandleIRQ		;16
	DCD HandleIRQ		;17
	DCD HandleIRQ		;18
	DCD HandleIRQ		;19
	DCD HandleIRQ		;20
	DCD HandleIRQ		;21
	DCD HandleIRQ		;22
	DCD HandleIRQ		;23
	DCD HandleIRQ		;24
	DCD HandleIRQ		;25
	DCD HandleIRQ		;26
	DCD HandleIRQ		;27
	DCD HandleIRQ		;28
	DCD HandleIRQ		;29
	DCD HandleIRQ		;30
	DCD HandleIRQ		;31

INTRETURN
	MSR CPSR_c,#2_11011111		;Switch to SYS mode WITH INTRUPT DISABLE,ISR end

	LDR R0,=OSIntNesting	
	LDRB R1,[R0]				;OSIntNesting=>R1

	CMP R1,#0					;Test whether recover stack or not(for nesting interrupt exit)
	BNE NoRecoverSP
	LDR R0,=OSTCBCur
	LDR R0,[R0]
	LDR R13,[R0]				;Recover SP_usr ONLY WHEN no interrupt left
NoRecoverSP
	LDMIA R13!,{R0,R1}			;Get CPSR&PC
	MSR CPSR_c,#2_11010010		;Switch to IRQ mode WITH INTRUPT DISABLE
	MOV LR,R1					;PC->LR_irq
	MSR SPSR_cxsf,R0			;CPSR->SPSR_irq
	MSR CPSR_c,#2_11011111		;Switch to SYS mode WITH INTRUPT DISABLE
	LDMIA R13!,{R0-R12,LR}		;Recover normal register
	MSR CPSR_c,#2_11010010		;Switch to IRQ mode WITH INTRUPT DISABLE

	MOVS PC,LR

;-----------------------------------------------------------------------------------------------------------------------------
HandlerFIQ
	STMDB SP!,BK_FIQ	   ;STMFD
	BL FIQ_Exception

	LDMIA SP!,BK_FIQ	   ;LDMFD
	SUBS PC,LR,#4

FIQ_Exception   ;code exception behaviour here

	BX LR ;return

BUSINIT
	DCD (B7_BWCON<<28):OR:(B6_BWCON<<24):OR:(B5_BWCON<<20):OR:(B4_BWCON<<16):OR:(B3_BWCON<<12):OR:(B2_BWCON<<8):OR:(B1_BWCON<<4) ; rBWSCON
	DCD (0x3<<13):OR:(0x3<<11):OR:(0x7<<8):OR:(0x3<<6):OR:(0x3<<4):OR:(0x1<<2):OR:(0<<0) ; rBANKCON0
	DCD (1<<13):OR:(1<<11):OR:(6<<8):OR:(1<<6):OR:(1<<4):OR:(0<<2):OR:(0<<0) ; rBANKCON1
	DCD (1<<13):OR:(1<<11):OR:(6<<8):OR:(1<<6):OR:(1<<4):OR:(0<<2):OR:(0<<0) ; rBANKCON2
	DCD (1<<13):OR:(1<<11):OR:(6<<8):OR:(1<<6):OR:(1<<4):OR:(0<<2):OR:(0<<0) ; rBANKCON3
	DCD (1<<13):OR:(1<<11):OR:(6<<8):OR:(1<<6):OR:(1<<4):OR:(0<<2):OR:(0<<0) ; rBANKCON4
	DCD (1<<13):OR:(1<<11):OR:(6<<8):OR:(1<<6):OR:(1<<4):OR:(0<<2):OR:(0<<0) ; rBANKCON5
	DCD (3<<15):OR:(0<<2):OR:(1<<0) ; rBANKCON6(SDRAM),Trcd=18ns ,Column address number = 9-bit
	DCD (3<<15):OR:(0<<2):OR:(1<<0) ; rBANKCON7(SDRAM),Trcd=18ns ,Column address number = 9-bit
	DCD (1<<23):OR:(0<<22):OR:(0<<20):OR:(1<<18):OR:(1269) ; rREFRESH(SDRAM),period=7.8125uS(64ms/8192),Trp=18ns,Trc=60ns,Tsrc=42ns
	DCD (1<<7):OR:(1<<5):OR:(1<<4):OR:(1<<0) ; rBANKSIZE(64MB)
	DCD (3<<4) ; rMRSRB6
	DCD (3<<4) ; rMRSRB6
	END
