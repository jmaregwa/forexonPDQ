
#include <time.h>
#include "posapi.h"
#include "ftp2.h"
#include "Lcd.h"
#include "MacroDefine.h"
#include "Comm.h"
#include "FtpTest.h"
#include "FunctionList.h"

extern COMM_CFG_PARAM g_oCommCfgParam;

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines were long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */

int InitFtpParamter(FTP_PARAM *pFtpParam)
{
    // ftp initial
    strcpy((char*)pFtpParam->strIpAddress, "192.168.1.139");  // for ftp server
    strcpy((char*)pFtpParam->strPortNum, "9000");  // for ftp server
    strcpy((char*)pFtpParam->strUserName, "ryan");  // for ftp server
    strcpy((char*)pFtpParam->strPassword, "ryan");  // for ftp server
    strcpy((char*)pFtpParam->strDownRemoteFile, "/a/NewTMS_release_20110415.rar");  // for ftp server
    strcpy((char*)pFtpParam->strDownLocalFile, "DLoc1");  // for ftp server
    strcpy((char*)pFtpParam->strUpRemoteFile, "/b/a001.rar");  // for ftp server
 //   strcpy((char*)pFtpParam->strUpLocalFile, "ULoc1");  // for ftp server
    strcpy((char*)pFtpParam->strUpLocalFile, "DLoc1");  // for ftp server
    return 0;
}

 static U32 maketime(U32 year0, U32 mon0, U32 day, U32 hour, U32 min, U32 sec)
{
	U32 mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((U32)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}
/*
 * Convert Gregorian date to seconds since 01-01-1970 00:00:00.
 */
static int rtc_tm_to_time(struct rtc_time *tm, U32 *time)
{
	*time = maketime(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	return 0;
}

static void GetCurTime(struct rtc_time *tm)
{
	#define BCD2NUM(x)	(((x) & 0x0F) + (((x) >> 4) * 10))
	U08 buf[8];

	sysGetTime(buf);

	tm->tm_year = BCD2NUM(buf[0]) + 100;
	tm->tm_mon  = BCD2NUM(buf[1]) - 1;
	tm->tm_mday = BCD2NUM(buf[2]);
	tm->tm_hour = BCD2NUM(buf[3]);
	tm->tm_min  = BCD2NUM(buf[4]);
	tm->tm_sec  = BCD2NUM(buf[5]);
}

time_t Time(time_t *t)
{
	struct rtc_time rt;
	U32 time;

	GetCurTime(&rt);
	rtc_tm_to_time(&rt, &time);

	if (t)
		*t = time;

	return time;
}

static const U08 rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

#define SYS_LEAPS_THRU_END_OF(y)	((y)/4 - (y)/100 + (y)/400)
#define SYS_LEAP_YEAR(year)			((!(year % 4) && (year % 100)) || !(year % 400))

static int rtc_month_days(U32 month, U32 year)
{
	return rtc_days_in_month[month] + (SYS_LEAP_YEAR(year) && month == 1);
}

/*
 * Convert seconds since 01-01-1970 00:00:00 to Gregorian date.
 */
static void rtc_time_to_tm(U32 time, struct rtc_time *tm)
{
	register int days, month, year;

	days = time / 86400;
	time -= days * 86400;

	/* day of the week, 1970-01-01 was a Thursday */
	tm->tm_wday = (days + 4) % 7;

	year = 1970 + days / 365;
	days -= (year - 1970) * 365
		+ SYS_LEAPS_THRU_END_OF(year - 1)
		- SYS_LEAPS_THRU_END_OF(1970 - 1);
	if (days < 0) {
		year -= 1;
		days += 365 + SYS_LEAP_YEAR(year);
	}
	tm->tm_year = year - 1900;
	tm->tm_yday = days + 1;

	for (month = 0; month < 11; month++) {
		int newdays;

		newdays = days - rtc_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	tm->tm_mon = month;
	tm->tm_mday = days + 1;

	tm->tm_hour = time / 3600;
	time -= tm->tm_hour * 3600;
	tm->tm_min = time / 60;
	tm->tm_sec = time - tm->tm_min * 60;
	tm->tm_isdst = 0;
}


struct tm *GmTime(const time_t *timer)
{
	static struct tm tms;
	struct rtc_time rtctm;

	rtc_time_to_tm(*timer, &rtctm);
	tms.tm_sec	= rtctm.tm_sec;
	tms.tm_min	= rtctm.tm_min;
	tms.tm_hour	= rtctm.tm_hour;
	tms.tm_mday = rtctm.tm_mday;
	tms.tm_mon	= rtctm.tm_mon;
	tms.tm_year	= rtctm.tm_year;
	tms.tm_wday	= rtctm.tm_wday;
	tms.tm_yday	= rtctm.tm_yday;
	tms.tm_isdst= rtctm.tm_isdst;
	tms.tm_gmtoff = 0;
	tms.tm_zone = "";

	return &tms;
}


static int gs_iExitFlag = 0;

ftp_host_info_t g_oFtpHostInfo;

static int ftp_read_data_upload(void *arg, void *data, loff_t offset, int count)
{
	store_t *store = (store_t *)arg;
	time_t t2 = Time(NULL);
	time_t t3;
	struct tm t;
	int fd = store->fd;
	int retval;

	if (kbhit() == YES) {
		if (kbGetKey() == KEY_CANCEL) {
			return KEY_CANCEL;
		}
	}

	t3 = t2 - store->ts;
	t = *GmTime(&t3);

	fileSeek(fd, offset, SEEK_SET);
	if ((retval = fileRead(fd, data, count)) != count) {
		lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "O=%ld N=%d", offset, count);
 		lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "ReadErr:%d", retval);
		kbGetKeyMs(3000);
		return retval;
	}

	store->offset = offset + count;
	lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "UPLOAD %d%% %d/%d", store->offset * 100 / store->size,
				store->offset, store->size);
	lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "%02d:%02d:%02d %4dBps", t.tm_hour, t.tm_min, t.tm_sec,
				(int)(store->offset / t3));
	return 0;
}

static int do_ftp_login(int argc, char *argv[])
{
	int retval;
	int port = strtol(g_oCommCfgParam.oFtp.strPortNum, NULL, 10);

	strcpy(g_oFtpHostInfo.user, g_oCommCfgParam.oFtp.strUserName);
	strcpy(g_oFtpHostInfo.password, g_oCommCfgParam.oFtp.strPassword);

    retval = ftp_login2(&g_oFtpHostInfo, g_oCommCfgParam.oFtp.strIpAddress, port);
	if (0 != retval)
	{
        return retval;
	}

	return 0;
}

static int do_ftp_quit(int argc, char *argv[])
{
	ftp_logout2(&g_oFtpHostInfo);

	return 0;
}

static int FtpUploadTest(int argc, char *argv[])
{
    int iRet;
	store_t store;
	int	count;


    lcdCls();
    lcdDisplayCE(0, 0, 0x01, "    拨号...     ", "  Comm dial...  ");
    iRet = CommDial(ACTDIAL_MODE);
    if (0 != iRet)
    {
        lcdCls();
        lcdDisplayCE(0, 0, 0x01, "  拨号失败      ", " CommDial error ");
        lcdDisplay(0, 2, 0x01, "iRet=%d,", iRet);
        kbGetKeyMs(WAIT_TIME_OUT);
        return 1;
    }

    lcdCls();
	middle_print(0,  0, "FTP UPLOAD      ");
	lcdDisplay(0, 1, 0, "----------------");
	lcdDisplay(0, 2, 0, "LOGIN...        ");
	lcdDisplay(0, 3, 0, "%s:%s", g_oCommCfgParam.oFtp.strIpAddress,
        g_oCommCfgParam.oFtp.strPortNum);

	iRet = do_ftp_login(1, NULL);
	if (0 != iRet)
    {
		lcdDisplay(0, 2, 0, "LogErr:%d", iRet);
		goto end_ftpput;
	}

	lcdClrLine(1, 7);
	lcdDisplay(0, 1, 0, "Login Succ      ");
	lcdDisplay(0, 2, 0, "Uploading...    ");
	lcdDisplay(0, 3, 0, "Get FSize...    ");

	iRet = ftp_size2(&g_oFtpHostInfo, g_oCommCfgParam.oFtp.strUpRemoteFile, &count);
	if (0 == iRet)
	{
        lcdDisplay(0, 3, 0, "SIZE: %d", count);
	}
	else
	{
		lcdDisplay(0, 3, 0, "Unknown Size1");
	    lcdDisplay(0, 4, 0, g_oFtpHostInfo.err_info);
        count = 0;
	}

	lcdDisplay(0, 4, 0, "Loading...");

	strcpy(store.filename, g_oCommCfgParam.oFtp.strUpLocalFile);
	store.offset = count;
	store.ts = Time(NULL);
	store.fd = fileOpen(store.filename, O_RDWR);
	if (store.fd >= 0)
    {
		store.size = fileSeek(store.fd, 0, SEEK_END);
		fileSeek(store.fd, 0, SEEK_SET);
		iRet = ftp_put2(&g_oFtpHostInfo, g_oCommCfgParam.oFtp.strUpRemoteFile,
            store.size, ftp_read_data_upload, &store);
		fileClose(store.fd);

		if (0 == iRet)
	    {
			lcdDisplay(0, 3, 0, "Upload Succ");
	    }
		else
	    {
			lcdDisplay(0, 3, 0, "UpLoadErr:%d", iRet);
			lcdDisplay(0, 4, 0, g_oFtpHostInfo.err_info);
	    }

		lcdDisplay(0, 4, 0, "QUIT SRV...");

		if (0 != iRet)
			kbGetKeyMs(3000);
	}
    else
    {
        lcdCls();
		lcdDisplay(0, 0, 0, "as001");
		lcdDisplay(0, 2, 0, "%s", store.filename);
		lcdDisplay(0, 3, 0, "OpenErr:%d", store.fd);
	}

	do_ftp_quit(1, NULL);

end_ftpput:
    iRet = CommOnHook(TRUE);
    if (0 != iRet)
    {
        kbGetKeyMs(WAIT_TIME_OUT);
        lcdCls();
        lcdDisplayCE(0, 0, 0x01, "  挂机失败      ", "CommOnHook error");
        lcdDisplay(0, 6, 0x01, "iRet=%d,", iRet);
        return 1;
    }
    return 0;
}


static int ftp_store_data(void *arg, const void *data, int count)
{
	store_t *store = (store_t *)arg;
	time_t t2 = Time(NULL);
	time_t t3;
	struct tm t;

	if (kbhit() == YES) {
		if (kbGetKey() == KEY_CANCEL)
			return KEY_CANCEL;
	}
    if ((store->offset+count) <= store->size)
    {
        fileSeek(store->fd, store->offset, SEEK_SET);
        fileWrite(store->fd, (unsigned char*)data, count);
    }
	t3 = t2-store->ts;
	t = *GmTime(&t3);

	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "%d%% %d/%d", store->offset * 100 / store->size, store->offset, store->size);
	lcdDisplay(0, 5, DISP_ASCII|DISP_CLRLINE, "%02d:%02d:%02d %4dBps",
        t.tm_hour, t.tm_min, t.tm_sec, (int)((store->offset-store->StartOffset) / t3));

	return 0;
}

static int FtpDownloadTest(int argc, char *argv[])
{
	int count, iRet;
	store_t store;
	int retval;

    lcdCls();
    lcdDisplayCE(0, 0, 0x01, "    拨号...     ", "  Comm dial...  ");
    iRet = CommDial(ACTDIAL_MODE);
    if (0 != iRet)
    {
        lcdCls();
        lcdDisplayCE(0, 0, 0x01, "  拨号失败      ", " CommDial error ");
        lcdDisplay(0, 2, 0x01, "iRet=%d,", iRet);
        kbGetKeyMs(WAIT_TIME_OUT);
        return 1;
    }

	lcdCls();
	lcdDisplayCE(0, 0, DISP_CFONT|DISP_CLRLINE|DISP_INVLINE|DISP_MEDIACY,
        "  FTP DOWNLOAD  ", "    FTP下载     ");
	lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "---------------------");
	lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "FTP login...");
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "%s:%s",
        g_oCommCfgParam.oFtp.strIpAddress, g_oCommCfgParam.oFtp.strPortNum);

	retval = do_ftp_login(1, NULL);
	if (retval) {
		lcdDisplay(0, 5, DISP_ASCII|DISP_CLRLINE, "LogErr:%d", retval);
        sysDelayMs(1000);
		goto end_ftpget;
	}

	lcdClrLine(3, 7);
	lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "Get FSize...");

	retval = ftp_size2(&g_oFtpHostInfo, g_oCommCfgParam.oFtp.strDownRemoteFile, &count);
	if (!retval)
		lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "SIZE: %d", count);
	else
		lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "Unknown Size");

	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "Loading...");

	strcpy(store.filename, g_oCommCfgParam.oFtp.strDownRemoteFile);
	store.offset = 0;
	store.size = count;
	store.ts = Time(NULL);
    store.fd = fileOpen(g_oCommCfgParam.oFtp.strDownLocalFile, O_CREAT|O_RDWR);
    if (store.fd < 0)
    {
        lcdDisplay(0, 5, DISP_ASCII|DISP_CLRLINE, "Create file error1");
        sysDelayMs(1000);
		goto end_ftpget;
    }
    store.StartOffset = fileSize(g_oCommCfgParam.oFtp.strDownLocalFile);
    store.offset = store.StartOffset;
	lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "Start offset=%d...", store.StartOffset);
	retval = ftp_get2(&g_oFtpHostInfo, g_oCommCfgParam.oFtp.strDownRemoteFile,
        ftp_store_data, &store);
	if (!retval)
		lcdDisplay(0, 6, DISP_ASCII|DISP_CLRLINE, "Load Success");
	else
		lcdDisplay(0, 6, DISP_ASCII|DISP_CLRLINE, "LoadErr:%d", retval);
    fileClose(store.fd);
	lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "QUIT FTP...");

	do_ftp_quit(1, NULL);

end_ftpget:

//	lcdDisplay(0, 7, DISP_ASCII|DISP_CLRLINE, "PPP LOGOUT...");
    iRet = CommOnHook(TRUE);
    if (0 != iRet)
    {
        kbGetKeyMs(WAIT_TIME_OUT);
        lcdCls();
        lcdDisplayCE(0, 0, 0x01, "  挂机失败      ", "CommOnHook error");
        lcdDisplay(0, 6, 0x01, "iRet=%d,", iRet);
        return 1;
    }
    return 0;
}


// 输入端口
// strPortNo[in & out]:端口号
// 返回0表示成功，返回非0表示取消
static int s_GetFtpPort(char *strPortNo)
{
    int iTemp, iRet;
    U08 abyTemp[10];

    strPortNo[5] = 0;
    strcpy((char*)abyTemp, strPortNo);
    while (1)
    {
        lcdGoto(0, 6);
        iRet = kbGetString(0xe5, 1, 5, USER_OPER_TIMEOUT, abyTemp);
        if (KB_CANCEL == iRet)
        {
            return ERR_USERCANCEL;
        }
        iTemp = atoi((char *)abyTemp);
        if ((iTemp>0) && (iTemp<65535))
        {
            strcpy(strPortNo, (char*)abyTemp);
            return 0;
        }

        Display2Strings("无效端口号", "INV PORT #");
        PubBeepErr();
        kbGetKeyMs(4000);
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, 0x01, "     端口号     ", "  Port Number   ");
    }
}


static int s_SetFtpUserPassword(U08 *pbyUID, U08 *pbyPwd)
{
    int iRet;

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 3, DISP_CFONT, "ftp登陆用户名   ", "FTP LOGIN NAME  ");
    lcdGoto(0, 6);
    iRet = kbGetString(0xf5, 0, 32, USER_OPER_TIMEOUT, pbyUID);
    if (KB_CANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 3, DISP_CFONT, "ftp登陆密码     ", "FTP LOGIN PWD   ");
    lcdGoto(0, 6);
    iRet = kbGetString(0xf5, 0, 16, USER_OPER_TIMEOUT, pbyPwd);
    if (KB_CANCEL == iRet)
    {
        return ERR_USERCANCEL;
    }
    return 0;
}


int SetFtpParameter(int argc, char *argv[])
{
    U08 abyTemp[256];
    int iRet, iFlag = 0;

    lcdCls();
    lcdDisplayCE(0, 0, 0x81, "  修改ftp设置   ", "Set ftp cfg     ");
    lcdDisplayCE(0, 2, 0x01, "     IP地址     ", "  IP Address    ");

    g_oCommCfgParam.oFtp.strIpAddress[15] = 0;
    strcpy((char*)abyTemp, g_oCommCfgParam.oFtp.strIpAddress);
    lcdDisplay(0, 4, DISP_CFONT, abyTemp);
    iRet = GetIPAddress(FALSE, abyTemp, NULL);
    if ((0==iRet) && strcmp(g_oCommCfgParam.oFtp.strIpAddress, abyTemp))
    {
        iFlag = 1;
        strcpy(g_oCommCfgParam.oFtp.strIpAddress, abyTemp);
    }

    lcdClrLine(2, 7);
    lcdDisplayCE(0, 2, 0x01, "     端口号     ", "  Port Number   ");
    g_oCommCfgParam.oFtp.strPortNum[7] = 0;
    strcpy((char*)abyTemp, g_oCommCfgParam.oFtp.strPortNum);
    lcdDisplay(64, 4, DISP_CFONT, abyTemp);
    iRet = s_GetFtpPort((char*)abyTemp);
    if ((0==iRet) && strcmp(g_oCommCfgParam.oFtp.strPortNum,(char*)abyTemp))
    {
        iFlag = 1;
        strcpy(g_oCommCfgParam.oFtp.strPortNum, (char*)abyTemp);
    }

    // Set ftp login name and login password
    g_oCommCfgParam.oFtp.strUserName[31] = 0;
    strcpy((char*)abyTemp, g_oCommCfgParam.oFtp.strUserName);
    g_oCommCfgParam.oFtp.strPassword[31] = 0;
    strcpy((char*)&abyTemp[100], g_oCommCfgParam.oFtp.strPassword);
    iRet = s_SetFtpUserPassword(abyTemp, &abyTemp[100]);
    if ((0==iRet) &&
        (strcmp(g_oCommCfgParam.oFtp.strUserName,(char*)abyTemp)
        ||(strcmp(g_oCommCfgParam.oFtp.strPassword,(char*)&abyTemp[100]))))
    {
        iFlag = 1;
        strcpy(g_oCommCfgParam.oFtp.strUserName, (char*)abyTemp);
        strcpy(g_oCommCfgParam.oFtp.strPassword, (char*)&abyTemp[100]);
    }


    if (0 != iFlag)
    {
        iRet = SaveCommCfg(&g_oCommCfgParam);
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, 0x01, "  保存...       ", "Save...         ");
        if (0 == iRet)
        {
            lcdDisplayCE(0, 4, 0x01, "    保存成功    ", "    Save OK!    ");
        }
        else
        {
            lcdDisplayCE(0, 4, 0x01, "    保存失败    ", "  Save failed   ");
        }
    }
    else
    {
        lcdClrLine(2, 7);
        lcdDisplayCE(0, 2, 0x01, "  配置未改变    ", "Cfg not change  ");
        lcdDisplayCE(0, 4, 0x01, "  按任意键退出  ", "Press any key   ");
        lcdDisplayCE(0, 6, 0x01, "                ", " and exit       ");
        kbGetKeyMs(5000);
        lcdClrLine(2, 7);
    }
    return 0;
}


int CheckFileHash(int argc, char *argv[])
{
    //gDefaultFtpPara.localfile
    int fd, iFileSize, iRet, iOffset;
    U08 abyBuff[2048];
    SHA_INFO oShaInfo;

    fd = fileOpen(g_oCommCfgParam.oFtp.strDownLocalFile, O_RDWR);
    if (fd < 0)
    {
        lcdDisplay(0, 5, DISP_ASCII|DISP_CLRLINE, "File not exist");
        kbGetKey();
        return 0;
    }
    iFileSize = fileSize(g_oCommCfgParam.oFtp.strDownLocalFile);
    fileSeek(fd, 0, SEEK_SET);
    sha_init(&oShaInfo);
    iOffset = 0;
    while (1)
    {
        iRet = fileRead(fd, abyBuff, sizeof(abyBuff));
        if (iRet > 0)
        {
            sha_update(&oShaInfo, abyBuff, iRet);
            iOffset += iRet;
        }
        else
        {
            break;
        }
    }
    sha_final(abyBuff, &oShaInfo);
    lcdCls();
    if (iOffset > 0)
    {
        lcdDisplay(0, 0, 0, "Hash result:");
        lcdDisplay(0, 1, 0, "%02x %02x %02x %02x %02x %02x ", abyBuff[0], abyBuff[1],
            abyBuff[2], abyBuff[3], abyBuff[4], abyBuff[5]);
        lcdDisplay(0, 2, 0, "%02x %02x %02x %02x %02x %02x ", abyBuff[6], abyBuff[7],
            abyBuff[8], abyBuff[9], abyBuff[10], abyBuff[11]);
        lcdDisplay(0, 3, 0, "%02x %02x %02x %02x %02x %02x ", abyBuff[12], abyBuff[13],
            abyBuff[14], abyBuff[15], abyBuff[16], abyBuff[17]);
        lcdDisplay(0, 4, 0, "%02x %02x", abyBuff[18], abyBuff[19]);
    }
    else
    {
        lcdDisplay(0, 0, 0, "File size = 0");
    }
    kbGetKey();
    return 0;
}

static int ExitFunc(int argc, char *argv[])
{
	gs_iExitFlag = 1;
	return 0;
}

static MENU_ITEM theMenuItem[] = {
	{"SYS CONFIG",	    SetFtpParameter},
	{"DOWNLOAD FILE",	FtpDownloadTest},
	{"UPLOAD FILE",	    FtpUploadTest},
	{"Check File Hash",	CheckFileHash},
	{"QUIT",	        ExitFunc},
};

static  MENU_SELECT theMainMenu = {
	.strTitle = "FTP TEST",
	.iNrPrescreen	= 3,
	.iStart			= 0,
	.iCurrent	= 0,
	.psItem			= theMenuItem,
	.iItemNumber = ARRAY_SIZE(theMenuItem),
};

void FtpTest(void)
{
	int key;
	int (*func)(int argc, char *argv[]);

	gs_iExitFlag = 0;
	while (0 == gs_iExitFlag)
    {
		key = MenuSelect(&theMainMenu);
		if (KEY_ENTER == key)
        {
			func = theMainMenu.psItem[theMainMenu.iCurrent].func;
			if (NULL != func)
			{
                func(1, NULL);
			}
		}
		else if (KEY_CANCEL == key)
        {
			break;
		}
	}
}


