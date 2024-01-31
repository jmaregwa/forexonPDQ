#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include "MacroDefine.h"
#include "font.h"
#include <time.h>
#if  (SUPPORT_OPENSSL_RSA|SUPPORT_OPENSSL_CONNECT)
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#endif

#define  NO_DISP          36
#include "Posapi.h"
#include "Comm.h"
#include "Lcd.h"
#include "Base.h"
#include "FunctionList.h"
#define   COMM_USER_OPER_TIMEOUT 30000
#include "basefunc.h"
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h> 
#include "alloc.h"
#include <net/if.h>
#include "QR.h"
#include "pdf417.h"

COMM_CFG_PARAM g_oCommCfgParam;

static int g_iCurModule = 0;// 0：当前选中模块槽位0；1：当前选中模块槽位1。只有在设备同时支持GPRS、和WIFI时才有用

#define   LEN_TEMP_BUFF            (1024*4)
static U08 g_abySetBuff[LEN_TEMP_BUFF];
#ifndef     ON
#define     ON                  1
#endif

#ifndef     OFF
#define     OFF                 0
#endif

#define MANU_OPT_MAINB          0       //  主板测试
#define MANU_OPT_PORTB          1       //  接口板测试
#define MANU_OPT_MOD            2       //  模块入库检查,sam板
#define MANU_OPT_FULL           4       //  整机测试
#define MANU_OPT_MENU           5       //  单项测试
#define MANU_OPT_INTERNAL       10 
UINT32 datefilename[256],hexs[50],rateset[50],general[100],currencyfrom[50],currencyto[50],timenow[200],amountrec[50],recvstr[256],userid[9],invoiceno[20],Lstate[20],fullname[25],state[50];
UINT32 sendstr[200],amnt[10],currency[20],date[20],identification[20],idno[20],nationality[100],cname[100];
UINT32 ip[100],port[10];
int forextype=0, fromselected=0,closing=0, changerate=0,records=0,sent=0,synchstatus=0,synctotalrecords=0,totalsynched=0,presentrecord=0,synchedrecord=0,getnbal=0;
double rate=0,amountin=0, amountout=0;
UINT8 logfile[100],transfile[100];
char g_szMonthName1[13][4] = {"   ","JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

extern int g_iLangueSelect;
UINT32  bIsChnFont = 0;           		    // 当前是否使用中文字体
UINT32  k_MenuLanguage = 1;      
I32 CheckChn(U32 Mode)
{
    if ((0!=bIsChnFont) && (0!=(Mode&DISP_CFONT)) && (1==k_MenuLanguage))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void lcdDispMultiLang(I32 Col, I32 Line, U32 Mode, U08 *ch, U08 *en)
{
    if (CheckChn(Mode))
    {
        lcdDisplay(Col, Line, Mode, ch);
    }
    else
    {
        lcdDisplay(Col, Line, Mode, en);
    }
}
typedef struct _transaction_record_{
    U08 m_abyTransData[200];
}TRANS_RECORD;
typedef struct _transaction_record1_{
    U08 m_abyTransData1[200];
}LOGS_RECORD;
typedef struct _transaction_record2_{
    U08 m_abyTransData2[200];
}CONFIG_RECORD;
#define RECORD_FILE "TRANS_RECORD"
#define RECORD_FILE "LOGS_RECORD"
#define RECORD_FILE "CONFIG_RECORD"

int SaveNewRecord(const TRANS_RECORD *psRecord);
int SaveNewRecordLogs(const LOGS_RECORD *psRecord);

int SaveNewRecordsettings(const CONFIG_RECORD *psRecord)
{
U08 TimeStr[64];
 
sysGetTime(TimeStr);

 sprintf(date,"%02x%02x%02x",TimeStr[0],TimeStr[1],TimeStr[2],TimeStr[3],TimeStr[4],TimeStr[5]);
 
 sprintf(logfile,"logs%s",date);
    int fd, iFileLen, i;
    fd = fileOpen(logfile, O_RDWR);
    if (fd < 0)
    {
        fd = fileOpen(logfile, O_CREAT);
        if (fd < 0)
        {
            lcdDisplay(0, 4, 0x01, (U08*)"Create file failed.");
            kbGetKeyMs(1000);
            return -1;
        }
		
    }
    iFileLen = fileSize(logfile);
    i = iFileLen%sizeof(LOGS_RECORD);
	
    if (0 != i)
    {
	
        i = iFileLen - i;
        fileSeek(fd, i, SEEK_SET);
		
    }
    else
    {    
        fileSeek(fd, 0, SEEK_END);
    }
    fileWrite(fd, psRecord, sizeof(LOGS_RECORD));
	records= records+1;
	
    fileClose(fd);
	
    return 0;// success
}
int SaveNewRecord(const TRANS_RECORD *psRecord)
{
U08 TimeStr[64];
 
sysGetTime(TimeStr);

 sprintf(date,"%02x%02x%02x",TimeStr[0],TimeStr[1],TimeStr[2],TimeStr[3],TimeStr[4],TimeStr[5]);
 sprintf(transfile,"trans%s",date);
    int fd, iFileLen, i;
    fd = fileOpen(transfile, O_RDWR);
    if (fd < 0)
    {
        fd = fileOpen(transfile, O_CREAT);
        if (fd < 0)
        {
            lcdDisplay(0, 4, 0x01, (U08*)"Create file failed.");
            kbGetKeyMs(1000);
            return -1;
        }
		/*prnInit();
				prnPrintf("file created:%s",g_pbyTransFileName);
				prnStart();*/
    }
    iFileLen = fileSize(transfile);
    i = iFileLen%sizeof(TRANS_RECORD);
	//prnInit();
				//prnPrintf("filelength:%d",i);
				//prnStart();
    if (0 != i)
    {
	/*prnInit();
				prnPrintf("file not equal to zero\n");
				prnStart();*/
        i = iFileLen - i;
        fileSeek(fd, i, SEEK_SET);
		
    }
    else
    {    /*prnInit();
				prnPrintf("file lenght equal to null\n");
				prnStart();*/
        fileSeek(fd, 0, SEEK_END);
    }
    fileWrite(fd, psRecord, sizeof(TRANS_RECORD));
	records= records+1;
	
    fileClose(fd);
	/*prnInit();
				prnPrintf("file written:%s\n",psRecord);
				prnPrintf("number of records:%d\n",records);
				prnStart();*/
    return 0;// success
}
int ReadRecordSetting(int iRecordIndex, TRANS_RECORD *psRecord)
{
    int fd, iFileLen,i; 
	int j= 0,k=0;
	int tamount = 0;
	//slt=atoi(slots);
	//int i= 0;
	memset(psRecord,0x00,sizeof(psRecord));
	
    fd = fileOpen("setting", O_RDWR);
    if (fd < 0)
    {
	
        //kbGetKeyMs(1000);
		fd = fileOpen("setting", O_CREAT);
		prnInit();
		prnPrintf("creating...\n");
		prnStart();
        if (fd < 0)
        {
            lcdDisplay(0, 4, 0x01, (U08*)"Create file failed.");
            kbGetKeyMs(1000);
            return -1;
        }
		i = 0*sizeof(CONFIG_RECORD);
	 k=1*sizeof(CONFIG_RECORD);
    if (iFileLen < i)
    {
        //lcdDisplay(0, 4, 0x01, (U08*)"There has no record.");
        //kbGetKeyMs(1000);
        return 0;
    }
    fileSeek(fd, i, SEEK_SET);
    fileRead(fd, psRecord, sizeof(CONFIG_RECORD));
	fileSeek(fd, i, SEEK_SET);
	fileWrite(fd, "197.248.118.113", sizeof(CONFIG_RECORD));
	fileSeek(fd, k, SEEK_SET);
    fileRead(fd, psRecord, sizeof(CONFIG_RECORD));
	fileSeek(fd, k, SEEK_SET);
	fileWrite(fd, "9890", sizeof(CONFIG_RECORD));
	lcdCls();
	prnInit();
	prnPrintf("Setting..197.248.118.113,9890...\n");
	prnStart();
	
	
	
        lcdDisplay(0, 4, 0x01, (U08*)"Set Defults...");
		sysDelayMs(1000);
        //return -1;
    }
	

	
    iFileLen = fileSize("setting");
	iRecordIndex=1;
	//presentrecord=iRecordIndex;
    i = 0*sizeof(CONFIG_RECORD);
	j= 1*sizeof(CONFIG_RECORD);
    if (iFileLen < i)
    {
        lcdDisplay(0, 4, 0x01, (U08*)"There is no record.");
        kbGetKeyMs(1000);
        //return 0;
    }
	
    fileSeek(fd, i, SEEK_SET);
    fileRead(fd, psRecord, sizeof(CONFIG_RECORD));
	strcpy(ip,psRecord);
	sprintf(ip,"%s",psRecord);
	fileSeek(fd, j, SEEK_SET);
    fileRead(fd, psRecord, sizeof(CONFIG_RECORD));
	strcpy(port,psRecord);			
	sprintf(port,"%s",psRecord);		
				
	
   
	fileClose(fd);
    
	return 1;
}
int ReadRecords(int iRecordIndex, CONFIG_RECORD *psRecord)
{
    int fd, iFileLen,i; 
	int j= 0;
	int damount=0;
	
	//int i= 0;
	memset(psRecord,0x00,sizeof(psRecord));
	
    fd = fileOpen("setting", O_RDWR);
	
	
	
		
	
    if (fd < 0)
    {
        lcdDisplay(0, 4, 0x01, (U08*)"Open file failed.");
        kbGetKeyMs(1000);
        return -1;
    }
	
	
    iFileLen = fileSize("setting");
	
   // iFileLen = fileSize("rates");
	iRecordIndex=j;
	
    i = iRecordIndex*sizeof(CONFIG_RECORD);
    if (iFileLen < i)
    {
        
        return 0;
    }
    fileSeek(fd, i, SEEK_SET);
    fileRead(fd, psRecord, sizeof(CONFIG_RECORD));
	prnInit();
	prnPrintf("psread %s\n",psRecord);
		prnStart();		
				
				
	
	fileClose(fd);
    
	return 1;
}



int Configupdate(int iRecordIndex,UINT8* ip, UINT8* port,CONFIG_RECORD *psRecord)
{

    int fd, iFileLen,i,k; 
	int j= 0;
	UINT8 writedata[256];
	//int i= 0;
	memset(psRecord,0x00,sizeof(psRecord));
	prnInit();
	prnPrintf("opening config...\n");
	prnStart();
    fd = fileOpen("setting", O_RDWR);
        if (fd < 0)
    {
        fd = fileOpen("setting", O_CREAT);
        if (fd < 0)
        {
            lcdDisplay(0, 4, 0x01, (U08*)"Create file failed.");
            kbGetKeyMs(1000);
            return -1;
        }
    }
	//for (j = 0; j <= records; j++)
	//{
	
	/*prnInit();
				prnPrintf("file opened");
				prnStart();*/
    iFileLen = fileSize("setting");
	//iRecordIndex=j;
	//prnInit();
				//prnPrintf("file lenght:%d\n",iFileLen);
				//prnPrintf("record index:%d\n",iRecordIndex);
				//prnPrintf("j:%d\n",j);
				//prnStart();
    i = 0*sizeof(CONFIG_RECORD);
	k=1*sizeof(CONFIG_RECORD);
    if (iFileLen < i)
    {
        //lcdDisplay(0, 4, 0x01, (U08*)"There has no record.");
        //kbGetKeyMs(1000);
        //return 0;
    }
    
	fileSeek(fd, i, SEEK_SET);
	fileWrite(fd, ip, sizeof(CONFIG_RECORD));
	
	fileSeek(fd, k, SEEK_SET);
	fileWrite(fd, port, sizeof(CONFIG_RECORD));
	/*prnInit();
	prnPrintf("record read:%d\n",iRecordIndex);
				prnPrintf("file read:%s",psRecord);
				prnStart();
				prnPrintf("reading after done:%d\n",iRecordIndex);
				prnPrintf("file read:%s",psRecord);
				prnStart();*/
				
				//ReadRecord(iRecordIndex,readdata1);
	fileClose(fd);
    
	return 1;
}
int ReadRecordappend(int iRecordIndex, TRANS_RECORD *psRecord)
{

    int fd, iFileLen,i; 
	int j= 0;
	UINT8 writedata[256];
	//int i= 0;
	memset(psRecord,0x00,sizeof(psRecord));
	strcpy(writedata,"|STX|SENT|ETX|\n");
    fd = fileOpen("trans", O_RDWR);
    if (fd < 0)
    {
        //lcdDisplay(0, 4, 0x01, (U08*)"Open file failed.");
        //kbGetKeyMs(1000);
        return -1;
    }
	//for (j = 0; j <= records; j++)
	//{
	
	/*prnInit();
				prnPrintf("file opened");
				prnStart();*/
    iFileLen = fileSize("trans");
	//iRecordIndex=j;
	//prnInit();
				//prnPrintf("file lenght:%d\n",iFileLen);
				//prnPrintf("record index:%d\n",iRecordIndex);
				//prnPrintf("j:%d\n",j);
				//prnStart();
    i = iRecordIndex*sizeof(TRANS_RECORD);
    if (iFileLen < i)
    {
        //lcdDisplay(0, 4, 0x01, (U08*)"There has no record.");
        //kbGetKeyMs(1000);
        return 0;
    }
    fileSeek(fd, i, SEEK_SET);
    fileRead(fd, psRecord, sizeof(TRANS_RECORD));
	fileSeek(fd, i, SEEK_SET);
	fileWrite(fd, writedata, sizeof(TRANS_RECORD));
	/*prnInit();
	prnPrintf("record read:%d\n",iRecordIndex);
				prnPrintf("file read:%s",psRecord);
				prnStart();
				prnPrintf("reading after done:%d\n",iRecordIndex);
				prnPrintf("file read:%s",psRecord);
				prnStart();*/
				
				//ReadRecord(iRecordIndex,readdata1);
	fileClose(fd);
    
	return 1;
}
int GetRecordNumbersync(void)
{

    int iFileLen;
	
    
    iFileLen = fileSize("sync");
	//prnInit();
				//prnPrintf("file lenght:%d\n",iFileLen);
				//prnStart();
    //return (iFileLen/sizeof(TICKETS_RECORD));
	synctotalrecords=(iFileLen/sizeof(TRANS_RECORD));
	if(iFileLen< -1)
	{
	
	lcdCls();
		lcdDisplay(0, 4, 0x01, (U08*)"Nothing to do.");
		sysDelayMs(1000);
		return 1;
	}
	
	//prnInit();
				//prnPrintf("records for real:%d\n",synctotalrecords);
				//prnStart();
}
int getclientdetails(void)
{
	U08 abyTemp[20];
U08 TimeStr[25];
int relamount=0;
int iRet = 0;
sysGetTime(TimeStr);
UINT8 mytime[50];
lcdClrLine(2, 7);
//strcpy(userid,"1234");
		lcdDisplay(0, 2, DISP_CFONT, "Client Name");
		//lcdDisplay(0, 6, DISP_CFONT, "_");
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0xf5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter Name...");
		kbGetKey();
		return;
			//continue;
		}
		strcpy(cname, (UINT8*)abyTemp);
		lcdCls();
		lcdDisplay(0, 2, DISP_CFONT, "ID/PP");
		//lcdDisplay(0, 6, DISP_CFONT, "_");
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0x1F5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter ID/PP...");
		memset(identification,0x00,sizeof(identification));
		kbGetKey();
		return;
			//continue;
		}
		strcpy(identification, (UINT8*)abyTemp);
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "Nationality");
		memset(nationality,0x00,sizeof(nationality));
		//lcdDisplay(0, 6, DISP_CFONT, "_");
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0x1F5 , 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter Nationality...");
		kbGetKey();
		return;
			//continue;
		}
		strcpy(nationality, (UINT8*)abyTemp);
		forexreceipt();
				print();
				changerate=0;
			 	//ReadRecordSetting(0,"");
		testserver(sendstr);
		return;

}
 
int getcur(void)
{
	U08 abyTemp[20];
U08 TimeStr[25];
int relamount=0;
int iRet = 0;
sysGetTime(TimeStr);
UINT8 mytime[50];
lcdClrLine(2, 7);
//strcpy(userid,"1234");
		lcdDisplay(0, 2, DISP_CFONT, "Enter The Currency:");
		//lcdDisplay(0, 6, DISP_CFONT, "_");
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0xf5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter Cur...");
		kbGetKey();
		return;
			//continue;
		}
		strcpy(currency, (UINT8*)abyTemp);
		getnbal=1;
		UINT8 sendst[200];
		sprintf(sendst,"|STX|1232424|GB|%s|ETX|\n",currency);
		 	//ReadRecordSetting(0,"");
		testserverrate(sendst);
		lcdCls();
		lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Current %s Float: ",currency);
		lcdDisplay(0, 5, DISP_CFONT, "%s",amnt);
		sysDelayMs(6000);
		memset(currency,0x00,sizeof(currency));
		return;
		

}
 
 
 
 int testserver(UINT8* receiptno)
 
{
uint8_t     lenbuf[128];
U08 abyTemp[256];
    int32_t     iRet=0;
    int32_t     sockfd=0;
    uint32_t    BeginTime, EndTime;
    struct sockaddr    HostSockAddr;
    struct timeval     RecvTime;
    uint32_t        uiDataLen       = 1024;         //  data block size
    uint32_t        uiRecvTimeOut   = 180000;       //  data recv timer out ms
    uint8_t         ucIP[30]        = "197.248.118.113";
    in_port_t       netPort         = 9890;
    uint8_t         ucAPN[30]       = "internet";
    uint8_t         ucUID[70]       = "";
    uint8_t         ucUPWD[70]      = "";
    uint8_t     sendbuf[256], recvbuf[255];
    int32_t     sendlen=256, recvlen=255;
	uint8_t     sendbuff[256], recvbuff[255],recvstrpc[256];
	UINT8 receiptnumber[256];
	memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		
		
  iRet = WnetInit(20000); //  init module in 20 seconds.
    if(iRet != NET_OK)
    {
	lcdCls();
	lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "NO SIM");
	sysDelayMs(1000);
        return; //  can not find gprs or cdma module
    }

    ///////////  check sim status
	lcdCls();
    lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "check SIM...");
    iRet = WnetCheckSim();
    //ShowTWnetRet(3, iRet);
    if(iRet == -NET_ERR_PIN)
    {
        memset(lenbuf, 0, sizeof(lenbuf));
        lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "Input PIN:");
        lcdGoto(0, 5);
        iRet = kbGetString(0xad, 0, 21, -1, lenbuf);       // 0xb5
        if(iRet <= 0) return;
        iRet = WnetInputSimPin(lenbuf);
        if(iRet != NET_OK)
        {
           // ShowTWnetRet(5, iRet);
            return;
        }
    }
    else if(iRet != NET_OK)
    {
        kbFlush();
        kbGetKey();
        return;
    }

    ////////  PPP Login, ppp always login at the begining, need't logout unless the link broken.
iRet = PPPLogin(OPT_DEVWNET, ucAPN, ucUID, ucUPWD, 0, 0);   //If application adopts dialing in blocked method, It is recommended to set the timeout to 65000 ms.
    if(iRet != NET_OK)
    {
        return;
    }
    BeginTime = sysGetTimerCount();
    while(1)
    {
        EndTime = sysGetTimerCount();
        if(EndTime-BeginTime > 65000)   //  timeout
        {
            return;
        }
        iRet = PPPCheck(OPT_DEVWNET);
        if(iRet != -NET_ERR_LINKOPENING)
        {
            if(iRet == NET_OK)  //  PPP Login successfully
            {
			/*prnInit();
			prnPrintf("link open");
			prnStart();*/
                break;
            }
            else    //  PPP Login fail
            {
                return;
            }
        }
        sysDelayMs(200); //  still link opening
    }
	
if(sockfd > 0)  //  can not create a sockfd
    {
	NetClose(sockfd);    
	ShowTWnetRet(4,"port closed");
        //continue;
    }
	   
        sockfd = NetSocket(AF_INET, SOCK_STREAM, 0);    //  if want to open multi sock, repeat NetSocket and save return sockfd
    if(sockfd < 0)  //  can not create a sockfd
    {
	NetClose(sockfd);
	ShowTWnetRet(4,"can not create a sockfd");
	 sockfd = NetSocket(AF_INET, SOCK_STREAM, 0);
        //return 1;
    }
	ShowTWnetRet(4,"connecting..");
    //////  TCP open
    SockAddrset(&HostSockAddr, ucIP, netPort);
    iRet = NetConnect(sockfd, &HostSockAddr, sizeof(HostSockAddr));
    if(iRet < 0)  //  tcp open fail
    {
	ShowTWnetRet(4,"Server connection fail.");
	sysDelayMs(2000);
        NetClose(sockfd);    //  this step must be performed after NetSocket()
        return 1;
    }
	 ShowTWnetRet(4,"Server connected..");
	   
    
	RecvTime.tv_sec = uiRecvTimeOut/1000;
    RecvTime.tv_usec = uiRecvTimeOut%1000;
    NetSetsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &RecvTime, sizeof(RecvTime));
    BeginTime = sysGetTimerCount();
    recvlen = 0;
    while(1)        //  repeat receive data until you receive enough len.
    {
        iRet = NetRecv(sockfd, recvbuf+recvlen, sendlen-recvlen, 0);
		
        EndTime = sysGetTimerCount();
        if(iRet == 0)   //  receive nothing when timeout
        {
		
		ShowTWnetRet(4,"performed after NetSocket(1)");
		
		
            NetClose(sockfd);    //  this step must be performed after NetSocket()
            return 0;
        }
        else if(iRet > 0)
        {
            recvlen += iRet;
            if(recvlen == sendlen)  //  receive enough len.
            {
			
        //ShowTWnetRet(4,recvbuf);
		/*prnInit();
		prnPrintf("REcvinner:%s\n",recvbuf);
		prnStart();*/
               break;
            }
			/*prnInit();
		prnPrintf("REcvinnerouter:%s\n",recvbuf);
		prnPrintf("Starting the next test\n");
		prnStart();*/
			  //ShowTWnetRet(4,recvbuf);
			  memset(recvbuf,0,sizeof(recvbuf));
			  recvlen=0;
			  //strcpy(sendbuf, "|STX|auth|123456|123456|ETX|\r");
			  strcpy(sendbuf,receiptno);
			 // prnInit();
		//prnPrintf("sending:%s\n",sendbuf);
		//prnStart();
    sendlen = 256; //  just demo
    iRet = NetSend(sockfd, sendbuf, sendlen, 0);
    if(iRet != sendlen) //  send error
    {
	ShowTWnetRet(4,"send error");
        NetClose(sockfd);    //  this step must be performed after NetSocket()
        return 1;
    }
	ShowTWnetRet(4,"sent...");
	RecvTime.tv_sec = uiRecvTimeOut/1000;
    RecvTime.tv_usec = uiRecvTimeOut%1000;
    NetSetsockopt("", SOL_SOCKET, SO_RCVTIMEO, &RecvTime, sizeof(RecvTime));
    BeginTime = sysGetTimerCount();
		iRet = NetRecv(sockfd, recvbuf+recvlen, sendlen-recvlen, 0);
		 EndTime = sysGetTimerCount();
			if(iRet == 0)   //  receive nothing when timeout
        {
		ShowTWnetRet(4,"performed after NetSocket(1)");
		return 1;
		}
		else if(iRet > 0)
        {
            recvlen += iRet;
            if(recvlen == sendlen)  //  receive enough len.
            {
			
        //ShowTWnetRet(4,recvbuf);
		
               break;
            }
			
		//sprintf(recvstrpc,"|STX|%s|RC|",hexs);
		sprintf(recvstrpc,"|STX|SV|sync|true|ETX|");
		
			//ShowTWnetRet(3,"REsponse");
			//ShowTWnetRet(5,recvbuf);
			if(memcmp(recvstrpc, recvbuf, 16) == 0)
			{
			NetClose(sockfd);
			/*prnInit();
			prnPrintf(recvbuf);
			prnStart();*/
			//split(recvbuf);
			
			sent=1;
			
			}
			else{
			ShowTWnetRet(3,"An error occured, Retry ");
			sysDelayMs(2000);
			return 0;
			}
			
		memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		memset(sendbuff,0x00,sizeof(sendbuff));
		recvlen=0;
		iRet=0;
		
		NetClose(sockfd); 
		return 0;
		}
		else{
		   //  this step must be performed after NetSocket()
			 ShowTWnetRet(4,"performed after NetSocket(2)");
			 NetClose(sockfd); 
            return 1;
		
		}
        }
        else
        {
            NetClose(sockfd);    //  this step must be performed after NetSocket()
			 ShowTWnetRet(4,"performed after NetSocket(2)");
            return 1;
        }
    }
	
iRet=NetClose(sockfd);
iRet = PPPLogout(OPT_DEVWIFI);  
return ;
}


int ReadRecordsync(int iRecordIndex, TRANS_RECORD *psRecord)
{
    int fd, iFileLen,i; 
	int j= 0;
	int tamount = 0;
	//slt=atoi(slots);
	//int i= 0;
	memset(psRecord,0x00,sizeof(psRecord));
	
    fd = fileOpen("trans", O_RDWR);
    if (fd < 0)
    {
	lcdCls();
        lcdDisplay(0, 4, 0x01, (U08*)"Nothing to do.");
		sysDelayMs(1000);
        //kbGetKeyMs(1000);
        return -1;
    }
	GetRecordNumbersync();
	/*prnInit();
	prnPrintf("sync ticket records:%d\n",synctotalrecords);
				prnPrintf("file read:%s",psRecord);
				prnStart();*/
				if(synctotalrecords > totalsynched)
				{
				lcdCls();
				int sendr=synctotalrecords-totalsynched;
       // lcdDisplay(0, 0, 0x01, "CommDial error");
        //lcdDisplay(0, 2, 0x01, "iRet=%d,", iRet);
		lcdDisplay(0, 2, 0x01, "Total to Transactions:");
        lcdDisplay(0, 4, 0x01, "Records:%d",sendr);
		sysDelayMs(500);
				}

	for (j = 0; j < synctotalrecords; j++)
	
	{
	
	/*prnInit();
				prnPrintf("file ticket sales opened");
				prnStart();*/
    iFileLen = fileSize("sync");
	iRecordIndex=j;
	presentrecord=iRecordIndex;
    i = iRecordIndex*sizeof(TRANS_RECORD);
    if (iFileLen < i)
    {
        lcdDisplay(0, 4, 0x01, (U08*)"There is no record.");
        kbGetKeyMs(1000);
        return 0;
    }
    fileSeek(fd, i, SEEK_SET);
    fileRead(fd, psRecord, sizeof(TRANS_RECORD));
	
				if(memcmp(psRecord,"|STX|SENT|ETX|",14)==0)
				{ 
				//prnInit();
				
				synchedrecord=synchedrecord+1;
				//prnPrintf("synched records:%d\n",synchedrecord);
				//prnStart();
				}
				else
				{
				 	//ReadRecordSetting(0,"");
				testserver(psRecord);
				
			       if(sent==1)
				   {
				   ShowTWnetRet(4,"Sent Success");
				   ReadRecordappend(presentrecord,"");
				   }
				  
				//return 1;
				}
				
				if (synchedrecord==synctotalrecords)
					{
					lcdCls();
					lcdDisplay(0, 4, 0x01, (U08*)"Done Sync.");
					lcdDisplay(0, 4, 0x01, (U08*)"Nothing to do.");
					synchstatus=1;
					sysDelayMs(2000);
					
						/*prnInit();
						prnPrintf("Nothing to do");
						prnStart();*/
						return 1;
		
					}
				
				
    
                
            }
			
   
	fileClose(fd);
    
	return 1;
}




UINT8 DispMenuf[][16] =
{
    "KES     ",
    "UGX     ",
    "TZS     ",
    "USD     ",
	"EURO    ",
	"RWF     ",
	"GBP     ",
	"BUF     ",
	"EXIT    ",
    
};
INT32 gl_Menum2Num = 9;

UINT32 asc2ulong(UINT8 *asc)
{
    INT32 i, blen;
    UINT32 dwTmp;

    blen = strlen((INT8 *)asc);
    dwTmp = 0;

    for (i = 0; i < blen; i++)
    {
        if ((asc[i] < '0') || (asc[i] > '9'))
            return -1;
        dwTmp = dwTmp*10 + (asc[i] - 0x30);
    }
    return dwTmp;
}
// 显示数据

void ExitFunc(void)
{
    return KEY_CANCEL;
}
void DispData(UINT32 len, const U08 *pucData)
{
    UINT32 i = 0, j = 0, k = 2;

	for(j = 0; j < len; j++)
	{
        lcdDisplay(i, k, 0, "%02x", pucData[j]);
		i = i + 16;
		if(i >= 128)
		{
		   i = 0;
		   k++;
		}
		
		

		if(k >= 7)
		{
		    kbGetKey();
			lcdClrLine(2,7);
			k = 2;
		}
		
	}
	
}
void vOneTwo(UINT8 *One,UINT16 len,UINT8 *Two)
{
    UINT8  i;
    static UINT8 TAB[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    for (i = 0; i < len; i++)
    {
        Two[i * 2] = TAB[One[i] >> 4];
        Two[i * 2 + 1] = TAB[One[i] & 0x0f];
    }
}

void vTwoOne(UINT8 *in, UINT8 *out, INT32 in_len)
{
	INT32   i;

	for(i=0; i<in_len; i++)
	{
        if((in[i]>='0') && (in[i]<='9'))
        {
            out[i/2] = (out[i/2] & 0xF0) | ((in[i] - '0') & 0x0F);
        }
        else if((in[i]>='A') && (in[i]<='F'))
        {
            out[i/2] = (out[i/2] & 0xF0) | ((in[i] - 'A' + 0x0A) & 0x0F);
        }
        else if((in[i]>='a') && (in[i]<='f'))
        {
            out[i/2] = (out[i/2] & 0xF0) | ((in[i] - 'a' + 0x0A) & 0x0F);
        }
        else
        {
            return;
        }

        if(i%2 == 0)
        {
            out[i/2] = out[i/2] << 4;
        }
	}
}



INT32 selecttestmenuf(void)
{
    INT32 j = 0;
    INT32 Key = 0;
    UINT8 Cursor = 0;

    j = 0;
    kbFlush();

    // 根据AppNum，显示上下翻图标
    lcdSetIcon(ICON_DOWN,ON);
    lcdSetIcon(ICON_UP,ON);

    while(1)
    {
	
        lcdCls();
		if(fromselected==0){
       	lcdDisplay(0,0,DISP_CFONT|DISP_MEDIACY|DISP_REVERSE,"Currency Received  ");
			}
			else{
				lcdDisplay(0,0,DISP_CFONT|DISP_MEDIACY|DISP_REVERSE,"Currency To");
			}
        if(j < gl_Menum2Num)
        {
			if((Cursor % 3) == 0)
            {
                lcdDisplay(0, 2, DISP_CFONT|DISP_REVERSE, "%d-%s", j+1, DispMenuf[j]);
            }
            else
            {
                lcdDisplay(0, 2, DISP_CFONT, "%d-%s", j+1, DispMenuf[j]);
            }
        }

        if((j+1) < gl_Menum2Num)
        {
			if((Cursor % 3) == 1)
            {
                lcdDisplay(0, 4, DISP_CFONT|DISP_REVERSE, "%d-%s", j+2, DispMenuf[j+1]);
            }
            else
            {
                lcdDisplay(0, 4, DISP_CFONT, "%d-%s", j+2, DispMenuf[j+1]);
            }
        }

        if((j+2) < gl_Menum2Num)
        {
			if((Cursor % 3) == 2)
            {
                lcdDisplay(0, 6, DISP_CFONT|DISP_REVERSE, "%d-%s", j+3, DispMenuf[j+2]);
            }
            else
            {
                lcdDisplay(0, 6, DISP_CFONT, "%d-%s", j+3, DispMenuf[j+2]);
            }
        }

        Key = kbGetKey();

        switch(Key)
        {
            case KEY_DOWN:
                if(Cursor < (gl_Menum2Num - 1))
                {
                    Cursor++;
                }
                else
                {
                    Cursor = 0;
                }

                j = (Cursor/3)*3;
                break;
            case KEY_UP:
                if(Cursor != 0)
                {
                    Cursor--;
                }
                else
                {
                    // 到菜单顶，就不再显示上翻图标
                    Cursor = gl_Menum2Num - 1;
                }

                j = (Cursor/3)*3;
                break;
            case KEY_ENTER:
                lcdSetIcon(ICON_DOWN,OFF);
                lcdSetIcon(ICON_UP,OFF);
                return Cursor;
            case KEY_CANCEL:
                lcdSetIcon(ICON_DOWN,OFF);
                lcdSetIcon(ICON_UP,OFF);
                return -1;
        }
    }
}

int test_CODE128(char *inputcode )
{
    uint8_t     barimg[32*1024];
    barcode_t   bar;
    int         i;
    int         ret;
    int         inputmode;
    int         codelen;
    int         numcase = 0;

int fd;
int Ret;
UINT8 *databuff;
        inputmode = 2;
        codelen=strlen(inputcode);



     /* fd = fileOpen("trial",O_CREAT|O_RDWR);


        
Ret = fileSeek(fd, 0, SEEK_SET);
Ret = fileWrite(fd, inputcode, codelen);  
Ret = fileClose(fd);

*/

        lcdClrLine(2, 7);
        memset(&bar, 0, sizeof(bar));
        strcpy(bar.code_type, BAR_TYPE_CODE128);
        bar.bar_height = 8*8;
        bar.image_width = 324;
        bar.block_width = 0;
        bar.adj_percent = 28;
        bar.blank_width = 0;
        bar.space = 6;
		/*fd = fileOpen("trial", O_RDWR);
		Ret = fileSeek(fd, 0, SEEK_SET);
Ret = fileRead(fd, databuff, 16);
Ret = fileClose(fd);*/

//        snprintf(bar.bar_str, sizeof(bar.bar_str), "99887700000123456789%02d", i);
//        bar.bar_str_len = strlen(bar.bar_str);
        bar.bar_str_len = MIN((int)sizeof(bar.bar_str), codelen);
        memcpy(bar.bar_str, inputcode, bar.bar_str_len);

        strncpy(bar.FontName, FONT_SYSTEM, sizeof(bar.FontName));
        strncpy(bar.FontCharacter, FONT_CHARSET_ASCII, sizeof(bar.FontCharacter));
        bar.FontStyle = 0;
        bar.FontSize = 16;
        bar.print_style = BAR_PRINT_EXPAND;
        strcpy(bar.print_str, bar.bar_str);

        //lcdDisplay(0, 2, DISP_CFONT, "%s", bar.bar_str);

       // lcdDisplay(0, 6, DISP_CFONT|DISP_CLRLINE,  "Generate Img...");
        memset(barimg, 0, sizeof(barimg));
        ret = DrawBarCode(&bar, barimg);
        if(ret != OK)
        {
            lcdDisplay(0, 6, DISP_CFONT|DISP_CLRLINE,  "Gen ERR:%d", ret);
            kbGetKey();
        }

       // lcdDisplay(0, 6, DISP_CFONT|DISP_CLRLINE,  "Print...");
        ret = prnInit();
        prnLogo(EM_BMP_ROTATE0, barimg);
       // prnPrintf("code %d Bytes(HEX):\n", bar.bar_str_len);
        /*for(i=0; i<bar.bar_str_len; i++)
        {
            prnPrintf("%c", bar.bar_str[i], (i%8==7 || i==bar.bar_str_len-1) ? '\n' : ' ');
        }*/
		/*fd = fileOpen("trial", O_RDWR);
		Ret = fileSeek(fd, 0, SEEK_SET);
Ret = fileRead(fd, (U08*)databuff, 16);
Ret = fileClose(fd);*/
 //prnPrintf("writtendata ", databuff);
        //prnPrintf("\n\n\n\n");
        ret = prnStart();
        return(OK);
}

int receiptnogen(void)
{

int iRet;
UINT32 receiptnog[20];
U08 abyTemp[64], abyLastTime[20],abyTemp1[64];
sysGetTime(abyTemp);
sysReadSN(abyTemp1);
strcpy(hexs,abyTemp1);
memset(invoiceno,0x00,sizeof(invoiceno));
sprintf(receiptnog,"%02x%02x%c%c%c%02x",abyTemp[3],abyTemp[4],hexs[5],hexs[6],hexs[7],abyTemp[5]);

strcpy(invoiceno,receiptnog);
//prnStart();


}


int forexreceipt(void)
 {
 U08 TimeStr[64];
 U08 Abyt[64];
UINT8 day[20];
UINT32 descstr[200],crstr[200],date[100],time[100],datestr[200];
sysGetTime(TimeStr);
receiptnogen();
sysReadSN(Abyt);
 sprintf(timenow,"%02x-%02x-%02x %02x:%02x:%02x\n",TimeStr[2],TimeStr[1],TimeStr[0],TimeStr[3],TimeStr[4],TimeStr[5]);
 sprintf(date,"%02x-%02x-%02x",TimeStr[2],TimeStr[1],TimeStr[0]);
 sprintf(time,"%02x:%02x:%02x",TimeStr[3],TimeStr[4],TimeStr[5]);
 sprintf(datestr," Date %s Time %s\n",date,time);
 sprintf(descstr," CUR | FX AMT | RATE | %s\n",currencyfrom);
 sprintf(crstr," %s | %.2lf | %.2lf | %.2lf \n",currencyto,amountin,rate,amountout);
 strcpy(hexs,Abyt);
 prnInit();
		prnPrintf("\n");
		prnPrintf("\n");
		prnPrintf("\n");
	prnPrintf("******************************\n");
		prnPrintf("\n");
	prnPrintf("   Prestige Ventures Ltd\n");
	prnPrintf("        Kimironko,\n");
	prnPrintf("     Kigali,Rwanda\n");
	prnPrintf("   Mobile: +250 781 524 647\n");
	prnPrintf("      TIN: 103542127\n");
		//prnPrintf("\n");
	prnPrintf("******************************\n");
		//prnPrintf("\n");
	if (forextype==0){
	prnPrintf("  Buying of foreign Currency\n");
	} else{
	prnPrintf("  Selling of foreign Currency\n");
	}
		//prnPrintf("\n");
	prnPrintf("******************************\n");
		prnPrintf("\n");
	prnPrintf(datestr);
		//prnPrintf("\n");
	prnPrintf("******************************\n");
		prnPrintf("\n");
	prnPrintf(descstr);
		//prnPrintf("\n");
	prnPrintf("******************************\n");
		prnPrintf("\n");
	prnPrintf(crstr);
		//prnPrintf("\n");
	prnPrintf("______________________________\n");
		prnPrintf("\n");
	prnPrintf("Client Name: %s\n",cname);
	prnPrintf("ID/PP      : %s\n",identification);
	prnPrintf("Nationality: %s\n",nationality);
		prnPrintf("\n");
	prnPrintf("Sign       : _______________\n");	
	prnPrintf("\n");
	
	prnPrintf("You were served by: \n",fullname);
	prnPrintf("\n");
	PrintData();
	test_CODE128(invoiceno);
	
	prnInit();
	prnPrintf("\n");
	prnPrintf("    Turning ideas into reality\n");
	prnPrintf("\n");
	prnPrintf("         Thank You\n");
	//prnStart();
	
	prnPrintf("\n");
	prnPrintf("\n");
	prnPrintf("\n");
	prnPrintf("\n");
	prnPrintf("\n");
	prnStart();
 
 }
int floattest(void)
{
double result=0.90897;
prnInit();
prnPrintf("result %.2lf\n",result);
prnStart();
}



int selectingt(void)
{
INT32 Select = 0;

while(1)
    {
        Select = selecttestmenuf();
		
        lcdCls();
        switch(Select)
        {
        case -1:
            return;
        
       
			case 0:
           
		   strcpy(currencyto,"KES");
		   inputAmount();
		   fromselected=0;
		   return;
            break;
			case 1:
            
			strcpy(currencyto,"UGX");
			inputAmount();
			fromselected=0;
			return 0;
            break;	
			case 2:
            
			strcpy(currencyto,"TZS");
			inputAmount();
			fromselected=0;
			return 0;
            break;	
			case 3:
            
			strcpy(currencyto,"USD");
			inputAmount();
			fromselected=0;
			return 0;
            break;	
			case 4:
            
			strcpy(currencyto,"EURO");
			inputAmount();
			fromselected=0;
			return 0;
            break;
            case 5:
            
			strcpy(currencyto,"RWF");
			inputAmount();
			fromselected=0;
			return 0;
            break;	
			case 6:
            
			strcpy(currencyto,"GBP");
			inputAmount();
			fromselected=0;
			return 0;
            break;	
			case 7:
            
			strcpy(currencyto,"BUF");
			inputAmount();
			fromselected=0;
			return 0;
            break;	
		case 8:
		fromselected=0;
            ExitFunc();
            break;	
			
        
           
        default:
            break;
        }
    }
}



int selectingf(void)
{
INT32 Select = 0;

while(1)
    {
        Select = selecttestmenuf();
		
        lcdCls();
        switch(Select)
        {
        case -1:
            return;
        
       
			case 0:
           fromselected=1;
		   strcpy(currencyfrom,"KES");
		   selectingt();
		   return 0;
            break;
			case 1:
            fromselected=1;
			strcpy(currencyfrom,"UGX");
			selectingt();
			return 0;
            break;	
			case 2:
            fromselected=1;
			strcpy(currencyfrom,"TZS");
			selectingt();
			return 0;
            break;	
			case 3:
            fromselected=1;
			strcpy(currencyfrom,"USD");
			selectingt();
			return 0;
            break;	
			case 4:
            fromselected=1;
			strcpy(currencyfrom,"EURO");
			selectingt();
			return 0;
            break;
				case 5:
            
			strcpy(currencyto,"RWF");
			inputAmount();
			fromselected=0;
			return 0;
            break;	
			case 6:
            
			strcpy(currencyto,"GBP");
			inputAmount();
			fromselected=0;
			return 0;
            break;	
			case 7:
            
			strcpy(currencyto,"BUF");
			inputAmount();
			fromselected=0;
			return 0;
            break;	
		case 8:
            ExitFunc();
            break;	
			
        
           
        default:
            break;
        }
    }
}



static int g_aiModemConnectSpeed[17] = {MODEM_CONNECT_1200BPS, MODEM_CONNECT_2400BPS,
    MODEM_CONNECT_4800BPS, MODEM_CONNECT_7200BPS, MODEM_CONNECT_9600BPS,
    MODEM_CONNECT_12000BPS, MODEM_CONNECT_14400BPS, MODEM_CONNECT_16800BPS,
    MODEM_CONNECT_19200BPS, MODEM_CONNECT_21600BPS, MODEM_CONNECT_24000BPS,
    MODEM_CONNECT_26400BPS, MODEM_CONNECT_28800BPS, MODEM_CONNECT_31200BPS,
    MODEM_CONNECT_33600BPS, MODEM_CONNECT_48000BPS, MODEM_CONNECT_56000BPS};

#define COMM_TYPE_SELECT_NUM  7

static U08 g_abyCommTypeSelectEng[COMM_TYPE_SELECT_NUM][20] =
    {"GPRS        ","WIFI        ","PSTN        ","CDMA        ","PSTN PPP    ",
     "LAN         ","RS232       "};


//extern int g_iLangueSelect;

static int sg_iModemfd = -1;
static int sg_Tcpsocket =  -1 ;
static int sg_iRs232Status = -1;

static int g_iComInitStatus = 0xff;

static struct sockaddr     sg_stTcpsockaddr;

static ERR_INFO        sg_stCommErrMsg[] =
{
    {ERR_COMM_INV_PARAM, "参数错误",     "INVALID PARAM"},
    {ERR_COMM_INV_TYPE,  "无效通讯类型", "INV COMM TYPE"},
    {ERR_COMM_CANCEL,    "用户取消",     "USER CANCEL"},
    {ERR_COMM_TIMEOUT,   "通讯超时",     "TIMEOUT"},
    {ERR_COMM_COMERR,    "通讯错误",     "COMM ERROR"},
    {ERR_COMM_TOOBIG,    "数据包太大",   "DATA TOO BIG"},
    {0, "", ""},
};

static ERR_INFO        sg_stRS232ErrMsg[] =
{
    {PORT_RET_NOTEMPTY,    "发送缓冲区满", "OVERFLOW"},
    {PORT_RET_PORTERR,     "无效端口号",   "INVALID PORT"},
    {PORT_RET_NOTOPEN,     "端口未打开",   "PORT CLOSED"},
    {PORT_RET_BUSY,        "无可用端口",   "NO PORT AVAIL"},
    {PORT_RET_MODEM_INUSE, "端口被占用",   "PORT OCCUPY"},
    {PORT_RET_PARAERR,     "无效通讯参数", "INVALID PARAM"},
    {PORT_RET_TIMEOUT,     "串口超时",     "TIMEOUT"},
    {0, "", ""},
};

static ERR_INFO        sg_stPPPErrMsg[] =
{
    {NET_ERR_RETRY,        "通讯超时",        "TIMEOUT"},
    {NET_ERR_RSP,          "无应答,检查模块", "CHECK MODULE"},
    {NET_ERR_NOSIM,        "SIM卡不存在",     "NO SIM CARD"},
    {NET_ERR_PIN,          "需要输入PIN码",   "NEED SIM PIN"},
    {NET_ERR_PUK,          "需要输入PUK码",   "NEED SIM PUK"},
    {NET_ERR_PWD,          "PIN码错误",       "SIM PIN ERROR"},
    {NET_ERR_SIMDESTROY,   "SIM卡错误",       "NO SIM/NEED PIN"},
    {NET_ERR_CSQWEAK,      "信号太弱",        "SIGNAL TOO WEAK"},
    {NET_ERR_LINKCLOSED,   "网络无载波",      "NO CARRIER"},
    {NET_ERR_LINKOPENING,  "连接成功",        "LINK OK"},
    {NET_ERR_ATTACHING,    "正在查找网络",    "SEEKING NETWORK"},
    {NET_ERR_DETTACHED,    "已断开无线网络",  "DETTACH NETWORK"},
    {NET_ERR_EMERGENCY,    "检查SIM卡",       "PLS CHECK SIM"},
    {NET_ERR_RING,         "等待接入中",      "ACCEPTED ERR"},
    {NET_ERR_BUSY,         "正在通讯中",      "COMMINICATE ERR"},
    {NET_ERR_DIALING,      "模块正在拨号",    "LINKING"},
    {NET_ERR_UNKNOWN,      "未知错误码",      "UNKNOW ERR"},
    {NET_ERR_ABNORMAL,     "未知错误码",      "UNKNOW ERR"},
    {0, "", ""},
};

#ifndef ETIME
#define  ETIME   62
#endif
static ERR_INFO    sg_stModemErrMsg[] =
{
    {-MODEM_ERRNO_ERROR,             "设备异常",       "MODEM ERROR"},
    {-MODEM_ERRNO_BUSY,              "设 备 忙",       "MODEM BUSY"},
    {-MODEM_ERRNO_NO_DIALTONE,       "无拨号音",       "NO DIAL TONE"},
    {-MODEM_ERRNO_NO_CARRIER,        "线路载波丢失",   "LINE BREAK"},
    {-MODEM_ERRNO_NO_LINE,           "请接好电话线",   "LINE READY ?"},
    {-MODEM_ERRNO_NO_ANSWER,         "拨号无应答",     "NO ACK"},
    {-MODEM_ERRNO_OFF_HOOK,          "请先挂机",       "PLS ONHOOK"},
    {-MODEM_ERRNO_LINE_IN_USE,       "被叫线路忙",     "LINE BUSY"},
    {-MODEM_ERRNO_UN_OBTAINABLE,     "线路不可到达",   "UNOBTAINABLE"},
    {-MODEM_ERRNO_LINE_REVERSAL,     "线路反转",       "LINE REVERSAL"},
    {-MODEM_ERRNO_EXTENSION_IN_USE,  "旁置电话占用",   "PHONE OCCUPIED"},
    {ERR_COMM_STATUS_TIMEOUT,        "连接主机超时",   "TIME OUT"},
    {ERR_COMM_MODEM_INIT,            "初始化设备失败", "MODEM INIT ERROR"},
    {NET_ERR_RETRY,                  "重试次数超限",   "RETRY OVERRUN"},
    {4,                              "请接好电话线",   "LINE BREAK"},
    {5,                              "请接好电话线",   "LINE BREAK"},
    {16,                             "设 备 忙",       "MODEM BUSY"},
    {22,                             "参数错误",       "INVALID ARGUMENT"},
    {ETIMEDOUT,                      "连接主机超时",   "TIME OUT"},
    {ETIME,                          "连接主机超时",   "TIME OUT"},
    {0, "", ""},
};


typedef struct
{
    int     retcode;        //  api返回
    char     *descript_en;   //  错误描述信息(英文)
    char     *descript_ch;   //  错误描述信息(中文)
}tWnet_ret;

static tWnet_ret   WnetRet[] =
{
//  wnet
    {NET_OK,                "OK",           "成功"},
    {NET_ERR_RSP,           "RSP ERROR",    "错误的响应"},
    {NET_ERR_NOSIM,         "NO SIM",       "没有SIM卡"},
    {NET_ERR_PIN,           "NEED PIN",     "需要验证PIN"},
    {NET_ERR_PUK,           "NEED PUK",     "需要验证PUK"},
    {NET_ERR_PWD,           "PWD ERR",      "密码错误"},
    {NET_ERR_SIMDESTROY,    "SIM DESTROY",  "SIM毁坏"},
    {NET_ERR_CSQWEAK,       "CSQ TOO WEAK", "信号太弱"},
    {NET_ERR_LINKCLOSED,    "LINK CLOSED",  "链接断开"},
    {NET_ERR_LINKOPENING,   "LINK OPENING", "正在连接"},
    {NET_ERR_DETTACHED,     "DETTACHED",    "未注册网络"},
    {NET_ERR_ATTACHING,     "ATTACHING",    "搜寻网络"},
    {NET_ERR_EMERGENCY,     "EMERGENCY",    "紧急状态"},
    {NET_ERR_RING,          "RING...",      "有振铃"},
    {NET_ERR_BUSY,          "BUSY NOW",     "正在通话"},
    {NET_ERR_DIALING,       "DIALING...",   "拨号中"},
    {NET_ERR_UNKNOWN,       "UNKNOWN",      "未知错误"},
    {NET_ERR_ABNORMAL,      "ABNORMAL",     "异常错误"},
    {NET_ERR_NOMODULE,      "NO MODULE",    "无模块"},
    {ENOMEM,                "NO MEM",       "内存不足"},
//    {EFAULT,                "FAULT",        "错误"},
    {EINVAL,                "INVALID PARA", "无效参数"},
    {ENODATA,               "NO DATA",      "没有数据"},
    {ENONET,                "NO NET",       "没有网络"},
    {ENOLINK,               "NO LINK",      "没有链接"},
    {ECOMM,                 "SEND ERR",     "发送错误"},
    {EPROTO,                "PROTO ERR",    "协议错误1"},
    {EOVERFLOW,             "OVERFLOW",     "溢出"},
    {ENOTSOCK,              "NOT SOCK",     "没有SOCK"},
    {EPROTOTYPE,            "PROTO WRONG",  "协议错误2"},
    {ENOPROTOOPT,           "NO PROTO OPT", "无协议选项"},
    {EPROTONOSUPPORT,       "PROTO NO SUP", "协议不支持"},
    {EPFNOSUPPORT,          "PROTO FML",    "协议族未知"},
    {ENETDOWN,              "NET DOWN",     "网络断开"},
    {ENETUNREACH,           "NET UNREACH",  "网络不可达"},
    {ENETRESET,             "NET DROPPED",  "网络复位"},
    {ECONNRESET,            "CONN RESET",   "链接复位"},
    {EISCONN,               "IS CONN",      "已连接"},
    {ENOTCONN,              "NOT CONN",     "未连接"},
    {ESHUTDOWN,             "SHUT DOWN",    "断开"},
    {ETIMEDOUT,             "TIMEOUT",      "超时"},
    {ECONNREFUSED,          "REFUSED",      "链接被拒绝"},
    {EHOSTDOWN,             "HOST DOWN",    "主机断开"},
    {EHOSTUNREACH,          "HOST UNREACH", "主机不可达"},
    {EWIFI_BUSY,            "I/BUSY",       "I/BUSY"},
    {EWIFI_DONE,            "I/DONE",       "I/DONE"},
    {EWIFI_ONLINE,          "I/ONLINE",     "I/ONLINE"},
    {EWIFI_OFFLINE,         "I/OFFLINE",    "I/OFFLINE"},
    {EWIFI_RCV,             "I/RCV",        "I/RCV"},
    {EWIFI_PART,            "I/PART",       "I/PART"},
    {EWIFI_EOP,             "I/EOP",        "I/EOP"},
    {EWIFI_EOM,             "I/EOM",        "I/EOM"},
    {EWIFI_MBE,             "I/MBE",        "I/MBE"},
    {EWIFI_UPDATE,          "I/UPDATE",     "I/UPDATE"},
    {EWIFI_ILLDELIMITER,    "ILLDELIMITER", "非法分隔符"},
    {EWIFI_ILLVAL,          "ILLVAL",       "非法值"},
    {EWIFI_EXPCR,           "EXPCR",        "缺CR"},
    {EWIFI_EXPNUM,          "EXPNUM",       "缺号码"},
    {EWIFI_EXPCRORCOMMA,    "EXPCRORCOMMA", "缺CR或逗号"},
    {EWIFI_EXPDNS,          "EXPDNS",       "缺DNS"},
    {EWIFI_EXPTILDE,        "EXP':'or'~'",  "缺':'或'~'"},
    {EWIFI_EXPSTR,          "EXP STRING",   "缺字符串"},
    {EWIFI_EXPEQL,          "EXP':'or'='",  "缺':'或'='"},
    {EWIFI_EXPTXT,          "EXP TEXT",     "缺文本内容"},
    {EWIFI_SYNTAX,          "SYNTAX ERR",   "语法错误"},
    {EWIFI_EXPCOMMA,        "EXP ','",      "缺','"},
    {EWIFI_ILLCMD,          "ILL CMD CODE", "命令码非法"},
    {EWIFI_SETPARA,         "SET PARA ERR", "设置参数错"},
    {EWIFI_GETPARA,         "GET PARA ERR", "获取参数错"},
    {EWIFI_USERABORT,       "USER ABORT",   "用户取消"},
    {EWIFI_BUILDPPP,        "BUILD PPP",    "建立PPP错"},
    {EWIFI_BUILDSMTP,       "BUILD SMTP",   "建立SMTP错"},
    {EWIFI_BUILDPOP3,       "BUILD POP3",   "建立POP3错"},
    {EWIFI_MAXMINE,         "MAX MIME",     "MIME上限"},
    {EWIFI_INTMEMFAIL,      "INTMEM FAIL",  "内部存储错"},
    {EWIFI_USERABORTSYS,    "USERABORTSYS", "取消系统"},
    {EWIFI_HFCCTSH,         "FLC CTSH ERR", "流控CTSH错"},
    {EWIFI_USERABORTDDD,    "ABORT BY ---", "被---取消"},
    {EWIFI_RES2065,         "RES: 2065",    "预留:2065"},
    {EWIFI_RES2066,         "RES: 2066",    "预留:2066"},
    {EWIFI_CMDIGNORE,       "CMD IGNORE",   "命令被忽略"},
    {EWIFI_SNEXIST,         "SN EXIST",     "SN已存在"},
    {EWIFI_HOSTTO,          "HOST TIMEOUT", "主机超时"},
    {EWIFI_MODEMRESPOND,    "MDM RSP FAIL", "MDM响应错"},
    {EWIFI_NODIALTONE,      "NO DIAL TONE", "无拨号音"},
    {EWIFI_NOCARRIER,       "NO CARRIER",   "无载波"},
    {EWIFI_DIALFAIL,        "DIAL FAILED",  "拨号失败"},
    {EWIFI_CONNLOST,        "CONNECT LOST", "掉链"},
    {EWIFI_ACCESSISP,       "ACCESS ISP",   "访问ISP错"},
    {EWIFI_LOCPOP3,         "LOCATE POP3",  "定位POP3错"},
    {EWIFI_POP3TO,          "POP3 TIMEOUT", "POP3超时"},
    {EWIFI_ACCESSPOP3,      "ACCESS POP3",  "访问POP3错"},
    {EWIFI_POP3FAIL,        "POP3 FAILED",  "POP3错"},
    {EWIFI_NOSUITMSG,       "NO SUIT MSG",  "邮箱没消息"},
    {EWIFI_LOCSMTP,         "LOCATE SMTP",  "定位SMTP错"},
    {EWIFI_SMTPTO,          "SMTP TIMEOUT", "SMTP超时"},
    {EWIFI_SMTPFAIL,        "SMTP FAILED",  "SMTP错"},
    {EWIFI_RES2084,         "RES: 2084",    "预留:2084"},
    {EWIFI_RES2085,         "RES: 2085",    "预留:2085"},
    {EWIFI_WRINTPARA,       "WRITE PARA",   "写入参数错"},
    {EWIFI_WEBIPREG,        "REG WEB ERR",  "注册WEB错"},
    {EWIFI_WEBIPREG,        "REG SOCK ERR", "注册SOCK错"},
    {EWIFI_EMAILIPREG,      "REG MAIL ERR", "注册MAIL错"},
    {EWIFI_IPREGFAIL,       "REG IP ERR",   "注册IP错"},
    {EWIFI_RES2091,         "RES: 2091",    "预留:2091"},
    {EWIFI_RES2092,         "RES: 2092",    "预留:2092"},
    {EWIFI_RES2093,         "RES: 2093",    "预留:2093"},
    {EWIFI_ONLINELOST,      "LOST-REEST",   "掉链重连"},
    {EWIFI_REMOTELOST,      "REMOTE LOST",  "主机断链"},
    {EWIFI_RES2098,         "RES: 2098",    "预留:2098"},
    {EWIFI_RES2099,         "RES: 2099",    "预留:2099"},
    {EWIFI_DEFAULTRES,      "RESTORE ERR",  "恢复预设错"},
    {EWIFI_NOISPDEF,        "NO ISP No.",   "无ISP号码"},
    {EWIFI_NOUSRNDEF,       "NO USRN",      "无USRN"},
    {EWIFI_NOPWDENTER,      "NO PWD ENTER", "未输密码"},
    {EWIFI_NODNSDEF,        "NO DNS",       "无DNS"},
    {EWIFI_NOPOP3DEF,       "NO POP3",      "无POP3"},
    {EWIFI_NOMBXDEF,        "NO MAILBOX",   "无邮箱"},
    {EWIFI_NOMPWDDEF,       "NO MPWD",      "无邮箱密码"},
    {EWIFI_NOTOADEF,        "NO TOA",       "无TOA"},
    {EWIFI_NOREADEF,        "NO REA",       "无REA"},
    {EWIFI_NOSMTPDEF,       "NO SMTP",      "无SMTP"},
    {EWIFI_SDATAOVERFLOW,   "SDATA OVERF",  "SDATA溢出"},
    {EWIFI_ILLCMDMDM,       "ILL CMD MDM",  "无效命令"},
    {EWIFI_FWUPDT,          "FW NOT UPDAT", "FW未完成"},
    {EWIFI_EMAILPARAUPDT,   "UPD MAILPARA", "MPARA升级"},
    {EWIFI_SNETPARA,        "SNET PARA",    "缺SNET参数"},
    {EWIFI_PARSECA,         "ERR PARSE CA", "CA语法错"},
    {EWIFI_RES2117,         "RES: 2117",    "预留:2117"},
    {EWIFI_USRVPARA,        "NO USRV PARA", "无USRV参数"},
    {EWIFI_WLPPSHORT,       "WLPP SHORT",   "密码太短"},
    {EWIFI_RES2120,         "RES: 2120",    "预留:2120"},
    {EWIFI_RES2121,         "RES: 2121",    "预留:2121"},
    {EWIFI_SNETHIF0,        "SNET:HIF=0",   "SNET:HIF=0"},
    {EWIFI_SNETBAUD,        "SNET:BAUD",    "SNET:速率"},
    {EWIFI_SNETHIFPARA,     "SNET:HIF",     "SNET:HIF"},
    {EWIFI_NOSOCK,          "NO SOCK",      "无SOCK"},
    {EWIFI_SOCKEMPTY,       "SOCK EMPTY",   "SOCK空"},
    {EWIFI_SOCKUNUSE,       "SOCK NOT USE", "SOCK未使用"},
    {EWIFI_SOCKDOWN,        "SOCK DOWN",    "SOCK DOWN"},
    {EWIFI_NOAVAILDSOCK,    "NO VAIL SOCK", "无有效SOCK"},
    {EWIFI_PPPOPEN,         "PPP OPEN ERR", "PPP打开错"},
    {EWIFI_CREATSOCK,       "OPEN SOCK",    "SOCK打开错"},
    {EWIFI_SOCKSEND,        "SOCK SND ERR", "SOCK发送错"},
    {EWIFI_SOCKRECV,        "SOCK RCV ERR", "SOCK接收错"},
    {EWIFI_PPPDOWN,         "PPP DOWN",     "PPP DOWN"},
    {EWIFI_SOCKFLUSH,       "SOCK FLUSH E", "SOCK刷新错"},
    {EWIFI_NOCARRIERSOCK,   "SOCK NO CARR", "SOCK无载波"},
    {EWIFI_GENERALEXCEPT,   "GNRL EXCEPT",  "普通例外"},
    {EWIFI_OUTOFMEM,        "OUT OF MEM",   "内存溢出"},
    {EWIFI_LOCPORTINUSE,    "PORT IN USE",  "端口在用"},
    {EWIFI_LOADCA,          "SSL:LOAD CA",  "SSL:载入CA"},
    {EWIFI_NEGSSL3,         "SSL:NEG ERR",  "SSL:协商错"},
    {EWIFI_ILLSSLSOCK,      "SSL:ILL SOCK", "SSL:端口错"},
    {EWIFI_NOTRUSTCA,       "NO TRUST CA",  "无可信CA"},
    {EWIFI_RES2223,         "RES: 2223",    "预留:2223"},
    {EWIFI_SSLDECODE,       "SSL:DECODE E", "SSL:解码错"},
    {EWIFI_NOADDSSLSOCK,    "SSL:NO SOCK",  "SSL:无端口"},
    {EWIFI_SSLPACKSIZE,     "SSL:MAX SIZE", "SSL:包太大"},
    {EWIFI_SSNDSIZE,        "SSND:MAXSIZE", "SSND包太大"},
    {EWIFI_SSNDCHKSUM,      "SSND:CHK SUM", "SSND校验错"},
    {EWIFI_HTTPUNKNOWN,     "HTTP:UNKNOWN", "HTTP:未知"},
    {EWIFI_HTTPTO,          "HTTP:TIMEOUT", "HTTP:超时"},
    {EWIFI_HTTPFAIL,        "HTTP:FAIL",    "HTTP:失败"},
    {EWIFI_NOURL,           "NO URL",       "无URL"},
    {EWIFI_ILLHTTPNAME,     "HTTP:ILLNAME", "HTTP无效名"},
    {EWIFI_ILLHTTPPORT,     "HTTP:ILLPORT", "HTTP端口错"},
    {EWIFI_ILLURL,          "ILL URL ADDR", "URL地址错"},
    {EWIFI_URLTOOLONG,      "URL:TOO LONG", "URL太长"},
    {EWIFI_WWWFAIL,         "WWW FAIL",     "WWW错"},
    {EWIFI_MACEXIST,        "EXIST MAC",    "MAC已存在"},
    {EWIFI_NOIP,            "NO IP ADDR",   "无IP地址"},
    {EWIFI_WLANPWR,         "WLAN:PWR ERR", "WLAN上电错"},
    {EWIFI_WLANRADIO,       "WLAN:RADIO E", "WLAN电波错"},
    {EWIFI_WLENRESET,       "WLAN:RESET E", "WLAN复位错"},
    {EWIFI_WLANHWSETUP,     "WLAN:HW FAIL", "WLAN硬件错"},
    {EWIFI_WIFIBUSY,        "WIFI:BUSY",    "WIFI忙"},
    {EWIFI_ILLWIFICHNL,     "WIFI:CHANNEL", "WIFI信道错"},
    {EWIFI_ILLSNR,          "WIFI:SNR",     "WIFI SNR错"},
    {EWIFI_RES2500,         "RES: 2500",    "预留:2500"},
    {EWIFI_COMMPLATFORM,    "PLATFORM ACT", "平台已激活"},
    {EWIFI_RES2502,         "RES: 2502",    "预留:2502"},
    {EWIFI_RES2503,         "RES: 2503",    "预留:2503"},
    {EWIFI_RES2504,         "RES: 2504",    "预留:2504"},
    {EWIFI_ALLFTPINUSE,     "FTP HDL USE",  "FTP在使用"},
    {EWIFI_NOTANFTP,        "NOT AN FTP",   "不是FTP"},
    {EWIFI_FTPSVRFOUND,     "NO FTP SVR",   "无FTP主机"},
    {EWIFI_CONNFTPTO,       "FTP:TIMEOUT",  "FTP超时"},
    {EWIFI_FTPLOGIN,        "FTP:LOGIN",    "FTP登录错"},
    {EWIFI_FTPCMD,          "FTP:CMD ERR",  "FTP命令错"},
    {EWIFI_FTPDATASOCK,     "FTP:DSOCKERR", "FTP:DSOCK"},
    {EWIFI_FTPSEND,         "FTP:SEND ERR", "FTP发送错"},
    {EWIFI_FTPDOWN,         "FTP:SVR DOWN", "FTP主机断"},
    {EWIFI_RES2514,         "RES: 2514",    "预留:2514"},
    {EWIFI_TELNETSVR,       "TNET:NO SVR",  "TNET无主机"},
    {EWIFI_CONNTELNETTO,    "TNET:TIMEOUT", "TNET:超时"},
    {EWIFI_TELNETCMD,       "TNET:CMD ERR", "TNET命令错"},
    {EWIFI_TELNETDOWN,      "TNET:SVRDOWN", "TNET主机断"},
    {EWIFI_TELNETACTIVE,    "TNET:NOT ACT", "TNET未激活"},
    {EWIFI_TELNETOPENED,    "TNET:OPENED",  "TNET已打开"},
    {EWIFI_TELNETBINMODE,   "TNET:BIN MOD", "TNETBINMOD"},
    {EWIFI_TELNETASCMODE,   "TNET:ASC MOD", "TNETASCMOD"},
    {EWIFI_RES2558,         "RES: 2558",    "预留:2558"},
    {EWIFI_RES2559,         "RES: 2559",    "预留:2559"},
    {EWIFI_RETRIEVERSP,     "MAIL RETRIEV", "邮件恢复错"},
    {EWIFI_SNETSOCKCLOSE,   "SSOCK CLESED", "远端关闭"},
    {EWIFI_PINGDEST,        "PING DST ERR", "PING不可达"},
    {EWIFI_PINGNOREPLY,     "PING NOREPLY", "PING无回应"}
};

//COMM_FUNCTION *g_pCommFunction;

/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

//////////////////////////////////////////////////////////////////////////
// RS232 通讯模块
//////////////////////////////////////////////////////////////////////////
// 串口打开
static int s_RS232Dial(const RS232_PARAM *pRs232Param)
{
    int iRet;

    if (-1 != sg_iRs232Status)
    {
        return PORT_RET_OK;
    }

    iRet = portOpen(pRs232Param->byPortNo, (U08*)pRs232Param->szAttr);
    if (PORT_RET_OK != iRet)
    {
        return iRet;
    }

    sg_iRs232Status = 1;
    return PORT_RET_OK;
}


// 串?谥苯臃???
static int s_RS232RawTxd(const RS232_PARAM *pRs232Param, const U08 *psTxdData,
                    U32 uiDataLen, U32 uiTimeOutMs)
{
    int iRet;
    U32 uiOldTime, uiNowTime;

    if (uiTimeOutMs <= 0)
    {
        uiTimeOutMs = 2000;
    }
    uiOldTime = sysGetTimerCount();
    iRet = 0;
    while (1)
    {
        uiNowTime = sysGetTimerCount();
        if ((uiNowTime-uiOldTime) >= uiTimeOutMs)
        {
            break;
        }

        iRet = portSends(pRs232Param->byPortNo, (U08*)psTxdData,uiDataLen);
        if (PORT_RET_OK == iRet)
        {
            break;
        }
        sysDelayMs(50);
    }

    return iRet;
}

// STX+Len1+Len2+strings+ETX+CKS, CKS = Len1 -- ETX (^)
static int s_RS232NacTxd(const RS232_PARAM *pRs232Param, const U08 *psTxdData,
                   U32 uiDataLen, U32 uiTimeOutMs)
{
    int iRet;

    if (uiDataLen > LEN_TEMP_BUFF)
    {
        return ERR_COMM_TOOBIG;
    }

    g_abySetBuff[0] = STX;
    g_abySetBuff[1] = (uiDataLen/1000)<<4    | (uiDataLen/100)%10;    // convert to BCD
    g_abySetBuff[2] = ((uiDataLen/10)%10)<<4 | uiDataLen%10;
    memcpy(&g_abySetBuff[3], psTxdData, uiDataLen);
    g_abySetBuff[3+uiDataLen]   = ETX;
    g_abySetBuff[3+uiDataLen+1] = CalcLRC((U08*)psTxdData, uiDataLen,
        (U08)(g_abySetBuff[1] ^ g_abySetBuff[2] ^ ETX));

    iRet = s_RS232RawTxd(pRs232Param, g_abySetBuff, (U32)(uiDataLen+5), uiTimeOutMs);    // data
    if (0 != iRet)
    {
        return iRet;
    }

    return 0;
}


// 串口直接接收
static int s_RS232RawRxd(const RS232_PARAM *pRs232Param, U08 *psRxdData,
              U32 uiExpLen, U32 uiTimeOutMs, U32 *puiOutLen)
{
    int iRet, iFlag;
    U32 uiReadCnt, uiTemp;
    U32 uiOldTimeMs, uiNewTimeMs;

    uiReadCnt = iFlag = 0;
    uiTemp = 5000;
    uiOldTimeMs = sysGetTimerCount();
    while (uiReadCnt < uiExpLen)
    {
        if (0 == iFlag)
        {
            uiNewTimeMs = sysGetTimerCount();
            if ((uiNewTimeMs-uiOldTimeMs) >= uiTimeOutMs)
            {
                if (uiReadCnt > 0)
                {
                    break;
                }

                return ERR_COMM_TIMEOUT;
            }

            lcdDisplay(87,4,DISP_CFONT,"(%d)",(uiNewTimeMs-uiOldTimeMs)/1000);
            iRet = portCheckRecvBuf(pRs232Param->byPortNo);
            if (PORT_RET_NOTEMPTY != iRet)
            {
                if (PORT_RET_OK != iRet)
                {
                    return iRet;
                }

                continue;
            }
            else
            {
                iFlag = 1;
            }
        }

        iRet = portRecv(pRs232Param->byPortNo, psRxdData, uiTemp);
        if (0x00 == iRet)
        {    // 接收成功,继续
            uiTemp = 80;
            psRxdData++;
            uiReadCnt++;
        }
        else if (0xff == iRet)
        {
            if (uiReadCnt > 0)
            {
                break;
            }

            return ERR_COMM_TIMEOUT;
        }
        else
        {    // 非超时错误,退出
            return iRet;
        }
    }   // end of while( uiReadCnt<uiExpLen

    if (puiOutLen != NULL)
    {
        *puiOutLen = uiReadCnt;
    }

    return 0;
}

// STX+Len1+Len2+strings+ETX+CKS, CKS = Len1 -- ETX (^)
static int s_RS232NacRxd(const RS232_PARAM *pRs232Param, U08 *psRxdData,
            U32 uiExpLen, U32 uiTimeOutMs, U32 *puiOutLen)
{
    int iRet, iFlag;
    U32 uiReadCnt, uiLength;
    U32 uiOldTimeMs, uiNewTimeMs, uiTemp;

    if (uiExpLen > LEN_TEMP_BUFF)
    {
        return ERR_COMM_TOOBIG;
    }

    iFlag = 0;
    uiTemp = 5000;
    uiReadCnt = uiLength = 0;
    memset(g_abySetBuff, 0, sizeof(g_abySetBuff));
    uiOldTimeMs = sysGetTimerCount();
    while (1)
    {
        if (0 == iFlag)
        {
            uiNewTimeMs = sysGetTimerCount();
            if ((uiNewTimeMs-uiOldTimeMs) >= uiTimeOutMs)    // 检查定时器
            {
                if (uiReadCnt > 0)    // 已经读取到数据
                {
                    break;
                }

                return ERR_COMM_TIMEOUT;
            }

            lcdDisplay(87,4,DISP_CFONT,"(%d)",(uiNewTimeMs-uiOldTimeMs)/1000);
            iRet = portCheckRecvBuf(pRs232Param->byPortNo);
            if (PORT_RET_NOTEMPTY != iRet)
            {
                if (PORT_RET_OK != iRet)
                {
                    return iRet;
                }

                continue;
            }
            else
            {
                iFlag = 1;
            }
        }

        iRet = portRecv(pRs232Param->byPortNo, &g_abySetBuff[uiReadCnt], uiTemp);
        if (0 !=  iRet)
        {
            if (0xff == iRet)
            {
                continue;
            }
            return iRet;
        }
        uiTemp = 100;
        if (STX != g_abySetBuff[0])
        {
            continue;
        }

        uiReadCnt++;
        if (3 == uiReadCnt)
        {
            uiLength = ((g_abySetBuff[1]>>4)& 0x0F)*1000 + (g_abySetBuff[1]&0x0F)*100 +
                       ((g_abySetBuff[2]>>4)& 0x0F)*10   + (g_abySetBuff[2]&0x0F);
        }
        if (uiReadCnt == (uiLength+5))
        {    // read data ok, verify it ...
            if (  (g_abySetBuff[uiReadCnt-2]==ETX)
                && (CalcLRC(&g_abySetBuff[1],(U32)(uiReadCnt-1),0)==0))
            {
                break;
            }
            return ERR_COMM_COMERR;
        }
    }

    memcpy(psRxdData, &g_abySetBuff[3], uiLength);
    if (NULL != puiOutLen)
    {
        *puiOutLen = uiLength;
    }

    return 0;
}

// 串口发送数据
static int s_RS232Txd(const RS232_PARAM *pRs232Param, const U08 *psTxdData,
                   U32 uiDataLen, U32 uiTimeOutMs)
{
    int iRet;

    if (CM_RAW == pRs232Param->bySendMode)
    {
        iRet = s_RS232RawTxd(pRs232Param, psTxdData, uiDataLen, uiTimeOutMs);
    }
    else
    {
        iRet = s_RS232NacTxd(pRs232Param, psTxdData, uiDataLen, uiTimeOutMs);
    }

    return iRet;
}

// 串口接收
static int s_RS232Rxd(const RS232_PARAM *pRs232Param, U08 *psRxdData,
               U32 uiExpLen, U32 uiTimeOutMs, U32 *puiOutLen)
{
    int iRet;

    if (CM_RAW == pRs232Param->bySendMode)
    {
        iRet = s_RS232RawRxd(pRs232Param, psRxdData, uiExpLen, uiTimeOutMs, puiOutLen);
    }
    else
    {
        iRet = s_RS232NacRxd(pRs232Param, psRxdData, uiExpLen, uiTimeOutMs, puiOutLen);
    }

    return iRet;
}


// 串口关闭
static int s_RS232OnHook(U08 bReleaseAll, const RS232_PARAM *pRs232Param)
{
    int iRet;

    iRet = portClose(pRs232Param->byPortNo);
    if (0 == iRet)
    {
        sg_iRs232Status = -1;
    }

    return iRet;
}

static int s_SetSockAddr(const COMM_CFG_PARAM *psCommParam)
{
    if (   (psCommParam->byComType==COMM_TYPE_CDMA)
        || (psCommParam->byComType==COMM_TYPE_GPRS)
        || (psCommParam->byComType==COMM_TYPE_LAN)
        || (psCommParam->byComType==COMM_TYPE_WIFI))
    {
       return SockAddrset(&sg_stTcpsockaddr,"192.168.0.15",
		//return SockAddrset(&sg_stTcpsockaddr,"",
           2000);
    }
    return 0;
}

// 断开TCP连接
static int s_TcpClose(void)
{
    int iRet;

    iRet = 0;
    if (sg_Tcpsocket >= 0)
    {
        iRet = NetClose(sg_Tcpsocket);
    }
    sg_Tcpsocket = -1;

    return iRet;
}

//////////////////////////////////////////////////////////////////////////
// TCP 通讯模块
//////////////////////////////////////////////////////////////////////////
// 建立TCP连接
static int s_TcpConnect(const COMM_CFG_PARAM *psCommParam)
{
    int iRet, iRetyTimes;
    U32 uiOldTime;

    //  实际连接
    iRetyTimes = 1;
    uiOldTime = sysGetTimerCount();
    while (1)
    {
        if ( sysGetTimerCount() >= (uiOldTime+60000) )
        { // 拨号连接超时60s，则返回
            s_TcpClose();
            return NET_ERR_RETRY;
        }

        if (0 != kbhit())
        {
            if (kbGetKey() == KEY_CANCEL)
            {
                s_TcpClose();
                return ERR_USERCANCEL;
            }
        }

        if (iRetyTimes > 3 )
        {// 三次拨号连接不成功则关闭套节字，断开PPP连接
            s_TcpClose();
            return NET_ERR_RETRY;
        }

        if (sg_Tcpsocket < 0)
        {
            if (0 == g_oCommCfgParam.byUdpFlag)
            {
                sg_Tcpsocket = NetSocket(AF_INET, SOCK_STREAM, 0);  // TCP
            }
            else
            {
                sg_Tcpsocket = NetSocket(AF_INET, SOCK_DGRAM, 0); // UDP
            }
            if (sg_Tcpsocket < 0)
            {
                s_TcpClose();
                return sg_Tcpsocket;
            }
            iRet = s_SetSockAddr(psCommParam);
            if (iRet != NET_OK)
            {
                s_TcpClose();
                return iRet;
            }

            lcdClrLine(2,7);
            if(0 == g_iLangueSelect)
            {
                if (1 == g_oCommCfgParam.byUdpFlag)
                {
                    lcdDisplay(0, 4, DISP_CFONT, "UDPIP CONNECT(%d)", iRetyTimes++);
                }
                else
                {
                    lcdDisplay(0, 4, DISP_CFONT, "TCPIP CONNECT(%d)", iRetyTimes++);
                }
            }
            else
            {
                lcdDisplay(0, 3, DISP_CFONT, "   连 接 中 ... ");
                if (1 == g_oCommCfgParam.byUdpFlag)
                {
                    lcdDisplay(0, 5, DISP_CFONT, "UDPIP CONNECT(%d)", iRetyTimes++);
                }
                else
                {
                    lcdDisplay(0, 5, DISP_CFONT, "TCPIP CONNECT(%d)", iRetyTimes++);
                }
            }
            DrawRect(0, 17, 127, 63);

            iRet = NetConnect(sg_Tcpsocket, &sg_stTcpsockaddr, sizeof(struct sockaddr));
            if ((-NET_ERR_USER_CANCEL==iRet) || (NET_OK==iRet))
            {
                return iRet;
            }
            else
            {
                s_TcpClose();  // 挂断TCPIP重新连接
                continue;
            }
        }
        else
        {
            return 0;
        }
    }
}

// 发送数据
static int s_TcpTxd(const U08 *psTxdData, U32 uiDataLen)
{
    int  iRet,iRelSendlen;
    struct timeval stTimeVal;

    stTimeVal.tv_sec = 30;
    stTimeVal.tv_usec = 0;
    NetSetsockopt(sg_Tcpsocket, SOL_SOCKET, SO_SNDTIMEO, &stTimeVal, sizeof(stTimeVal));

    for (iRelSendlen=0; iRelSendlen<(int)uiDataLen;)
    {
        iRet = NetSend(sg_Tcpsocket, psTxdData+iRelSendlen, (uiDataLen-iRelSendlen), 0);
        if (iRet < 0)
        {
            s_TcpClose();
            lcdClrLine(4, 7);
            lcdDisplayCE(0, 4, 0x01, "数据发送错误... ","CommTxd error.  ");
            lcdDisplay(0, 6, 0x01, "iRet=%d,", iRet);
            return iRet;
        }
        iRelSendlen += iRet;
    }

    return 0;
}

// 接收数据
// 返回值为0或者ERR_COMM_TIMEOUT=4.
 static int s_TcpRxd(U08 *psRxdData, U32 uiExpLen,
                         U32 uiTimeOutMs, U32 *puiOutLen)
{
    int iRet, iRemainLen;
    U32 uiOldTime, uiNowTime;
    U32 ulRealRecvLen = 0;
    struct timeval stTimeVal;
    socklen_t Socklen;

    stTimeVal.tv_sec = 1;
    stTimeVal.tv_usec = 0;
    Socklen = sizeof(stTimeVal);
    NetSetsockopt(sg_Tcpsocket, SOL_SOCKET, SO_RCVTIMEO, &stTimeVal, Socklen);
    iRemainLen = uiExpLen;

    uiOldTime = sysGetTimerCount();
    while (1)
    {
        uiNowTime= sysGetTimerCount();
        if ((uiNowTime-uiOldTime) >= uiTimeOutMs)    // 检查定时器
        {
            iRet = ERR_COMM_TIMEOUT;
            s_TcpClose();
            lcdClrLine(4, 7);
            lcdDisplayCE(0, 4, 0x01, "数据接收超时... ", "Comm Rxd Timeout");
            break;
        }
        if (0 != kbhit())
        {
            if (KEY_CANCEL == kbGetKey())
            {
                iRet = ERR_COMM_CANCEL;
                s_TcpClose();
                lcdClrLine(4, 7);
                lcdDisplayCE(0, 4, 0x01, "用户取消        ", "User cancel!    ");
                break;
            }
        }

        lcdDisplay(87, 4, DISP_CFONT, "(%d)", (uiNowTime-uiOldTime)/1000);
        lcdDisplay(0, 6, DISP_CFONT, "(%d)", ulRealRecvLen);
        iRet = NetRecv(sg_Tcpsocket,psRxdData+ulRealRecvLen,iRemainLen,0);
        if (iRet > 0)
        {
            ulRealRecvLen += iRet;
            iRemainLen -= iRet;
            if (iRemainLen > 0)
            {
                continue; // 没接收完，继续接收
            }

            *puiOutLen = ulRealRecvLen;
            iRet = 0;
            break; // 接收完了，退出循环
        }
        if (-NET_ERR_USER_CANCEL == iRet)
        {
            return iRet;
        }
        else
        {
            sysDelayMs(20);
        }
    }

    return iRet;
}

// 0:Right, 1:Error
static int s_PSTN_CheckHandleValid(int iFd)
{
    if ((iFd>=1) && (iFd<=1024))
    {
        return 0;
    }
    return 1;
}

// Modem 挂机
static int s_PSTN_OnHook(U08 byReleaseAll)
{
    int iRet,iStatus;
    
    if (0 != s_PSTN_CheckHandleValid(sg_iModemfd))
    {
        return 0;
    }

    modem_wait_until_sent(sg_iModemfd);

    // 排除连续多次挂断的请求
    iRet = modem_get_status(sg_iModemfd, &iStatus);
    if ((PSTN_MODEM_OPERATE_NOT_FINISH|MODEM_STATE_CONNECT) != iStatus)
    {
        // iStatus的最高位表示modem后台操作是否完成，如果最高位为1表示后台正在处理，
        // 当前状态是不稳定状态；其它位表示当前设备状态，见前面状态宏定义。
        iRet = modem_hangup(sg_iModemfd);
    }

    if (0 != byReleaseAll)
    {
        while (1)
        {
            iRet = modem_get_status(sg_iModemfd, &iStatus);
            if (0 != iRet)
            {
                break;
            }

            if (0 != (iStatus&PSTN_MODEM_OPERATE_NOT_FINISH)) // 目前最高位为1表示操作还没完成，需要再次检测
            {
                sysDelayMs(100);
                continue;
            }

            break;
        }
        iRet = modem_close(sg_iModemfd);
        sg_iModemfd = -1;
    }

    return iRet;
}

// 0:Get status OK, other:Get status failed.
static int  s_PSTN_GetStatus(int *piStatus, const ModemDialParms_t *pstModemParam)
{
    int iRet, iStatus;
    int iOldTime, iTimeOut;

    iTimeOut = pstModemParam->dial_timeo;
    if ((iTimeOut >= 30) || (iTimeOut<=0))
    {
        iTimeOut = 30;
    }
    iOldTime = sysGetTimerCount();
    while (1)
    {
        if (sysGetTimerCount() >= (U32)(iOldTime+iTimeOut*1000))
        {
            // time out and return.
            modem_get_last_errno(sg_iModemfd,&iStatus);
            if (0 != iStatus)
            {
                return iStatus;
            }
            else
            {
                return ERR_COMM_STATUS_TIMEOUT;
            }
        }

        iRet = modem_get_status(sg_iModemfd, &iStatus);
        if (0 != iRet)
        {
            // Get status failed. 
            return iRet; // return -EINVAL/-EBADFD
        }

        if (0 == (iStatus&PSTN_MODEM_OPERATE_NOT_FINISH))
        {
            *piStatus = iStatus;
            return 0;
        }
        sysDelayMs(100);
        // The operation has not finished, need to check again.
    }
}

 
static int  s_PSTN_OpenModem(void)
{
    int   iFd;

    // The right fd is from 1 to 1024, the error fd is a negative number.
    iFd = modem_open(MODEM_PATHNAME_NEW8110, O_RDWR);
    if (iFd < 0)
    {
        iFd = modem_open(MODEM_PATHNAME_NEW6110, O_RDWR);
    }

    return iFd;
}

static int  s_PSTN_InitModem(const ModemDialParms_t *pstModemParam)
{
    int  iRet, iStatus;
    ModemDialParms_t  stOldModemParam;

    // Before setting parameter, open the modem first.
    if (0 != s_PSTN_CheckHandleValid(sg_iModemfd))
    {
        sg_iModemfd = s_PSTN_OpenModem();
        if (0 != s_PSTN_CheckHandleValid(sg_iModemfd))
        {
            return ERR_COMM_MODEM_INIT;
        }
    }

    iStatus = MODEM_STATE_NOT_INIT;
    iRet = s_PSTN_GetStatus(&iStatus, pstModemParam);
    if ((0!=iRet) || (MODEM_STATE_NOT_INIT==iStatus))
    {
        return MODEM_STATE_NOT_INIT;
    }

    iRet = modem_get_dial_parms(sg_iModemfd, &stOldModemParam);
    if (   (0!=iRet)
        || (0!=memcmp(&stOldModemParam,pstModemParam,sizeof(ModemDialParms_t))))
    {
        return modem_set_dial_parms(sg_iModemfd,pstModemParam);
    }
    else
    {
        return 0;
    }
}

// Set PSTN dial telephone number
int s_PSTN_SetTel(U08 *pszTelNo, const U08 *pszPromptInfo)
{
    int iRet;

    lcdClrLine(2, 7);
    lcdDisplay(0, 4, 0x01, (U08*)pszPromptInfo);
    lcdGoto(0, 6);
    iRet = kbGetString(0xf5, 0, 24, USER_OPER_TIMEOUT, pszTelNo);
    if (KB_CANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }
    return 0;
}

// 修改Modem参数
static void s_PSTN_ParamSet(PSTN_PARAM *pPstnParam)
{
    int   iRet, iCurMsg, iNumber, i;
    U08 byMode, byRet, byCurPage, abyBuff[32];

    byMode = LCD_EXIT_MODE0|LCD_MODE_2ITEM_PER_LINE|LCD_MODE_SHOW_USER_SELECT;
    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  PSTN参数设置  ", "PSTN DIAL PARAM ");
    iCurMsg = pPstnParam->byPreDial + 1;
    iRet = LcdTwoSelectOneShort("    PRE DIAL    ", "ON", "OFF",  iCurMsg, 120000, byMode);
    if ((1==iRet) || (2==iRet))
    {
        pPstnParam->byPreDial = iRet - 1;
    }

    if (0 == pPstnParam->bySupportPPP)
    {
        lcdCls();
        lcdDisplayCE(0, 0, 0x81, "  PSTN参数设置  ", "PSTN DIAL PARAM ");
        if (MODEM_COMM_ASYNC == pPstnParam->stDialPara.connect_mode)
        {
            iCurMsg = 1;
        }
        else if (MODEM_COMM_SYNC == pPstnParam->stDialPara.connect_mode)
        {
            iCurMsg = 2;
        }
        iRet = LcdTwoSelectOneShort("  CONNECT MODE  ", "ASYNC", "SYNC", iCurMsg, 120000, byMode);
        if (1 == iRet)
        {
            pPstnParam->stDialPara.connect_mode = MODEM_COMM_ASYNC;
        }
        else if (2 == iRet)
        {
            pPstnParam->stDialPara.connect_mode = MODEM_COMM_SYNC;
        }
    }
    else
    {
        pPstnParam->stDialPara.connect_mode = MODEM_COMM_ASYNC;
    }


    if (MODEM_COMM_ASYNC == pPstnParam->stDialPara.connect_mode)
    {
        iNumber = 17;
    }
    else
    {
        iNumber = 2;
    }
    iCurMsg = 0;
    for (i=0; i<iNumber; i++)
    {
        sprintf(&g_abySetBuff[i*ITEM_LENGTH], "%d", g_aiModemConnectSpeed[i]);
        if (pPstnParam->stDialPara.connect_speed == g_aiModemConnectSpeed[i])
        {
            iCurMsg = i+1;
        }
    }
    byMode = LCD_EXIT_MODE0|LCD_MODE_2ITEM_PER_LINE|LCD_MODE_SHOW_USER_SELECT;
    byCurPage = 0;
    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  PSTN参数设置  ", "PSTN DIAL PARAM ");
    byRet = LcdGetItemBigTimeout(g_abySetBuff, iNumber, iCurMsg, byMode, 120000, &byCurPage);
    if ((0!=byRet) && (byRet<=iNumber))
    {
        pPstnParam->stDialPara.connect_speed = g_aiModemConnectSpeed[byRet-1];
    }


    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  PSTN参数设置  ", "PSTN DIAL PARAM ");
    iCurMsg = pPstnParam->stDialPara.dial_mode + 1;
    byMode = LCD_EXIT_MODE0|LCD_MODE_2ITEM_PER_LINE|LCD_MODE_SHOW_USER_SELECT;
    iRet = LcdTwoSelectOneShort("   DIAL MODE    ", "DTMF", "PULSE", iCurMsg, 120000, byMode);
    if ((1 == iRet) || (2 == iRet))
    {
        pPstnParam->stDialPara.dial_mode = iRet - 1;
    }


    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  PSTN参数设置  ", "PSTN DIAL PARAM ");
    lcdDisplay(0, 2, 0x01, "OLD:%d(s)", pPstnParam->stDialPara.dial_pause);
    lcdDisplay(0, 4, 0x01, "DIAL PAUSE:");
    sprintf((char*)abyBuff, "%d", pPstnParam->stDialPara.dial_pause);
    lcdGoto(0, 6);
    iRet = kbGetString(0xe5, 0, 2, 120000, abyBuff);
    if (iRet >= 0)
    {
        pPstnParam->stDialPara.dial_pause = atoi((char *)abyBuff);
    }

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  PSTN参数设置  ", "PSTN DIAL PARAM ");
    lcdDisplay(0, 2, 0x01, "OLD:%d(s)", pPstnParam->stDialPara.dial_timeo);
    lcdDisplay(0, 4, 0x01, "DIAL TIMEOUT:");
    sprintf((char*)abyBuff, "%d", pPstnParam->stDialPara.dial_timeo);
    lcdGoto(0, 6);
    iRet = kbGetString(0xe5, 0, 2, 120000, abyBuff);
    if (iRet >= 0)
    {
        pPstnParam->stDialPara.dial_timeo = atoi((char *)abyBuff);
    }

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  PSTN参数设置  ", "PSTN DIAL PARAM ");
    lcdDisplay(0, 2, 0x01, "OLD:%d(s)", pPstnParam->stDialPara.idle_timeo);
    lcdDisplay(0, 4, 0x01, "IDLE TIME:");
    lcdGoto(0, 6);
    sprintf((char*)abyBuff, "%d", pPstnParam->stDialPara.idle_timeo);
    iRet = kbGetString(0xe5, 0, 3, 120000, abyBuff);
    if (iRet >= 0)
    {
        pPstnParam->stDialPara.idle_timeo = atoi((char *)abyBuff);
    }

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  PSTN参数设置  ", "PSTN DIAL PARAM ");
    iCurMsg = pPstnParam->stDialPara.extension_chk + 1;
    byMode = LCD_EXIT_MODE0|LCD_MODE_2ITEM_PER_LINE|LCD_MODE_SHOW_USER_SELECT;
    iRet = LcdTwoSelectOneShort("  CHECK LINE    ", "NO", "YES", iCurMsg, 120000, byMode);
    if ((1 == iRet) || (2 == iRet))
    {
        pPstnParam->stDialPara.extension_chk = iRet - 1;
    }
    s_PSTN_SetTel(pPstnParam->szTxnTelNo, "TXN TEL:        ");

    iRet = s_PSTN_InitModem(&pPstnParam->stDialPara);
    if (0 != iRet)
    {
        lcdCls();
        lcdDisplayCE(0, 0, 0x81, "  PSTN参数设置  ", "PSTN DIAL PARAM ");
        lcdDisplayCE(0, 4, 0x01, "modem初始化错误 ", "Init modem error");
        lcdDisplay(0, 6, 0x01, "ret=%d,", iRet);
        kbGetKeyMs(60000);
    }
}

// modem 拨号
static int s_PSTNDial(U08 ucDialMode, const PSTN_PARAM *pPstnParam)
{
    int iRet, iStatus, iDialTimes, iLastErr;
    U32 uiOldTime;
    int iReInitFlag;

    if (PREDIAL_MODE == ucDialMode)
    {
        iStatus = MODEM_STATE_NOT_INIT;
        uiOldTime = sysGetTimerCount();
        while (1)
        {
            if (sysGetTimerCount() >= (uiOldTime+10000))
            {
                s_PSTN_OnHook(TRUE);
                return NET_ERR_RETRY;  // time out and return
            }

            // Check and open
            if (0 != s_PSTN_CheckHandleValid(sg_iModemfd))
            {
                sg_iModemfd = s_PSTN_OpenModem();
                if (0 != s_PSTN_CheckHandleValid(sg_iModemfd))
                {
                    return ERR_COMM_MODEM_INIT;
                }
            }

            iStatus = MODEM_STATE_NOT_INIT;
            iRet = s_PSTN_GetStatus(&iStatus, &pPstnParam->stDialPara);
            if (0 != iRet)
            {
                // The handle is valid, but can not get the status success,
                s_PSTN_OnHook(TRUE);
                continue;
            }

            switch (iStatus)
            {
            case MODEM_STATE_NOT_INIT:
                s_PSTN_OnHook(TRUE);
                sg_iModemfd = s_PSTN_OpenModem();
                if (0 != s_PSTN_CheckHandleValid(sg_iModemfd))
                {
                    return ERR_COMM_MODEM_INIT;
                }
                break;

            case MODEM_STATE_NO_SET_MODE:
                // 在调用modem_close()接口之后或者机器重启后，需要调用该接口
                iRet = modem_set_dial_parms(sg_iModemfd, &pPstnParam->stDialPara);
                if (0 != iRet)
                {
                    return ERR_COMM_MODEM_NOPARAM;
                }
                break;

            case MODEM_STATE_SYNC_MODE:  // 该状态表示正在设置,需要等待设置成功
            case MODEM_STATE_ASYN_MODE:  // 该状态表示正在设置,需要等待设置成功
                break;

            case MODEM_STATE_DISCONNECT:
                return modem_dialing(sg_iModemfd, pPstnParam->szTxnTelNo);

            case MODEM_STATE_WAITTING:  // 被拨号状态，先挂断当前被拨连接，再重新拨号
                s_PSTN_OnHook(FALSE);
                break;

            default:
                return 0;
            }
        }

        return ERR_COMM_MODEM_INIT;
    }

    iDialTimes = 0;
    iReInitFlag = 0;
    uiOldTime = sysGetTimerCount();
    while (1)
    {
        if ( sysGetTimerCount() >= (uiOldTime+60000) )
        { // 拨号连接超时60s，则返回
            s_PSTN_OnHook(TRUE);
            return NET_ERR_RETRY;
        }

        if (0 != kbhit())
        {
            if (KEY_CANCEL == kbGetKey())
            {
                s_PSTN_OnHook(FALSE);
                return ERR_USERCANCEL;
            }
        }

        lcdClrLine(2, 7);
        if (0 == g_iLangueSelect)
        {
            lcdDisplay(0, 4, DISP_CFONT, "  DIALING...(%d)", iDialTimes);
        }
        else
        {
            lcdDisplay(0, 3, DISP_CFONT, "    拨 号 中   ");
            lcdDisplay(0, 5, DISP_CFONT, "  DIALING...(%d)", iDialTimes);
        }
        DrawRect(0, 17, 127, 63);
        
        // Check and open
        if (0 != s_PSTN_CheckHandleValid(sg_iModemfd))
        {
            sg_iModemfd = s_PSTN_OpenModem();
            if (0 != s_PSTN_CheckHandleValid(sg_iModemfd))
            {
                return ERR_COMM_MODEM_INIT;
            }
        }

        iStatus = MODEM_STATE_NOT_INIT;
        iRet = s_PSTN_GetStatus(&iStatus, &pPstnParam->stDialPara);
        if (0 != iRet)
        {
            return iRet;
        }

        switch (iStatus)
        {
        case MODEM_STATE_NOT_INIT:
            s_PSTN_OnHook(TRUE);
            sg_iModemfd = s_PSTN_OpenModem();
            if (0 != s_PSTN_CheckHandleValid(sg_iModemfd))
            {
                return ERR_COMM_MODEM_INIT;
            }
            break;

        case MODEM_STATE_NO_SET_MODE:
            iRet = modem_set_dial_parms(sg_iModemfd, &pPstnParam->stDialPara);
            if (0 != iRet)
            {
                return ERR_COMM_MODEM_NOPARAM;
            }
            break;

        case MODEM_STATE_SYNC_MODE:  // 该状态表示正在设置,需要等待设置成功
        case MODEM_STATE_ASYN_MODE:  // 该状态表示正在设置,需要等待设置成功
        case MODEM_STATE_DAILING:
        case MODEM_STATE_CONNECT_SDLC:
            sysDelayMs(100); // 延时后再查看
            break;

        case MODEM_STATE_DISCONNECT:
            if (iDialTimes >= 3)  // 三次不成功则退出失败
            {
                modem_get_last_errno(sg_iModemfd, &iLastErr);
                if (iLastErr == 0)
                {
                    iLastErr = MODEM_ERRNO_NO_CARRIER;
                }
                return iLastErr;
            }

            if (1 == iReInitFlag)
            {
                s_PSTN_OnHook(TRUE);
                iReInitFlag = 0;
                continue;
            }

            iRet = modem_dialing(sg_iModemfd, pPstnParam->szTxnTelNo);
            if (0 != iRet)
            {
                return iRet;
            }
            iDialTimes++;
            iReInitFlag = 1;
            break;

        case MODEM_STATE_WAITTING:  // 被拨号状态，先挂断当前被拨连接，再重新拨号
            s_PSTN_OnHook(FALSE);
            break;

        case MODEM_STATE_CONNECT:
            return 0;

        default:
            return iStatus;
        }
    }

    return MODEM_ERRNO_ERROR;
}

// Modem 发送
static int s_PSTNTxd(const U08 *psTxdData, U32 uiDataLen, U32 uiTimeOutMs)
{
    int  iRet,iRelSendlen;

    for (iRelSendlen=0; iRelSendlen<(int)uiDataLen;)
    {
        iRet = modem_write_timeout(sg_iModemfd, psTxdData+iRelSendlen,
            (uiDataLen-iRelSendlen), uiTimeOutMs);
        if (iRet < 0)
        {
            s_PSTN_OnHook(FALSE);   // 发送失败后断开连接，下次重新连接
            return iRet;
        }
        iRelSendlen += iRet;
    }

    return 0;
}

// Modem接收
static int s_PSTNRxd(U08 *psRxdData, U32 uiExpLen, U32 uiTimeOutMs, U32 *puiOutLen)
{
    int iRet = MODEM_ERRNO_ERROR;
    U32 uiOldTime, uiNowTime;
    U32 ulRealRecvLen = 0, ulRemainLen;

    uiOldTime = sysGetTimerCount();
    ulRemainLen = uiExpLen;
    while (1)
    {
        uiNowTime = sysGetTimerCount();
        if ((uiNowTime-uiOldTime) >= uiTimeOutMs)
        {
            return ERR_COMM_TIMEOUT;
        }

        lcdDisplay(87, 4, DISP_CFONT,"(%d)", (uiNowTime-uiOldTime)/1000);
        iRet = modem_read_timeout(sg_iModemfd, psRxdData+ulRealRecvLen, ulRemainLen, 850);
        if (iRet > 0)
        {
            ulRealRecvLen += iRet;
            if (ulRealRecvLen >= uiExpLen)
            {
                *puiOutLen = ulRealRecvLen;
                return 0;
            }
            ulRemainLen -= iRet;
        }
        else if (0 == iRet)
        {
            sysDelayMs(100);
        }
        else
        {
            return iRet;  // iRet < 0
        }
    }
}


static int s_ModuleSwitchCheck(U08 byCommType)
{
    int iSwitchFlag, iRet;
    U32 uiOldTime;

    iSwitchFlag = 1;
    if (0 == g_oCommCfgParam.abyDevInfo[11])
    {
        iSwitchFlag = 0; // Not WIFI module, not need to switch
    }
    if ((0==g_oCommCfgParam.abyDevInfo[9]) && (0==g_oCommCfgParam.abyDevInfo[10]))
    {
        iSwitchFlag = 0; // Not GPRS and CDMA module, not need to switch
    }
    if (COMM_TYPE_WIFI == byCommType)
    {
        // Use GPRS/CDMA and need to switch to WIFI
        if ((0==g_iCurModule) && (1==iSwitchFlag))
        {
            g_iCurModule = 1;
            WnetSelectModule(1);
        }
    }
    else
    {
        // Use WIFI and need to switch to GPRS/CDMA
        if ((0!=g_iCurModule) && (1==iSwitchFlag))
        {
            g_iCurModule = 0;
            WnetSelectModule(0);
        }
    }

    uiOldTime = sysGetTimerCount();
    while (1)
    {
        if (sysGetTimerCount() >= (uiOldTime+60*1000))
        {
            return NET_ERR_RETRY;
        }

        if (0 != kbhit())
        {
            if (KEY_CANCEL == kbGetKey())
            {
                return ERR_USERCANCEL;
            }
        }
        // Restart the WIFI or GPRS(/CDMA)
        iRet = WnetInit(WIRELESS_INITIAL_TIME);
        if (NET_OK == iRet)
        {
            break;
        }
        sysDelayMs(1000);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// GPRS/CDMA 通讯模块
//////////////////////////////////////////////////////////////////////////

static int s_NetGetCodeNumber(void)
{
    return (ARRAY_SIZE(WnetRet));
}

//  返回无线模块返回值的提示串指针
static char *s_ShowWnetPrompt(int IsChPrompt, int RetCode)
{
    int i;
    int codenum;

    codenum = s_NetGetCodeNumber();
    for (i=0; i<codenum; i++)
    {
        if ((RetCode==WnetRet[i].retcode) || (RetCode==-WnetRet[i].retcode))
        {
            if (0 != IsChPrompt)
            {
                return(WnetRet[i].descript_ch);
            }
            else
            {
                return(WnetRet[i].descript_en);
            }
        }
    }
    return(NULL);
}

// 无线模块挂机
static int s_PPPClose(U08 bRelease, U08 byCommType)
{
    int iRet;
    int iDevType;

    iRet = 0;
    if (sg_Tcpsocket >= 0)
    {
        iRet = NetClose(sg_Tcpsocket);
        sg_Tcpsocket = -1;
    }

    iDevType = OPT_DEVWNET;
    if (COMM_TYPE_PSTN_PPP == byCommType)
    {
        iDevType = OPT_DEVMODEM;
    }
#if SUPPORT_GPRS_ALWAYS_LINK
 //   if (bRelease)
//    {
 //       iRet = PPPLogout(iDevType);
 //   }
#else
    if (bRelease)
    {
        iRet = PPPLogout(iDevType);
    }
#endif
    return iRet;
}


// 显示信号强度
static int  s_DispWirelessSignal(void)
{
    int iRet, iSignal;
    U32 dwTime1, dwTime2;

    dwTime1 = sysGetTimerCount();

    while (1)
    {
        iRet = WnetSignal(&iSignal);
        if (0 == iRet)
        {
            if (99 != iSignal)
            {
                break;
            }
            lcdSetIcon(ICON_SIGNAL, 0);
        }
        else if (-NET_ERR_USER_CANCEL == iRet)
        {
            return iRet;
        }
        dwTime2 = sysGetTimerCount();
        if ((dwTime2-dwTime1) > 20000)
        {
            return iRet;
        }
        sysDelayMs(200);
    }

    if((iSignal >= 1) && (iSignal <= 7))
    {
        iSignal = 1;
    }
    else if((iSignal >= 8) && (iSignal <= 13))
    {
        iSignal = 3;
    }
    else if((iSignal >= 14) && (iSignal <= 19))
    {
        iSignal = 4;
    }
    else if((iSignal >= 20) && (iSignal <= 25))
    {
        iSignal = 5;
    }
    else if((iSignal >= 26) && (iSignal <= 31))
    {
        iSignal = 6;
    }
    lcdSetIcon(ICON_SIGNAL, iSignal);
    return iRet;
}

static int s_InitWirelessModule(const WNET_PARAM *psWlsParam)
{
    int   iRet,iCnt;

    sg_Tcpsocket = -1;

    iRet = s_ModuleSwitchCheck(COMM_TYPE_GPRS);
    if (ERR_USERCANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }

    iCnt = 2;  // 2次检查SIM卡，中延时2S，间防止SIM卡上电时间不够的问题
    while (iCnt--)
    {
        // check PIN of SIM card
        iRet = WnetCheckSim();
        if (-NET_ERR_PIN == iRet)
        {
            iRet =  WnetInputSimPin((char*)psWlsParam->szSimPin);
            if (NET_OK != iRet)
            {
                return iRet;
            }

            s_DispWirelessSignal();
            return 0;
        }
        else if (-NET_ERR_USER_CANCEL == iRet)
        {
            return iRet;
        }
        else
        {
            if (NET_OK != iRet)
            {
                sysDelayMs(2000);;
                continue;
            }
            else
            {
                s_DispWirelessSignal();
                return 0;
            }
        }
    }

    s_DispWirelessSignal();

    return iRet;
}

static int s_WirelessReset(const COMM_CFG_PARAM *psCommParam)
{
    int   iRet;

    iRet = s_PPPClose(TRUE, psCommParam->byComType);
    if (-NET_ERR_USER_CANCEL == iRet)
    {
        return iRet;
    }
    iRet = WnetReset();
    if (NET_OK != iRet)
    {
        return iRet;
    }
    sysDelayMs(5000); // 复位后，延时5s

    return s_InitWirelessModule(&(psCommParam->oGprsCdma));
}

/*static */int s_WirelessLogin(const COMM_CFG_PARAM *psCommParam)
{
    int iRet, iSignal, iFlag;
    U32 dwTime1, dwTime2;

    kbFlush();

    iFlag = 0;
    dwTime1 = sysGetTimerCount();
    while (1)
    {
        iRet = WnetSignal(&iSignal);
        if (0 == iRet)
        {
            if ((99!=iSignal) && (iSignal>7))
            {
                if (1 == iFlag)
                {
                    sysDelayMs(2000);; // Form no signal to have, it is need to wait 2 seconds. 
                }
                break;
            }
        }
        else if (-NET_ERR_USER_CANCEL == iRet)
        {
            return iRet;
        }
        dwTime2 = sysGetTimerCount();
        if ((dwTime2-dwTime1) > 60000)
        {
            return iRet;
        }
        sysDelayMs(1000);
        iFlag = 1;
    }

    iRet = PPPLogin(OPT_DEVWNET, psCommParam->oGprsCdma.szAPN,
                            psCommParam->oGprsCdma.szUID,
                            psCommParam->oGprsCdma.szPwd, 0, 60000);
    if (-NET_ERR_LINKOPENING == iRet)
    {
        return NET_OK;
    }
    return iRet;
}

static int s_WirelessDial(const COMM_CFG_PARAM *psCommParam)
{
    int iRet;

    iRet = s_WirelessLogin(psCommParam);
    if (0 != iRet)
    {
        return iRet;
    }
    return s_TcpConnect(psCommParam);
}



//////////////////////////////////////////////////////////////////////////
// WIFI 通讯模块
//////////////////////////////////////////////////////////////////////////
int SelectMenu(int StartLine, int EndLine, int DispMode, int TimeOutMs,
               int PromptNum, int PreCursor, U08 *pPrompt[])
{
    int Cursor, Begin;
    int Height, i, LcdMode, DispLine;
    int iRet;

    if ((PreCursor>=0) && (PreCursor<PromptNum))
    {
        Cursor = PreCursor;
    }
    else
    {
        Cursor = 0;
    }
    Begin  = Cursor;
    EndLine = MIN(EndLine, 7);
    EndLine = MAX(EndLine, 0);
    StartLine = MIN(EndLine, StartLine);
    StartLine = MAX(StartLine, 0);

    if (0 != (DispMode&DISP_CFONT))
    {
        Height = 2;
    }
    else
    {
        Height = 1;
    }

    if ((EndLine+1-StartLine) < Height)
    {
        return KB_ERROR;
    }
    while(1)
    {
        lcdClrLine(StartLine, EndLine);
        DispLine = StartLine;
        for (i=Begin; i<PromptNum; i++)
        {
            if (i == Cursor)
            {
                LcdMode = DispMode ^ DISP_REVERSE;
            }
            else
            {
                LcdMode = DispMode;
            }
            lcdDisplayCE(0, DispLine, LcdMode, pPrompt[i*2], pPrompt[i*2+1]);
            DispLine += Height;
            if ((DispLine+Height-1) > 7)
            {
                break;
            }
        }
        if (0 != Begin)
        {
            lcdSetIcon(ICON_UP, OPENICON);
        }
        else
        {
            lcdSetIcon(ICON_UP, CLOSEICON);
        }
        if (i < (PromptNum-1))
        {
            lcdSetIcon(ICON_DOWN, OPENICON);
        }
        else
        {
            lcdSetIcon(ICON_DOWN, CLOSEICON);
        }
        iRet = kbGetKeyMs(TimeOutMs);
        lcdSetIcon(ICON_DOWN, CLOSEICON);
        lcdSetIcon(ICON_UP, CLOSEICON);
        switch (iRet)
        {
        case KEY_ENTER:
            lcdClrLine(StartLine, EndLine);
            return(Cursor);
        case KEY_TIMEOUT:
            lcdClrLine(StartLine, EndLine);
            return(KB_TIMEOUT);
        case KEY_CANCEL:
            lcdClrLine(StartLine, EndLine);
            return(KB_CANCEL);
        case KEY_UP:
            Cursor--;
            if (Cursor < 0)
            {
                Cursor = PromptNum - 1;
                Begin  = Cursor + 1 - (EndLine+1-StartLine)/Height;
                Begin  = MAX(Begin, 0);
            }
            else if (Cursor < Begin)
            {
                //  Begin--;
                Begin = Cursor + 1 - (EndLine+1-StartLine)/Height;
                Begin  = MAX(Begin, 0);
            }
            break;
        case KEY_DOWN:
            Cursor++;
            if (Cursor >= PromptNum)
            {
                Cursor = 0;
                Begin  = 0;
            }
            else if ((Cursor-Begin+1) > ((EndLine+1-StartLine)/Height))
            {
                //  Begin++;
                Begin = Cursor;
            }
            break;
        default :
            break;
        }
    }
}

static int s_TManuSetNetAddr(struct in_addr *ipvalue)
{
    char value[1024];
    int iRet;
    struct in_addr tempip;

    while(1)
    {
        memset(value, 0, sizeof(value));
        inet_ntop(AF_INET, ipvalue, value, sizeof(value));
        lcdClrLine(4, 7);
        lcdGoto(0, 4);
        iRet = kbGetString(KB_BIG_ALPHA, 7, 15, -1, value);
        if (iRet <= 0)
        {
            return(iRet);
        }
        iRet = inet_pton(AF_INET, value, &tempip);
        if (1 != iRet)
        {
            lcdDisplayCE(0, 6, DISP_CFONT, "地址无效", "Invalid Addr");
            iRet = kbGetKey();
            if (KEY_CANCEL == iRet)
            {
                return(KB_CANCEL);
            }
        }
        else
        {
            if (*(U32 *)&tempip != *(U32 *)ipvalue)
            {
                *ipvalue = tempip;
                return(YES);
            }
            return(NO);
        }
    }
    return(NO);
}

static int s_TManuSetTCPIPCfg(int devtype)
{
    int iRet;
    tcpipcfg_t devcfg;
    char ipaddr[64];

    lcdCls();
    lcdDisplayCE(0, 0, DISP_CFONT|DISP_MEDIACY, "配置TCPIP", "CFG TCPIP");
    memset(&devcfg, 0, sizeof(devcfg));
    iRet = NetGetTCPIPCfg(devtype, &devcfg);
    if (NET_OK != iRet)
    {
        lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "读取配置错误", "Get CFG error");
        lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "RET = %d", iRet);
        kbGetKey();
        return(iRet);
    }
    lcdDisplay(0, 0, DISP_ASCII|DISP_CLRLINE, "DHCP: %s", devcfg.dhcp==NO ? "DISABLE" : "ENABLE");
    memset(ipaddr, 0, sizeof(ipaddr));
    inet_ntop(AF_INET, &(devcfg.localip), ipaddr, sizeof(ipaddr));
    lcdDisplay(0, 1, DISP_ASCII|DISP_CLRLINE, "IP  : %s", ipaddr);

    memset(ipaddr, 0, sizeof(ipaddr));
    inet_ntop(AF_INET, &(devcfg.subnet), ipaddr, sizeof(ipaddr));
    lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "MASK: %s", ipaddr);

    memset(ipaddr, 0, sizeof(ipaddr));
    inet_ntop(AF_INET, &(devcfg.gateway), ipaddr, sizeof(ipaddr));
    lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "GATE: %s", ipaddr);

    memset(ipaddr, 0, sizeof(ipaddr));
    inet_ntop(AF_INET, &(devcfg.dns1), ipaddr, sizeof(ipaddr));
    lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "DNS1: %s", ipaddr);

    memset(ipaddr, 0, sizeof(ipaddr));
    inet_ntop(AF_INET, &(devcfg.dns2), ipaddr, sizeof(ipaddr));
    lcdDisplay(0, 5, DISP_ASCII|DISP_CLRLINE, "DNS2: %s", ipaddr);

    lcdDisplayCE(0, 6, DISP_CFONT, "[0]-修改", "[0]-Modify");

    kbFlush();
    iRet = kbGetKey();
    if (KEY0 == iRet)
    {
        lcdCls();
        lcdDisplayCE(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, "配置TCPIP", "CFG TCPIP");

        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT, "设置DHCP:", "SET DHCP:");
        lcdDisplayCE(0, 4, DISP_CFONT, "1:启用*   0:禁用", "1:ENABLE*  0:DIS");
        if (NO == devcfg.dhcp)
        {
            lcdDisplayCE(0, 6, DISP_CFONT, "当前: 禁用", "CUR: DISABLE");
        }
        else
        {
            lcdDisplayCE(0, 6, DISP_CFONT, "当前: 启用", "CUR: ENABLE");
        }
        kbFlush();
        iRet = kbGetKey();
        if (KEY0 == iRet)
        {
            devcfg.dhcp = NO;
        }
        else if (KEY1 == iRet)
        {
            devcfg.dhcp = YES;
        }
        else if (KEY_CANCEL == iRet)
        {
            return (KB_CANCEL);
        }

        if (NO == devcfg.dhcp)
        {
            lcdClrLine(2, 7);
            lcdDisplayCE(0, 2, DISP_CFONT, "设置IP地址:", "SET IP:");
            s_TManuSetNetAddr(&devcfg.localip);

            lcdClrLine(2, 7);
            lcdDisplayCE(0, 2, DISP_CFONT, "设置子网掩码:", "SET MASK ADDR:");
            s_TManuSetNetAddr(&devcfg.subnet);

            lcdClrLine(2, 7);
            lcdDisplayCE(0, 2, DISP_CFONT, "设置网关:", "SET GATEWAY:");
            s_TManuSetNetAddr(&devcfg.gateway);

            lcdClrLine(2, 7);
            lcdDisplayCE(0, 2, DISP_CFONT, "设置DNS1:", "SET DNS1:");
            s_TManuSetNetAddr(&devcfg.dns1);

            lcdClrLine(2, 7);
            lcdDisplayCE(0, 2, DISP_CFONT, "设置DNS2:", "SET DNS2:");
            s_TManuSetNetAddr(&devcfg.dns2);
        }
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 4, DISP_CFONT, "保存设置...", "SAVE CFG...");
        iRet = NetSetTCPIPCfg(devtype, &devcfg);
        if (NET_OK != iRet)
        {
            lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "保存配置错误", "Save CFG error");
            lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "RET = %d", iRet);
            kbGetKey();
            return(iRet);
        }
    }
    return(OK);
}


static int s_TManuSetAP(void)
{
    int iRet;
    WiFiAPInfo_t surround_ap[64];
    U08 *aplist[128];
    int APnum, i;
    U08 tempbuf[128];
    WiFiAPx_t user_ap;
    WiFiDefAP_t def_ap;
    WiFiConfig_t user_config;

    memset(&user_ap, 0, sizeof(user_ap));
    kbFlush();
    lcdCls();
    lcdDisplayCE(0, 0, DISP_CFONT|DISP_CLRLINE|DISP_REVERSE, "   设置接入AP   ", " SET CONNECT AP ");
    lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "1: 手工设置",     "1: Manual Setup");
    lcdDisplayCE(0, 4, DISP_CFONT|DISP_CLRLINE, "0: 从周围AP选择", "0: Scan AP");
    iRet = kbGetKey();
    if (KEY1 == iRet)
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "设置SSID:", "Set SSID:");
        lcdGoto(0, 4);
        memset(tempbuf, 0, sizeof(tempbuf));
        iRet = kbGetString(KB_SMALL_ALPHA, 1, 32, -1, tempbuf);
        if (iRet >= 0)
        {
            strcpy(user_ap.SSID, tempbuf);
        }
        else
        {
            return KB_CANCEL;
        }
    }
    else
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "正在搜索...", "Scan...");
        APnum = WifiScanAP(ARRAY_SIZE(surround_ap), surround_ap);
        if (APnum < 0)
        {
            lcdClrLine(2, 7);
            lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "搜索AP失败", "Scan AP Error");
            lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "RET = %d", APnum);
            lcdDisplayCE(0, 6, DISP_CFONT|DISP_CLRLINE, s_ShowWnetPrompt(ON, APnum), s_ShowWnetPrompt(OFF, APnum));
            kbFlush();
            kbGetKey();
            return APnum;
        }
        else if (0 == APnum)
        {
            lcdClrLine(2, 7);
            lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "搜索不到AP", "No AP");
            kbFlush();
            kbGetKey();
            return KB_CANCEL;
        }
        for (i=0; i<APnum; i++)
        {
            aplist[i*2] = surround_ap[i].SSID;
            aplist[i*2+1] = surround_ap[i].SSID;
        }
        iRet = SelectMenu(2, 7, DISP_ASCII|DISP_CLRLINE, -1, APnum, 0, aplist);
        if (iRet < 0)
        {
            return iRet;
        }
        strcpy(user_ap.SSID, surround_ap[iRet].SSID);
        user_ap.SecurityType = surround_ap[iRet].SecurityType;
    }

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "设置信道:", "Set Channel:");
    lcdGoto(0, 4);
    memset(tempbuf, 0, sizeof(tempbuf));
    iRet = kbGetString(KB_SMALL_NUM, 1, 2, -1, tempbuf);
    if (iRet >= 0)
    {
        user_config.Channel = atol(tempbuf);
    }
    else
    {
        return KB_CANCEL;
    }

    lcdClrLine(0, 7);
    lcdDisplayCE(0, 0, DISP_CFONT|DISP_CLRLINE, "设置加密模式:", "Set Security:");
    lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "0-NONE    1-WEP64 opn");
    lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "2-WEP64 sh 3-WPA TKIP");
    lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "4-WEP128 open        ");
    lcdDisplay(0, 5, DISP_ASCII|DISP_CLRLINE, "5-WEP128 share       ");
    lcdDisplay(0, 6, DISP_ASCII|DISP_CLRLINE, "6-WPA2 CC 7-WPA TK+CC");
    lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "8-WPA2 TK+CC         ");
    switch(user_ap.SecurityType)
    {
    case WIFI_SECURITY_NONE:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: NONE");
        break;
    case WIFI_SECURITY_WEP64_OPEN:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: WEP64 open authentication");
        break;
    case WIFI_SECURITY_WEP128_OPEN:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: WEP128 open authentication");
        break;
    case WIFI_SECURITY_WPA_PSK_TKIP:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: WPA-PSK TKIP");
        break;
    case WIFI_SECURITY_WPA2_PSK_CCMP:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: WPA2-PSK CCMP");
        break;
    case WIFI_SECURITY_WPA_TKIP_AND_WPA2_CCMP:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: WPA-PSK TKIP + CCMP (auto)");
        break;
    case WIFI_SECURITY_WEP64_SHARE:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: WEP64 share authentication");
        break;
    case WIFI_SECURITY_WEP128_SHARE:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: WEP128 share authentication");
        break;
    case WIFI_SECURITY_WPA_TKIP_AND_CCMP:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "WPA-PSK TKIP + CCMP");
        break;
    case WIFI_SECURITY_WPA2_TKIP_AND_CCMP:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "WPA2-PSK TKIP + CCMP");
        break;
    case WIFI_SECURITY_WPATKIP_E:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: WPATKIP_E");
        break;
    case WIFI_SECURITY_WPA2AES_E:
        lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "DEF: WPA2AES_E");
        break;
    }
    iRet = kbGetKey();
    switch(iRet)
    {
    case KEY_CANCEL:
        return(KB_CANCEL);
    case KEY0:
        user_ap.SecurityType = WIFI_SECURITY_NONE;
        break;
    case KEY1:
        user_ap.SecurityType = WIFI_SECURITY_WEP64_OPEN;
        break;
    case KEY2:
        user_ap.SecurityType = WIFI_SECURITY_WEP64_SHARE;
        break;
    case KEY3:
        user_ap.SecurityType = WIFI_SECURITY_WPA_PSK_TKIP;
        break;
    case KEY4:
        user_ap.SecurityType = WIFI_SECURITY_WEP128_OPEN;
        break;
    case KEY5:
        user_ap.SecurityType = WIFI_SECURITY_WEP128_SHARE;
        break;
    case KEY6:
        user_ap.SecurityType = WIFI_SECURITY_WPA2_PSK_CCMP;
        break;
    case KEY7:
        user_ap.SecurityType = WIFI_SECURITY_WPA_TKIP_AND_CCMP;
        break;
    case KEY8:
        user_ap.SecurityType = WIFI_SECURITY_WPA2_TKIP_AND_CCMP;
        break;
    default:
        break;
    }

    switch (user_ap.SecurityType)
    {
    case WIFI_SECURITY_NONE:
        break;
    case WIFI_SECURITY_WEP64_OPEN:
    case WIFI_SECURITY_WEP64_SHARE:
    case WIFI_SECURITY_WEP128_OPEN:
    case WIFI_SECURITY_WEP128_SHARE:
        //  copy to wep mode parms
        memset(&def_ap, 0, sizeof(def_ap));
        strncpy(def_ap.SSID, user_ap.SSID, sizeof(def_ap.SSID));
        def_ap.WEPMode = user_ap.SecurityType;

        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "选择WEP密钥组:", "Set WEPKEY Index");
        lcdDisplayCE(0, 4, DISP_CFONT|DISP_CLRLINE, "[1..4]", "Range:[1..4]");
        lcdGoto(0, 6);
        memset(tempbuf, 0, sizeof(tempbuf));
        iRet = kbGetString(KB_SMALL_NUM, 1, 1, -1, tempbuf);
        if (iRet >= 0)
        {
            def_ap.WEPKeyIdx = atol(tempbuf);
            if ((def_ap.WEPKeyIdx<1) || (def_ap.WEPKeyIdx>4))
            {
                def_ap.WEPKeyIdx = 1;
            }
        }
        else
        {
            return KB_CANCEL;
        }

        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "设置WEP密钥[", "Set WEP KEY[");
        lcdDisplay(64, 2, DISP_CFONT, "%d]:", def_ap.WEPKeyIdx);
        lcdGoto(0, 4);
        memset(tempbuf, 0, sizeof(tempbuf));
        iRet = kbGetString(KB_SMALL_ALPHA, 1, 26, -1, tempbuf);
        if (iRet >= 0)
        {
            strcpy(user_ap.WEPKey, tempbuf);
            strcpy(def_ap.WEPKey[def_ap.WEPKeyIdx-1], tempbuf);
        }
        else
        {
            return KB_CANCEL;
        }
        break;
    case WIFI_SECURITY_WPA_PSK_TKIP:
    case WIFI_SECURITY_WPA2_PSK_CCMP:
    case WIFI_SECURITY_WPA_TKIP_AND_CCMP:
    case WIFI_SECURITY_WPA2_TKIP_AND_CCMP:
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "设置WPA PSK:", "Set WPA PSK:");
        lcdGoto(0, 4);
        memset(tempbuf, 0, sizeof(tempbuf));
        iRet = kbGetString(KB_SMALL_ALPHA, 1, 32, -1, tempbuf);
        if (iRet >= 0)
        {
            strcpy(user_ap.WPAPSK, tempbuf);
        }
        else
        {
            return KB_CANCEL;
        }
        break;
    }

    user_config.IchipPowerSave = 5;
    user_config.WLANPowerSave = 5;
    user_config.RoamingMode = 0;
    user_config.PeriodicScanInt = 5;
    user_config.RoamingLowSNR = 10;
    user_config.RoamingHighSNR = 30;

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "激活设置...", "Active Setting..");
    if (   (user_ap.SecurityType==WIFI_SECURITY_WEP64)
        || (user_ap.SecurityType==WIFI_SECURITY_WEP128))
    {
        iRet = WifiSetDefAP(&def_ap);
    }
    else
    {
        iRet = WifiSetAP(0, &user_ap);
    }
    if (NET_OK != iRet)
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "设置失败1", "Set Error 1");
        lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "RET = %d", iRet);
        lcdDisplayCE(0, 6, DISP_CFONT|DISP_CLRLINE, s_ShowWnetPrompt(ON, iRet), s_ShowWnetPrompt(OFF, iRet));
        kbFlush();
        kbGetKey();
        return iRet;
    }
    iRet = WifiSetParms(&user_config);
    if (NET_OK != iRet)
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "设置失败2", "Set Error 2");
        lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "RET = %d", iRet);
        lcdDisplayCE(0, 6, DISP_CFONT|DISP_CLRLINE, s_ShowWnetPrompt(ON, iRet), s_ShowWnetPrompt(OFF, iRet));
        kbFlush();
        kbGetKey();
        return iRet;
    }

    s_TManuSetTCPIPCfg(OPT_DEVWIFI);

    lcdCls();
    lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "复位 Wi-Fi...", "Reset Wi-Fi...");

    iRet = WifiSoftReset(0);
    if (NET_OK != iRet)
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "复位失败", "Reset Error");
        lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "RET = %d", iRet);
        lcdDisplayCE(0, 6, DISP_CFONT|DISP_CLRLINE, s_ShowWnetPrompt(ON, iRet), s_ShowWnetPrompt(OFF, iRet));
        kbFlush();
        kbGetKey();
        return iRet;
    }
    return OK;
}


static int s_SelectAndSetWIFIAP(void)
{
    int iRet;
    char localIP[32];
    WiFiStatus_t curstatus;

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  WIFI参数设置  ", "  WIFI CONFIG   ");
    lcdDisplayCE(0, 4, 0x01, "获取当前连接... ", "Get Cur CONN... ");
    kbFlush();
    while(1)
    {
        iRet = WifiGetCurConnect(&curstatus);
        if (NET_OK != iRet)
        {
            lcdCls();
            lcdDisplayCE(0, 0, DISP_CFONT|DISP_CLRLINE|DISP_REVERSE, "  当前连接热点  ", " CUR CONNECT AP ");
            lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "获取当前连接失败", "Get Cur CONN ERR");
            lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "RET = %d", iRet);
            lcdDisplayCE(0, 6, DISP_CFONT|DISP_CLRLINE, s_ShowWnetPrompt(ON, iRet), s_ShowWnetPrompt(OFF, iRet));
            kbFlush();
            kbGetKey();
            return iRet;
        }
        memset(localIP, 0x00, sizeof(localIP));
        iRet = WifiGetLocalIP(localIP);
        if (NET_OK != iRet)
        {
            lcdCls();
            lcdDisplayCE(0, 0, DISP_CFONT|DISP_CLRLINE|DISP_REVERSE, "  当前连接热点  ", " CUR CONNECT AP ");
            lcdDisplayCE(0, 2, DISP_CFONT|DISP_CLRLINE, "获取当前IP失败", "Get Cur IP ERR");
            lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "RET = %d", iRet);
            lcdDisplayCE(0, 6, DISP_CFONT|DISP_CLRLINE, s_ShowWnetPrompt(ON, iRet), s_ShowWnetPrompt(OFF, iRet));
            kbFlush();
            kbGetKey();
            return iRet;
        }
        lcdCls();
        lcdDisplayCE(0, 0, DISP_CFONT|DISP_CLRLINE|DISP_REVERSE, "  当前连接热点  ", " CUR CONNECT AP ");
        switch (curstatus.Status)
        {
        case WIFI_STATUS_NOTPRESENT:
            lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "STATUS: Not present");
            break;
        case WIFI_STATUS_DISABLED:
            lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "STATUS: Disable");
            break;
        case WIFI_STATUS_SEARCHING:
            lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "STATUS: Searching");
            break;
        case WIFI_STATUS_CONNECTED:
            lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "STATUS: Connected");
            break;
        case WIFI_STATUS_OUTOFRANGE:
            lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "STATUS: Out of range");
            break;
        default:
            lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "STATUS: Unknown %d", curstatus.Status);
            break;
        }
        lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "SSID: %s", curstatus.SSID);
        lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "Channel:%d SIG:%d%%", curstatus.Channel, curstatus.SigLevel);
        lcdDisplay(0, 5, DISP_ASCII|DISP_CLRLINE, "IP: %s", localIP);
        lcdDisplayCE(0, 6, DISP_CFONT|DISP_CLRLINE, "[0]-修改", "[0]-Modify");
        iRet = kbGetKeyMs(1000);
        if (KEY_CANCEL == iRet)
        {
            return KB_CANCEL;
        }
        else if (KEY0 == iRet)
        {
            s_TManuSetAP();
        }
        else if (KEY_TIMEOUT != iRet)
        {
            if (WIFI_STATUS_CONNECTED != curstatus.Status)
            {
                lcdDisplayCE(0, 6, DISP_CFONT|DISP_CLRLINE, "未连接到AP", "Not Link To AP");
                kbFlush();
                kbGetKeyMs(1000);
            }
            else if (0 == (strcmp(localIP, "0.0.0.0")))
            {
                lcdDisplayCE(0, 6, DISP_CFONT|DISP_CLRLINE, "等待分配IP", "Wait IP Allocate");
                kbFlush();
                kbGetKeyMs(1000);
            }
            else
            {
                return OK;
            }
        }
    }
}

static int s_InitWiFiModule(const COMM_CFG_PARAM *psCommParam)
{
    int iRet, iRetry, iTemp;
    char szlocalIP[32];
    WiFiStatus_t stWiFiStatus;

    sg_Tcpsocket = -1;

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  WIFI参数设置  ", "  WIFI CONFIG   ");
    lcdDisplayCE(0, 4, 0x01, "检查模块切换... ", "  Check module  ");
    lcdDisplayCE(0, 6, 0x01, "                ", "    switch...   ");
    iRet = s_ModuleSwitchCheck(COMM_TYPE_WIFI);
    if (ERR_USERCANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  WIFI参数设置  ", "  WIFI CONFIG   ");
    lcdDisplayCE(0, 4, 0x01, " 初始化WIFI...  ", " Intial WIFI... ");
    memset(szlocalIP, 0, sizeof(szlocalIP));
    iRetry = 0;
    while (1)
    {
        // 先检测连接状态
        while (1)
        {
            iRetry++;
            lcdClrLine(2,7);
            lcdDisplayCE(0, 3, DISP_CFONT, "   请稍候...    ", "  PLS WAIT...   ");
            lcdDisplayCE(0, 5, DISP_CFONT, "WIFI连接中..(", "CONNECTING..(");
            lcdDisplay(104, 5, DISP_CFONT, (U08*)"%d)", iRetry);
            DrawRect(0, 17, 127, 63);

            iRet = WifiGetCurConnect(&stWiFiStatus);
            iTemp = iRet;
            if (   (NET_OK!=iRet)
                || (WIFI_STATUS_CONNECTED!=stWiFiStatus.Status))
            {
                if (NET_OK == iTemp)
                {
                    iTemp = EWIFI_ONLINELOST;
                }
                lcdClrLine(2,7);
                lcdDisplayCE(0, 2, DISP_CFONT, "获取当前连接失败", " DIDN'T CONNECT  ");
                lcdDisplayCE(0, 4, DISP_CFONT, " 按功能或菜单键 ", "PLS ENTER FUNC  ");
                lcdDisplayCE(0, 6, DISP_CFONT, "设置WIFI配置参数", "OR MENU TO SET  ");
                iRet = kbGetKeyMs(6000);
                if ((KEY_FN==iRet) || (KEY_MENU==iRet))
                {
                    iRet = s_SelectAndSetWIFIAP();
                    if (iRetry >= 3)
                    {
                        return iTemp;
                    }
                    sysDelayMs(1000);// 延时1s
                    continue;
                }
                else
                {
                    return iTemp;
                }
            }
            break;
        }

        // 检测是否获取本地IP地址
        iRetry = 0;
        while (1)
        {
            iRetry++;
            lcdClrLine(2,7);
            lcdDisplayCE(0,3,DISP_CFONT,"   请稍候...    ", "  PLS WAIT...   ");
            lcdDisplayCE(0,5,DISP_CFONT,"获取本地IP..(",    "LOCAL IP... (");
            lcdDisplay(104, 5, DISP_CFONT,"%d)",iRetry);
            DrawRect(0, 17, 127, 63);
            memset(szlocalIP, 0, sizeof(szlocalIP));
            iRet = WifiGetLocalIP(szlocalIP);
            if (NET_OK != iRet)
            {
                iTemp = iRet;
                lcdClrLine(2,7);
                lcdDisplayCE(0, 2, DISP_CFONT, "获取当前连接失败", " DON'T CONNECT  ");
                lcdDisplayCE(0, 4, DISP_CFONT, " 按功能或菜单键 ", "PLS ENTER FUNC  ");
                lcdDisplayCE(0, 6, DISP_CFONT, "设置WIFI配置参数", "OR MENU TO SET  ");
                iRet = kbGetKeyMs(6000);
                if ((KEY_FN==iRet) || (KEY_MENU==iRet))
                {
                    iRet = s_SelectAndSetWIFIAP();
                    if (iRetry >= 3)
                    {
                        return  iTemp;
                    }
                    continue;
                }
                else
                {
                    return  iTemp;
                }
            }

            if (0 == strcmp(szlocalIP,"0.0.0.0"))
            {
                if (iRetry >= 3)
                {
                    return  EWIFI_SOCKIPREG;
                }
                sysDelayMs(2000);;
                continue;
            }
            else
            {
                break;
            }
        }

        if (0 != strcmp(szlocalIP,"0.0.0.0"))
        {
            strcpy((char*)psCommParam->oWifi.szLocalIP,szlocalIP);
            break;
        }
    }

    return 0;
}

static int s_GetWiFiStatus(void)
{
    int iRet, iRetry, iTemp;
    char szlocalIP[32];
    WiFiStatus_t stWiFiStatus;

    memset(szlocalIP, 0, sizeof(szlocalIP));
    iRetry = 0;
    while (1)
    {
        // 先检测连接状态
        while (1)
        {
            iRetry++;
            lcdClrLine(2,7);
            lcdDisplayCE(0, 3, DISP_CFONT, "   请稍候...    ", "  PLS WAIT...   ");
            lcdDisplayCE(0, 5, DISP_CFONT, "WIFI连接中..(", "CONNECTING..(");
            lcdDisplay(104, 5, DISP_CFONT, (U08*)"%d)", iRetry);
            DrawRect(0, 17, 127, 63);
            iRet = WifiGetCurConnect(&stWiFiStatus);
            iTemp = iRet;
            if ((NET_OK!=iRet) || (WIFI_STATUS_CONNECTED!=stWiFiStatus.Status))
            {
                if (NET_OK == iTemp)
                {
                    iTemp = EWIFI_ONLINELOST;
                }

                if (iRetry >= 3)
                {
                    return  iTemp;
                }
                sysDelayMs(2000);;// 延时1s
                continue;
            }

            break;
        }

        // 检测是否获取本地IP地址
        iRetry = 0;
        while (1)
        {
            iRetry++;
            lcdClrLine(2,7);
            lcdDisplayCE(0, 3, DISP_CFONT, "   请稍候...    ", "  PLS WAIT...   ");
            lcdDisplayCE(0, 5, DISP_CFONT, "获取本地IP..(", "LOCAL IP..  (");
            lcdDisplay(104, 5, DISP_CFONT, (U08*)"%d)", iRetry);
            DrawRect(0, 17, 127, 63);
            memset(szlocalIP, 0, sizeof(szlocalIP));
            iRet = WifiGetLocalIP(szlocalIP);
            if (NET_OK != iRet)
            {
                if (iRetry >= 3)
                {
                    return  iRet;
                }
                continue;
            }

            if (0 == strcmp(szlocalIP,"0.0.0.0"))
            {
                if (iRetry >= 3)
                {
                    return  EWIFI_SOCKIPREG;
                }
                sysDelayMs(2000);;
                continue;
            }
            else
            {
                break;
            }
        }

        if (0 != strcmp(szlocalIP, "0.0.0.0"))
        {
            break;
        }
    }

    return 0;
}

static int s_WiFiDial(const COMM_CFG_PARAM *psCommParam)
{
    int iRet;

    if (0 != g_iComInitStatus)
    {
        iRet = s_InitWiFiModule(psCommParam);
        g_iComInitStatus = iRet;
        if (0 != iRet)
        {
            lcdCls();
            lcdDisplayCE(0, 0, 0x81, "  WIFI参数设置  ", "  WIFI CONFIG   ");
            lcdDisplayCE(0, 4, 0x01, " WIFI初始化错误 ", "WIFI init error.");
            lcdDisplay(0, 6, 0x01, " iRet =%d ", iRet);
            kbGetKeyMs(30000);
            return iRet;
        }
    }

    // 检测是否连接上WIFI,这个时候如果没有连接成功就不提示用户进行WIFI设置而是直接返回
    iRet = s_GetWiFiStatus();
    if (0 != iRet)
    {
        return iRet;
    }

    return s_TcpConnect(psCommParam);
}

// 设置通讯模块参数
static int s_CommSetCfgParam(COMM_CFG_PARAM *psCommParam)
{
    int        iLen;

    if (NULL == psCommParam)
    {
        return ERR_COMM_INV_PARAM;
    }

    switch (psCommParam->byComType)
    {
    case COMM_TYPE_RS232:
        iLen = strlen((char *)psCommParam->oRs232.szAttr);
        if ((iLen<10) || (iLen>20))    // 简单检查参数
        {
            return ERR_COMM_INV_PARAM;
        }
        return 0;

    case COMM_TYPE_PSTN:
        if (0 == g_oCommCfgParam.abyDevInfo[2])
        {
            return ERR_COMM_INV_PARAM;
        }
        if (0 == psCommParam->oPstn.szTxnTelNo[0])
        {
            return ERR_COMM_INV_PARAM;
        }
        if (    (psCommParam->oPstn.bySendMode!=MODEM_COMM_ASYNC)
             && (psCommParam->oPstn.bySendMode!=MODEM_COMM_SYNC))
        {
            return ERR_COMM_INV_PARAM;
        }
        return 0;

    case COMM_TYPE_LAN:
        if (0 == g_oCommCfgParam.abyDevInfo[8])
        {
            return ERR_COMM_INV_PARAM;
        }
        break;

    case COMM_TYPE_CDMA:
        if (0 == g_oCommCfgParam.abyDevInfo[10])
        {
            return ERR_COMM_INV_PARAM;
        }
        if (0 == psCommParam->oGprsCdma.szAPN[0])
        {
            return ERR_COMM_INV_PARAM;
        }
        break;
    case COMM_TYPE_GPRS:
        if (0 == g_oCommCfgParam.abyDevInfo[9])
        {
            return ERR_COMM_INV_PARAM;
        }

        if (0 == psCommParam->oGprsCdma.szAPN[0])
        {
            return ERR_COMM_INV_PARAM;
        }
        break;

    case COMM_TYPE_WIFI:
        if (0 == g_oCommCfgParam.abyDevInfo[11])
        {
            return ERR_COMM_INV_PARAM;
        }
        break;

    default:
        return ERR_COMM_INV_TYPE;
    }
    if (   (psCommParam->szRemoteIP[0]==0)
        || (psCommParam->nRemotePort<=0))
    {
        return ERR_COMM_INV_PARAM;
    }
    return 0;
}


// 检查IP地址
// pszIPAddr[in]:字符串形式的IP地址
// pbyIp[out]:4字节的16进制IP地址
static int s_CheckIPAddress(const char *pszIPAddr, U08 *pbyIp)
{
    int i, j, k, iLen, iDotOffset[3];
    U08 abyTemp[8];

    iLen = strlen(pszIPAddr);
    if ((iLen<7) || (iLen>15))
    {
        return FALSE;
    }
    if (0 == ISAllCharInString((U08*)pszIPAddr, iLen, (U08*)"0123456789.", 11))
    {
        return FALSE;
    }
    j = 0;
    for (i=0; i<iLen; i++)
    {
        if ('.'==pszIPAddr[i])
        {
            if (j < 3)
            {
                iDotOffset[j] = i;
                j++;
            }
            else
            {
                return FALSE;
            }
        }
    }
    if (3 != j)
    {
        return FALSE;
    }
    for (i=0; i<4; i++)
    {
        memset(abyTemp, 0, sizeof(abyTemp));
        switch (i)
        {
        case 0:
            k = iDotOffset[0];
            memcpy(abyTemp, &pszIPAddr[0], k);
            break;
        case 1:
            k = iDotOffset[1] - iDotOffset[0] - 1;
            memcpy(abyTemp, &pszIPAddr[iDotOffset[0]+1], k);
            break;
        case 2:
            k = iDotOffset[2] - iDotOffset[1] - 1;
            memcpy(abyTemp, &pszIPAddr[iDotOffset[1]+1], k);
            break;
        default:
            k = iLen - iDotOffset[2] - 1;
            memcpy(abyTemp, &pszIPAddr[iDotOffset[2]+1], k);
            break;
        }
        if ((0==k) || (k>3))
        {
            return FALSE;
        }
        j = atoi(abyTemp);
        if (j > 255)
        {
            return FALSE;
        }
        if (NULL != pbyIp)
        {
            pbyIp[i] = (U08)j;
        }
    }
    return TRUE;
}

// 0:success, 1:failed
int SaveCommCfg(const COMM_CFG_PARAM *psCfg)
{
    int fd, iRet;

    fileRemove(POS_DEVICE_INFO);
    fd = fileOpen(POS_DEVICE_INFO, O_CREAT);
    if (fd >= 0)
    {
        fileSeek(fd, 0, SEEK_SET);
        iRet = fileWrite(fd, (U08*)psCfg, sizeof(COMM_CFG_PARAM));
        fileClose(fd);
        if (sizeof(COMM_CFG_PARAM) == iRet)
        {
            return 0;
        }
    }
    return 1;
}

// 0:success, 1:failed
static int s_ReadCommCfg(COMM_CFG_PARAM *psCfg)
{
    int fd, iRet;

    fd = fileOpen(POS_DEVICE_INFO, O_RDWR);
    if (fd >= 0)
    {
        fileSeek(fd, 0, SEEK_SET);
        iRet = fileRead(fd, (U08*)psCfg, sizeof(COMM_CFG_PARAM));
        fileClose(fd);
        if (sizeof(COMM_CFG_PARAM) == iRet)
        {
            return 0;
        }
    }

    return 1;
}


// 输入IP地址
// bAllowNull:是否允许IP为空，0不允许，非0允许
// pszIPAddress[in&out]:字符串形式的IP地址
// pbyIp[out]:4字节的16进制IP地址
// 返回0表示成功设置，返回非零表示用户取消
int GetIPAddress(U08 bAllowNull, U08 *pszIPAddress, U08 *pbyIp)
{
    int iRet;

    pszIPAddress[15] = 0;
    while (1)
    {
        lcdGoto(0, 6);
        iRet = kbGetString(0xf5, 7, 15, USER_OPER_TIMEOUT, pszIPAddress);
        if (KB_CANCEL == iRet)
        {
            return ERR_USERCANCEL;
        }
        if ((0!=bAllowNull) && (0==pszIPAddress[0]))
        {
            *pszIPAddress = 0;
            break;
        }
        if (0 != s_CheckIPAddress((char *)pszIPAddress,pbyIp))
        {
            break;
        }

        Display2Strings("无效IP地址", "INV IP ADDR");
        PubBeepErr();
        kbGetKeyMs(40000);
    }

    return 0;
}

// 输入端口
// pszPrompts[in]:显示提示信息
// piPortNo[in & out]:端口号输出
// 返回0表示成功，返回非0表示取消
static int s_GetIPPort(U08 *pszPrompts, int *piPortNo)
{
    int iTemp, iRet;
    U08 abyTemp[10];

    sprintf(abyTemp, "%d", *piPortNo);
    while (1)
    {
        lcdClrLine(2, 7);
        lcdDisplay(0, 2, DISP_CFONT, "%.16s", pszPrompts);
        lcdGoto(0, 6);
        iRet = kbGetString(0xe5, 1, 5, USER_OPER_TIMEOUT, abyTemp);
        if (KB_CANCEL == iRet)
        {
            return ERR_USERCANCEL;
        }
        iTemp = atoi((char *)abyTemp);
        if ((iTemp>0) && (iTemp<65535))
        {
            *piPortNo = iTemp;
            return 0;
        }

        Display2Strings("无效端口号", "INV PORT #");
        PubBeepErr();
        kbGetKeyMs(4000);
    }
}

// 设置TCP/IP参数
int SetTcpIpParam(const U08 *pbyChnTitle, const U08 *pbyEngTitle)
{
    int iRet;

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, (U08*)pbyChnTitle, (U08*)pbyEngTitle);
    lcdDisplayCE(0, 2, DISP_CFONT, "远端IP", "REMOTE IP");
    iRet = GetIPAddress(FALSE, g_oCommCfgParam.szRemoteIP, NULL);
    if (0 != iRet)
    {
        return iRet;
    }
    iRet = s_GetIPPort((U08 *)"PORT", &g_oCommCfgParam.nRemotePort);
    if (0 != iRet)
    {
        return iRet;
    }

    if (COMM_TYPE_LAN != g_oCommCfgParam.byComType)
    {
        return 0;
    }

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 2, DISP_CFONT, "本地IP", "LOCAL IP");
    iRet = GetIPAddress(TRUE, g_oCommCfgParam.oTcpIp.szLocalIP, NULL);
    if (0 != iRet)
    {
        return iRet;
    }

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 2, DISP_CFONT, "地址掩码", "IP MASK");
    iRet = GetIPAddress(TRUE, g_oCommCfgParam.oTcpIp.szNetMask, NULL);
    if (0 != iRet)
    {
        return iRet;
    }

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 2, DISP_CFONT, "网关", "GATEWAY IP");
    iRet = GetIPAddress(TRUE, g_oCommCfgParam.oTcpIp.szGatewayIP, NULL);
    if (0 != iRet)
    {
        return iRet;
    }

    return 0;
}

static int s_SetDialParam(U08 *pbyAPN, U08 *pbyUID, U08 *pbyPwd)
{
    int iRet;

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 3, DISP_CFONT, "拨号号码", "APN");
    lcdGoto(0, 6);
    iRet = kbGetString(0xf5, 0, 32, USER_OPER_TIMEOUT, pbyAPN);
    if (KB_CANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 3, DISP_CFONT, "登陆用户名", "LOGIN NAME");
    lcdGoto(0, 6);
    iRet = kbGetString(0xf5, 0, 32, USER_OPER_TIMEOUT, pbyUID);
    if (KB_CANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 3, DISP_CFONT, "登陆密码", "LOGIN PWD");
    lcdGoto(0, 6);
    iRet = kbGetString(0xf5, 0, 16, USER_OPER_TIMEOUT, pbyPwd);
    if (KB_CANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }
    return 0;
}

static int s_SetWirelessParam(const U08 *pbyChnTitle, const U08 *pbyEngTitle)
{
    int iRet;

    SetTcpIpParam(pbyChnTitle, pbyEngTitle);

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, pbyChnTitle, pbyEngTitle);

    strcpy(g_oCommCfgParam.oGprsCdma.szAPN, "#91");
    strcpy(g_oCommCfgParam.oGprsCdma.szUID, "saf");
    strcpy(g_oCommCfgParam.oGprsCdma.szPwd, "data");
    s_SetDialParam(g_oCommCfgParam.oGprsCdma.szAPN, g_oCommCfgParam.oGprsCdma.szUID,
        g_oCommCfgParam.oGprsCdma.szPwd);

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 2, DISP_CFONT, "设置SIM PIN", "SIM PIN");
    lcdGoto(0, 6);
    iRet = kbGetString(0xf5, 0, 16, USER_OPER_TIMEOUT, g_oCommCfgParam.oGprsCdma.szSimPin);
    if (KB_CANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }

    return 0;
}

static int s_SetWIFIParam(void)
{
    int iRet, iFlag;
    WiFiStatus_t stWiFiStatus;
    char szlocalIP[20];

    if (0 == g_oCommCfgParam.abyDevInfo[11])
    {
        return ERR_COMM_INV_TYPE; // not support WIFI module
    }
    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  WIFI参数设置  ", "  WIFI CONFIG   ");
    lcdDisplayCE(0, 4, 0x01, "   WIFI初始化   ", "  WIFI initial  ");
    WnetInit(WIRELESS_INITIAL_TIME);
    iFlag = 1;
    while (iFlag)
    {
        iRet = WifiGetCurConnect(&stWiFiStatus);
        lcdCls();
        lcdDisplayCE(0, 0, 0x81, "  WIFI参数设置  ", "  WIFI CONFIG   ");
        lcdDisplay(0,2,DISP_ASCII, "%16.16s", stWiFiStatus.SSID);
        if (WIFI_STATUS_CONNECTED != stWiFiStatus.Status)
        {
            lcdDisplay(0, 3, DISP_ASCII, "STATUS: DISCONNECT");
        }
        else
        {
            lcdDisplay(0, 3, DISP_ASCII, "STATUS: CONNECTED");
        }
        lcdDisplay(0, 4, DISP_ASCII, "SIG: %d%", stWiFiStatus.SigLevel);
        if (stWiFiStatus.LnkQual > 75)
        {
            lcdDisplay(0, 4, DISP_ASCII, "QUALITY: EXCELLED");
        }
        else if (stWiFiStatus.LnkQual > 50)
        {
            lcdDisplay(0, 4, DISP_ASCII, "QUALITY: GOOD");
        }
        else
        {
            lcdDisplay(0, 4, DISP_ASCII, "QUALITY: BAD");
        }

        memset(szlocalIP, 0, sizeof(szlocalIP));
        iRet = WifiGetLocalIP(szlocalIP);
        if ((0!=iRet) || (0==strcmp(szlocalIP,"0.0.0.0")))
        {
            lcdDisplay(0, 5, DISP_ASCII, "LOCAL IP: 0.0.0.0");
        }
        else
        {
            lcdDisplay(0, 5, DISP_ASCII, "LOCAL:%s", szlocalIP);
        }
        lcdDisplay(0, 6, DISP_CFONT, "[FN/MENU] MODIFY");
        iRet = kbGetKeyMs(10000);;
        switch (iRet)
        {
        case KEY_FN:
        case KEY_MENU:
            s_SelectAndSetWIFIAP();
            break;
        default:
            iFlag = 0;
            break;
        }
    }

    iRet = s_InitWiFiModule(&g_oCommCfgParam);
    g_iComInitStatus = iRet;

    return SetTcpIpParam("  WIFI参数设置  ", " SET WIFI PARAM ");
}

static int s_PSTN_PPP_SetParam(void)
{
    int iRet;

    iRet = s_SetDialParam(g_oCommCfgParam.oPstn.szAPN, g_oCommCfgParam.oPstn.szUID,
        g_oCommCfgParam.oPstn.szPwd);
    if (ERR_USERCANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }
    return SetTcpIpParam("PSTN PPP参数设置", "SET PSTNPPP PARA");
}


// 通讯模块初始化
int s_CommInitModule(const COMM_CFG_PARAM *psCommParam)
{
    int iRet;

    s_CommSetCfgParam((COMM_CFG_PARAM *)psCommParam);

    switch (psCommParam->byComType)
    {
    case COMM_TYPE_WIFI:
//        sysDelayMs(5000);
        iRet = s_InitWiFiModule(psCommParam);
        g_iComInitStatus = iRet;
        break;

    case COMM_TYPE_CDMA:
    case COMM_TYPE_GPRS:
        iRet = s_InitWirelessModule(&psCommParam->oGprsCdma);
        break;

    case COMM_TYPE_PSTN:
    case COMM_TYPE_PSTN_PPP:
        iRet = s_PSTN_InitModem(&psCommParam->oPstn.stDialPara);
        break;

    default:
        iRet = 0;
        break;
    }

    return iRet;
}

// 通讯模块初始化
int CommResetModule(const COMM_CFG_PARAM *psCommParam)
{
    if (   (COMM_TYPE_GPRS!=psCommParam->byComType)
        && (COMM_TYPE_CDMA!=psCommParam->byComType))
    {
        return 0;
    }

    s_CommSetCfgParam((COMM_CFG_PARAM *)psCommParam);

    return s_WirelessReset(psCommParam);
}

static void s_InitCommCfg(COMM_CFG_PARAM *psCfg)
{
    lcdDisplayCE(0, 2, 0x01, "  读取硬件配置  ", "Read hardware   ");
    lcdDisplayCE(0, 4, 0x01, "                ", "module configure");
    lcdDisplayCE(0, 6, 0x01, "                ", "                ");

    // read hardware module configure
    sysReadConfig(psCfg->abyDevInfo);  // When we call this API, it will block more then 1~ 20 seconds

    lcdDisplayCE(0, 2, 0x01, "  初始化配置    ", "Init configure  ");
    lcdDisplayCE(0, 4, 0x01, "                ", "                ");
    lcdDisplayCE(0, 6, 0x01, "                ", "                ");
    // PSN default parameter
    psCfg->oPstn.stDialPara.connect_mode = MODEM_COMM_SYNC;
    psCfg->oPstn.stDialPara.connect_speed = MODEM_CONNECT_2400BPS;
    psCfg->oPstn.stDialPara.dial_mode = MODEM_DAIL_DTMF;
    psCfg->oPstn.stDialPara.dial_pause = 1;
    psCfg->oPstn.stDialPara.dial_timeo = 30;
    psCfg->oPstn.stDialPara.idle_timeo = 60;
    psCfg->oPstn.stDialPara.extension_chk = 1;
    psCfg->oPstn.stDialPara.region = ModemRegion(China);
//    psCfg->oPstn.stDialPara.region = ModemChina;
//    psCfg->oPstn.stDialPara.region = ModemRegion(Russia);
//    psCfg->oPstn.stDialPara.region = ModemRegion(Colombia);
//    psCfg->oPstn.stDialPara.region = ModemRegion(Denmark);
//    psCfg->oPstn.stDialPara.region = ModemRegion(India);
//    psCfg->oPstn.stDialPara.region = ModemRegion(Singapore);
//    psCfg->oPstn.stDialPara.region = ModemRegion(SriLanka);
//    psCfg->oPstn.stDialPara.region = ModemRegion(SouthAfrica);
//    psCfg->oPstn.stDialPara.region = ModemRegion(Pakistan);

    // RS232 inital
    psCfg->oRs232.byPortNo = PORT_COM1;
    psCfg->oRs232.bySendMode = CM_RAW;
    strcpy((char*)psCfg->oRs232.szAttr, "115200,8,n,1");

    // GPRS initial
    strcpy((char*)psCfg->oGprsCdma.szAPN, "safaricom");  // for China Mobile
    strcpy((char*)psCfg->oGprsCdma.szUID, "saf");  // for China Mobile
    strcpy((char*)psCfg->oGprsCdma.szPwd, "data");  // for China Mobile

    // CDMA initial
//    strcpy((char*)psCfg->oGprsCdma.szAPN, "#777");  // for China Telecom
//    strcpy((char*)psCfg->oGprsCdma.szUID, " ");  // for China Telecom
//    strcpy((char*)psCfg->oGprsCdma.szPwd, " ");  // for China Telecom

    // ftp initial
    InitFtpParamter(&(psCfg->oFtp));
}

void StartInitComm(void)
{
    int iRet;

    memset(&g_oCommCfgParam, 0, sizeof(COMM_CFG_PARAM));
    if (0 != s_ReadCommCfg(&g_oCommCfgParam))
    {
        // Read the hardware configure from system API and save it to file system.
        // Call this API will cost about 25 seconds or more.
        s_InitCommCfg(&g_oCommCfgParam);

        iRet = SaveCommCfg(&g_oCommCfgParam);
        if (0 != iRet)
        {
            lcdCls();
            lcdDisplayCE(0, 0, 0x01, "保存通讯参数失败", "Save COMM param ");
            lcdDisplayCE(0, 2, 0x01, "                ", "failed.         ");
            lcdDisplayCE(0, 4, 0x01, "  文件系统已满  ", "File sys error. ");
            lcdDisplayCE(0, 6, 0x01, "     请结算     ", "Pls settlement. ");
            kbGetKeyMs(120000);
            return ;
        }
        else
        {
            ReadParamConfigure();

            CommTypeSelect();
            CommParamSet();
        }
    }

    ReadParamConfigure();
    s_CommInitModule(&g_oCommCfgParam);
}

void UdpTcpSelect(void)
{
g_oCommCfgParam.byUdpFlag = 0;
    /*int iRet;

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  连接类型选择  ", "Connect type slt");
    lcdDisplay(0, 2, 0x01,   "1-TCP           ");
    lcdDisplay(0, 4, 0x01,   "2-UDP           ");
    if (0 != g_oCommCfgParam.byUdpFlag)
    {
        lcdDisplay(0, 6, 0x01,   "Cur:UDP");
    }
    else
    {
        lcdDisplay(0, 6, 0x01,   "Cur:TCP");
    }
    while (1)
    {
        iRet = kbGetKey();
        if (KEY_CANCEL==iRet)
        {
            return ;
        }
        else if (KEY1==iRet)
        {
            g_oCommCfgParam.byUdpFlag = 0;
            return ;
        }
        else if (KEY2==iRet)
        {
            g_oCommCfgParam.byUdpFlag = 1;
            return ;
        }
    }*/
}

void CommParamSet(void)
{
    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  通讯参数设置  ", "Set COMM param  ");
    switch (g_oCommCfgParam.byComType)
    {
    case COMM_TYPE_PSTN:
        s_PSTN_ParamSet(&(g_oCommCfgParam.oPstn));
        break;
    case 0:
    case COMM_TYPE_GPRS:
        s_SetWirelessParam("  GPRS参数设置  ", " SET GPRS PARAM ");
        UdpTcpSelect();
        break;
    case COMM_TYPE_WIFI:
        s_SetWIFIParam();
        UdpTcpSelect();
        break;
    case COMM_TYPE_CDMA:
        s_SetWirelessParam("  CDMA参数设置  ", " SET CDMA PARAM ");
        UdpTcpSelect();
        break;
    case COMM_TYPE_RS232:
        break;
    case COMM_TYPE_LAN:
        SetTcpIpParam(" TCP/IP参数设置 ", "  TCP/IP PARA   ");
        break;
    case COMM_TYPE_PSTN_PPP:
        s_PSTN_ParamSet(&(g_oCommCfgParam.oPstn));
        s_PSTN_PPP_SetParam();
        break;
    default:
        GetKeyBlockMs(RESPOND_TIMEOUT);
        return ;
    }
    SaveCommCfg(&g_oCommCfgParam);
}


void CommTypeSelect(void)
{
    U08 byRet, byCurPage;

    /*lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  通讯类型选择  ", "COMM type select");
    lcdGoto(0, 2);*/
    byRet = LcdGetItemBigTimeout((U08*)g_abyCommTypeSelectEng,
        COMM_TYPE_SELECT_NUM, g_oCommCfgParam.byComType,
        LCD_MODE_2ITEM_PER_LINE|LCD_EXIT_MODE0|LCD_MODE_SHOW_USER_SELECT,
        RESPOND_TIMEOUT, &byCurPage);
/*
    switch (byRet)
    {
    case 0:  // parameter error
        lcdGoto(0, 2);
        lcdPrintf("Fatal error!");
        while (1);
    case 1:
        g_oCommCfgParam.byComType = COMM_TYPE_GPRS;
        g_oCommCfgParam.byTcpipDevType = OPT_DEVWNET;
        g_iComInitStatus = 0xff;
        break;
    case 2:
        g_oCommCfgParam.byComType = COMM_TYPE_WIFI;
        g_oCommCfgParam.byTcpipDevType = OPT_DEVWIFI;
        break;
    case 3:
        g_oCommCfgParam.byComType = COMM_TYPE_PSTN;
        g_oCommCfgParam.byTcpipDevType = 0;
        g_iComInitStatus = 0xff;
        break;
    case 4:
        g_oCommCfgParam.byComType = COMM_TYPE_CDMA;
        g_oCommCfgParam.byTcpipDevType = OPT_DEVWNET;
        g_iComInitStatus = 0xff;
        break;
    case 5:
        g_oCommCfgParam.byComType = COMM_TYPE_PSTN_PPP;
        g_oCommCfgParam.byTcpipDevType = OPT_DEVMODEM;
        g_iComInitStatus = 0xff;
        break;
    case 6:
        g_oCommCfgParam.byComType = COMM_TYPE_LAN;
        g_oCommCfgParam.byTcpipDevType = OPT_DEVETH;
        g_iComInitStatus = 0xff;
        break;
    case 7:
        g_oCommCfgParam.byComType = COMM_TYPE_RS232;
        g_oCommCfgParam.byTcpipDevType = 0;
        g_iComInitStatus = 0xff;
        break;
    case 0xff:
    case 0xfe:
        return ;
    default:
        GetKeyBlockMs(RESPOND_TIMEOUT);
        return ;
		 g_oCommCfgParam.byComType = COMM_TYPE_WIFI;
        g_oCommCfgParam.byTcpipDevType = OPT_DEVWIFI;
    }*/
}



/****************************************************************************
Function     :  int CommDial(U08 ucDialMode)
Descript     :  communication dial function
Input param. :  1. U08 ucDialMode : Only for PSTN
Output param.:  NULL
Return       :  0:success, other:failed
  History    :
      Author       date time   version      reason
  1. Ryan Huang    2011-05-17  V1.0         Create
****************************************************************************/
int CommDial(U08 ucDialMode)
{
    int iRet;

    if (   (PREDIAL_MODE==ucDialMode)
        && ((COMM_TYPE_PSTN!=g_oCommCfgParam.byComType)
          ||(COMM_TYPE_PSTN_PPP!=g_oCommCfgParam.byComType)))
    {
        return 0;
    }
    switch (g_oCommCfgParam.byComType)
    {
    case COMM_TYPE_RS232:
        iRet = s_RS232Dial(&g_oCommCfgParam.oRs232);
        break;

    case COMM_TYPE_PSTN:
        iRet = s_PSTNDial(ucDialMode, &g_oCommCfgParam.oPstn);
        break;

    case COMM_TYPE_LAN:
        iRet = s_TcpConnect(&g_oCommCfgParam);
        break;

    case COMM_TYPE_CDMA:
    case COMM_TYPE_GPRS:
        iRet = s_WirelessDial(&g_oCommCfgParam);
        break;

    case COMM_TYPE_WIFI:
        iRet = s_WiFiDial(&g_oCommCfgParam);
        break;

    case COMM_TYPE_PSTN_PPP:
        iRet = PPPLogin(OPT_DEVMODEM, g_oCommCfgParam.oPstn.szAPN,
            g_oCommCfgParam.oPstn.szUID, g_oCommCfgParam.oPstn.szPwd, 0, 20000);
        if (0 == iRet)
        {
            iRet = s_TcpConnect(&g_oCommCfgParam);
        }
        break;
    default:
        iRet = ERR_COMM_INV_TYPE;
    }

    return iRet;
}

// 通讯模块发送数据
int CommTxd(const U08 *psTxdData, U32 uiDataLen, U32 uiTimeOutMs)
{
    int iRet;

    if (NULL == psTxdData)
    {
        lcdClrLine(4, 7);
        lcdDisplayCE(0, 4, 0x01, "Command Error    ", "CommTxd error.  ");
        lcdDisplay(0, 6, 0x01, "psTxdData=NULL");
        return ERR_COMM_INV_PARAM;
    }

    switch (g_oCommCfgParam.byComType)
    {
    case COMM_TYPE_RS232:
        iRet = s_RS232Txd(&g_oCommCfgParam.oRs232, psTxdData, uiDataLen, uiTimeOutMs);
        break;

    case COMM_TYPE_PSTN:
        iRet = s_PSTNTxd(psTxdData, uiDataLen, uiTimeOutMs);
        break;

    case COMM_TYPE_LAN:
    case COMM_TYPE_WIFI:
        iRet = s_TcpTxd(psTxdData, uiDataLen);
        break;

    case COMM_TYPE_CDMA:
    case COMM_TYPE_GPRS:
    case COMM_TYPE_PSTN_PPP:
        iRet = s_TcpTxd(psTxdData, uiDataLen);
        break;

    default:
        lcdClrLine(4, 7);
        lcdDisplayCE(0, 4, 0x01, "数据发送错误    ", "CommTxd error.  ");
        lcdDisplay(0, 6, 0x01, "ComType=%d", g_oCommCfgParam.byComType);
        iRet = ERR_COMM_INV_TYPE;
    }

    return iRet;
}

// 通讯模块接收数据
int CommRxd(U08 *psRxdData, U32 uiExpLen, U32 uiTimeOutMs, U32 *puiOutLen)
{
    int iRet;

    if (NULL == psRxdData)
    {
        return ERR_COMM_INV_PARAM;
    }

    switch (g_oCommCfgParam.byComType)
    {
    case COMM_TYPE_RS232:
        if (uiTimeOutMs <= 5000)
        {
            uiTimeOutMs = 5000;
        }
        iRet = s_RS232Rxd(&g_oCommCfgParam.oRs232, psRxdData, uiExpLen,
            uiTimeOutMs, puiOutLen);
        break;

    case COMM_TYPE_PSTN:
        if (uiTimeOutMs <= 15000)
        {
            uiTimeOutMs = 15000;
        }
        iRet = s_PSTNRxd(psRxdData, uiExpLen, uiTimeOutMs, puiOutLen);
        break;

    case COMM_TYPE_LAN:
    case COMM_TYPE_WIFI:
        if (uiTimeOutMs <= 10000)
        {
            uiTimeOutMs = 10000;
        }
        iRet = s_TcpRxd(psRxdData, uiExpLen, uiTimeOutMs, puiOutLen);
        break;

    case COMM_TYPE_CDMA:
    case COMM_TYPE_GPRS:
    case COMM_TYPE_PSTN_PPP:
        if (uiTimeOutMs <= 15000)
        {
            uiTimeOutMs = 15000;
        }
        iRet = s_TcpRxd(psRxdData, uiExpLen, uiTimeOutMs, puiOutLen);
        break;

    default:
        iRet = ERR_COMM_INV_TYPE;
    }

    return iRet;
}

// 通讯模块断开链路(MODEM挂机或者TCP断开TCP连接等)
int CommOnHook(U08 byReleaseAll)
{
    int iRet;
    int Col, Line;
    U08 Buff[5120]; // 当前屏幕为128*64，建议开辟的缓冲区大小为2K；

    lcdStore(&Col, &Line, Buff);
    lcdClrLine(0, 7);
    lcdDisplayCE(0, 4, 1, "正在挂断...     ", "On hook...      ");

    switch (g_oCommCfgParam.byComType)
    {
    case COMM_TYPE_RS232:
        iRet = s_RS232OnHook(byReleaseAll, &g_oCommCfgParam.oRs232);
        break;

    case COMM_TYPE_PSTN:
        iRet = s_PSTN_OnHook(byReleaseAll);
        break;

    case COMM_TYPE_LAN:
    case COMM_TYPE_WIFI:
        iRet = s_TcpClose();
        break;

    case COMM_TYPE_CDMA:
    case COMM_TYPE_GPRS:
    case COMM_TYPE_PSTN_PPP:
        iRet = s_PPPClose(byReleaseAll, g_oCommCfgParam.byComType);
        break;

    default:
        iRet = ERR_COMM_INV_TYPE;
    }

    lcdRestore(Col, Line, Buff);
    return iRet;
}

void GetAllErrMsg(int iErrCode, ERR_INFO *pstInfo, COMM_ERR_MSG *pstCommErrMsg)
{
    int iCnt;

    for (iCnt=0; pstInfo[iCnt].iErrCode!=0; iCnt++)
    {
        if (pstInfo[iCnt].iErrCode == iErrCode)
        {
            sprintf((char *)pstCommErrMsg->szChnMsg, "%.16s", pstInfo[iCnt].szChnMsg);
            sprintf((char *)pstCommErrMsg->szEngMsg, "%.16s", pstInfo[iCnt].szEngMsg);
            break;
        }
    }
}

// 获取通讯错误信息
void CommGetErrMsg(int iErrCode, COMM_ERR_MSG *pstCommErrMsg)
{
    int iReErrCode;

    if (NULL == pstCommErrMsg)
    {
        return;
    }

    iReErrCode = iErrCode;

    switch (g_oCommCfgParam.byComType)
    {
    case COMM_TYPE_RS232:
        sprintf((char *)pstCommErrMsg->szChnMsg, "串口错误:%04X", iReErrCode);
        sprintf((char *)pstCommErrMsg->szEngMsg, "PORT ERR:%04X", iReErrCode);
        GetAllErrMsg(iErrCode, sg_stRS232ErrMsg, pstCommErrMsg);
        break;

    case COMM_TYPE_PSTN:
        sprintf((char *)pstCommErrMsg->szChnMsg, "拨号错误:%04X", iReErrCode);
        sprintf((char *)pstCommErrMsg->szEngMsg, "MODEM ERR:%04X", iReErrCode);
        GetAllErrMsg(iErrCode, sg_stModemErrMsg, pstCommErrMsg);
        break;

    case COMM_TYPE_LAN:
        sprintf((char *)pstCommErrMsg->szChnMsg, "TCPIP错误:%04X", iReErrCode);
        sprintf((char *)pstCommErrMsg->szEngMsg, "TCPIP ERR:%04X", iReErrCode);
        GetAllErrMsg(iErrCode, sg_stPPPErrMsg, pstCommErrMsg);
        break;
    case COMM_TYPE_CDMA:
    case COMM_TYPE_GPRS:
        sprintf((char *)pstCommErrMsg->szChnMsg, "无线错误:%04X", iReErrCode);
        sprintf((char *)pstCommErrMsg->szEngMsg, "WIRE ERR:%04X", iReErrCode);
        GetAllErrMsg(iErrCode, sg_stPPPErrMsg, pstCommErrMsg);
        break;
    default:
        sprintf((char *)pstCommErrMsg->szChnMsg, "通讯错误:%04X", iReErrCode);
        sprintf((char *)pstCommErrMsg->szEngMsg, "COMM ERR:%04X", iReErrCode);
        GetAllErrMsg(iErrCode, sg_stCommErrMsg, pstCommErrMsg);
        break;
    }
}


#if SUPPORT_OPENSSL_RSA


static char g_strBase64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

extern RSA *g_oRsaPubKey;

// 将二进制码转换成Base64编码
void Base64Encode(const U08 *pbyBinIn, int iBinLen, char *strBase64Out, int *piBaseLen)
{
    int i, j, k;
    U32 dwTemp;

    j = iBinLen%3;
    k = iBinLen/3;
    for (i=0; i<k; i++)
    {
        dwTemp = pbyBinIn[i*3];
        dwTemp <<= 8;
        dwTemp += pbyBinIn[i*3+1];
        dwTemp <<= 8;
        dwTemp += pbyBinIn[i*3+2];
        strBase64Out[i*4+3] = g_strBase64Chars[dwTemp&0x3f];
        dwTemp >>= 6;
        strBase64Out[i*4+2] = g_strBase64Chars[dwTemp&0x3f];
        dwTemp >>= 6;
        strBase64Out[i*4+1] = g_strBase64Chars[dwTemp&0x3f];
        dwTemp >>= 6;
        strBase64Out[i*4] = g_strBase64Chars[dwTemp&0x3f];
    }
    if (1 == j)
    {
        dwTemp = pbyBinIn[iBinLen-1];
        dwTemp <<= 4;
        strBase64Out[k*4+1] = g_strBase64Chars[dwTemp&0x3f];
        dwTemp >>= 6;
        strBase64Out[k*4] = g_strBase64Chars[dwTemp&0x3f];
        strBase64Out[k*4+2] = '=';
        strBase64Out[k*4+3] = '=';
        *piBaseLen = k*4+4;
    }
    else if (2 == j)
    {
        dwTemp = pbyBinIn[iBinLen-2];
        dwTemp <<= 8;
        dwTemp += pbyBinIn[iBinLen-1];
        dwTemp <<= 2;
        strBase64Out[k*4+2] = g_strBase64Chars[dwTemp&0x3f];
        dwTemp >>= 6;
        strBase64Out[k*4+1] = g_strBase64Chars[dwTemp&0x3f];
        dwTemp >>= 6;
        strBase64Out[k*4] = g_strBase64Chars[dwTemp&0x3f];
        strBase64Out[k*4+3] = '=';
        *piBaseLen = k*4+4;
    }
    else
    {
        *piBaseLen = k*4;
    }
}

static U08 ConvertString(char ch)
{
    if ((ch>='A') && (ch<='Z'))
    {
        return (ch-'A');
    }
    else if ((ch>='a')&&(ch<='z'))
    {
        return (ch-'a'+26);
    }
    else if ((ch>='0')&&(ch<='9'))
    {
        return (ch-'0'+52);
    }
    else if ('+'==ch)
    {
        return 62;
    }
    else if (0x2f==ch)
    {
        return 63;
    }
    else
    {
        return 255;
    }
}

// 将Base64编码转换成二进制码
int Base64Decode2(const char *strBase64In, int iBase64Len, U08 *pbyBinOut, int *piBinLen)
{
    int i, j, k, l;
    U32 dwTemp;

    static char strBuff[2048];

    if (iBase64Len > 2048)
    {
        return -1;
    }

    j = 0;
    for (i=0; i<iBase64Len; i++)
    {
        if (   ((strBase64In[i]>='A') && (strBase64In[i]<='Z'))
            || ((strBase64In[i]>='a') && (strBase64In[i]<='z'))
            || ((strBase64In[i]>='0') && (strBase64In[i]<='9'))
            || ('+'==strBase64In[i]) || ('/'==strBase64In[i]))
        {
            strBuff[j] = strBase64In[i];
            j++;
        }
    }
    k = j%4;
    if ((0==j) || (1==k))
    {
        return -2;
    }
    l = j/4;
    for (i=0; i<l; i++)
    {
        dwTemp = ConvertString(strBuff[i*4]);
        dwTemp <<= 6;
        dwTemp += ConvertString(strBuff[i*4+1]);
        dwTemp <<= 6;
        dwTemp += ConvertString(strBuff[i*4+2]);
        dwTemp <<= 6;
        dwTemp += ConvertString(strBuff[i*4+3]);
        pbyBinOut[i*3] = (U08)(dwTemp>>16);
        pbyBinOut[i*3+1] = (U08)(dwTemp>>8);
        pbyBinOut[i*3+2] = (U08)(dwTemp);
    }
    if (2 == k)
    {
        dwTemp = ConvertString(strBuff[j-2]);
        dwTemp <<= 6;
        dwTemp += ConvertString(strBuff[j-1]);
        dwTemp >>= 4;
        pbyBinOut[l*3] = (U08)(dwTemp);
        *piBinLen = l*3+1;
    }
    else if (3 == k)
    {
        dwTemp = ConvertString(strBuff[j-3]);
        dwTemp <<= 6;
        dwTemp += ConvertString(strBuff[j-2]);
        dwTemp <<= 6;
        dwTemp += ConvertString(strBuff[j-1]);
        dwTemp >>= 2;
        pbyBinOut[l*3] = (U08)(dwTemp>>8);
        pbyBinOut[l*3+1] = (U08)dwTemp;
        *piBinLen = l*3+2;
    }
    else
    {
        *piBinLen = l*3;
    }
    return 0;
}

#endif

extern COMM_CFG_PARAM g_oCommCfgParam;

int RsaSend(const char *strSendMsg, U32 uiTimeOutMs)
{
    int iLen, iRet, i;
    static U08 abyTemp[1024], byLrc;

#if  SUPPORT_OPENSSL_RSA
    int iBase64Len;
    static U08 abyBase64[1024];

    i = (g_oRsaPubKey->n->dmax-1)*4;
    iLen = strlen(strSendMsg);
    if (iLen > (i-11))
    {
        lcdCls();
        lcdDisplay(0, 0, 0x01, "Send len error");
        lcdDisplay(0, 2, 0x01, "iLen=%d,%d,  ", iLen, i);
        return -1;
    }
    iRet = RSA_public_encrypt(iLen, strSendMsg, abyTemp, g_oRsaPubKey, RSA_PKCS1_PADDING);
    Base64Encode(abyTemp, i, abyBase64, &iBase64Len);

    abyTemp[0] = (U08)iBase64Len;
    abyTemp[1] = (U08)(iBase64Len>>8);
    abyTemp[2] = 0;
    abyTemp[3] = 0;
    memcpy(&abyTemp[4], abyBase64, iBase64Len);
    byLrc = 0;
    for (i=0; i<iBase64Len; i++)
    {
        byLrc ^= abyBase64[i];
    }
    abyTemp[4+iBase64Len] = byLrc;
    iLen = iBase64Len+5;
    iRet = CommTxd(abyTemp, iLen, 3000);
    if (0 != iRet)
    {
        return -2;
    }
#else
    iLen = strlen(strSendMsg);
    abyTemp[0] = (U08)iLen;
    abyTemp[1] = (U08)(iLen>>8);
    abyTemp[2] = 0;
    abyTemp[3] = 0;
    memcpy(&abyTemp[4], strSendMsg, iLen);
    byLrc = 0;
    for (i=0; i<iLen; i++)
    {
        byLrc ^= strSendMsg[i];
    }
    abyTemp[4+iLen] = byLrc;
    iRet = CommTxd(abyTemp, iLen+5, 3000);
    if (0 != iRet)
    {
        return -2;
    }
#endif

    return 0;
}

int RsaRecv(U08 *psRxdData, U32 uiTimeOutMs, U32 *puiOutLen)
{
    int iRet, iLen, iExpLen, i;
    U08 byLrc;

#if  SUPPORT_OPENSSL_RSA
    static U08 abyBuff[2048];

    iRet = CommRxd(psRxdData, 4, uiTimeOutMs, &iLen);
    if (0 != iRet)
    {
        return iRet;
    }
    iExpLen = psRxdData[1];
    iExpLen <<= 8;
    iExpLen += psRxdData[0];
    if (iExpLen > 1024)
    {
        lcdClrLine(4, 7);
        lcdDisplay(0, 4, 0x01, "CommRxd len err.");
        lcdDisplay(0, 6, 0x01, "iExpLen=%d,", iExpLen);
        return -1;
    }
//    lcdCls();
//    lcdDisplay(0, 0, 0x01, "CommRxd len=%d", iExpLen+5);
    iRet = CommRxd(&psRxdData[4], iExpLen+1, uiTimeOutMs, &iLen);
    if (0 != iRet)
    {
        return iRet;
    }
    *puiOutLen = iExpLen+5;
    byLrc = 0;
    iExpLen++;
    for (i=0; i<iExpLen; i++)
    {
        byLrc ^= psRxdData[i+4];
    }
    if (0 != byLrc)
    {
        lcdClrLine(4, 7);
        lcdDisplay(0, 4, 0x01, "Comm Rxd Lrc    ");
        lcdDisplay(0, 6, 0x01, "error.%x,", byLrc);
        return -25;
    }
    iRet = Base64Decode2(&psRxdData[4], iExpLen-1, abyBuff, &iLen);
    if (0 != iRet)
    {
        lcdClrLine(4, 7);
        lcdDisplay(0, 4, 0x01, "Base64 decode   ");
        lcdDisplay(0, 6, 0x01, "error.%d", iRet);
        return iRet;
    }
    memcpy(psRxdData, abyBuff, iLen);
    *puiOutLen = iLen;
#else
    iRet = CommRxd(psRxdData, 4, uiTimeOutMs, &iLen);
    if (0 != iRet)
    {
        return iRet;
    }
    iExpLen = psRxdData[1];
    iExpLen <<= 8;
    iExpLen += psRxdData[0];
    if (iExpLen > 1024)
    {
        lcdClrLine(4, 7);
        lcdDisplay(0, 4, 0x01, "CommRxd len err.");
        lcdDisplay(0, 6, 0x01, "iExpLen=%d,", iExpLen);
        return -1;
    }
//    lcdCls();
//    lcdDisplay(0, 0, 0x01, "CommRxd len=%d", iExpLen+5);
    iRet = CommRxd(&psRxdData[4], iExpLen+1, uiTimeOutMs, &iLen);
    if (0 != iRet)
    {
        return iRet;
    }
    *puiOutLen = iExpLen+5;
    byLrc = 0;
    iExpLen++;
    for (i=0; i<iExpLen; i++)
    {
        byLrc ^= psRxdData[i+4];
    }
    if (0 != byLrc)
    {
        lcdClrLine(4, 7);
        lcdDisplay(0, 4, 0x01, "Comm Rxd Lrc    ");
        lcdDisplay(0, 6, 0x01, "lrc=%x,", byLrc);
        return -25;
    }
    memcpy(psRxdData, &psRxdData[4], iExpLen);
    *puiOutLen = iExpLen;
#endif
    return 0;

}



#define COMM_TEST_NUM  7

U08 g_abyCommTestEng[COMM_TEST_NUM][20] =
    {"GetIP test  ","Port test   ","Send Recv Test","CDMA        ","PTSN PPP    ",
     "LAN         ","RS232       ","SendRecvTest"};

void GetIptest(void)
{
    U08 abyTemp[16], abyIp[8];

    memcpy(abyTemp, "192.168.0.15", 15);
    abyTemp[15] = 0;
    lcdDisplay(0, 0, 0x01, "IP test");
    GetIPAddress(FALSE, abyTemp, abyIp);
    lcdCls();
    lcdDisplay(0, 0, 0x01, "IP valid");
    lcdDisplay(0, 2, 0x01, "%d.%d.%d.%d", abyIp[0], abyIp[1], abyIp[2], abyIp[3]);
   // GetKeyBlockMs(RESPOND_TIM EOUT);
}
void GetPorttest(void)
{
    int iPort;

    iPort = 8080;
    if (0 == s_GetIPPort("Ip test", &iPort))
    {
        lcdCls();
        lcdDisplay(0, 0, 0x01, "Port valid");
        lcdDisplay(0, 2, 0x01, "%d", iPort);
    }
    else
    {
        lcdCls();
        lcdDisplay(0, 0, 0x01, "Port invalid");
        lcdDisplay(0, 2, 0x01, "%d", iPort);
    }
    GetKeyBlockMs(RESPOND_TIMEOUT);
}
void ShowTWnetRet(int ds, uint8_t message[50] )
{
lcdCls();
 lcdDisplay(0,ds, 0,"%s",message);
	        sysDelayMs(200);
}

int admin(void)
{

U08 abyTemp[10];
U08 abyTemp1[64];

    int32_t     iRet=0;
    
    uint8_t     sendbuf[256], recvbuf[255],username[256],password[256];
    
	memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		memset(abyTemp,0x00,sizeof(abyTemp));
		sysReadSN(abyTemp1);
		
	sprintf(hexs,"%s",abyTemp1);
		
		
		
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "USERNAME:");
		
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0xf5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter username");
		kbGetKey();
		return;
			//continue;
		}
		strcpy(username, (UINT8*)abyTemp);
		
				lcdClrLine(2,7);
lcdDisplay(0,2,DISP_CFONT,"PASSWORD:");
	//lcdDisplay(0,4,DISP_CFONT,"YYMMDDHHMM");
	lcdGoto(0, 6);
	memset(abyTemp, 0x00, sizeof(abyTemp));
	iRet = kbGetString(0x08, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
	if(iRet <=0)
	{
		lcdClrLine(2,7);
        lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"Enter password");
        kbGetKey();
        //continue;
		return;  
	}
		strcpy(password, (char*)abyTemp);
strcpy (Lstate,"****");
//strcpy(userid,"1234");

if(strcmp(Lstate,password) == 0)
			{
			
			ShowTWnetRet(4,"success Login");
				
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "NEW IP ADRESS:");
		
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0xf5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter New Ip");
		kbGetKey();
		return;
			//continue;
		}
		strcpy(ip, (UINT8*)abyTemp);
		
				lcdClrLine(2,7);
lcdDisplay(0,2,DISP_CFONT," New Port:");
	//lcdDisplay(0,4,DISP_CFONT,"YYMMDDHHMM");
	lcdGoto(0, 6);
	memset(abyTemp, 0x00, sizeof(abyTemp));
	iRet = kbGetString(0xf5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
	if(iRet <=0)
	{
		lcdClrLine(2,7);
        lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"Enter new port");
        kbGetKey();
        //continue;
		return;  
	}
		strcpy(port, (char*)abyTemp);
			Configupdate(0,ip,port,"");
			}
			else{
			ShowTWnetRet(4,"Not Authenticated");
			}
			
}


int testL(void)
{

U08 abyTemp[10];
U08 abyTemp1[64];

    int32_t     iRet=0;
    
    uint8_t     sendbuf[256], recvbuf[255],username[256],password[256];
    
	memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		memset(abyTemp,0x00,sizeof(abyTemp));
		sysReadSN(abyTemp1);
		
	sprintf(hexs,"%s",abyTemp1);
		
		strcpy(recvstr,"|STX|PA|TRUE|");
		
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "USERNAME:");
		
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0xf5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter username");
		kbGetKey();
		return;
			//continue;
		}
		strcpy(username, (UINT8*)abyTemp);
		
				lcdClrLine(2,7);
lcdDisplay(0,2,DISP_CFONT,"PASSWORD:");
	//lcdDisplay(0,4,DISP_CFONT,"YYMMDDHHMM");
	lcdGoto(0, 6);
	memset(abyTemp, 0x00, sizeof(abyTemp));
	iRet = kbGetString( 0x6D, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
	if(iRet <=0)
	{
		lcdClrLine(2,7);
        lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"Enter password");
        kbGetKey();
        //continue;
		return;  
	}
		strcpy(password, (char*)abyTemp);
strcpy (Lstate,"TRUE");
strcpy(userid,"1234");

if(strcmp(Lstate,"TRUE") == 0)
			{
			
			ShowTWnetRet(4,"successLstate");
			
			transtypes();
			}
			else{
			ShowTWnetRet(4,"screw this");
			}
			
}




int ratenew(void)
{
U08 abyTemp[20];
U08 TimeStr[25];
int iRet = 0;
sysGetTime(TimeStr);
UINT8 mytime[50];
lcdClrLine(2, 7);
changerate=1;
//strcpy(userid,"1234");
		lcdDisplay(0, 2, DISP_CFONT, "Enter New Rate:");
		//lcdDisplay(0, 6, DISP_CFONT, "_");
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0x1E5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter New Rate");
		kbGetKey();
		return;
			//continue;
		}
		strcpy(rateset, (UINT8*)abyTemp);
		
		
		rate=atof(rateset);
		
       return 0;
}

int newrates()
{

int iRet, iFlag;

    U08 abyTemp[64], abyLastTime[20],abyTemp1[64];
sysGetTime(abyTemp);
sysReadSN(abyTemp1);
strcpy(hexs,abyTemp1);

while(1)
{
    
			

lcdCls();
            lcdDispMultiLang(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, "测试磁卡", "PRESTIGE");
            lcdDisplay(0, 2, 0x40, " %s%02x %02x:%02x:%02x VS:1 ",
                    g_szMonthName1[abyTemp[1]], abyTemp[2], abyTemp[3], abyTemp[4], abyTemp[5], abyTemp[6]);
lcdDisplay(0, 4, 0x60, "1. ENTER NEW RATE ");
lcdDisplay(0, 5, 0x60, "9. PROCEED        ");
//lcdDisplay(0, 5, 0x60, "PRESS ENTER TO PROCEED  ");

//lcdDisplay(0, 4, 0x60, " 2.MAKE PAYMENT  ");
 //}
        iRet = kbGetKeyMs(1000);
        switch (iRet)
        {
        case KEY_MENU:
           // SetupMenu();
            //iFlag = 1;
            break;
        case KEY_FN:
            //FunctionsMenu();
            //iFlag = 1;
            break;
        case KEY1:
            //xreports();
			changerate=1;
			ratenew();
            break;
        case KEY2:
            //GetBaseStationInfoFromAPI();
			//getcartype();
			//manualentry();
	        //payment();
            //kbGetKey();
			//zreport();
			
            //iFlag = 1;
            break;
        case KEY3:
            //Wnet_SwitchProtocolModeTest();
            //iFlag = 1;
			//split();
            break;
       case KEY4:
           // WnetDnsResolveTest();
            break;
        case KEY5:
           // WifiPingTest();
            break;
        case KEY6:
           // GetNeighbourBaseStationInfoFromAPI();
            break;
        case KEY7:
           // Wnet_SetUserKeyCancelMode(0);
            break;
			case KEY8:
           // Wnet_SetUserKeyCancelMode(0);
            break;
			case KEY9:
           //changerate=0;
		   if(rate<=0)
		 {
		 
		 lcdCls();
		lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Zero Rate?? \n");
		sysDelayMs(2000);
		 newrates();
		 } else{
			return 1;
			}
            break;
        case KEY_ENTER:
		
        return 0;
            break;
       
        case KEY_CANCEL:
            kbFlush();
            //sysExit(0);
            return ;
        default:
            break;
        }
		

}

}




int splitL(UINT8* recvbuf)
{
UINT8 recvbuff[256],error[50],name[256],serial[256],uid[256];
UINT8* pch;
int stxfound=0;
int etxfound=0;
int devicefound=0;
int statefound=0;
int eventfound=0;
int namefound=0;
int uidfound=0;
 int i=0;  
memset(state,0x00,sizeof(state));
//strcpy(recvbuff,"|STX|POS1|PA|TRUE|KURIA EVANS|ETX|");
//strcpy(recvbuff,recvbuf);

pch = strtok (recvbuf,"|");
// pch = strtok (recvbuff,"|");
if(strstr(pch, "STX") != NULL)
    {
    	
       
        stxfound = 1;
    
    
    while (pch != NULL)
    {
        pch = strtok (NULL, "|"); 
        if(stxfound == 1 && eventfound == 0)
        {
            
            
                
                eventfound = 1;
                
            
        }
		
       else  if(stxfound == 1 && devicefound == 0 && eventfound == 1)
        {
            
            strcpy(serial,pch);
                
                devicefound = 1;
                
            
        }
		
       else if(stxfound == 1 && devicefound == 1 && eventfound == 1 && statefound == 0)
        {
            
            strcpy(error,pch);
			 memcpy(Lstate,error,50);
		statefound=1;
		
        }
        else if(stxfound == 1 && devicefound == 1 && statefound == 1 && namefound == 0)
        {
            
            
                strcpy(name,pch);
				sprintf(fullname,"%s",name);
				/*prnInit();
				prnPrintf("name found:%s\n",fullname);
				prnStart();*/

                namefound = 1;
               
            
            }
           
            else if(stxfound == 1 && devicefound == 1 && statefound == 1&& namefound == 1 &&  uidfound== 0  )
            {
              
                strcpy(uid,pch);
				sprintf(userid,"%s",uid);
				//prnInit();
				//prnPrintf("userid found:%s\n",userid);
				//prnStart();

               uidfound = 1;
               
               
           } 
		   else if(stxfound == 1 && devicefound == 1 && statefound == 1&& namefound == 1 &&  etxfound== 0  )
            {
              
                if(strcmp(pch,"ETX") == 0)
                {
                
                etxfound = 1;
                
              }
			}
               
            
       
        
        
       
        
        i++;
    }//loop back
    return (1);
	}
    else
    {
        return(0);
    }
	}

	
int splitR(UINT8* recvbuf)
{
UINT8 recvbuff[256],status[50],detail[100],func[6];
UINT8* pch;
int stxfound=0;
int etxfound=0;
int funcfound=0;
int statusfound=0;
int detailfound=0;
int generalfound=0;

 int i=0;  
memset(status,0x00,sizeof(status));
strcpy(recvbuff,"|STX|RT|OK|0.123|1|ETX|\n");
//strcpy(recvbuff,recvbuf);

//pch = strtok (recvbuf,"|");
 pch = strtok (recvbuf,"|");
if(strstr(pch, "STX") != NULL)
    {
    	
       
        stxfound = 1;
    
    
    while (pch != NULL)
    {
        pch = strtok (NULL, "|"); 
        if(stxfound == 1 && funcfound == 0)
        {
            
            
                
                funcfound = 1;
                
            
        }
		
       else  if(stxfound == 1 && funcfound == 1 && statusfound == 0)
        {
            
            strcpy(status,pch);
                
                statusfound = 1;
                
            
        }
		
       else if(stxfound == 1 && funcfound == 1 && statusfound == 1 && generalfound == 0)
        {
            
            strcpy(general,pch);
			
			 if(strcmp(status,"OK") == 0)
			 {
			 rate=atof(general);
			 
			 }
			 if(strcmp(status,"OK") == 0)
			 {
			 rate=atof(general);
			 
			 }
			 else if(strcmp(status,"ERROR") == 0)
			 {
			 strcpy(general,pch);
			 
			 }
		generalfound=1;
		
        }
        else if(stxfound == 1 && funcfound == 1 && statusfound == 1 && generalfound == 1 && detailfound == 0)
        {
            
            
                //if(strcmp(status,"INSUFF") == 0)

               detailfound = 1;
               
            
            
           
            
               
               
           } 
		   else if(stxfound == 1 && funcfound == 1 && statusfound == 1 && generalfound == 1 && detailfound == 1 &&  etxfound== 0  )
            {
              
                if(strcmp(pch,"ETX") == 0)
                {
                
                etxfound = 1;
                
              }
			}
               
            
       
        
        
       
        
        i++;
    }//loop back
    return (1);
	}
    else
    {
        return(0);
    }
	}
	
	
int splitObal(UINT8* recvbuf)
{
UINT8 recvbuff[256],number[50],even[20],curf[100],amountf[100];
UINT8* pch;
int stxfound=0;
int etxfound=0;
int eventfound=0;
int numberfound=0;
int curfound=0;
int amountfound=0;
int no=0;

 int i=0, j=0,k=0;  
strcpy(recvbuff,"|STX|OPbal|2|UGX|12|KES|100|ETX|\n");
//strcpy(recvbuff,recvbuf);
if(closing==1){

				   prnInit();
				   prnPrintf("** FOREX CLOSING BALANCES **\n");
				   prnStart();

}
else{

				   prnInit();
				   prnPrintf("** FOREX OPENING BALANCES **\n");
				   prnStart();
				   }
pch = strtok (recvbuf,"|");
 //pch = strtok (recvbuff,"|");
if(strstr(pch, "STX") != NULL)
    {
    	
       
        stxfound = 1;
    
    
    while (pch != NULL)
    {
        pch = strtok (NULL, "|"); 
        if(stxfound == 1 && eventfound == 0)
        {
            
            
                
                eventfound = 1;
				 
                
            
        }
		
       else  if(stxfound == 1 && eventfound == 1 && numberfound == 0)
        {
            
            strcpy(number,pch);
                j=atoi(number);
                numberfound = 1;
				
            
        }
		
       else if(stxfound == 1 && eventfound == 1 && numberfound == 1&&  etxfound== 0 )
        {
            
			
			
             if(strcmp(pch,"ETX") == 0)
                {
                k=0;
				i=0;
                etxfound = 1;
				prnInit();
				   prnPrintf("** THE END **\n");
				   prnPrintf("\n");
				   prnPrintf("\n");
				   prnPrintf("\n");
				   prnPrintf("\n");
				   prnStart();
                
              }
			  else{ 
			   
			   k=k+1;
			  strcpy(number,pch);
                j=atoi(number);
                numberfound = 1;
				
                
			      
			        if( k % 2 == 0)
				   {
				   strcpy(amountf,pch);
				  
					prnInit();
				   prnPrintf("**  %s   %s **\n",curf,amountf);
				   prnStart();
				   }
				   else{
				   
				    strcpy(curf,pch);
					
				   
				   }
				   
				   
			  }
			  
			  
			  
		
        } 
            
       
        
        
       
        
        i++;
    }//loop back
    return (1);
	}
    else
    {
        return(0);
    }
	}


int testserverL(void)
{
U08 abyTemp[10];
U08 abyTemp1[64];
int usid;
uint8_t     lenbuf[128];
    int32_t     iRet=0;
    int32_t     sockfd=0;
    uint32_t    BeginTime, EndTime;
    struct sockaddr    HostSockAddr;
    struct timeval     RecvTime;
    uint32_t        uiDataLen       = 1024;         //  data block size
    uint32_t        uiRecvTimeOut   = 180000;       //  data recv timer out ms
    uint8_t         ucIP[30]        = "197.248.118.113";
    in_port_t       netPort         = 9890;
    uint8_t         ucAPN[30]       = "internet";
    uint8_t         ucUID[70]       = "";
    uint8_t         ucUPWD[70]      = "";
    uint8_t     sendbuf[256], recvbuf[255],username[256],password[256],sendstr[256];
    int32_t     sendlen=256, recvlen=255;
	uint8_t     sendbuff[256], recvbuff[255];
	UINT8 receiptnumber[256];
	memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		memset(abyTemp,0x00,sizeof(abyTemp));
		sysReadSN(abyTemp1);
		//char* pend;
	prnInit();
	//ReadRecordSetting(0,"");
	//prnPrintf("terminal ids: %s\n",hexs);
	
	sprintf(hexs,"%s",abyTemp1);
	//printf(recvstr,"|STX|%s|PA|",hexs);
	//prnPrintf("terminal ids: %s\n",hexs);
	//prnPrintf("recvs: %s\n",recvstr);
	
		strcpy(recvstr,"|STX|PA|TRUE|");
		//prnPrintf("recvs: %s\n",recvstr);
		//prnStart();
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "USERNAME:");
		//lcdDisplay(0, 6, DISP_CFONT, "_");
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0xf5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter username");
		kbGetKey();
		return;
			//continue;
		}
		strcpy(username, (UINT8*)abyTemp);
		
				lcdClrLine(2,7);
lcdDisplay(0,2,DISP_CFONT,"PASSWORD:");
	//lcdDisplay(0,4,DISP_CFONT,"YYMMDDHHMM");
	lcdGoto(0, 6);
	memset(abyTemp, 0x00, sizeof(abyTemp));
	iRet = kbGetString(0x08, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
	if(iRet <=0)
	{
		lcdClrLine(2,7);
        lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"Enter password");
        kbGetKey();
        //continue;
		return;  
	}
		strcpy(password, (char*)abyTemp);
	//prnInit();
	//prnPrintf("recvs: %s\n",recvstr);
	sprintf(sendstr,"|STX|PA|PA|%s|%s|ETX|\r",username,password);	
	//prnPrintf("sendstr%s\n",sendstr);
	//prnStart();
	//prnInit();
	//prnPrintf("starting");
	//prnStart();
  iRet = WnetInit(20000); //  init module in 20 seconds.
    if(iRet != NET_OK)
    {
	lcdCls();
	lcdClrLine(2,7);
        lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"NO GPRS MODULE");
		sysDelayMs(2000);
        //kbGetKey();
        return; //  can not find gprs or cdma module
    }

    ///////////  check sim status
	lcdCls();
    lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "checking SIM...");
	
    iRet = WnetCheckSim();
   // ShowTWnetRet(3, iRet);
    if(iRet == -NET_ERR_PIN)
    {
        memset(lenbuf, 0, sizeof(lenbuf));
		lcdCls();
        lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "Input PIN:");
        lcdGoto(0, 5);
        iRet = kbGetString(0xad, 0, 21, -1, lenbuf);       // 0xb5
        if(iRet <= 0) return;
        iRet = WnetInputSimPin(lenbuf);
        if(iRet != NET_OK)
        {
		lcdCls();
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "SIM ERROR");
	
	sysDelayMs(2000);
            //ShowTWnetRet(5, iRet);
            return;
        }
    }
    else if(iRet != NET_OK)
    {
		lcdCls();
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "SIM ERROR");
	
	sysDelayMs(2000);
        kbFlush();
        kbGetKey();
        return;
    }
//prnInit();
			//prnPrintf("am here");
			//prnStart();
    ////////  PPP Login, ppp always login at the begining, need't logout unless the link broken.
iRet = PPPLogin(OPT_DEVWNET, ucAPN, ucUID, ucUPWD, 0, 0);   //If application adopts dialing in blocked method, It is recommended to set the timeout to 65000 ms.
    if(iRet != NET_OK)
    {
	lcdCls();
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "CONNECTION ERROR");
	
	sysDelayMs(2000);
        return;
    }
    BeginTime = sysGetTimerCount();
    while(1)
    {
        EndTime = sysGetTimerCount();
        if(EndTime-BeginTime > 65000)   //  timeout
        {
            return;
        }
        iRet = PPPCheck(OPT_DEVWNET);
        if(iRet != -NET_ERR_LINKOPENING)
        {
            if(iRet == NET_OK)  //  PPP Login successfully
            {
			//prnInit();
			//prnPrintf("link open");
			//prnStart();
                break;
            }
            else    //  PPP Login fail
            {
                return;
            }
        }
        sysDelayMs(200); //  still link opening
    }
	
if(sockfd > 0)  //  can not create a sockfd
    {
	NetClose(sockfd);    
	ShowTWnetRet(4,"port closed");
        //continue;
    }
	   
        sockfd = NetSocket(AF_INET, SOCK_STREAM, 0);    //  if want to open multi sock, repeat NetSocket and save return sockfd
    if(sockfd < 0)  //  can not create a sockfd
    {
	ShowTWnetRet(4,"can not create a sockfd");
	 sockfd = NetSocket(AF_INET, SOCK_STREAM, 0);
        //return 1;
    }
	ShowTWnetRet(4,"connecting....");
    //////  TCP open
    SockAddrset(&HostSockAddr, ucIP, netPort);
    iRet = NetConnect(sockfd, &HostSockAddr, sizeof(HostSockAddr));
    if(iRet < 0)  //  tcp open fail
    {
	ShowTWnetRet(4,"Server connection fail.");
        NetClose(sockfd);  
sysDelayMs(2000);		//  this step must be performed after NetSocket()
        return 1;
    }
	 ShowTWnetRet(4,"Server connected..");
	   
    
	RecvTime.tv_sec = uiRecvTimeOut/1000;
    RecvTime.tv_usec = uiRecvTimeOut%1000;
    NetSetsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &RecvTime, sizeof(RecvTime));
    BeginTime = sysGetTimerCount();
    recvlen = 0;
    while(1)        //  repeat receive data until you receive enough len.
    {
        iRet = NetRecv(sockfd, recvbuf+recvlen, sendlen-recvlen, 0);
		
        EndTime = sysGetTimerCount();
        if(iRet == 0)   //  receive nothing when timeout
        {
		ShowTWnetRet(4,"performed after NetSocket(1)");
		
            NetClose(sockfd);    //  this step must be performed after NetSocket()
            return 0;
        }
        else if(iRet > 0)
        {
            recvlen += iRet;
            if(recvlen == sendlen)  //  receive enough len.
            {
			
        
               break;
            }
			
			  memset(recvbuf,0,sizeof(recvbuf));
			  recvlen=0;
			 // strcpy(sendbuf, "|STX|auth|123456|123456|ETX|\r");
			  strcpy(sendbuf,sendstr);
			  
    sendlen = 256; //  just demo
    iRet = NetSend(sockfd, sendbuf, sendlen, 0);
    if(iRet != sendlen) //  send error
    {
	ShowTWnetRet(4,"send error");
        NetClose(sockfd);    //  this step must be performed after NetSocket()
        return 1;
    }
	ShowTWnetRet(4,"sent...");
	RecvTime.tv_sec = uiRecvTimeOut/1000;
    RecvTime.tv_usec = uiRecvTimeOut%1000;
    NetSetsockopt("", SOL_SOCKET, SO_RCVTIMEO, &RecvTime, sizeof(RecvTime));
    BeginTime = sysGetTimerCount();
		iRet = NetRecv(sockfd, recvbuf+recvlen, sendlen-recvlen, 0);
		 EndTime = sysGetTimerCount();
			if(iRet == 0)   //  receive nothing when timeout
        {
		ShowTWnetRet(4,"performed after NetSocket(1)");
		return 1;
		}
		else if(iRet > 0)
        {
            recvlen += iRet;
            if(recvlen == sendlen)  //  receive enough len.
            {
			
        //ShowTWnetRet(4,recvbuf);
		
               break;
            }
		
		
			if(memcmp(recvstr, recvbuf, 13) == 0)
			{
			NetClose(sockfd); 
			splitL(recvbuf);
			usid=atoi(userid);
			 	//ReadRecordSetting(0,"");
			testserverobl();
			
		 
			NetClose(sockfd); 
			ShowTWnetRet(4,"successful login");
			
			transtypes();			
			}
			else{ ShowTWnetRet(3,"ERROR TRY TO LOGIN AGAIN");
			sysDelayMs(4000);
			}
	
			
			
		memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		memset(sendbuff,0x00,sizeof(sendbuff));
		recvlen=0;
		iRet=0;
	
		
		NetClose(sockfd);    //  this step must be performed after NetSocket()
        return 1;
		}
		NetClose(sockfd); 
		return 1;
		}
		else{
		   //  this step must be performed after NetSocket()
			 ShowTWnetRet(4,"performed after NetSocket(2)");
			 NetClose(sockfd); 
            return 0;
		
		}
   
    }
	

iRet=NetClose(sockfd);
iRet = PPPLogout(OPT_DEVWIFI);  
return ;
}
int testserverrate( UINT8* recb)
{
U08 abyTemp[10];
U08 abyTemp1[64];
int usid;
uint8_t     lenbuf[128];
    int32_t     iRet=0;
    int32_t     sockfd=0;
    uint32_t    BeginTime, EndTime;
    struct sockaddr    HostSockAddr;
    struct timeval     RecvTime;
    uint32_t        uiDataLen       = 1024;         //  data block size
    uint32_t        uiRecvTimeOut   = 180000;       //  data recv timer out ms
    uint8_t         ucIP[30]        = "197.248.118.113";
    in_port_t       netPort         = 9890;
    uint8_t         ucAPN[30]       = "internet";
    uint8_t         ucUID[70]       = "";
    uint8_t         ucUPWD[70]      = "";
    uint8_t     sendbuf[256], recvbuf[255],username[256],password[256],sendstr[256];
    int32_t     sendlen=256, recvlen=255;
	uint8_t     sendbuff[256], recvbuff[255];
	UINT8 receiptnumber[256];
	memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		memset(abyTemp,0x00,sizeof(abyTemp));
		sysReadSN(abyTemp1);
		
	sprintf(hexs,"%s",abyTemp1);
	  if(getnbal==0)
	  { 
		strcpy(recvstr,"|STX|RT|\n");
		}
		else if (getnbal==1)
	   {
		strcpy(recvstr,"|STX|GB|\n");
		}
  iRet = WnetInit(20000); //  init module in 20 seconds.
    if(iRet != NET_OK)
    {
	lcdCls();
	lcdClrLine(2,7);
        lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"NO GPRS MODULE");
		sysDelayMs(2000);
        //kbGetKey();
        return; //  can not find gprs or cdma module
    }

    ///////////  check sim status
	lcdCls();
    lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "checking SIM...");
	
    iRet = WnetCheckSim();
   // ShowTWnetRet(3, iRet);
    if(iRet == -NET_ERR_PIN)
    {
        memset(lenbuf, 0, sizeof(lenbuf));
		lcdCls();
        lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "Input PIN:");
        lcdGoto(0, 5);
        iRet = kbGetString(0xad, 0, 21, -1, lenbuf);       // 0xb5
        if(iRet <= 0) return;
        iRet = WnetInputSimPin(lenbuf);
        if(iRet != NET_OK)
        {
		lcdCls();
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "SIM ERROR");
	
	sysDelayMs(2000);
            //ShowTWnetRet(5, iRet);
            return;
        }
    }
    else if(iRet != NET_OK)
    {
		lcdCls();
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "SIM ERROR");
	
	sysDelayMs(2000);
        kbFlush();
        kbGetKey();
        return;
    }
//prnInit();
			//prnPrintf("am here");
			//prnStart();
    ////////  PPP Login, ppp always login at the begining, need't logout unless the link broken.
iRet = PPPLogin(OPT_DEVWNET, ucAPN, ucUID, ucUPWD, 0, 0);   //If application adopts dialing in blocked method, It is recommended to set the timeout to 65000 ms.
    if(iRet != NET_OK)
    {
	lcdCls();
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "CONNECTION ERROR");
	
	sysDelayMs(2000);
        return;
    }
    BeginTime = sysGetTimerCount();
    while(1)
    {
        EndTime = sysGetTimerCount();
        if(EndTime-BeginTime > 65000)   //  timeout
        {
            return;
        }
        iRet = PPPCheck(OPT_DEVWNET);
        if(iRet != -NET_ERR_LINKOPENING)
        {
            if(iRet == NET_OK)  //  PPP Login successfully
            {
			//prnInit();
			//prnPrintf("link open");
			//prnStart();
                break;
            }
            else    //  PPP Login fail
            {
                return;
            }
        }
        sysDelayMs(200); //  still link opening
    }
	
if(sockfd > 0)  //  can not create a sockfd
    {
	NetClose(sockfd);    
	ShowTWnetRet(4,"port closed");
        //continue;
    }
	   
        sockfd = NetSocket(AF_INET, SOCK_STREAM, 0);    //  if want to open multi sock, repeat NetSocket and save return sockfd
    if(sockfd < 0)  //  can not create a sockfd
    {
	ShowTWnetRet(4,"can not create a sockfd");
	 sockfd = NetSocket(AF_INET, SOCK_STREAM, 0);
        //return 1;
    }
	ShowTWnetRet(4,"connecting....");
    //////  TCP open
    SockAddrset(&HostSockAddr, ucIP, netPort);
    iRet = NetConnect(sockfd, &HostSockAddr, sizeof(HostSockAddr));
    if(iRet < 0)  //  tcp open fail
    {
	ShowTWnetRet(4,"Server connection fail.");
        NetClose(sockfd);  
sysDelayMs(2000);		//  this step must be performed after NetSocket()
        return 1;
    }
	 ShowTWnetRet(4,"Server connected..");
	   
    
	RecvTime.tv_sec = uiRecvTimeOut/1000;
    RecvTime.tv_usec = uiRecvTimeOut%1000;
    NetSetsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &RecvTime, sizeof(RecvTime));
    BeginTime = sysGetTimerCount();
    recvlen = 0;
    while(1)        //  repeat receive data until you receive enough len.
    {
        iRet = NetRecv(sockfd, recvbuf+recvlen, sendlen-recvlen, 0);
		
        EndTime = sysGetTimerCount();
        if(iRet == 0)   //  receive nothing when timeout
        {
		ShowTWnetRet(4,"performed after NetSocket(1)");
		
            NetClose(sockfd);    //  this step must be performed after NetSocket()
            return 0;
        }
        else if(iRet > 0)
        {
            recvlen += iRet;
            if(recvlen == sendlen)  //  receive enough len.
            {
			
        
               break;
            }
			
			  memset(recvbuf,0,sizeof(recvbuf));
			  recvlen=0;
			 // strcpy(sendbuf, "|STX|auth|123456|123456|ETX|\r");
			  strcpy(sendbuf,recb);
			  
    sendlen = 256; //  just demo
    iRet = NetSend(sockfd, sendbuf, sendlen, 0);
    if(iRet != sendlen) //  send error
    {
	ShowTWnetRet(4,"send error");
        NetClose(sockfd);    //  this step must be performed after NetSocket()
        return 1;
    }
	ShowTWnetRet(4,"sent...");
	RecvTime.tv_sec = uiRecvTimeOut/1000;
    RecvTime.tv_usec = uiRecvTimeOut%1000;
    NetSetsockopt("", SOL_SOCKET, SO_RCVTIMEO, &RecvTime, sizeof(RecvTime));
    BeginTime = sysGetTimerCount();
		iRet = NetRecv(sockfd, recvbuf+recvlen, sendlen-recvlen, 0);
		 EndTime = sysGetTimerCount();
			if(iRet == 0)   //  receive nothing when timeout
        {
		ShowTWnetRet(4,"performed after NetSocket(1)");
		return 1;
		}
		else if(iRet > 0)
        {
            recvlen += iRet;
            if(recvlen == sendlen)  //  receive enough len.
            {
			
        //ShowTWnetRet(4,recvbuf);
		
               break;
            }
		
		
			if(memcmp(recvstr, recvbuf, 8) == 0)
			{
			NetClose(sockfd); 
			if (getnbal==0)
			{
			splitR(recvbuf);
			
			}
			else if (getnbal==1){
			splitbal(recvbuf);
			}
			
			
			
			
			}
			else{ ShowTWnetRet(3,"ERROR TRY  AGAIN");
			sysDelayMs(4000);
			}
	
			
			
		memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		memset(sendbuff,0x00,sizeof(sendbuff));
		recvlen=0;
		iRet=0;
	
		
		NetClose(sockfd);    //  this step must be performed after NetSocket()
        return 1;
		}
		NetClose(sockfd); 
		return 1;
		}
		else{
		   //  this step must be performed after NetSocket()
			 ShowTWnetRet(4,"performed after NetSocket(2)");
			 NetClose(sockfd); 
            return 0;
		
		}
   
    }
	

iRet=NetClose(sockfd);
iRet = PPPLogout(OPT_DEVWIFI);  
return ;
}
 
  

int testserverobl(void)
{
U08 abyTemp[10];
U08 abyTemp1[64];
int usid;
uint8_t     lenbuf[128];
    int32_t     iRet=0;
    int32_t     sockfd=0;
    uint32_t    BeginTime, EndTime;
    struct sockaddr    HostSockAddr;
    struct timeval     RecvTime;
    uint32_t        uiDataLen       = 1024;         //  data block size
    uint32_t        uiRecvTimeOut   = 180000;       //  data recv timer out ms
    uint8_t         ucIP[30]        = "197.248.118.113";
    in_port_t       netPort         = 9890;
    uint8_t         ucAPN[30]       = "internet";
    uint8_t         ucUID[70]       = "";
    uint8_t         ucUPWD[70]      = "";
    uint8_t     sendbuf[256], recvbuf[255],username[256],password[256],sendstr[256];
    int32_t     sendlen=256, recvlen=255;
	uint8_t     sendbuff[256], recvbuff[255];
	UINT8 receiptnumber[256];
	memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		memset(abyTemp,0x00,sizeof(abyTemp));
		sysReadSN(abyTemp1);
		
	sprintf(hexs,"%s",abyTemp1);
	  
		strcpy(recvstr,"|STX|OBL|\n");
		
  iRet = WnetInit(20000); //  init module in 20 seconds.
    if(iRet != NET_OK)
    {
	lcdCls();
	lcdClrLine(2,7);
        lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"NO GPRS MODULE");
		sysDelayMs(2000);
        //kbGetKey();
        return; //  can not find gprs or cdma module
    }

    ///////////  check sim status
	lcdCls();
    lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "checking SIM...");
	
    iRet = WnetCheckSim();
   // ShowTWnetRet(3, iRet);
    if(iRet == -NET_ERR_PIN)
    {
        memset(lenbuf, 0, sizeof(lenbuf));
		lcdCls();
        lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "Input PIN:");
        lcdGoto(0, 5);
        iRet = kbGetString(0xad, 0, 21, -1, lenbuf);       // 0xb5
        if(iRet <= 0) return;
        iRet = WnetInputSimPin(lenbuf);
        if(iRet != NET_OK)
        {
		lcdCls();
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "SIM ERROR");
	
	sysDelayMs(2000);
            //ShowTWnetRet(5, iRet);
            return;
        }
    }
    else if(iRet != NET_OK)
    {
		lcdCls();
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "SIM ERROR");
	
	sysDelayMs(2000);
        kbFlush();
        kbGetKey();
        return;
    }
//prnInit();
			//prnPrintf("am here");
			//prnStart();
    ////////  PPP Login, ppp always login at the begining, need't logout unless the link broken.
iRet = PPPLogin(OPT_DEVWNET, ucAPN, ucUID, ucUPWD, 0, 0);   //If application adopts dialing in blocked method, It is recommended to set the timeout to 65000 ms.
    if(iRet != NET_OK)
    {
	lcdCls();
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "CONNECTION ERROR");
	
	sysDelayMs(2000);
        return;
    }
    BeginTime = sysGetTimerCount();
    while(1)
    {
        EndTime = sysGetTimerCount();
        if(EndTime-BeginTime > 65000)   //  timeout
        {
            return;
        }
        iRet = PPPCheck(OPT_DEVWNET);
        if(iRet != -NET_ERR_LINKOPENING)
        {
            if(iRet == NET_OK)  //  PPP Login successfully
            {
			//prnInit();
			//prnPrintf("link open");
			//prnStart();
                break;
            }
            else    //  PPP Login fail
            {
                return;
            }
        }
        sysDelayMs(200); //  still link opening
    }
	
if(sockfd > 0)  //  can not create a sockfd
    {
	NetClose(sockfd);    
	ShowTWnetRet(4,"port closed");
        //continue;
    }
	   
        sockfd = NetSocket(AF_INET, SOCK_STREAM, 0);    //  if want to open multi sock, repeat NetSocket and save return sockfd
    if(sockfd < 0)  //  can not create a sockfd
    {
	ShowTWnetRet(4,"can not create a sockfd");
	 sockfd = NetSocket(AF_INET, SOCK_STREAM, 0);
        //return 1;
    }
	ShowTWnetRet(4,"connecting....");
    //////  TCP open
    SockAddrset(&HostSockAddr, ucIP, netPort);
    iRet = NetConnect(sockfd, &HostSockAddr, sizeof(HostSockAddr));
    if(iRet < 0)  //  tcp open fail
    {
	ShowTWnetRet(4,"Server connection fail.");
        NetClose(sockfd);  
sysDelayMs(2000);		//  this step must be performed after NetSocket()
        return 1;
    }
	 ShowTWnetRet(4,"Server connected..");
	   
    
	RecvTime.tv_sec = uiRecvTimeOut/1000;
    RecvTime.tv_usec = uiRecvTimeOut%1000;
    NetSetsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &RecvTime, sizeof(RecvTime));
    BeginTime = sysGetTimerCount();
    recvlen = 0;
    while(1)        //  repeat receive data until you receive enough len.
    {
        iRet = NetRecv(sockfd, recvbuf+recvlen, sendlen-recvlen, 0);
		
        EndTime = sysGetTimerCount();
        if(iRet == 0)   //  receive nothing when timeout
        {
		ShowTWnetRet(4,"performed after NetSocket(1)");
		
            NetClose(sockfd);    //  this step must be performed after NetSocket()
            return 0;
        }
        else if(iRet > 0)
        {
            recvlen += iRet;
            if(recvlen == sendlen)  //  receive enough len.
            {
			
        
               break;
            }
			
			  memset(recvbuf,0,sizeof(recvbuf));
			  recvlen=0;
			 // strcpy(sendbuf, "|STX|auth|123456|123456|ETX|\r");
			  strcpy(sendbuf,"|STX|12334|OBL|ETX|\n");
			  
    sendlen = 256; //  just demo
    iRet = NetSend(sockfd, sendbuf, sendlen, 0);
    if(iRet != sendlen) //  send error
    {
	ShowTWnetRet(4,"send error");
        NetClose(sockfd);    //  this step must be performed after NetSocket()
        return 1;
    }
	ShowTWnetRet(4,"sent...");
	RecvTime.tv_sec = uiRecvTimeOut/1000;
    RecvTime.tv_usec = uiRecvTimeOut%1000;
    NetSetsockopt("", SOL_SOCKET, SO_RCVTIMEO, &RecvTime, sizeof(RecvTime));
    BeginTime = sysGetTimerCount();
		iRet = NetRecv(sockfd, recvbuf+recvlen, sendlen-recvlen, 0);
		 EndTime = sysGetTimerCount();
			if(iRet == 0)   //  receive nothing when timeout
        {
		ShowTWnetRet(4,"performed after NetSocket(1)");
		return 1;
		}
		else if(iRet > 0)
        {
            recvlen += iRet;
            if(recvlen == sendlen)  //  receive enough len.
            {
			
        //ShowTWnetRet(4,recvbuf);
		
               break;
            }
		
			if(memcmp(recvstr, recvbuf, 8) == 0)
			{
			NetClose(sockfd); 
			
			splitObal(recvbuf);
			
			
			
			
			
			
			
			}
			else{ ShowTWnetRet(3,"ERROR TRY  AGAIN");
			sysDelayMs(4000);
			}
	
			
			
		memset(sendbuf,0x00,sizeof(sendbuf));
		memset(recvbuf,0x00,sizeof(recvbuf));
		memset(sendbuff,0x00,sizeof(sendbuff));
		recvlen=0;
		iRet=0;
	
		
		NetClose(sockfd);    //  this step must be performed after NetSocket()
        return 1;
		}
		NetClose(sockfd); 
		return 1;
		}
		else{
		   //  this step must be performed after NetSocket()
			 ShowTWnetRet(4,"performed after NetSocket(2)");
			 NetClose(sockfd); 
            return 0;
		
		}
   
    }
	

iRet=NetClose(sockfd);
iRet = PPPLogout(OPT_DEVWIFI);  
return ;
}
 
 
 
 
 int splitbal(UINT8* recvbuf)
{
UINT8 recvbuff[256],currency[50],amount[50],event[50];
UINT8* pch;
int stxfound=0;
int etxfound=0;
int amountfound=0;
int curfound=0;
int eventfound=0;
 int i=0;  
memset(amount,0x00,sizeof(amount));
memset(currency,0x00,sizeof(currency));


pch = strtok (recvbuf,"|");
// pch = strtok (recvbuff,"|");
if(strstr(pch, "STX") != NULL)
    {
    	
       
        stxfound = 1;
    
    
    while (pch != NULL)
    {
        pch = strtok (NULL, "|"); 
        if(stxfound == 1 && eventfound == 0)
        {
            strcpy(currency,pch);
           
                
               eventfound = 1;
					
            
        }
		else if(stxfound == 1 && eventfound == 1 && curfound == 0)
        {
            strcpy(currency,pch);
            
                
                curfound = 1;
					
            
        }
       else  if(stxfound == 1 && eventfound == 1 && curfound == 1 && amountfound == 0)
        {
            
            strcpy(amount,pch);
			strcpy(amnt,pch);
			
			
                amountfound = 1;
					
        }
		
       
        else if(stxfound == 1 && eventfound == 1 && curfound == 1 && amountfound == 1 && etxfound == 0)
        {
            
            
                 if(strcmp(pch,"ETX") == 0)
                {
                
                etxfound = 1;
				
                
              }
               
            
            }
           
            
               
            
       
        
        
       
        
        i++;
    }//loop back
    return (1);
	}
    else
    {
        return(0);
    }
	}


int inputAmount(void)
{
U08 abyTemp[20];
U08 TimeStr[25];
int relamount=0;
int iRet = 0;
sysGetTime(TimeStr);
UINT8 mytime[50];
lcdClrLine(2, 7);
//strcpy(userid,"1234");
		lcdDisplay(0, 2, DISP_CFONT, "Amount Received:");
		//lcdDisplay(0, 6, DISP_CFONT, "_");
		lcdGoto(0, 6);
		memset(abyTemp,0x00,sizeof(abyTemp));
		iRet = kbGetString(0x1E5, 0, 32, COMM_USER_OPER_TIMEOUT, abyTemp);
		if (iRet <= 0)
		{
			lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Enter Amount...");
		kbGetKey();
		return;
			//continue;
		}
		strcpy(amountrec, (UINT8*)abyTemp);
		
		
		sprintf(sendstr,"|STX|1234536|RT|%d|%s|%s|%s|ETX|\n",forextype,currencyfrom,currencyto,amountrec);
		//prnPrintf("Sending.. %d\n",sendstr);
		//prnStart();
	 	//ReadRecordSetting(0,"");
		testserverrate(sendstr);
		if(strcmp(general,"ERROR") == 0)
                {
                
				lcdCls();
		lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Error:",general);
		sysDelayMs(2000);
		return 0;
                
              }
		
		lcdCls();
		lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Rate is %.2lf\n",rate);
		sysDelayMs(2000);
		
		newrates();
		 if(rate<=0)
		 {
		 
		 lcdCls();
		lcdClrLine(2, 7);
		lcdDisplay(0, 3, DISP_CFONT, "Zero Rate?? \n");
		sysDelayMs(2000);
		 newrates();
		 }
		 else {
		amountin = atof(amountrec);
		amountout= rate*amountin;
		
		//prnInit();
		sprintf(sendstr,"|STX|67568|SV|%d|%s|%s|%s|%.2lf|%s|%s|%.2lf|ETX|\n",forextype,currencyfrom,currencyto,amountrec,amountout,userid,hexs,rate);
		//prnPrintf("Sending.. %d\n",sendstr);
		//prnStart();
		getclientdetails();
				
				
		if(sent==0)
		{
		
		SaveNewRecord(sendstr);
		}
		
		
		
		return 0;
		}
				

}

int print(void)
{
int iRet, iFlag;
    U08 abyTemp[64], abyLastTime[20],abyTemp1[64];
sysGetTime(abyTemp);
sysReadSN(abyTemp1);
strcpy(hexs,abyTemp1);


while(1)
{
      
            if (abyTemp[1] > 12)
            {
                abyTemp[1] = 12;
            }
			
			

lcdCls();
            lcdDispMultiLang(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, "测试磁卡", "PRESTIGE");
            lcdDisplay(0, 2, 0x40, " %s%02x %02x:%02x:%02x VS:1   ",
                    g_szMonthName1[abyTemp[1]], abyTemp[2], abyTemp[3], abyTemp[4], abyTemp[5], abyTemp[6]);
lcdDisplay(0, 4, 0x60, " 1.Print Copy ");
lcdDisplay(0, 5, 0x60, " 2.Exit   ");


        iRet = kbGetKeyMs(1000);
        switch (iRet)
        {
        case KEY_MENU:
            SetupMenu();
            iFlag = 1;
            break;
        case KEY_FN:
            FunctionsMenu();
            iFlag = 1;
            break;
        case KEY1:
            forexreceipt();
			return 0;
            break;
        case KEY2:
            
			
			return 0;
			
            break;
        
        case KEY_CANCEL:
		kbFlush();
		
			return 0;
            
            //sysExit(0);
            return ;
        default:
            break;
        }
		
}
}

int transtypes(void)
{
int iRet, iFlag;
    U08 abyTemp[64], abyLastTime[20],abyTemp1[64];
sysGetTime(abyTemp);
sysReadSN(abyTemp1);
strcpy(hexs,abyTemp1);


while(1)
{
      
            if (abyTemp[1] > 12)
            {
                abyTemp[1] = 12;
            }
			
			

lcdCls();
            lcdDispMultiLang(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, "测试磁卡", "FOREX BUREAU");
           // lcdDisplay(0, 2, 0x40, " %s%02x %02x:%02x:%02x VS:1   ",
                   // g_szMonthName1[abyTemp[1]], abyTemp[2], abyTemp[3], abyTemp[4], abyTemp[5], abyTemp[6]);
lcdDisplay(0, 2, 0x60, " 1.Buy Foreign Currency    ");
lcdDisplay(0, 3, 0x60, " 2.Sell Foreign Currency   ");
lcdDisplay(0, 4, 0x60, " 3.Check Balance           ");
//lcdDisplay(0, 5, 0x60, " 4.Download Rates          ");
lcdDisplay(0, 5, 0x60, " 4.Close Shift             ");
lcdDisplay(0, 6, 0x60, " 9.Exit                    ");

        iRet = kbGetKeyMs(1000);
        switch (iRet)
        {
        case KEY_MENU:
            SetupMenu();
            iFlag = 1;
            break;
        case KEY_FN:
            FunctionsMenu();
            iFlag = 1;
            break;
        case KEY1:
            forextype=0;
			selectingf();
			fromselected=0;
			
            break;
        case KEY2:
            
			
			forextype=1;
            selectingf();
			fromselected=0;
			
            break;
        case KEY3:
            
			
			getcur();
            break;
			case KEY4:
            
			
			closing=1;
			fromselected=0;
		 	//ReadRecordSetting(0,"");
			testserverobl();
			closing=0;
			return;
            break;
			case KEY5:
            
			//printclosingbal();
			
			fromselected=0;
			return 0;
            break;
			case KEY6:
            
			
			
			
            break;
			case KEY7:
            
			
			
			
            break;
        
        case KEY9:
		
		kbFlush();
		fromselected=0;
			return 0;
           // sysExit(0);
           
            
            break;
        case KEY_CANCEL:
		kbFlush();
		fromselected=0;
			return 0;
            
            //sysExit(0);
            return ;
        default:
            break;
        }
		
}
}




void CommSetTest(void)
{
    U08 byRet, byCurPage;

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "    通讯测试    ", "   COMM test    ");
    lcdGoto(0, 2);
    byRet = LcdGetItemBigTimeout((U08*)g_abyCommTestEng, COMM_TEST_NUM, 0,
        LCD_MODE_1ITEM_PER_LINE|LCD_EXIT_MODE0|LCD_MODE_SHOW_USER_SELECT,
        RESPOND_TIMEOUT, &byCurPage);

    switch (byRet)
    {
    case 0:  // parameter error
        lcdGoto(0, 2);
        lcdPrintf("Fatal error!");
        while (1);
    case 1:
        GetIptest();
        break;
    case 2:
        GetPorttest();
        break;
    case 3:
		//testserver(myreceiptno);
        kbGetKeyMs(30000);
        break;
    case 4:
        g_oCommCfgParam.byComType = COMM_TYPE_CDMA;
        g_oCommCfgParam.byTcpipDevType = OPT_DEVWNET;
        break;
    case 5:
        g_oCommCfgParam.byComType = COMM_TYPE_PSTN_PPP;
        g_oCommCfgParam.byTcpipDevType = OPT_DEVMODEM;
        break;
    case 6:
        g_oCommCfgParam.byComType = COMM_TYPE_LAN;
        g_oCommCfgParam.byTcpipDevType = OPT_DEVETH;
        break;
    case 7:
        g_oCommCfgParam.byComType = COMM_TYPE_RS232;
        g_oCommCfgParam.byTcpipDevType = 0;
        break;
    case 0xff:
    case 0xfe:
        return ;
    default:
        GetKeyBlockMs(RESPOND_TIMEOUT);
        return ;
    }
}


#if SUPPORT_OPENSSL_CONNECT

#define METHOD_SSLV23            0
#define METHOD_SSLV2            1
#define METHOD_SSLV3            2
#define METHOD_TLSV1            3


static int theSSLMethod = METHOD_SSLV23;
static int theCertVerify = 0;
//static char theCAfile[64] = "server.crt";
static char theCAfile[64] = "SERVER.CRT";
BIO *bio_err = NULL;
BIO *bio_out = NULL;

static void ssl_print_error(void)
{
#if 1
    ulong err;
    const char  *file;
    int   line;

    while ((err = ERR_get_error_line(&file, &line)) != 0) {
//		BIO_printf(bio_err, "%s:%d:%s\n", file, line, ERR_error_string(err, NULL));
    }
#else
    ERR_print_errors(bio_err);
#endif
}

static SSL_METHOD *load_ssl_method(void)
{
    #define SSL_method(v)        v##_method()
//	#define SSL_method(v)		v##_client_method()
    SSL_METHOD *method;

    switch (theSSLMethod) {
        case METHOD_SSLV23:
            method = SSL_method(SSLv23);
            break;
        case METHOD_SSLV2:
            method = SSL_method(SSLv2);
            break;
        case METHOD_SSLV3:
            method = SSL_method(SSLv3);
            break;
        case METHOD_TLSV1:
            method = SSL_method(TLSv1);
            break;
        default:
            method = NULL;
            break;
    }

    return method;
}

static int verify_depth = 3;
static int verify_error = 0;
static int verify_callback(int ok, X509_STORE_CTX *ctx)
{
    char buf[256];
    X509 *err_cert;
    int err,depth;

    err_cert = X509_STORE_CTX_get_current_cert(ctx);
    err      = X509_STORE_CTX_get_error(ctx);
    depth    = X509_STORE_CTX_get_error_depth(ctx);

    X509_NAME_oneline(X509_get_subject_name(err_cert),buf,sizeof buf);
//	BIO_printf(bio_err, "depth=%d %s\n",depth,buf);
    if (!ok)
    {
//		BIO_printf(bio_err,"verify error:num=%d:%s\n",err,X509_verify_cert_error_string(err));
        if (verify_depth >= depth)
        {
            ok=1;
            verify_error=X509_V_OK;
        }
        else
        {
            ok=0;
            verify_error=X509_V_ERR_CERT_CHAIN_TOO_LONG;
        }
    }
    switch (ctx->error)
    {
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
        X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert),buf,sizeof buf);
//		BIO_printf(bio_err,"issuer= %s\n",buf);
        break;
    case X509_V_ERR_CERT_NOT_YET_VALID:
    case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
//		BIO_printf(bio_err,"notBefore=");
//		ASN1_TIME_print(bio_err,X509_get_notBefore(ctx->current_cert));
//		BIO_printf(bio_err,"\n");
        break;
    case X509_V_ERR_CERT_HAS_EXPIRED:
    case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
//		BIO_printf(bio_err,"notAfter=");
//		ASN1_TIME_print(bio_err,X509_get_notAfter(ctx->current_cert));
//		BIO_printf(bio_err,"\n");
        break;
    default:
        break;
    }

//	BIO_printf(bio_err,"verify return:%d\n",ok);
    return(ok);
}


int SslConnect(SSL_CTX **ctx, SSL **ssl)
{
    SSL_METHOD    *meth=NULL;
    X509        *server_cert;
    char        *str;
    int            skt = -1, err;

    skt = sg_Tcpsocket;
//	meth = load_ssl_method();
    OpenSSL_add_ssl_algorithms(); /*初始化*/
    SSL_load_error_strings();     /*为打印调试信息作准备*/

    meth = SSLv23_client_method(); /*采用什么协议(SSLv2/SSLv3/TLSv1)在此指定*/

    *ctx  = SSL_CTX_new(meth);
    if (!(*ctx)) {
        return 1;
    }
// 	if (theCertVerify)
    {
//		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
//		if (!SSL_CTX_load_verify_locations(ctx, theCAfile, NULL))
        {
//			ssl_print_error();
//			return 2;
        }
		/*if (!SSL_CTX_set_default_verify_paths(ctx)) // 8110在这里死机，6110 OK
	    {
			ssl_print_error();
			return 2;
		}*/
    }
    *ssl = SSL_new(*ctx);
    SSL_set_fd(*ssl, skt);
    err = SSL_connect(*ssl);
    if (err < 0)
    {
        ssl_print_error();
        return 3;
    }

	// Get server's certificate (note: beware of dynamic allocation) - opt
    server_cert = SSL_get_peer_certificate(*ssl);
    if (NULL != server_cert)
    {
        str = X509_NAME_oneline(X509_get_subject_name(server_cert),0,0);
        OPENSSL_free(str);
        str = X509_NAME_oneline(X509_get_issuer_name(server_cert),0,0);
        OPENSSL_free(str);

    	// We could do all sorts of certificate verification stuff here before
    	//   deallocating the certificate.
        X509_free(server_cert);
        return 0;
    }
    else
    {
        ssl_print_error();
        return 4;
    }
}

int SslTxd(SSL **ssl, const U08 *psTxdData, U32 uiDataLen)
{
    if (uiDataLen == SSL_write(*ssl, psTxdData, uiDataLen))
    {
        return 0;
    }
    return 1;
}

int SslRxd(SSL **ssl, U08 *psRxdData, U32 uiExpLen, U32 uiTimeOutMs, U32 *puiOutLen)
{
    int iRet, iCurLen;
    U32 uiOldTimeMs;

    uiOldTimeMs = sysGetTimerCount();
    iCurLen = 0;
    while (1)
    {
        if ((sysGetTimerCount()-uiOldTimeMs) > uiTimeOutMs)
        {
            return 1;
        }
        iRet = SSL_read(*ssl, psRxdData, uiExpLen);
        if (iRet > 0)
        {
            iCurLen += iRet;
            if (iCurLen >= uiExpLen)
            {
                *puiOutLen = uiExpLen;
                return 0;
            }
        }
    }
}

int SslClose(SSL_CTX **ctx, SSL **ssl)
{
    SSL_shutdown(*ssl);    // send SSL/TLS close_notify

    if (sg_Tcpsocket >= 0)
    {
        NetClose(sg_Tcpsocket);        // Clean up.
    }
    sg_Tcpsocket = -1;
    if (*ssl)
    {
        SSL_free(*ssl);
    }
    *ssl = NULL;
    if (*ctx)
    {
        SSL_CTX_free(*ctx);
    }
    *ctx = NULL;
    return 0;
}

#endif
























