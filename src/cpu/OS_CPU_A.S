;********************************************************************************************************
;                                               uC/OS-II
;                                         The Real-Time Kernel
;
;                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
;                                          All Rights Reserved
;
;
;                                          ARM9 Specific code
;                                          LARGE MEMORY MODEL
;
;                                           Keil MDK
;                                       (ARM920T Compatible Target)
;
; File         : OS_CPU_A.S
; By           : Spartan 117
;********************************************************************************************************

;********************************************************************************************************
;                                    PUBLIC and EXTERNAL REFERENCES
;********************************************************************************************************

	PRESERVE8
	
	EXPORT OSTickISR
	EXPORT OSStartHighRdy
	EXPORT OSCtxSw
	EXPORT OSIntCtxSw

	IMPORT OSTimeTick
	IMPORT OSIntEnter
	IMPORT OSIntExit
	IMPORT OSTaskSwHook
	IMPORT HandleIRQ

	IMPORT OSTCBHighRdy
	IMPORT OSTCBCur
	IMPORT OSPrioHighRdy
	IMPORT OSPrioCur
	IMPORT OSRunning
	IMPORT OSIntNesting
	IMPORT OSIntStk

OS_IntStkSIZE	EQU 100
OS_StkItemSIZE	EQU 4

USRMODE			EQU 0x10
FIQMODE			EQU 0x11
IRQMODE			EQU 0x12
SVCMODE			EQU 0x13
ABTMODE			EQU 0x17
UNDMODE			EQU 0x1B
SYSMODE			EQU 0x1F
IRQMSK			EQU 0x80
FIQMSK			EQU 0x40
rSRCPND			EQU 0x4A000000
rINTPND			EQU 0x4A000010


	CODE32
	AREA	OS_CPU_S,CODE,READONLY,ALIGN=2

;*********************************************************************************************************
;                                          START MULTITASKING
;                                       void OSStartHighRdy(void)
;
; The stack frame is assumed to look as follows:
;
; OSTCBHighRdy=>OSTCBStkPtr -=> DS                               (Low memory)
;                               ES
;                               DI
;                               SI
;                               BP
;                               SP
;                               BX
;                               DX
;                               CX
;                               AX
;                               OFFSET  of task code address
;                               SEGMENT of task code address
;                               Flags to load in PSW
;                               OFFSET  of task code address
;                               SEGMENT of task code address
;                               OFFSET  of 'pdata'
;                               SEGMENT of 'pdata'               (High memory)
;
; Note : OSStartHighRdy() MUST:
;           a) Call OSTaskSwHook() then,
;           b) Set OSRunning to TRUE,
;           c) Switch to the highest priority task.
;*********************************************************************************************************

OSStartHighRdy;This function running in SYS mode
			MOV LR,PC				;PC=PC+8
			LDR PC,=OSTaskSwHook	;Call user defined task switch hook

			LDR R0,=OSRunning
			LDR R1,=1
			STRB R1,[R0]			;OSRunning = TRUE (Indicates that multitasking has started)

			MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE

			LDR R0,=OSTCBHighRdy
			LDR R0,[R0]
			LDR R13,[R0]			;SP = OSTCBHighRdy=>OSTCBStkPtr
		
			LDMIA R13!,{R0,R1}		;Get CPSR&PC

			MSR CPSR_c,#2_11010011	;Switch to SVC mode WITH INTRUPT DISABLE
			
			MOV LR,R1				;PC=>LR_svc
			MSR SPSR_cxsf,R0		;CPSR=>SPSR_svc

			MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE

			LDMIA R13!,{R0-R12,LR}	;Recover normal register

			MSR CPSR_c,#2_11010011	;Switch to SVC mode WITH INTRUPT DISABLE

			MOVS PC,LR				;Run Task


;*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From task level)
;                                           void OSCtxSw(void)
;
; Note(s): 1) Upon entry,
;             OSTCBCur     points to the OS_TCB of the task to suspend
;             OSTCBHighRdy points to the OS_TCB of the task to resume
;
;          2) The stack frame of the task to suspend looks as follows:
;
;                 SP => OFFSET  of task to suspend    (Low memory)
;                       SEGMENT of task to suspend
;                       PSW     of task to suspend    (High memory)
;
;          3) The stack frame of the task to resume looks as follows:
;
;                 OSTCBHighRdy=>OSTCBStkPtr -=> DS                               (Low memory)
;                                               ES
;                                               DI
;                                               SI
;                                               BP
;                                               SP
;                                               BX
;                                               DX
;                                               CX
;                                               AX
;                                               OFFSET  of task code address
;                                               SEGMENT of task code address
;                                               Flags to load in PSW             (High memory)
;*********************************************************************************************************

OSCtxSw;This function running in SVC mode

			MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE

			STMDB R13!,{R0-R12,LR}	;Store normal register

			MSR CPSR_c,#2_11010011	;Switch to SVC mode WITH INTRUPT DISABLE

			MRS R0,SPSR				;CPSR=>R0
			MOV R1,LR				;PC=>R1

			MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE

			STMDB R13!,{R0,R1}		;Store CPSR&PC


			LDR R0,=OSTCBCur
			LDR R0,[R0]
			STR R13,[R0]			;Store SP to TCB,Prior task saved

			MOV LR,PC				;PC=PC+8
			LDR PC,=OSTaskSwHook	;Call user defined task switch hook

			LDR R0,=OSPrioHighRdy
			LDRB R1,[R0]
			LDR R0,=OSPrioCur
			STRB R1,[R0]			;OSPrioCur = OSPrioHighRdy

			LDR R0,=OSTCBHighRdy
			LDR R1,[R0]
			LDR R0,=OSTCBCur
			STR R1,[R0]				;OSTCBCur = OSTCBHighRdy

			LDR R13,[R1]			;SP = OSTCBHighRdy=>OSTCBStkPtr 

			LDMIA R13!,{R0,R1}		;Get CPSR&PC

			MSR CPSR_c,#2_11010011	;Switch to SVC mode WITH INTRUPT DISABLE
			
			MOV LR,R1				;PC=>LR_svc
			MSR SPSR_cxsf,R0		;CPSR=>SPSR_svc

			MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE

			LDMIA R13!,{R0-R12,LR}	;Recover normal register

			MSR CPSR_c,#2_11010011	;Switch to SVC mode WITH INTRUPT DISABLE

			MOVS PC,LR				;Run next Task


;*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From an ISR)
;                                        void OSIntCtxSw(void)
;
; Note(s): 1) Upon entry,
;             OSTCBCur     points to the OS_TCB of the task to suspend
;             OSTCBHighRdy points to the OS_TCB of the task to resume
;
;          2) The stack frame of the task to suspend looks as follows:
;
;             OSTCBCur=>OSTCBStkPtr -----=>  DS                              (Low memory)
;                                            ES
;                                            DI
;                                            SI
;                                            BP
;                                            SP
;                                            BX
;                                            DX
;                                            CX
;                                            AX
;                                            OFFSET  of task code address
;                                            SEGMENT of task code address
;                                            Flags to load in PSW            (High memory)
;
;
;          3) The stack frame of the task to resume looks as follows:
;
;             OSTCBHighRdy=>OSTCBStkPtr -=> DS                               (Low memory)
;                                           ES
;                                           DI
;                                           SI
;                                           BP
;                                           SP
;                                           BX
;                                           DX
;                                           CX
;                                           AX
;                                           OFFSET  of task code address
;                                           SEGMENT of task code address
;                                           Flags to load in PSW             (High memory)
;*********************************************************************************************************

OSIntCtxSw;This function run in SYS mode

			MOV LR,PC				;PC=PC+8
			LDR PC,=OSTaskSwHook	;Call user defined task switch hook

			MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE

			LDR R0,=OSPrioHighRdy
			LDRB R1,[R0]
			LDR R0,=OSPrioCur
			STRB R1,[R0]			;OSPrioCur = OSPrioHighRdy

			LDR R0,=OSTCBHighRdy
			LDR R1,[R0]
			LDR R0,=OSTCBCur
			STR R1,[R0]				;OSTCBCur = OSTCBHighRdy

			LDR R13,[R1]			;SP = OSTCBHighRdy=>OSTCBStkPtr 

			LDMIA R13!,{R0,R1}		;Get CPSR&PC
			MSR CPSR_c,#2_11010010	;Switch to IRQ mode WITH INTRUPT DISABLE
			MOV LR,R1				;PC=>LR_irq
			MSR SPSR_cxsf,R0		;CPSR=>SPSR_irq
			MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE
			LDMIA R13!,{R0-R12,LR}	;Recover normal register
			MSR CPSR_c,#2_11010010	;Switch to IRQ mode WITH INTRUPT DISABLE

			MOVS PC,LR				;Run next Task


;*********************************************************************************************************
;                                            HANDLE TICK ISR
;
; Description: This function is called 199.99 times per second or, 11 times faster than the normal DOS
;              tick rate of 18.20648 Hz.  Thus every 11th time, the normal DOS tick handler is called.
;              This is called chaining.  10 times out of 11, however, the interrupt controller on the PC
;              must be cleared to allow for the next interrupt.
;
; Arguments  : none
;
; Returns    : none
;
; Note(s)    : The following C-like pseudo-code describe the operation being performed in the code below.
;
;              Save all registers on the current task's stack;
;              OSIntNesting++;
;              if (OSIntNesting == 1) {
;                 OSTCBCur=>OSTCBStkPtr = SS:SP
;              }
;              OSTickDOSCtr--;
;              if (OSTickDOSCtr == 0) {
;                  OSTickDOSCtr = 11;
;                  INT 81H;               Chain into DOS every 54.925 mS
;                                         (Interrupt will be cleared by DOS)
;              } else {
;                  Send EOI to PIC;       Clear tick interrupt by sending an End-Of-Interrupt to the 8259
;                                         PIC (Priority Interrupt Controller)
;              }
;              OSTimeTick();              Notify uC/OS-II that a tick has occured
;              OSIntExit();               Notify uC/OS-II about end of ISR
;              Restore all registers that were save on the current task's stack;
;              Return from Interrupt;
;*********************************************************************************************************
;
OSTickISR;This function running in IRQ mode
			MSR CPSR_c,#2_11010010	;Switch to IRQ mode WITH INTRUPT DISABLE
			LDR R0,=rINTPND
			LDR R1,[R0]
			LDR R0,=rSRCPND
			STR R1,[R0]
			LDR R0,=rINTPND
			STR R1,[R0]				;Clear TIMER4's Interrupt Request

			MSR CPSR_c,#2_00011111	;Switch to SYS mode WITH INTRUPT ENABLE

			MOV LR,PC
			LDR PC,=OSTimeTick

			MOV LR,PC
			LDR PC,=OSIntExit

			MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE,ISR end

			LDR R0,=OSIntNesting	
			LDRB R1,[R0]			;OSIntNesting=>R1

			CMP R1,#0				;Test whether recover stack or not(for nesting interrupt exit)
			BNE NoRecoverSP			;This label should change for different ISR
 			LDR R0,=OSTCBCur
			LDR R0,[R0]
			LDR R13,[R0]			;Recover SP_usr ONLY WHEN no interrupt left
NoRecoverSP
			LDMIA R13!,{R0,R1}		;Get CPSR&PC
			MSR CPSR_c,#2_11010010	;Switch to IRQ mode WITH INTRUPT DISABLE
			MOV LR,R1				;PC=>LR_irq
			MSR SPSR_cxsf,R0		;CPSR=>SPSR_irq
			MSR CPSR_c,#2_11011111	;Switch to SYS mode WITH INTRUPT DISABLE
			LDMIA R13!,{R0-R12,LR}	;Recover normal register
			MSR CPSR_c,#2_11010010	;Switch to IRQ mode WITH INTRUPT DISABLE

			MOVS PC,LR


			END
