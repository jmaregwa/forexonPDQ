
#ifndef     _CARD_H_
#define     _CARD_H_

#define  DOUBLE_PIN_KEY_ID  6
#define  DOUBLE_MAC_KEY_ID  8
#define  MASTER_KEY1       16
#define  MASTER_KEY2       20
#define   KEK2_KEY         14
#define   PPK_KEY_ID       14


#define NO_SWIPE_INSERT        0x00    // û��ˢ��/�忨
#define CARD_SWIPED            0x01    // ˢ��
#define CARD_INSERTED        0x02    // �忨
#define CARD_KEYIN            0x04    // ���俨��
#define FALLBACK_SWIPED        0x08    // ˢ��(FALLBACK)
#define CARD_PASSIVE        0x20    // ��Ƶ����

///������2��״̬���ж�
#define  TRUE    1
#define  FALSE   0

#define  E_TRANS_CANCEL   1    //���ױ�ȡ��
#define  E_TRANS_FAIL     2    //����ʧ��
#define  E_NO_TRANS       3    // �޽���
#define  E_MAKE_PACKET    4    // �����
#define  E_ERR_CONNECT    5    //����ʧ��
#define  E_SEND_PACKET    6    //��������
#define  E_RECV_PACKET    7    //�հ�����
#define  E_RESOLVE_PACKET 8    //�������
#define  E_REVERSE_FAIL   9    //����ʧ��
#define  E_NO_OLD_TRANS   10   //��ԭʼ����
#define  E_TRANS_VOIDED   11   //�����ѱ�����
#define  E_ERR_SWIPE      12   //ˢ������
#define  E_MEM_ERR        13   //�ļ�����ʧ��
#define  E_PINPAD_KEY     14   //������̻�����Կ����
#define  E_TIP_AMOUNT      15   //С�ѽ���
#define  E_TIP_NO_OPEN      16   //С��û��
#define  E_TRANS_HAVE_ADJUESTED 17 //�����Ѿ�������
#define  E_FILE_OPEN      18   //���ļ���
#define  E_FILE_SEEK      19   //��λ�ļ���
#define  E_FILE_READ      20   //���ļ���
#define  E_FILE_WRITE      21   //д�ļ���
#define  E_CHECK_MAC_VALUE 22  //�հ�MACУ���
#define  NO_DISP          36


#define  E_NEED_FALLBACK    51 //��ҪFALLBACK
#define  E_NEED_INSERT        52 // ��Ҫ�忨����

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

    COMM_CFG_PARAM stTxnCommCfg; // �ն�ͨѶ���� ����
}POS_PARAM_STRC;


#endif
