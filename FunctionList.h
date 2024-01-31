
#ifndef _FUNCTION_LIST_H_
#define _FUNCTION_LIST_H_

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#include "MacroDefine.h"
#include "Comm.h"
#include "Lcd.h"

#if  (SUPPORT_OPENSSL_RSA|SUPPORT_OPENSSL_CONNECT)
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#endif

void PubBeepErr(void);

void Bcd2Dword(U08 *pbyIn, int iLen, U32 *pdwOut);

void Byte2Dword(U08 *pbyIn, int iLen, U32 *pdwOut);


int GetKeyBlockMs(U32 uiTimeOutMs);

void lcdDisplayCE(U08 col, U08 row, U08 mode, const char *pCHN , const char *pEN);

U08 LcdGetItemTimeoutCE(const U08 *pbyChnItemString, const U08 *pbyEngItemString,
             U08 byItemNumber, U08 byExitMode, int iTimeoutMs, U08 *pbyCurPage);


U08 LcdGetItemBigTimeout(const U08 *pbyItemString, U08 byItemNumber, U08 byCurItem,
                U08 byMode, int iTimeoutMs, U08 *pbyCurPage);

int LcdTwoSelectOneShort(const U08 *pbyTitle, const U08 *pbyMsg1, const U08 *pbyMsg2, int iCurMsg,
            int iTimeoutMs, U08 byMode);

void   DrawRect(U32 uiBegRow, U32 uiBegCol, U32 uiEndRow, U32 uiEndCol);

void Display2Strings(const char *pszStringHz, const char *pszStringEn);

void CommTypeSelect(void);

INT32 CommOnHook(U08 byReleaseAll);

void StartInitComm(void);

void CommParamSet(void);

void UdpTcpSelect(void);

int RsaSend(const char *strSendMsg, U32 uiTimeOutMs);

int RsaRecv(U08 *psRxdData, U32 uiTimeOutMs, U32 *puiOutLen);


INT32 CommDial(U08 ucDialMode);


void lcdDisplayExtCE(U08 col, U08 row, U08 mode, int iNum, char *pEN);

int ReadParamConfigure(void);

int ReadMessageConfigure(void);

U08 SaleTranGetData(int iNeedAmount);

int LoadAppParam(void);

int SaveAppParam(void);

int SetTcpIpParam(const U08 *pbyChnTitle, const U08 *pbyEngTitle);


int SaveCommCfg(const COMM_CFG_PARAM *psCfg);


int GetIPAddress(U08 bAllowNull, U08 *pszIPAddress, U08 *pbyIp);

int InitFtpParamter(FTP_PARAM *pFtpParam);

void middle_print(int line, int mode, const char *prompt);

int MenuSelect(MENU_SELECT *psMenu);

#if SUPPORT_OPENSSL_CONNECT
int SslConnect(SSL_CTX **ctx, SSL **ssl);

int SslTxd(SSL **ssl, const U08 *psTxdData, U32 uiDataLen);

int SslRxd(SSL **ssl, U08 *psRxdData, U32 uiExpLen, U32 uiTimeOutMs, U32 *puiOutLen);

int SslClose(SSL_CTX **ctx, SSL **ssl);
#endif

#endif

