
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Posapi.h"

#include "Lcd.h"
#include "Param.h"
#include "Base.h"
#include "Comm.h"
#include "Card.h"
#include "FunctionList.h"

POS_PARAM_STRC stPosParam;

extern PARAMS myParams[MESSAGE_PARAM_NUM];

/*=================================================================*/
/*  功能描述：判断是否是一张EMV卡片								   */
/*  返回值  ：TRUE --> EMV卡片									   */
/*            FALSE--> 非EMV卡片						           */
/*=================================================================*/
U08 IsEmvCard(U08 *pszTrack2Data)
{
    U08 *pszSeperator;

    if (0 == *pszTrack2Data)
    {
        return FALSE;
    }

    pszSeperator = strchr(pszTrack2Data, '=');
    if (NULL == pszSeperator)
    {
        return FALSE;
    }
    if (('2'==pszSeperator[5]) || ('6'==pszSeperator[5]))
    {
        return TRUE;
    }

    return FALSE;
}

void ShowGetCardScreen(U08 ucMode)
{
    ucMode &= 0x7f;

    lcdClrLine(2, 7);
    switch (ucMode)
    {
    case CARD_SWIPED:
        lcdDisplayCE(0, 2, DISP_CFONT, "请刷卡-->       ", "SWIPE-->        ");
        return;

    case CARD_INSERTED:
        lcdDisplayCE(0, 2, DISP_CFONT, "请插入IC卡>>>   ", "INSERT IC>>>    ");
        return;

    case CARD_KEYIN:
        lcdDisplayCE(0, 2, DISP_CFONT, "请输入卡号:     ", "INPUT CARD NO:  ");
        return;

    case CARD_SWIPED|CARD_KEYIN:
        lcdDisplayCE(0, 2, DISP_CFONT, "请刷卡或输卡号  ", "SWIPE/INPUT:    ");
        return;

    case CARD_SWIPED|CARD_INSERTED:
        lcdDisplayCE(0, 2, DISP_CFONT, "请插入IC卡或刷卡", "INSERT/SWIPE:   ");
        return;

    case CARD_INSERTED|CARD_KEYIN:
        lcdDisplayCE(0, 2, DISP_CFONT, "请插IC卡或输卡号", "INSERT/INPUT:   ");
        return;

    case CARD_INSERTED|CARD_KEYIN|CARD_SWIPED:
        lcdDisplayCE(0, 2, DISP_CFONT, "请插/刷卡/输卡号", "INSERT/SWIPE/IN ");
        return;

    default:
        return;
    }
}

U08 DetectCardEvent(U08 ucMode)
{
	//磁头上电、打开、清缓冲
    if (0 != (ucMode&CARD_SWIPED))
    {
        magClose();
        magOpen();
        magReset();
    }
    kbFlush();
    while (1)
    {
        if (YES == kbhit())
        {
            return CARD_KEYIN; //有按键事件
        }
        if ((ucMode&CARD_SWIPED) && (YES==magSwiped()))
        {
            return CARD_SWIPED; //有刷卡事件
        }
    }
}

/****************************************************************************
 函数功能:    手工输入卡号，占用屏幕4到7行
****************************************************************************/
U08 ManualEntryCardNo(U08 *pszCardNo)
{
    U08 sBuff[21];
    int iRet;

    lcdClrLine(4, 7);
    lcdGoto(0, 4);
    iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_MAXLEN+KB_EN_SHIFTLEFT,
        13, 19, (stPosParam.iOprtLimitTime*1000), sBuff);
    if (iRet < 0)
    {
        return E_TRANS_CANCEL;
    }
    sprintf((char *)pszCardNo, "%.*s", iRet, sBuff);
    stPosParam.ucSwipedFlag = CARD_KEYIN;

    return OK;
}

/****************************************************************************
 功能描述:      校验年月合法性,日期格式为:YYMM
****************************************************************************/
int CheckYYMM(char *sYYMM)
{
    int m, i;
    char szMonth[3];

    for (i=0; i<4; i++)
    {
        if ((sYYMM[i]<'0') || (sYYMM[i]>'9'))
        {
            return 1;
        }
    }

    sprintf(szMonth, "%.2s", sYYMM+2);
    m = atoi(szMonth);

    if ((m>12) || (m<=0))
    {
        return 1;
    }

    return 0;
}

/****************************************************************************
 函数功能:    输入有效期
****************************************************************************/
U08 ManualEntryExpDate(U08 *szExpDate)
{
    U08 inBuf[6];
    char chkDate[9];
    int iRet;

    while (1)
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT,              "请输入卡有效期  ", "Expiry Date:    ");
        lcdDisplayCE(0, 4, DISP_CFONT|DISP_MEDIACY, "(年年月月):     ", "(YYMM):         ");
        memset(inBuf, 0, sizeof(inBuf));
        lcdGoto(80, 6);
        iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP,
            0, 4, (stPosParam.iOprtLimitTime*1000), inBuf);
        if (iRet < 0)
        {
            return E_TRANS_CANCEL;
        }
        if (0 == strlen((char *)inBuf)) //不输入有效期
        {
            break;
        }
        if (4 != strlen((char *)inBuf))
        {
            continue;
        }
        memcpy(chkDate, inBuf, 4);
        if (0 == CheckYYMM(chkDate))
        {
            strcpy(stPosParam.szExpDate, inBuf);
            break;
        }
        else
        {
            PubBeepErr();
            lcdClrLine(6, 7);
            lcdDisplayCE(0, 6, DISP_CFONT, "有效期格式错    ", "Expiry Date Err.");
            kbGetKeyMs(1000);
        }
    }
    return OK;
}

/****************************************************************************
函数功能: 从磁道中取卡号
****************************************************************************/
U08 GetCardNoFromTrack(U08 *szCardNo, U08 *track2)
{
    int i, j, k;

    for (j=0; j<20; j++)
    {
        if ((track2[j]>='0') && (track2[j]<='9'))
        {
            break;
        }
    }
    k = 0;
    for (i=j; i<20; i++)
    {
        if ((track2[i]>='0') && (track2[i]<='9'))
        {
            szCardNo[k] = track2[i];
            k++;
        }
        else
        {
            break;
        }
    }
    szCardNo[k] = 0;
    return OK;
}

/****************************************************************************
 函数功能: 在刷卡情况下，取磁道信息
****************************************************************************/
U08 GetTrackData(U08 *pszTrack1, U08 *pszTrack2, U08 *pszTrack3, U08 *pszCardNo)
{
    U08    ucRet;

    ucRet = magRead(pszTrack1, pszTrack2, pszTrack3);
    if (0x00 != (ucRet&0x70))
    {
        return E_ERR_SWIPE;
    }
    magClose();

    return GetCardNoFromTrack(pszCardNo, pszTrack2);
}

/************************************************************************
 * 刷卡事件处理函数
 ************************************************************************/
U08 SwipeCardProc(void)
{
    U08    ucRet;

	// 读磁道数据
    stPosParam.ucSwipedFlag = NO_SWIPE_INSERT;
    ucRet = GetTrackData(stPosParam.szTrack1, stPosParam.szTrack2, stPosParam.szTrack3, stPosParam.szCardNo);
    if (OK != ucRet)
    {
        return ucRet;
    }
    return OK;
}

void MaskPan(U08 *pszInPan, U08 *pszOutPan)
{
    U08 szBuff[30];
    int iCnt, iPanLen;

    memset(szBuff, 0, sizeof(szBuff));
    iPanLen = strlen((U08 *)pszInPan);
    for (iCnt=0; iCnt<iPanLen; iCnt++)
    {
        if ((iCnt<6) || ((iCnt>=iPanLen-4)&&(iCnt<iPanLen)))
        {
            szBuff[iCnt] = pszInPan[iCnt];
        }
        else
        {
            szBuff[iCnt] = '*';
        }
    }

    sprintf((U08 *)pszOutPan, "%.*s", LEN_PAN, szBuff);
}


/****************************************************************************
 函数功能: 显示卡号，在屏幕的第2行
****************************************************************************/
U08 DispCardNo(U08 *pszCardNo)
{
    U08 szTempCardNo[21];

    MaskPan(pszCardNo, szTempCardNo);
	/*
	lcdClrLine(2, 7);
	lcdDisplayCE(0, 2, DISP_CFONT, "卡号   CARD INFO", "       CARD INFO");

	if( strlen((char *)pszCardNo)>16 )
	{
		lcdDisplay(0, 4, DISP_ASCII, "%.20s", szTempCardNo);
	}
	else
	{
		lcdDisplay(0, 4, DISP_CFONT, "%s", szTempCardNo);
	}

	lcdDisplayCE(0, 6, DISP_CFONT, "    确认(ENTER)?", "        (ENTER)?");
	if( kbGetKeyMs(30*1000)!=KEY_ENTER )
	{
		return E_TRANS_CANCEL;
	}*/
    return OK;
}


U08 PosGetCard(U08 ucMode)
{
    U08    ucRet;

    if (NO_SWIPE_INSERT != stPosParam.ucSwipedFlag)
    {
        return OK;
    }

    while (1)
    {
        ShowGetCardScreen(ucMode);
        ucRet = DetectCardEvent(ucMode);
        if (CARD_KEYIN == ucRet)
        {
            if (0 != (ucMode&CARD_KEYIN))    // 允许输入卡号
            {
                ucRet = ManualEntryCardNo(stPosParam.szCardNo);
                if (OK != ucRet)
                {
                    return ucRet;
                }
                return ManualEntryExpDate(stPosParam.szExpDate);
            }
            else if (KEY_CANCEL == kbGetKey())
            {
                return E_TRANS_CANCEL;
            }
        }
        else if (CARD_SWIPED == ucRet)
        {
            ucRet = SwipeCardProc();
            if (OK == ucRet)
            {
                return DispCardNo(stPosParam.szCardNo);
            }
            else if (E_ERR_SWIPE == ucRet)    // 刷卡错误
            {
                lcdClrLine(2, 7);
                lcdDisplayCE(0, 2, DISP_CFONT, "刷卡错误        ", "  SWIPE ERROR   ");
                lcdDisplayCE(0, 4, DISP_CFONT, "请继续刷卡-->   ", "PLS SWIPE AGAIN ");
                lcdDisplayCE(0, 6, DISP_CFONT, "或按取消键退出  ", "[CANCEL] TO EXIT");
                kbGetKeyMs(2000);
            }
            else
            {
                return ucRet;    // 其他错误
            }
        }
    }
}

U08 AppGetAmount(int length, U08 flag)
{
    U08 abyBuff[17];
    int iRet, iTime;

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 2, DISP_CFONT, "请输入金额:     ", "INPUT AMOUNT:   ");
    memset(abyBuff, 0, sizeof(abyBuff));
    iTime = atoi(myParams[9].ptr); // Get timeout value
    stPosParam.iAmount = 0;
    do{
        lcdGoto(48, 6);
        iRet = kbGetString(KB_EN_NUM+KB_EN_FLOAT+KB_EN_BIGFONT+KB_EN_REVDISP, 1, length, (iTime*1000), abyBuff);
        if (iRet < 0)
        {
            stPosParam.iAmount = 0;
            return E_TRANS_CANCEL;
        }
        stPosParam.iAmount = atol((char *)abyBuff);
    }while (0 == stPosParam.iAmount);
    return OK;
}

U08 SaleTranGetData(int iNeedAmount)
{
    U08 ucRet;

    if (0 != iNeedAmount)
    {
        ucRet = AppGetAmount(9, TRAN_AMOUNT);
        if (OK == ucRet)
        {
            return ucRet;
        }
    }

//    ucRet = PosGetCard(CARD_SWIPED|CARD_KEYIN);
    ucRet = PosGetCard(CARD_SWIPED);
    if (OK == ucRet)
    {
        return ucRet;
    }

    return OK;
}

U08 ExtractPAN(U08* cardno, U08* pan)
{
    int len;

    len = strlen((char*)cardno);
    if ((len<13) || (len>19))
    {
        return E_ERR_SWIPE;
    }

    memset(pan, '0', 16);

    memcpy(pan+4, cardno+len-13, 12);
    pan[16] = 0;

    return OK;
}

// Standard CARD input PIN
U08 EnterCardPIN(U08 ucFlag, const U08 *pbyCardNo, U08 *pbyPinBlockOut)
{
    U08 PAN[30], ucRet;
    int iRet;

    lcdCls();
    lcdDisplayCE(0, 2, DISP_CFONT, "请输密码：", "INPUT CARD PIN:");
    lcdGoto(0, 5);

    ucRet = ExtractPAN((U08*)pbyCardNo, PAN);
    if (OK == ucRet)
    {
        return ucRet;
    }

    iRet = PedGetPin(DOUBLE_PIN_KEY_ID, "0, 4, 5, 6", PAN, FORMAT_0, pbyPinBlockOut);
    switch (iRet)
    {
    case PED_RET_OK:
        return OK;

    case PED_RET_NOPIN:
//20080716 IC卡部分NOPIN要返回PED_RET_NOPIN
        if (0x00 != (ucFlag&0x80))
        {
            return PED_RET_NOPIN;
        }
        else
        {
            return OK;
        }

    case PED_RET_CANCEL:
    case PED_RET_TIMEOUT:
        return E_TRANS_CANCEL;
    case PED_RET_NOKEY:
        return PED_RET_NOKEY;
    default:
        return E_PINPAD_KEY;
    }
    return OK;

}

void ErrorBeep(void)
{
    sysBeef(6, 200);
    sysDelayMs(200);
}

/****************************************************************************
Function     :  void WritePedKey(const U08 *pbyMasterKeyPlainText,
    const U08 *pbyMacKeyCipherText, const U08 *pbyPinKeyCipherText)
Descript     :  Save master key, MAC key and PIN key
Input param. :  1. const U08 *pbyMasterKeyPlainText : Master key data 16 bytes
                2. const U08 *pbyMacKeyCipherText : MAC key data 16 bytes
                3. const U08 *pbyPinKeyCipherText : PIN key data 16 bytes
Output param.:  NULL
Return       :  NULL
  History    :
      Author       date time   version      reason
  1. Ryan Huang    2011-05-17  V1.0         Create
****************************************************************************/
void WritePedKey(const U08 *pbyMasterKeyPlainText,
    const U08 *pbyMacKeyCipherText, const U08 *pbyPinKeyCipherText)
{
    int iRet;

    iRet = PedWriteMasterKey(PARITY_NONE+TDEA_NONE, 1, MASTER_KEY2, 16, (U08*)pbyMasterKeyPlainText);
    if (PED_RET_OK != iRet)
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 4, DISP_CFONT|DISP_MEDIACY, "  装主密钥错误  ", " LOAD KEY ERROR ");
        ErrorBeep();
        kbGetKeyMs(3000);
    }

    iRet = PedWritePinKey(PARITY_NONE, MASTER_KEY2, DOUBLE_PIN_KEY_ID, 16, (U08*)pbyPinKeyCipherText);
    if (PED_RET_OK != iRet)
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 4, DISP_CFONT|DISP_MEDIACY, " 装载PPK密钥错误", " LOAD PPK ERROR ");
        ErrorBeep();
        kbGetKeyMs(3000);
    }

    iRet = PedWriteMacKey(PARITY_NONE, MASTER_KEY2, DOUBLE_MAC_KEY_ID, 16, (U08*)pbyMacKeyCipherText);
    if (PED_RET_OK != iRet)
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 4, DISP_CFONT|DISP_MEDIACY, " 装载MAC密钥错误", " LOAD PPK ERROR ");
        ErrorBeep();
        kbGetKeyMs(3000);
    }
}

void FailBeep(void)
{
    sysBeef(6, 700);
}


void DispFileErrInfo(void)
{
    int err;

    err = errno;

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 2, DISP_CFONT, "    文件错误    ", "  FILE ERROR    ");
    switch (err)
    {
    case FILE_EXIST:
        lcdDisplayCE(0, 6, DISP_CFONT, "  文件已存在    ", "FILE EXISTED    ");
        break;
    case FILE_NOEXIST:
        lcdDisplayCE(0, 6, DISP_CFONT, "  文件不存在    ", " FILE NO EXIST  ");
        break;
    case MEM_OVERFLOW:
        lcdDisplayCE(0, 6, DISP_CFONT, "   空间不足     ", "  MEM OVERFLOW  ");
        break;
    case TOO_MANY_FILES:
        lcdDisplayCE(0, 6, DISP_CFONT, "   文件太多     ", " TOO MANY FILES ");
        break;
    case INVALID_HANDLE:
        lcdDisplayCE(0, 6, DISP_CFONT, "   无效句柄     ", " INVALID HANDLE ");
        break;
    case INVALID_MODE:
        lcdDisplayCE(0, 6, DISP_CFONT, "   无效模式     ", "  INVALID MODE  ");
        break;
    case FILE_NOT_OPENED:
        lcdDisplayCE(0, 6, DISP_CFONT, "  文件未打开    ", "   NOT OPENED   ");
        break;
    case END_OVERFLOW:
        lcdDisplayCE(0, 6, DISP_CFONT, "   后移溢出     ", "  END OVERFLOW  ");
        break;
    case TOP_OVERFLOW:
        lcdDisplayCE(0, 6, DISP_CFONT, "   前移溢出     ", "  TOP OVERFLOW  ");
        break;
    case NO_PERMISSION:
        lcdDisplayCE(0, 6, DISP_CFONT, "   权限不作     ", " NO_PERMISSION  ");
        break;
    default:
        lcdDisplayCE(0, 6, DISP_CFONT, "   其它错误     ", "  OTHER ERROR   ");
        break;
    }
    FailBeep();
    kbGetKeyMs(3000);
}



int  SaveAppParam(void)
{
    int fd;
    int iRet;

    fd = fileOpen(FILE_APP_LOG, O_RDWR|O_CREAT);
    if (fd < 0)
    {
        DispFileErrInfo();
        return E_MEM_ERR;
    }

    iRet = fileSeek(fd, (int)0, SEEK_SET);
    if (iRet < 0)
    {
        DispFileErrInfo();
        fileClose(fd);
        return E_MEM_ERR;
    }
    iRet = fileWrite(fd, (U08 *)&stPosParam, sizeof(struct  _NEWPOS_PARAM_STRC));
    if (iRet != sizeof(struct _NEWPOS_PARAM_STRC))
    {
        DispFileErrInfo();
        fileClose(fd);
        return E_MEM_ERR;
    }
    fileClose(fd);
    return OK;
}


int LoadAppParam(void)
{
    int fd;
    int iRet;

    fd = fileOpen(FILE_APP_LOG, O_RDWR);
    if (fd < 0)
    {
        DispFileErrInfo();
        return E_MEM_ERR;
    }
    iRet = fileSeek(fd, (int)0, SEEK_SET);
    if (iRet < 0)
    {
        DispFileErrInfo();
        fileClose(fd);
        return E_MEM_ERR;
    }

    iRet = fileRead(fd, (U08 *)&stPosParam, sizeof(struct _NEWPOS_PARAM_STRC));
    fileClose(fd);

    if (iRet != sizeof(struct _NEWPOS_PARAM_STRC))
    {
        DispFileErrInfo();
        return E_MEM_ERR;
    }

    return OK;
}





