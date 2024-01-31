
#include "Posapi.h"
#include "Param.h"
#include "Comm.h"
#include "Base.h"

#include "MacroDefine.h"
#include "FunctionList.h"

extern COMM_CFG_PARAM g_oCommCfgParam;

U08 *g_pyMsgParamFilename = "cfg.ini";


#define MESSAGE_BUFF_LEN  (20*1024)


PARAMS myParams[MESSAGE_PARAM_NUM] =
{
    {"RemoteIp",   "41.215.22.90",    0, STRING_ITEM},      // 0
    {"RemotePort", "3800",             0, STRING_ITEM},      // 1
    {"RsaFile",    "PUB.PEM",          0, STRING_ITEM},      // 2
    {"TermID",     "00000000",         0, STRING_ITEM},      // 3
    {"Payment",    "1210",             0, STRING_ITEM},      // 4
    {"Topup",      "1120",             0, STRING_ITEM},      // 5
    {"Balance",    "1230",             0, STRING_ITEM},      // 6
    {"SetPIN",     "1240",             0, STRING_ITEM},
    {"MSG009",     "Initial COMM... ", 0, STRING_ITEM},      // 8
    {"OptTime",    "120",              0, STRING_ITEM},      // 9
    {"Logo",       "LOGO.BMP",         0, STRING_ITEM},      // 10
    {"Langue",     "0",                0, STRING_ITEM},
    {"MSG013",     "",                 0, STRING_ITEM},      // 12
    {"MSG014",     "",                 0, STRING_ITEM},
    {"dot",        ",",                0, STRING_ITEM},      // 14
    {"CommType",   "w",                0, STRING_ITEM},      // 15
    {"GprsAPN",    "safaricom",            0, STRING_ITEM},      // 16
    {"GprsUID",    "saf",             0, STRING_ITEM},      // 17
    {"GprsPwd",    "data",             0, STRING_ITEM},      // 18
};



int ReadParamConfigure(void)
{
    int fd, iRet, iFileSize, i, iLen;
    U08 abySearch[MAX_MSG_NAME_LEN+4];
    U08 abyMsgBuff[MESSAGE_BUFF_LEN];

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "读取消息配置文件", " Loading... ");
    lcdDisplay(0, 2, 0x01, "Please Wait...");
    iFileSize = fileSize(g_pyMsgParamFilename);
    if ((iFileSize<20) || (iFileSize>=MESSAGE_BUFF_LEN))
    {
        return 0;
    }
    fd = fileOpen(g_pyMsgParamFilename, O_RDWR);
    if (fd < 0)
    {
        lcdDisplayCE(0, 4, 0x01, "  打开文件错误  ", "Open file failed");
        kbGetKeyMs(KEY_BLOCK_TIME);
        return 2;
    }
    iRet = fileSeek(fd, 0, SEEK_SET);
    if (iRet < 0)
    {
        fileClose(fd);
        lcdDisplayCE(0, 4, 0x01, "  定位文件错误  ", "Seek file failed");
        kbGetKeyMs(KEY_BLOCK_TIME);
        return 3;
    }
    iRet = fileRead(fd, abyMsgBuff, iFileSize);
    fileClose(fd);
    if (iFileSize != iRet)
    {
        lcdDisplayCE(0, 4, 0x01, "  读取文件错误  ", "Read file failed");
        lcdDisplay(0, 6, 0x01, "iRet=%d", iRet);
        kbGetKeyMs(KEY_BLOCK_TIME);
        return 4;
    }
    abyMsgBuff[iFileSize] = 0;
    for (i=0; i<MESSAGE_PARAM_NUM; i++)
    {
        iLen = strlen(myParams[i].Name);
        abySearch[0] = '<';
        abySearch[1] = '<';
        strcpy(&abySearch[2], myParams[i].Name);
        abySearch[iLen+2] = '>';
        abySearch[iLen+3] = 0;
        iRet = SearchString(abyMsgBuff, iFileSize, &abySearch[1], iLen+2, 0);
        if (iRet > 0)
        {
//            myParams[i].ptr = &g_abyMsgBuff[iRet];
            memcpy(myParams[i].ptr, &abyMsgBuff[iRet], sizeof(myParams[i].ptr));
            abySearch[1] = '/';
            iRet = SearchString(&abyMsgBuff[iRet], iFileSize-iRet, abySearch, iLen+3, 1);
            if (iRet >= 0)
            {
                if ((iRet-2) > MAX_MSG_DATA_LEN)
                {
                    iRet = MAX_MSG_DATA_LEN-2;
                }
                myParams[i].wPtrLen = iRet + 1;
                myParams[i].ptr[iRet + 1] = 0;
            }
        }
    }
    fileRemove(g_pyMsgParamFilename);

    strcpy(g_oCommCfgParam.szRemoteIP, myParams[0].ptr);
    g_oCommCfgParam.nRemotePort = atoi(myParams[1].ptr);
    strcpy(g_oCommCfgParam.szTermID, myParams[3].ptr);
    memcpy(g_oCommCfgParam.szPayment, myParams[4].ptr, 4);
    g_oCommCfgParam.szPayment[4] = 0;
    memcpy(g_oCommCfgParam.szTopup, myParams[5].ptr, 4);
    g_oCommCfgParam.szTopup[4] = 0;
    memcpy(g_oCommCfgParam.szBalance, myParams[6].ptr, 4);
    g_oCommCfgParam.szBalance[4] = 0;
    memcpy(g_oCommCfgParam.szSetPin, myParams[7].ptr, 4);
    g_oCommCfgParam.szSetPin[4] = 0;

    if (('w'==myParams[15].ptr[0]) || ('W'==myParams[15].ptr[0]))
    {
        g_oCommCfgParam.byComType = COMM_TYPE_WIFI;
    }
    else if (('g'==myParams[15].ptr[0]) || ('G'==myParams[15].ptr[0]))
    {
        g_oCommCfgParam.byComType = COMM_TYPE_GPRS;
        strcpy(g_oCommCfgParam.oGprsCdma.szAPN, myParams[16].ptr);
        strcpy(g_oCommCfgParam.oGprsCdma.szUID, myParams[17].ptr);
        strcpy(g_oCommCfgParam.oGprsCdma.szPwd, myParams[18].ptr);
    }
    else
    {
        g_oCommCfgParam.byComType = COMM_TYPE_WIFI;
    }

    SaveCommCfg(&g_oCommCfgParam);
    return 0;
}









