
#ifndef     _COMM_H_
#define     _COMM_H_

#include "modem_iface.h"

#ifndef U08
#define U08 unsigned char
#endif

#ifndef U16
#define U16  unsigned short
#endif

#ifndef U32
#define U32 unsigned int
#endif


#ifndef ERR_BASE
#define ERR_BASE            0x10000
#endif
#define ERR_USERCANCEL        (ERR_BASE+0x06)

#define NET_ERR_RETRY                1000
#define ERR_COMM_INV_PARAM         0x0001        // 参数错误
#define ERR_COMM_INV_TYPE          0x0002        // 无效通讯类型
#define ERR_COMM_CANCEL            0x0003        // 用户取消
#define ERR_COMM_TIMEOUT           0x0004        // 通讯超时
#define ERR_COMM_COMERR            0x0005
#define ERR_COMM_TOOBIG            0x0006
#define ERR_COMM_MODEM_NOPARAM     0x0009        // 设置参数失败

#define _TCP_BCD_LEN_BYTE          0x01  // TCP通讯,BCD 格式的长度字节


#define   MODEM_PATHNAME_NEW8110       "/dev/modem0"
#define   MODEM_PATHNAME_NEW6110       "/dev/ttyS1"

#define COMM_TYPE_NOT_SET  0
#define COMM_TYPE_GPRS     1
#define COMM_TYPE_WIFI     2
#define COMM_TYPE_PSTN     3
#define COMM_TYPE_CDMA     4
#define COMM_TYPE_PSTN_PPP 5
#define COMM_TYPE_LAN      6
#define COMM_TYPE_RS232    7
#define COMM_TYPE_MAX      7

// for RS232 communication
#define STX             0x02
#define ETX             0x03
#define ENQ             0x05
#define ACK             0x06
#define NAK             0x15

#define ERR_COMM_RS232_BASE        0x01000        // RS232类错误
#define ERR_COMM_MODEM_BASE        0x02000        // Modem类错误
#define ERR_COMM_TCPIP_BASE        0x04000        // TCPIP类错误
#define ERR_COMM_WIRELESS_BASE     0x08000        // GPRS/CDMA类错误
#define ERR_COMM_WIFI_BASE         0x10000        // WIFI类错误

#define ERR_COMM_STATUS_TIMEOUT    0x0007      // 获取状态超时
#define ERR_COMM_MODEM_INIT        0x0008      // 初始化失败

#define PSTN_MODEM_OPERATE_NOT_FINISH  0x80000000


#define POS_DEVICE_INFO    "_PDI.sys"

/********************** Internal structure declaration *********************/
typedef struct _tagERR_INFO
{
    INT32 iErrCode;
    char *szChnMsg;
    char *szEngMsg;
}ERR_INFO;

// 通讯模块错误信息
typedef struct _tagCOMM_ERR_MSG
{
    char    *szChnMsg;        // 中文错误信息
    char    *szEngMsg;        // 英文错误信息
}COMM_ERR_MSG;



// RS232 configure parameter
typedef struct _RS232_PARAM_
{
    U08 byPortNo;      // Port #, COM1, COM2 ....
    U08 bySendMode;    // Data send mode
    U08 szAttr[20+2];  // Serial port open parameter, "9600,8,n,1","115200,8,n,1", ....
}RS232_PARAM;

// PSTN configure parameter
typedef struct _PSTN_PARAM_
{
    U08 bySendMode;             // Data send mode
    U08 byPreDial;              // 0:Open, 1:Close
    U08 bySupportPPP;           // PPP falg, 1:support, 0:not
    U08 szTxnTelNo[100+1];      // dial number
    U08 szAPN[64+1];  // dial APN, only for PSTN PPP
    U08 szUID[64+1];  // dial user name, only for PSTN PPP
    U08 szPwd[16+2];  // dial user password, only for PSTN PPP
    ModemDialParms_t stDialPara; // modem dial parameter
}PSTN_PARAM;

// TCP/IP配置参数
typedef struct _TCPIP_PARAM_
{
    U08 szNetMask[24+1];
    U08 szGatewayIP[24+1];
    U08 szLocalIP[24+1];
    U08 szDNS1[32+1];
    U08 szDNS2[32+4];
}TCPIP_PARAM;

// GPRS/CDMA configurations
typedef struct _WNET_PARAM_
{
    U08 szAPN[64+1];  ///拨号号码 CDMA: #777; GPRS: cmnet
    U08 szUID[64+1];
    U08 szPwd[16+1];
    U08 szSimPin[16+1];     // SIM card PIN
    U08 szDNS[32+4];
}WNET_PARAM;

// WIFI  AP 配置参数
typedef struct _WIFI_PARAM_
{
    int iChannel;                  //  Wireless LAN communication channel(0 - 13)－－通讯信道
    int iSecurityType;             //  Sets the Wireless LAN security type. －－ 加密模式
    int iWEPKeyIdx;                //  WEP key index (1 - 4) -- WEP密钥组
    char szListWEPKey[4][32];       //  4 sets of WEP Key  --- 密钥组密钥
    char szWPAPSK[64];              //  Personal Shared Key Pass-Phrase,
    char szSSID[64];                //  SSID
}WIFI_PARAM;

// FTP 配置参数
typedef struct _FTP_PARAM_
{
    char strIpAddress[32];
    char strPortNum[8];
    char strUserName[32];
    char strPassword[32];
    char strUpRemoteFile[256];
    char strUpLocalFile[16];
    char strDownRemoteFile[256];
    char strDownLocalFile[16];
}FTP_PARAM;

// WIRELESS 配置参数
typedef struct _COMM_CFG_PARAM_
{
    U08 byComType;  // 0:error; 1:TCPIP; 2:WIFI; 3:GPRS; 4:CDMA; 5:PSTN; 6:RS232
    U08 byTcpipDevType; // WNET=1, ETH=2, WIFI=3, MODEM(PSTN PPP)=4
    U08 byTcpHeadLanType; // 1:BCD, 0:
    U08 szRemoteIP[24+1];
    U08 szTermID[16];
    U08 szPayment[8];
    U08 szTopup[8];
    U08 szBalance[8];
    U08 szSetPin[8];
    U08 abyDevInfo[64]; // [2]:modem, [8]:LAN,[9]:GPRS, [10]:CDMA,[11]:WIFI
    int nRemotePort;
    U08 byUdpFlag;    // 0:TCP, 1:UDP
    U08 szRev[3];
    RS232_PARAM oRs232;
    PSTN_PARAM oPstn;
    WNET_PARAM oGprsCdma;
    TCPIP_PARAM oWifi;
    TCPIP_PARAM oTcpIp;
    FTP_PARAM oFtp;
}COMM_CFG_PARAM;


#endif

