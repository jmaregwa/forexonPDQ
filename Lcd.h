#ifndef _LCD_H_
#define _LCD_H_

#ifndef U08
#define U08 unsigned char
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef U32
#define U32 unsigned int
#endif

// 每行最多显示19个小字符
#define ITEM_LENGTH   20


#define PAGE_ITEM_NUM  3
#define LCD_HALF_WIDTH    64

#define LCD_HIGH_MINI  8
#define FILE_NAME_MAX_LEN  17


#define LCD_SHOW_SMALL_FLAG     0x01
#define LCD_SHOW_TWO_LINE_FLAG  0x02


#define LCD_EXIT_MODE0             0x00
#define LCD_EXIT_MODE1             0x01
#define LCD_EXIT_MODE2             0x02

#define LCD_MODE_SHOW_PAGE         0x08

#define LCD_MODE_2ITEM_PER_LINE    0x10
#define LCD_MODE_1ITEM_PER_LINE    0x00

#define LCD_MODE_SHOW_USER_SELECT  0x20

typedef struct {
    char *strMsg;
    int  (*func)(int argc, char *argv[]);
}MENU_ITEM;

typedef struct {
    char strTitle[32];
    int iNrPrescreen;
    int iStart;
    int iCurrent;
    MENU_ITEM *psItem;
    int iItemNumber;
}MENU_SELECT;

#endif

