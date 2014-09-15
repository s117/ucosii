#ifdef   SYSTEM_CODE_C
#define  SYSTEM_CODE_EXT
#else
#define  SYSTEM_CODE_EXT  extern
#endif

#ifdef   UTILS_C
#define  UTILS_C_EXT
#else
#define  UTILS_C_EXT  extern
#endif

#ifdef USER_LOGGER_C
#define USER_LOGGER_C_EXT
#else
#define USER_LOGGER_C_EXT	extern
#endif

/*
*********************************************************************************************************
*                                            SYSTEM_CODE.C
*********************************************************************************************************
*/
SYSTEM_CODE_EXT void HandleIRQ(int IntOffset);
SYSTEM_CODE_EXT __inline void delay(int msec);
//SYSTEM_CODE_EXT __inline void idleMS(int ms);
SYSTEM_CODE_EXT int getstream0(INT8U* serial_in);
SYSTEM_CODE_EXT void transbin0(INT8U* dest,int size);
SYSTEM_CODE_EXT INT8U sendbin0(INT8U* dest,int size);
SYSTEM_CODE_EXT void recbin0(INT8U* dest,int size);
SYSTEM_CODE_EXT void flush0(void);
SYSTEM_CODE_EXT int getstream1(INT8U* serial_in);
SYSTEM_CODE_EXT void transbin1(INT8U* dest,int size);
SYSTEM_CODE_EXT INT8U sendbin1(INT8U* dest,int size);
SYSTEM_CODE_EXT void recbin1(INT8U* dest,int size);
SYSTEM_CODE_EXT void flush1(void);
SYSTEM_CODE_EXT INT32U VICVectAddr[32];
/*
*********************************************************************************************************
*                                            UTILS_C.C
*********************************************************************************************************
*/
UTILS_C_EXT void	init_led(void);
UTILS_C_EXT void	S3C_InitTimer_PWM(void);
UTILS_C_EXT void	S3C_StartTimer4(void);
UTILS_C_EXT void	S3C_StartPWMBuzzer(void);
UTILS_C_EXT void	S3C_StopPWMBuzzer(void);
UTILS_C_EXT void	S3C_SetPWMBuzzer(INT16U TCNTB0,INT16U TCMPB0);
UTILS_C_EXT void	S3C_SysInit(void);
//UTILS_C_EXT __inline void LCDSet_RS(INT8U RS);
//UTILS_C_EXT __inline void LCDSet_RW(INT8U RW);
//UTILS_C_EXT __inline void LCDSet_E(INT8U E);
//UTILS_C_EXT __inline void LCDSet_RS_RW_E(INT8U RS,INT8U RW,INT8U E);
//UTILS_C_EXT __inline void LCDSet_DATA(INT8U DATA);
//UTILS_C_EXT __inline void LCDSet_Param(INT8U RS,INT8U RW,INT8U E,INT8U DATA);
//UTILS_C_EXT void LCD1602_delay(void);
//UTILS_C_EXT void msgbox(INT8U MODE,INT8U DATA);
UTILS_C_EXT void LCDInit(void);
UTILS_C_EXT void LCDClear(void);
UTILS_C_EXT void LCDChar(INT8U ADD,char CH);
UTILS_C_EXT void LCDString(INT8U Address,char *Str);
