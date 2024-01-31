
#ifndef     _CARD_H_
#define     _CARD_H_

#define  DOUBLE_PIN_KEY_ID  6
#define  DOUBLE_MAC_KEY_ID  8
#define  MASTER_KEY1       16
#define  MASTER_KEY2       20
#define   KEK2_KEY         14
#define   PPK_KEY_ID       14


#define NO_SWIPE_INSERT        0x00    // 没有刷卡/插卡
#define CARD_SWIPED            0x01    // 刷卡
#define CARD_INSERTED        0x02    // 插卡
#define CARD_KEYIN            0x04    // 手输卡号
#define FALLBACK_SWIPED        0x08    // 刷卡(FALLBACK)
#define CARD_PASSIVE        0x20    // 射频卡打卡

///适用于2种状态的判断
#define  TRUE    1
#define  FALSE   0

#define  E_TRANS_CANCEL   1    //交易被取消
#define  E_TRANS_FAIL     2    //交易失败
#define  E_NO_TRANS       3    // 无交易
#define  E_MAKE_PACKET    4    // 打包错
#define  E_ERR_CONNECT    5    //联接失败
#define  E_SEND_PACKET    6    //发包错误
#define  E_RECV_PACKET    7    //收包错误
#define  E_RESOLVE_PACKET 8    //解包错误
#define  E_REVERSE_FAIL   9    //冲正失败
#define  E_NO_OLD_TRANS   10   //无原始交易
#define  E_TRANS_VOIDED   11   //交易已被撤销
#define  E_ERR_SWIPE      12   //刷卡错误
#define  E_MEM_ERR        13   //文件操作失败
#define  E_PINPAD_KEY     14   //密码键盘或者密钥出错
#define  E_TIP_AMOUNT      15   //小费金额超限
#define  E_TIP_NO_OPEN      16   //小费没开
#define  E_TRANS_HAVE_ADJUESTED 17 //交易已经被调整
#define  E_FILE_OPEN      18   //打开文件错
#define  E_FILE_SEEK      19   //定位文件错
#define  E_FILE_READ      20   //读文件错
#define  E_FILE_WRITE      21   //写文件错
#define  E_CHECK_MAC_VALUE 22  //收包MAC校验错
#define  NO_DISP          36


#define  E_NEED_FALLBACK    51 //需要FALLBACK
#define  E_NEED_INSERT        52 // 需要插卡交易

#define ICC_USER    0

#define LEN_PAN                    19


#define  PARAM_OPEN        '1'
#define  PARAM_CLOSE    '0'

#define  TRACK1_LEN        79
#define  TRACK2_LEN        40
#define  TRACK3_LEN        107

#define TRAN_AMOUNT        0
#define TIP_AMOUNT         1
#define ADJUST_AMOUNT      2
#define IC_AMOUNT          3
#define TOPUP_AMOUNT       4


#define FILE_APP_LOG    "SysParam.log"


typedef struct _NEWPOS_PARAM_STRC {
    U08 szUnitChnName[41];
    U08 szEngName[41];
    U08 ucSwipedFlag;
    U08 szTrack1[TRACK1_LEN+1];
    U08 szTrack2[TRACK2_LEN+1];
    U08 szTrack3[TRACK3_LEN+1];
    U08 szCardNo[21];
    U08 szExpDate[5];
    U08 szPin[16];
    U08 szTermID[9+1];

    int iOprtLimitTime;
    int iAmount;

    COMM_CFG_PARAM stTxnCommCfg; // 终端通讯配置 －－
}POS_PARAM_STRC;


#endif
