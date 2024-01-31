
#ifndef     _PARAM_H_
#define     _PARAM_H_

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD  unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#define STRING_ITEM    0x00    //保证最后一位为0x00;
#define ASSCII_ITEM    0x01
#define HEX_ITEM       0x02
#define BIN_ITEM       0x03


#define  STR_PARAMS(A)     (U08*)A,     sizeof(A)
#define  CHAR_PARAMS(A)    (U08*)&A,    sizeof(INT8)
#define  INT_PARAMS(A)     (U08*)&A,    sizeof(INT32)
#define  LONG_PARAMS(A)    (U08*)&A,    sizeof(INT32)

#define KEY_BLOCK_TIME  10000

#define MAX_MSG_NAME_LEN 20
#define MAX_MSG_DATA_LEN 32
typedef struct
{
    char Name[MAX_MSG_NAME_LEN];
    char ptr[MAX_MSG_DATA_LEN];
    WORD wPtrLen;
    WORD wType;
}PARAMS;


#define L_MAX_MSG_NAME_LEN  4
#define L_MAX_MSG_DATA_LEN 24
#define L_MESSAGE_PARAM_NUM  450

typedef struct
{
    char strName[L_MAX_MSG_NAME_LEN];
    char strMsg[MAX_MSG_DATA_LEN];
}MESSAGE_CFG;

#define MESSAGE_PARAM_NUM  19
extern PARAMS myParams[MESSAGE_PARAM_NUM];

#define MESSAGE_CFG_NUM  19

#define LOGO_ITEM_NUM  10
#define LANGUE_ITEM_NUM  11


#endif

