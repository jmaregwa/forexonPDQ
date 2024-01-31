
#ifndef _BASE_H_
#define _BASE_H_

#ifndef U08
#define U08 unsigned char
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef U32
#define U32 unsigned int
#endif

#define ANS1_INTEGER           0x02
#define ANS1_BIT_STRING        0x03
#define ANS1_OCTET_STRING      0x04
#define ANS1_NULL              0x05
#define ANS1_OBJECT_IDENTIFIER 0x06
#define ANS1_UTF8String        0x0c
#define ANS1_PrintableString   0x13
#define ANS1_T61String         0x14
#define ANS1_IA5String         0x16
#define ANS1_UTCTime           0x17
#define ANS1_SEQUENCE          0x30
#define ANS1_SET               0x31


typedef struct
{
    U32 ModLen;          /* length in bytes of modulus */
    U08 Modulus[4096];   /* modulus */
    U08 Exp[4];          /* public exponent */
}RSA_PUBLIC_KEY;



void U08ToU32(U08 *pbyIn, int iLen, U32 *pdwOut);

U08 CalcLRC(U08 *psData, U32 uiLength, U08 ucInit);

int ISAllCharInString(U08 *pAllChar, int AllCharLen, U08 *str, int strLen);

int ISCharInString(char cChar, char *str);

int SearchString(U08 *pbyBeSearch, int iBeLen, U08 *pbySearch, int iLen, int iSearchType);

int SearchItemData(const char *strBeSearch, const char *strSearch, char *strOut);



#endif


