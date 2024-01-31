#ifndef _MACRO_DEFINE_H_
#define _MACRO_DEFINE_H_

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef U08
#define U08 unsigned char
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef U32
#define U32 unsigned int
#endif

#ifndef I08
#define I08 char
#endif

#ifndef I16
#define I16 short
#endif

#ifndef I32
#define I32 int
#endif


#define BIT(x)  (1<<(x))

#define NUM_MAXZHCHARS            16          // 每行最多字符数目(大字体)
#define NUM_MAXCOLS               128         // 每行最多列数

#define WAIT_TIME_OUT             10000

//#define RESPOND_TIMEOUT           120000  //120s
#define RESPOND_TIMEOUT           1200000  //1200s

#define USER_OPER_TIMEOUT         60000        // 用户超时设置

#define WNET_COMM_INIT_TIMEOUT    60000        // 60 seconds
#define WIRELESS_INITIAL_TIME     60000

#define PAGE_ITEM_NUM  3
#define LCD_HALF_WIDTH    64

#define LCD_HIGH_MINI  8
#define FILE_NAME_MAX_LEN  17


// 拨号方式
#define PREDIAL_MODE              0        // 预拨号/预连接
#define ACTDIAL_MODE              1        // 实际拨号/连接

// 发送数据方式
#define CM_RAW                    0        // 原数据发送方式(不添加起止字符,For RS232)
#define CM_SYNC                   1        // 同步
#define CM_ASYNC                  2        // 异步

#define SUPPORT_OPENSSL_RSA       0

#define SUPPORT_OPENSSL_CONNECT   1
#define SUPPORT_GPRS_ALWAYS_LINK  0  // 1:GPRS Always online； 0:When transaction finish, the application disconnect the GPRS link


#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

#endif

