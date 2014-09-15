#define NAME_SIZE 64
#define MAX_PATH_SIZE 256

#define DATA_SIZE 0x64
#define ID_SIZE 0x64

#define TYPE_KEY 0x1	  //登记钥匙
#define TYPE_SIGN 0x2	  //登入
#define TYPE_LOGOUT 0x3	  //登出
#define TYPE_DEFAULT 0x4  //默认
#define TYPE_QUERY 0x5	  //查询
#define TYPE_INSERT 0x6	  //插入
#define TYPE_BEEING 0xFF 

#define TYPE_BE_SUCCESS_IN 0x101
#define TYPE_BE_SUCCESS_OUT 0x102
#define TYPE_BE_FAIL 0x103
#define TYPE_BE_BEEP 0x104
#define TYPE_BE_SPECIAL 0x105
#define TYPE_BE_IN 0x106
#define TYPE_BE_NOT_IN 0x107
#define TYPE_BE_STOP 0x108

#define MAGIC 0x05214783


struct PortInfo
{
	INT32U iMagicNumber;	
	INT32U iSave;
	INT32U iType;
	INT32U iData;
};
typedef struct PortInfo PackageFormat;

