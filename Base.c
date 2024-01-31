
#include "Posapi.h"
#include "Base.h"
#include "Param.h"


// 0:fault, 1:ok
int ISAllCharInString(U08 *pAllChar, int AllCharLen, U08 *str, int strLen)
{
    int i, j;
    int iFlag = 0;
    for (i=0; i<AllCharLen; i++)
    {
        iFlag = 0;
        for (j=0; j<strLen; j++)
        {
            if (pAllChar[i] == str[j])
            {
                iFlag = 1;
                break;
            }
        }
        if (0 == iFlag)
        {
            return 0;
        }
    }
    return 1;
}

// 0:fault, 1:ok
int ISCharInString(char cChar, char *str)
{
    while (0 != *str)
    {
        if (cChar == *str)
        {
            return 1;
        }
        str++;
    }
    return 0;
}


void PubBeepErr(void)
{
    sysBeef(1, 200);
    sysDelayMs(200);
}

/******************************************************************************
Function   : void U32ToU08(U32 dwInData, U08 *pbyOutData)
Decryption : convert a dword variable to 4 bytes data.
Input para.: 1. dwInData : dword data input
Outut para.: 1. pbyOutData : byte result output
Return     : NULL
History    :
      Author       Date         Version    Modified Content
  1.  Ryan Huang  2011-05-20    V1.0       Create
******************************************************************************/
void U32ToU08(U32 dwInData, U08 *pbyOutData)
{
    pbyOutData[0] = (U08)(dwInData>>24);
    pbyOutData[1] = (U08)(dwInData>>16);
    pbyOutData[2] = (U08)(dwInData>>8);
    pbyOutData[3] = (U08)(dwInData);
}

void U08ToU32(U08 *pbyIn, int iLen, U32 *pdwOut)
{
    int i, j;
    for (i=0; i<iLen; i+=4)
    {
        j = i>>2;
        pdwOut[j] = pbyIn[i];
        pdwOut[j] <<= 8;
        pdwOut[j] += pbyIn[i+1];
        pdwOut[j] <<= 8;
        pdwOut[j] += pbyIn[i+2];
        pdwOut[j] <<= 8;
        pdwOut[j] += pbyIn[i+3];
    }
}

// ¼ÆËãLRC
U08 CalcLRC(U08 *psData, U32 uiLength, U08 ucInit)
{
    while( uiLength>0 )
    {
        ucInit ^= *psData++;
        uiLength--;
    }

    return ucInit;
}

// iSearchType=0:Header, =1:tailed
int SearchString(U08 *pbyBeSearch, int iBeLen, U08 *pbySearch, int iLen, int iSearchType)
{
    int i, j, k, iFlag;

    if ((iLen <= 0) || (iBeLen<=iLen))
    {
        return -1;
    }
    k = iBeLen - iLen;
    for (i=0; i<=k; i++)
    {
        iFlag = 1;
        for (j=0; j<iLen; j++)
        {
            if (pbyBeSearch[i+j] != pbySearch[j])
            {
                iFlag = 0;
                break;
            }
        }
        if (1 == iFlag)
        {
            if (0 == iSearchType)
            {
                return (i+iLen);
            }
            else
            {
                return (i-1);
            }
        }
    }
    return -2;
}

int SearchItemData(const char *strBeSearch, const char *strSearch, char *strOut)
{
    char strTemp[60];
    int iLen, iHeader, iTail, iBeSize;

    iBeSize = strlen(strBeSearch);
    iLen = strlen(strSearch);
    strTemp[0] = '<';
    strTemp[1] = '<';
    strcpy(&strTemp[2], strSearch);
    strTemp[iLen+2] = '>';
    strTemp[iLen+3] = 0;
    iHeader = SearchString((U08*)strBeSearch, iBeSize, &strTemp[1], iLen+2, 0);
    if (iHeader > 0)
    {
        strTemp[1] = '/';
        iTail = SearchString((U08*)&strBeSearch[iHeader], iBeSize-iHeader, strTemp, iLen+3, 1);
        if (iTail >= 0)
        {
            memcpy(strOut, &strBeSearch[iHeader], iTail+1);
            strOut[iTail+1] = 0;
            return 0; // OK
        }
    }
    return 1; // failed
}

