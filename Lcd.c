
#include "Posapi.h"
#include "Lcd.h"
#include "MacroDefine.h"
#include "FunctionList.h"

extern int g_iLangueSelect;


/****************************************************************************
  ������     :  U08 s_LcdShowItem(U08 *pbyItemString, U08 byItemNumber,
                             U08 byShowPage)
  ����       :  ��ʾ��Ϣ��
  �������   :  1��U08 *pbyItemString : ��Ϣ�������ݣ���17���ֽ�Ϊһ����Ϣ��ÿ
                   ����Ϣ�����һ���ֽڱ�������0��pbyItemString���ڴ�ռ䲻С��
                   byItemNumber*17��
                2��U08 byItemNumber : Ҫ��ʾ����Ϣ������Ϣ������ȡֵ1~253
                3��U08 byShowPage : ��ʾ�ڼ�ҳ����Ϣ��ȡֵ��0��ʼ����ҳ��-1
  �������   :  ��
  ����ֵ     :  ����1��ʾ����������󣬷���0��ʾ���������ȷ������Ϣ����ʾ������
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2010-05-18  V1.0         ����
****************************************************************************/
static U08 s_LcdShowItem(const U08 *pbyItemString, U08 byItemNumber, U08 byShowPage)
{
    U08 i, byPageNum;

    if ((byItemNumber>253) || (0==byItemNumber) || (NULL==pbyItemString))
    {
        return 1;
    }
    byPageNum = (byItemNumber+PAGE_ITEM_NUM-1)/PAGE_ITEM_NUM;
    if (byPageNum <= byShowPage)
    {
        return 1;
    }
/*    for (i=0; i<byItemNumber; i++)
    {
        pbyItemString[i*FILE_NAME_MAX_LEN+16] = 0x00;
    }*/
    lcdClrLine(2, LCD_HIGH_MINI-1);
    if (g_iLangueSelect)
    {
//        lcdDisplay(72, 0, 0x81, "%02d/%02dҳ", byShowPage+1, byPageNum);
        lcdDisplay(88, 0, 0x81, "%02d/%02d", byShowPage+1, byPageNum);
    }
    else
    {
        lcdDisplay(72, 0, 0x81, "Pg%02d/%02d", byShowPage+1, byPageNum);
    }
    for (i=1; i<=PAGE_ITEM_NUM; i++)
    {
        if ((byShowPage*PAGE_ITEM_NUM+i) <= byItemNumber)
        {
            lcdDisplay(0, i*2, 0x01, "%d.%s", i,
                &pbyItemString[(byShowPage*PAGE_ITEM_NUM+i-1)*FILE_NAME_MAX_LEN]);
        }
    }
    return 0;
}

static U08 s_LcdShowSmallItem1(const U08 *pbyItemString, U08 byItemNumber, U08 byCurItem,
            U08 byShowPage, U08 byMode)
{
    U08 i, byPageNum, byItemNumPerPage, byItem;

    if ((byItemNumber>253) || (0==byItemNumber) || (NULL==pbyItemString))
    {
        return 1;
    }
    byItemNumPerPage = PAGE_ITEM_NUM*2+1;
    byPageNum = (byItemNumber+byItemNumPerPage-1)/byItemNumPerPage;
    if (byPageNum <= byShowPage)
    {
        return 1;
    }
    lcdClrLine(1, LCD_HIGH_MINI-1);
    if (0 != (byMode&0x08))
    {
        lcdDisplay(84, 0, 0x80, "Pg%02d/%02d", byShowPage+1, byPageNum);
    }

    for (i=1; i<=byItemNumPerPage; i++)
    {
        byItem = byShowPage*byItemNumPerPage+i;
        if (byItem <= byItemNumber)
        {
            if (byCurItem == byItem)
            {
                lcdDisplay(0, i, 0x00, "%d>%s", i,
                    &pbyItemString[(byItem-1)*ITEM_LENGTH]);
            }
            else
            {
                lcdDisplay(0, i, 0x00, "%d.%s", i,
                    &pbyItemString[(byItem-1)*ITEM_LENGTH]);
            }
        }
    }
    return 0;
}

static U08 s_LcdShowBigItem(const U08 *pbyItemString, U08 byItemNumber, U08 byCurItem,
        U08 byShowPage, U08 byMode)
{
    U08 i, byPageNum, byItemNumPerPage, byItem, abyShowNum[30];
    int iCol, iLine;

    if ((byItemNumber>253) || (0==byItemNumber) || (NULL==pbyItemString))
    {
        return 1;
    }
    if (0 != (byMode&LCD_MODE_2ITEM_PER_LINE))
    {
        byItemNumPerPage = PAGE_ITEM_NUM*2;
    }
    else
    {
        byItemNumPerPage = PAGE_ITEM_NUM;
    }
    byPageNum = (byItemNumber+byItemNumPerPage-1)/byItemNumPerPage;
    if (byPageNum <= byShowPage)
    {
        return 1;
    }
    lcdClrLine(2, LCD_HIGH_MINI-1);
    if (0 != (byMode&0x08))
    {
        lcdDisplay(88, 0, 0x81, " P%d/%d", byShowPage+1, byPageNum);
    }

    for (i=1; i<=byItemNumPerPage; i++)
    {
        byItem = byShowPage*byItemNumPerPage+i;
        if (byItem <= byItemNumber)
        {
            if (byCurItem == byItem)
            {
                sprintf(abyShowNum, "%d>%s", i, &pbyItemString[(byItem-1)*ITEM_LENGTH]);
            }
            else
            {
                sprintf(abyShowNum, "%d.%s", i, &pbyItemString[(byItem-1)*ITEM_LENGTH]);
            }
            iLine = i*2;
            iCol = 0;
            if (0 != (byMode&LCD_MODE_2ITEM_PER_LINE))
            {
                if (0 == (i&0x01))
                {
                    iCol = 64;
                }
                iLine = (i+1)&0xfe;
            }
            lcdDisplay(iCol, iLine, 0x01, "%s", abyShowNum);
        }
    }
    return 0;
}

static void s_LcdShowBigUserSelect(const U08 *pbyItemString, U08 bySelectItem)
{
    lcdClrLine(2, LCD_HIGH_MINI-1);
    lcdDisplayCE(0, 2, 0x01, "�û�ѡ��:", "Your select:");
    lcdDisplay(0, 5, 0x81, "                ");
    lcdDisplay(0, 5, 0x81, (U08*)&pbyItemString[(bySelectItem-1)*ITEM_LENGTH]);
    GetKeyBlockMs(3000);
}

static void s_LcdShowUserSelect(const U08 *pbyMsg1, const U08 *pbyMsg2, int iCurMsg, U08 byMode)
{
    if (0 == (byMode&LCD_MODE_SHOW_USER_SELECT))
    {
        return ;
    }
    if (0 == (byMode&LCD_MODE_2ITEM_PER_LINE))
    {
        lcdClrLine(2, LCD_HIGH_MINI-1);
    }
    else
    {
        lcdClrLine(4, LCD_HIGH_MINI-1);
    }
    lcdDisplayCE(0, 4, 0x01, "�û�ѡ��:", "Your select:");
    lcdDisplay(0, 6, 0x01, "                ");
    if (1 == iCurMsg)
    {
        lcdDisplay(40, 6, 0x01, (U08*)pbyMsg1);
    }
    else
    {
        lcdDisplay(40, 6, 0x01, (U08*)pbyMsg2);
    }
    GetKeyBlockMs(3000);
}

void lcdDisplayCE(U08 col, U08 row, U08 mode, const char *pCHN , const char *pEN)
{
    if (1 == g_iLangueSelect)
    {
        lcdDisplay(col, row, mode, (U08*)pCHN);
    }
    else
    {
        lcdDisplay(col, row, mode, (U08*)pEN);
    }
}


/****************************************************************************
  ������     :  U08 LcdGetItemTimeout(const U08 *pbyItemString,
                       U08 byItemNumber, U08 byExitMode, int iTimeout100ms)
  ����       :  ��ʾ��Ϣ���������û�ѡ����Ϣ��
  �������   :  1��U08 *pbyItemString : ��Ϣ�������ݣ���17���ֽ�Ϊһ����Ϣ��ÿ
                   ����Ϣ�����һ���ֽڱ�������0��pbyItemString���ڴ�ռ䲻С��
                   byItemNumber*17��
                2��U08 byItemNumber : Ҫ��ʾ����Ϣ������Ϣ������ȡֵ1~253
                3��U08 byExitMode : �˳�ģʽ��1��ʾ��'1'��'2'��'3'��'Cancel'��
                   �˳���modem=2��ʾֻ�а�'Cancel'�����˳�
                4��int iTimeoutMs : ��ʱʱ�䣬��λms
  �������   :  ��
  ����ֵ     :  ����0��ʾ����������󣬷���255��ʾ�û�����ȡ����������1~253��ʾ�û�
                ѡ�����Ϣ�����ţ�����254��ʾ��ʱ�˳���
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2010-05-18  V1.0         ����
****************************************************************************/
U08 LcdGetItemTimeout(const U08 *pbyItemString, U08 byItemNumber,
                U08 byExitMode, int iTimeoutMs, U08 *pbyCurPage)
{
    int iKey;

    if ((byItemNumber>253) || (0==byItemNumber) || (NULL==pbyItemString))
    {
        return 0;
    }
    if ((*pbyCurPage) > ((byItemNumber-1)/PAGE_ITEM_NUM))
    {
        *pbyCurPage = 0;
    }
    while (1)
    {
        (void)s_LcdShowItem(pbyItemString, byItemNumber, (*pbyCurPage));
        iKey = GetKeyBlockMs(iTimeoutMs);
        if (KEY_CANCEL == iKey)
        {
            return 0xff;
        }
        else if (KEY_TIMEOUT == iKey)
        {
            return 0xfe;  // timeout
        }
        if ((KEY_DOWN==iKey) && ((*pbyCurPage)<((byItemNumber-1)/PAGE_ITEM_NUM)))
        {
            (*pbyCurPage)++;
            if (((byItemNumber-1)/PAGE_ITEM_NUM) == (*pbyCurPage))
            {
                lcdSetIcon(ICON_DOWN, CLOSEICON);
            }
            lcdSetIcon(ICON_UP, OPENICON);
            continue;
        }
        if ((KEY_UP==iKey) && ((*pbyCurPage)>0))
        {
            (*pbyCurPage)--;
            lcdSetIcon(ICON_DOWN, OPENICON);
            if (0 == (*pbyCurPage))
            {
                lcdSetIcon(ICON_UP, CLOSEICON);
            }
            continue;
        }
        if (1 == byExitMode)
        {
            if (   (iKey>='1')
                && (iKey<=(PAGE_ITEM_NUM+'0'))
                && (((*pbyCurPage)*PAGE_ITEM_NUM+iKey-'0')<=byItemNumber))
            {
                return ((*pbyCurPage)*PAGE_ITEM_NUM+iKey-'0');
            }
        }
    }
}

U08 LcdGetItemTimeoutCE(const U08 *pbyChnItemString, const U08 *pbyEngItemString,
             U08 byItemNumber, U08 byExitMode, int iTimeoutMs, U08 *pbyCurPage)
{
    if (1 == g_iLangueSelect)
    {
        return LcdGetItemTimeout(pbyChnItemString, byItemNumber, byExitMode, iTimeoutMs, pbyCurPage);
    }
    else
    {
        return LcdGetItemTimeout(pbyEngItemString, byItemNumber, byExitMode, iTimeoutMs, pbyCurPage);
    }
}

U08 LcdGetItemSmall1Timeout(const U08 *pbyItemString, U08 byItemNumber, U08 byCurItem,
                U08 byMode, int iTimeoutMs, U08 *pbyCurPage)
{
    int iKey, iLastKey;
    U08 byItemNumPerPage, byExitMode;

    iLastKey = 0xff;
    byItemNumPerPage = PAGE_ITEM_NUM*2+1;

    if ((byItemNumber>253) || (0==byItemNumber) || (NULL==pbyItemString))
    {
        return 0;
    }
    if ((*pbyCurPage) > ((byItemNumber-1)/byItemNumPerPage))
    {
        *pbyCurPage = 0;
    }
    byExitMode = byMode&0x07;
    while (1)
    {
        (void)s_LcdShowSmallItem1(pbyItemString, byItemNumber, byCurItem, (*pbyCurPage), byMode);
        iKey = GetKeyBlockMs(iTimeoutMs);
        if (KEY_CANCEL == iKey)
        {
            return 0xff;
        }
        else if (KEY_TIMEOUT == iKey)
        {
            return 0xfe;  // timeout
        }
        if ((KEY_DOWN==iKey) && ((*pbyCurPage)<((byItemNumber-1)/byItemNumPerPage)))
        {
            (*pbyCurPage)++;
            if (((byItemNumber-1)/byItemNumPerPage) == (*pbyCurPage))
            {
                lcdSetIcon(ICON_DOWN, CLOSEICON);
            }
            lcdSetIcon(ICON_UP, OPENICON);
            continue;
        }
        if ((KEY_UP==iKey) && ((*pbyCurPage)>0))
        {
            (*pbyCurPage)--;
            lcdSetIcon(ICON_DOWN, OPENICON);
            if (0 == (*pbyCurPage))
            {
                lcdSetIcon(ICON_UP, CLOSEICON);
            }
            continue;
        }
        switch (byExitMode)
        {
        case 0:
            if (   (iKey>='1')
                && (iKey<=(byItemNumPerPage+'0'))
                && (((*pbyCurPage)*byItemNumPerPage+iKey-'0')<=byItemNumber))
            {
                iLastKey = (*pbyCurPage)*byItemNumPerPage+iKey-'0';
                byCurItem = iLastKey;
            }
            else if (KEY_ENTER == iKey)
            {
                if (0xff != iLastKey)
                {
                    return iLastKey;
                }
            }
            break;
        case 1:
            if (   (iKey>='1')
                && (iKey<=(byItemNumPerPage+'0'))
                && (((*pbyCurPage)*byItemNumPerPage+iKey-'0')<=byItemNumber))
            {
                return ((*pbyCurPage)*byItemNumPerPage+iKey-'0');
            }
            break;
        case 2:
            break;
        }
    }
}

/****************************************************************************
  ������     :  U08 LcdGetItemBig2Timeout(const U08 *pbyItemString, U08 byItemNumber,
                     U08 byCurItem, U08 byMode, int iTimeoutMs, U08 *pbyCurPage)
  ����       :  ��ʾ��Ϣ���������û�ѡ����Ϣ��
                ���е�11���ַ��Ǳ�����ʾ�����û�����ǰ��Ҫ��ʾ���⡣
                ����û�����ʾҳ��͵�ǰҳ�������������Ǳ�����ʾ����
  �������   :  1��U08 *pbyItemString : ��Ϣ�������ݣ���17���ֽ�Ϊһ����Ϣ��ÿ
                   ����Ϣ�����һ���ֽڱ�������0��pbyItemString���ڴ�ռ䲻С��
                   byItemNumber*17��
                2��U08 byItemNumber : Ҫ��ʾ����Ϣ������Ϣ������ȡֵ1~253
                3��U08 byCurItem : ��ǰѡ�����ţ�ȡֵ1~byItemNumber
                4��U08 byMode : bit0~2��ʾ�˳�ģʽ��
                                       =0��ʾ��ֻ�а�'Enter'������'Cancel'�����˳���
                                       =1��ʾ��'1'��'2'��'3'��'4'��'5'��'6'��'Cancel'���˳���
                                       =2��ʾֻ�а�'Cancel'�����˳���
                                 bit3:��ʾ�Ƿ���ʾ��ǰҳ����1��ʾ��0����ʾ��
                                 bit4:��ʾÿ����ʾ������1��ʾ2�0��ʾ1��
                                 bit5:�Ƴ�ǰ�Ƿ���ʾ�û���ѡ��0��1��
                                 bit6~7:����
                5��int iTimeoutMs : ��ʱʱ�䣬��λms
                6��U08 *pbyCurPage : ��ǰҳ�룬��0��ʼ
  �������   :  1��U08 *pbyCurPage : ��ǰҳ��
  ����ֵ     :  ����0��ʾ����������󣬷���255��ʾ�û�����ȡ����������1~253��ʾ�û�
                ѡ�����Ϣ�����ţ�����254��ʾ��ʱ�˳���
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2010-05-18  V1.0         ����
****************************************************************************/
U08 LcdGetItemBigTimeout(const U08 *pbyItemString, U08 byItemNumber, U08 byCurItem,
                U08 byMode, int iTimeoutMs, U08 *pbyCurPage)
{
    int iKey, iLastKey;
    U08 byItemNumPerPage, byExitMode, byPage, byItem;

    iLastKey = 0xff;
    if ((0!=byCurItem) && (byCurItem<=byItemNumber))
    {
        iLastKey = byCurItem;
    }
    if (0 != (byMode&LCD_MODE_2ITEM_PER_LINE))
    {
        byItemNumPerPage = PAGE_ITEM_NUM*2;
    }
    else
    {
        byItemNumPerPage = PAGE_ITEM_NUM;
    }

    if ((byItemNumber>253) || (0==byItemNumber) || (NULL==pbyItemString))
    {
        return 0;
    }
    if ((*pbyCurPage) > ((byItemNumber-1)/byItemNumPerPage))
    {
        *pbyCurPage = 0;
    }
    if (0 == (byMode&0x08))
    {
        if ((0!=byCurItem) && (byCurItem<=byItemNumber))
        {
            if (byItemNumber > byItemNumPerPage)
            {
                byPage = (byCurItem-1)/byItemNumPerPage;
                byItem = (byCurItem-1)%byItemNumPerPage+1;
                lcdDisplay(88, 0, 0x81, "(%d.%d)    ", byPage+1, byItem);
            }
            else
            {
                lcdDisplay(88, 0, 0x81, " (%d)    ", byCurItem);
            }
        }
    }
    if (byItemNumber > byItemNumPerPage)
    {
        byPage = (byItemNumber+byItemNumPerPage-1)/byItemNumPerPage;
        if (byPage > ((*pbyCurPage)+1))
        {
            lcdSetIcon(ICON_DOWN, OPENICON);
        }
        else
        {
            lcdSetIcon(ICON_DOWN, CLOSEICON);
        }
        if (0 != (*pbyCurPage))
        {
            lcdSetIcon(ICON_UP, OPENICON);
        }
        else
        {
            lcdSetIcon(ICON_UP, CLOSEICON);
        }
    }
    else
    {
        lcdSetIcon(ICON_UP, CLOSEICON);
        lcdSetIcon(ICON_DOWN, CLOSEICON);
    }
    byExitMode = byMode&0x07;
    while (1)
    {
        (void)s_LcdShowBigItem(pbyItemString, byItemNumber, byCurItem, (*pbyCurPage), byMode);
        iKey = GetKeyBlockMs(iTimeoutMs);
        if (KEY_CANCEL == iKey)
        {
            return 0xff;
        }
        else if (KEY_TIMEOUT == iKey)
        {
            return 0xfe;  // timeout
        }
        if ((KEY_DOWN==iKey) && ((*pbyCurPage)<((byItemNumber-1)/byItemNumPerPage)))
        {
            (*pbyCurPage)++;
            if (((byItemNumber-1)/byItemNumPerPage) == (*pbyCurPage))
            {
                lcdSetIcon(ICON_DOWN, CLOSEICON);
            }
            lcdSetIcon(ICON_UP, OPENICON);
            continue;
        }
        if ((KEY_UP==iKey) && ((*pbyCurPage)>0))
        {
            (*pbyCurPage)--;
            lcdSetIcon(ICON_DOWN, OPENICON);
            if (0 == (*pbyCurPage))
            {
                lcdSetIcon(ICON_UP, CLOSEICON);
            }
            continue;
        }
        switch (byExitMode)
        {
        case 0:
            if (   (iKey>='1')
                && (iKey<=(byItemNumPerPage+'0'))
                && (((*pbyCurPage)*byItemNumPerPage+iKey-'0')<=byItemNumber))
            {
                iLastKey = (*pbyCurPage)*byItemNumPerPage+iKey-'0';
                byCurItem = iLastKey;
            }
            else if (KEY_ENTER == iKey)
            {
                if (0xff != iLastKey)
                {
                    if (0 != (byMode&LCD_MODE_SHOW_USER_SELECT))
                    {
                        s_LcdShowBigUserSelect(pbyItemString, iLastKey);
                    }
                    return iLastKey;
                }
            }
            break;
        case 1:
            if (   (iKey>='1')
                && (iKey<=(byItemNumPerPage+'0'))
                && (((*pbyCurPage)*byItemNumPerPage+iKey-'0')<=byItemNumber))
            {
                iLastKey = (*pbyCurPage)*byItemNumPerPage+iKey-'0';
                if (0 != (byMode&LCD_MODE_SHOW_USER_SELECT))
                {
                    s_LcdShowBigUserSelect(pbyItemString, iLastKey);
                }
                return iLastKey;
            }
            break;
        case 2:
            break;
        }
    }
}

/****************************************************************************
  ������     :  int LcdTwoSelectOneShort(const U08 *pbyMsg1, const U08 *pbyMsg2, int iCurMsg,
                            int iTimeoutMs, U08 byMode)
  ����       :  ��ʾ��Ϣ���������û�ѡ����Ϣ��
                ���е�11���ַ��Ǳ�����ʾ�����û�����ǰ��Ҫ��ʾ���⡣
                ����û�����ʾҳ��͵�ǰҳ�������������Ǳ�����ʾ����
  �������   :  1��U08 *pbyMsg1 : ��Ϣ��������1
                2��U08 *pbyMsg2 : ��Ϣ��������2
                3��int iCurMsg : ��ǰѡ����Ϣ��ţ�ȡֵ1����2
                4��int iTimeoutMs : ��ʱʱ�䣬��λms
                5��U08 byMode : bit0~3��ʾ�˳�ģʽ��
                                       =0��ʾ��ֻ�а�'Enter'������'Cancel'�����˳���
                                       =1��ʾ��'1'��'2'��'Cancel'���˳���
                                 bit4:��ʾÿ����ʾ������1��ʾ2�0��ʾ1��
                                 bit5:�˳�ǰ�Ƿ���ʾ�û���ѡ��0��1��
                                 bit6~7:����
  �������   :  ��
  ����ֵ     :  ����0��ʾ����������󣬷���255��ʾ�û�����ȡ����������1����2��ʾ�û�
                ѡ�����Ϣ�����ţ�����254��ʾ��ʱ�˳���
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2010-05-18  V1.0         ����
****************************************************************************/
int LcdTwoSelectOneShort(const U08 *pbyTitle, const U08 *pbyMsg1, const U08 *pbyMsg2, int iCurMsg,
            int iTimeoutMs, U08 byMode)
{
    int iKey;
    U08 byExitMode;

    if ((NULL==pbyMsg1) || (NULL==pbyMsg2))
    {
        return 0;
    }
    if (0 == (byMode&LCD_MODE_2ITEM_PER_LINE))
    {
        lcdDisplay(0,  0, 0x81, (U08*)pbyTitle);
        lcdDisplayCE(0, 2, 0x01, "��ǰ:", "Cur: ");
        if ((1==iCurMsg) || (2==iCurMsg))
        {
            lcdDisplay(40, 2, 0x01, "(%d)", iCurMsg);
        }
    }
    else
    {
        lcdDisplay(0,  2, 0x01, (U08*)pbyTitle);
        lcdDisplayCE(0, 4, 0x01, "��ǰ:", "Cur: ");
        if ((1==iCurMsg) || (2==iCurMsg))
        {
            lcdDisplay(40, 4, 0x01, "(%d)", iCurMsg);
        }
    }
    byExitMode = byMode&0x07;
    while (1)
    {
        if (0 != (byMode&LCD_MODE_2ITEM_PER_LINE))
        {
            if (1 == iCurMsg)
            {
                lcdDisplay(0,  6, 0x01, "1>%s", pbyMsg1);
                lcdDisplay(64, 6, 0x01, "2.%s", pbyMsg2);
            }
            else
            {
                lcdDisplay(0,  6, 0x01, "1.%s", pbyMsg1);
                lcdDisplay(64, 6, 0x01, "2>%s", pbyMsg2);
            }
        }
        else
        {
            if (1 == iCurMsg)
            {
                lcdDisplay(0, 4, 0x01, "1>%s", pbyMsg1);
                lcdDisplay(0, 6, 0x01, "2.%s", pbyMsg2);
            }
            else
            {
                lcdDisplay(0, 4, 0x01, "1.%s", pbyMsg1);
                lcdDisplay(0, 6, 0x01, "2>%s", pbyMsg2);
            }
        }
        iKey = GetKeyBlockMs(iTimeoutMs);
        switch (iKey)
        {
        case KEY_CANCEL:
            return 0xff;
        case KEY_TIMEOUT:
            return 0xfe;
        case KEY1:
        case KEY2:
            if (0 != byExitMode)
            {
                s_LcdShowUserSelect(pbyMsg1, pbyMsg2, iKey-KEY0, byMode);
                return (iKey-KEY0);
            }
            iCurMsg = iKey-KEY0;
            break;
        case KEY_ENTER:
            if (0 == byExitMode)
            {
                if ((1==iCurMsg) || (2==iCurMsg))
                {
                    s_LcdShowUserSelect(pbyMsg1, pbyMsg2, iCurMsg, byMode);
                    return iCurMsg;
                }
            }
            break;
        }
    }
}

void   DrawRect(U32 uiBegRow, U32 uiBegCol, U32 uiEndRow, U32 uiEndCol)
{
    U32 uiPointx, uiPointy;

#ifdef  COMPILED_IN_VC
    lcdDrawFrame(uiBegRow, uiBegCol, uiEndRow, uiEndCol,1);
    return;
#endif

    if (uiBegRow >= 127)
    {
        uiEndRow = 127;
    }
    if (uiEndRow >= 127)
    {
        uiEndRow = 127;
    }
    if (uiEndRow <= uiBegRow)
    {
        uiEndRow = uiBegRow;
    }

    if (uiBegCol >= 63)
    {
        uiBegCol = 63;
    }
    if (uiEndCol >= 63)
    {
        uiEndCol = 63;
    }
    if (uiEndCol <= uiBegCol)
    {
        uiEndCol = uiBegCol;
    }

    for (uiPointx=uiBegRow; uiPointx<=uiEndRow;uiPointx++)
    {
        lcdStipple(uiPointx, uiBegCol, ON);
        lcdStipple(uiPointx, uiEndCol, ON);
    }

    for (uiPointy=uiBegCol; uiPointy<=uiEndCol; uiPointy++)
    {
        lcdStipple(uiBegRow, uiPointy, ON);
        lcdStipple(uiEndRow, uiPointy, ON);
    }

    return;
}

void PubDisplayMsg(U08 ucLine, const U08 *pszMsg)
{
    int iLen;
    U08 ucMode;

    if (NULL == pszMsg)
    {
        return;
    }

    ucMode =  (ucLine & DISP_REVERSE) | DISP_CFONT;
    ucLine &= ~DISP_REVERSE;
    lcdClrLine(ucLine, (U08)(ucLine+1));

    iLen = strlen((char *)pszMsg);
    if (iLen > NUM_MAXZHCHARS)
    {
        iLen = NUM_MAXZHCHARS;
    }

    lcdDisplay((U08)((NUM_MAXCOLS-8*iLen)/2), ucLine, ucMode,
            "%.*s", iLen, (char *)pszMsg);

    return;
}

void Display2Strings(const char *pszStringHz, const char *pszStringEn)
{
    lcdClrLine(2, 7);
    if (0 == g_iLangueSelect)
    {
        PubDisplayMsg(4, pszStringEn);
    }
    else
    {
        PubDisplayMsg(3, pszStringHz);
        PubDisplayMsg(5, pszStringEn);
    }
    DrawRect(0, 17, 127, 63);
}


typedef struct {
	int		property;
	char	buffer[17];
} fb_t;

static fb_t theFB[4];
static int exitflag = 0;

void middle_print(int line, int mode, const char *prompt);


void InitFrameBuffer(void)
{
	int i;

	for (i = 0; i<ARRAY_SIZE(theFB); i++) {
		theFB[i].property = 1;
		memset(theFB[i].buffer, ' ', sizeof(theFB[i].buffer)-1);
		theFB[i].buffer[sizeof(theFB[i].buffer)-1] = '\0';
	}
}


int fbprintf(int x, int y, int property, const char *format,...)
{
	va_list args;
	int cnt;

	if (x < 0 || x >= sizeof(theFB[0].buffer)-1 ||
		y < 0 || y >= ARRAY_SIZE(theFB))
		return 0;

	theFB[y].property = property;
	va_start(args, format);
	cnt = vsnprintf(&theFB[y].buffer[x], sizeof(theFB[y].buffer) - x, format, args);
	va_end(args);

	return cnt;
}

void ShowFrameBuffer(void)
{
	int i, cnt;

	for (i = 0; i<ARRAY_SIZE(theFB); i++) {
		if (! (theFB[i].property & (1<<6))) {
			cnt = strlen(theFB[i].buffer);
			memset(&theFB[i].buffer[cnt], ' ', sizeof(theFB[i].buffer)-1-cnt);
			theFB[i].buffer[sizeof(theFB[i].buffer)-1] = '\0';
		}

		lcdDisplay(0, i*2, theFB[i].property, "%s", theFB[i].buffer);
	}

}

void middle_print(int line, int mode, const char *prompt)
{
	char buf[17];
	int len;

	len = strlen(prompt);
	memset(buf, ' ', sizeof(buf)-1);
	if (len > sizeof(buf)-1)
		len = sizeof(buf)-1;

	memcpy(&buf[(sizeof(buf)-1 - len)/2], prompt, len);
	buf[sizeof(buf)-1] = '\0';

	lcdDisplay(0, line, mode, buf);
}

static int DisplayMenu(const MENU_SELECT *menu)
{
	int i;

    InitFrameBuffer();
	middle_print(0,  BIT(0) | BIT(6), menu->strTitle);
	for (i = 0; i < menu->iItemNumber; i++)
    {
		if ((i < menu->iStart) || (i > (menu->iStart + menu->iNrPrescreen)))
			continue;

		if ( menu->iCurrent == i)
			fbprintf(0, i - menu->iStart + 1, BIT(0) | BIT(5) | BIT(7), "*%s", menu->psItem[i].strMsg);
		else
			fbprintf(0, i - menu->iStart + 1, BIT(0), " %s", menu->psItem[i].strMsg);
	}
    ShowFrameBuffer();

	return 0;
}


int MenuSelect(MENU_SELECT *psMenu)
{
	int flag = 0, fresh = 1;
	int key = KEY_CANCEL;
    int iRet;

	while (0 == flag)
    {
		if (0 != fresh)
        {
			DisplayMenu(psMenu);
			fresh = 0;
		}

		lcdSetIcon(ICON_UP,   psMenu->iStart == 0 ? 0 : 1);
		lcdSetIcon(ICON_DOWN, psMenu->iStart + psMenu->iNrPrescreen >= psMenu->iItemNumber ? 0 : 1);

		key = kbGetKey();
		switch (key)
        {
		case KEY_UP:
			fresh = 1;
			psMenu->iCurrent--;
			if (psMenu->iCurrent < 0)
				psMenu->iCurrent = 0;
			break;
		case KEY_DOWN:
			fresh = 1;
			psMenu->iCurrent++;
			if (psMenu->iCurrent >= psMenu->iItemNumber)
				psMenu->iCurrent = psMenu->iItemNumber-1;
			break;
		case KEY_ENTER:
            if (NULL != psMenu->psItem[psMenu->iCurrent].func)
            {
                iRet = psMenu->psItem[psMenu->iCurrent].func(1, NULL);
                if (KEY_CANCEL == iRet)
                {
                    flag = 1;
                }
            }
			fresh = 1;
            break;
		case KEY_CANCEL:
			flag = 1;
			break;
		default:
			break;
		}
		if (psMenu->iStart > psMenu->iCurrent)
			psMenu->iStart = psMenu->iCurrent;

		if (psMenu->iStart + psMenu->iNrPrescreen <= psMenu->iCurrent)
			psMenu->iStart = psMenu->iCurrent - psMenu->iNrPrescreen + 1;

	}

	return key;
}


