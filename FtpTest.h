
#ifndef     _FTP_TEST_H_
#define     _FTP_TEST_H_


#ifndef U08
#define U08 unsigned char
#endif

#ifndef U16
#define U16  unsigned short
#endif

#ifndef U32
#define U32 unsigned int
#endif

#define bit(x)		(1 << (x))



struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};


void FtpTest(void);




#endif

