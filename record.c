typedef struct _transaction_record_{
    U08 m_abyTransData[24];
}TRANS_RECORD; // Notice: the structure length must be 4*n, and n=1,2,3,....

#define RECORD_FILE "TRANS_RECORD"

int SaveNewRecord(const TRANS_RECORD *psRecord);
int ReadRecord(int iRecordIndex, TRANS_RECORD *psRecord);
int GetRecordNumber(void);

U08 *g_pbyTransFileName = "TRANS_RECORD";

int SaveNewRecord(const TRANS_RECORD *psRecord)
{
    int fd, iFileLen, i;
    fd = fileOpen(g_pbyTransFileName, O_RDWR);
    if (fd < 0)
    {
        fd = fileOpen(g_pbyTransFileName, O_CREAT);
        if (fd < 0)
        {
            lcdDisplay(0, 4, 0x01, (U08*)"Create file failed.");
            kbGetKeyMs(KEY_BLOCK_TIME);
            return -1;
        }
    }
    iFileLen = fileSize(g_pbyTransFileName);
    i = iFileLen%sizeof(TRANS_RECORD);
    if (0 != i)
    {
        i = iFileLen - i;
        fileSeek(fd, i, SEEK_SET);
    }
    else
    {
        fileSeek(fd, 0, SEEK_END);
    }
    fileWrite(fd, psRecord, sizeof(TRANS_RECORD));
    fileClose(fd);
    return 0;// success
}

int ReadRecord(int iRecordIndex, TRANS_RECORD *psRecord)
{
    int fd, iFileLen, i;
    fd = fileOpen(g_pbyTransFileName, O_RDWR);
    if (fd < 0)
    {
        lcdDisplay(0, 4, 0x01, (U08*)"Open file failed.");
        kbGetKeyMs(KEY_BLOCK_TIME);
        return -1;
    }
    iFileLen = fileSize(g_pbyTransFileName);
    i = iRecordIndex*sizeof(TRANS_RECORD);
    if (iFileLen < i)
    {
        lcdDisplay(0, 4, 0x01, (U08*)"There has not record.");
        kbGetKeyMs(KEY_BLOCK_TIME);
        return 0;
    }
    fileSeek(fd, i, SEEK_SET);
    fileRead(fd, psRecord, sizeof(TRANS_RECORD));
    fileClose(fd);
    return 1;
}
int GetRecordNumber(void);
{
    int iFileLen;
    
    iFileLen = fileSize(g_pbyTransFileName);
    return (iFileLen/sizeof(TRANS_RECORD));
}