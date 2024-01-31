#include <stdio.h>
#include <stdlib.h>
#include "posapi.h"
#include "zint.h"
#include "alloc.h"
#include "bmp.h"
#include "QR.h"

void test_bmp(struct zint_symbol *symbol)
{
	char *p=NULL;

	int file_size = 0;
	file_size=bmp_get_size(symbol->bitmap_width, symbol->bitmap_height);
	p=(char*)malloc(file_size);
	if(p==NULL){
		lcdDisplay(0,7, DISP_ASCII, "mallloc p failed!\r\n");
		kbGetKey();
		return;
	}
	memset(p, 0, file_size);
	bmp_create_bmpfile(p, symbol->bitmap, symbol->bitmap_width, symbol->bitmap_height);

	//print image


	prnInit();
	prnPrintf("Code %d:\r\n", symbol->symbology);

	prnLogo(0, p);
	

	prnPrintf("\n\n\n\n");
	prnPrintf("\n");

	while(1)
	{
		sysDelayMs(1000*1);
		if(prnStart()==0)
		{
			sysDelayMs(1000*2);
			break;
		}

	}		



	free(p);
}

int zint_test(uint8_t*  QR_Str)
{
	int error;
//	uint8_t  temp[20];
    struct zint_symbol *my_symbol = NULL;

    my_symbol = ZBarcode_Create();
   /*
   if(my_symbol != NULL)
    {
    	lcdDisplay(0, 7, DISP_ASCII, "Symbol successfully created!\n");
    }
*/
    if(0==ZBarcode_ValidID(BARCODE_QRCODE)){
    	ZBarcode_Delete(my_symbol);
    	return -1;
    }
 //   memset(temp,0,sizeof(temp));
    my_symbol->symbology = BARCODE_QRCODE;
    my_symbol->scale	 = 3;//2; //scale the image width*2 and height *2
 //  sprintf(temp,"%d",QR_Str);
  error = ZBarcode_Encode(my_symbol, QR_Str, 0); 

/*    
if(error != 0){
    	lcdDisplay(0, 7, DISP_ASCII, "ZBarcode_Encode: %s\n", my_symbol->errtxt);
    }
*/
    error = ZBarcode_Buffer(my_symbol, 180);
/*    if(error != 0){
    	lcdCls();
    	lcdDisplay(0, 7, DISP_ASCII, "ZBarcode_Encode: %s\n", my_symbol->errtxt);
    	kbGetKey();
    }
*/
    //translate to bmp format and print
    test_bmp(my_symbol);


    ZBarcode_Delete(my_symbol);

    return 0;
}


int Bar_zint_test(uint8_t*  QR_Str,int BarType)
{
	int error;
//	uint8_t  temp[20];
    struct zint_symbol *my_symbol = NULL;

    my_symbol = ZBarcode_Create();

   if(my_symbol != NULL)
    {
    	lcdDisplay(0, 2, DISP_ASCII, "Symbol successfully created!\n");
    }

    if(0==ZBarcode_ValidID(BarType)){
    	ZBarcode_Delete(my_symbol);
    	return -1;
    }
 //   memset(temp,0,sizeof(temp));
    my_symbol->symbology = BarType;
    my_symbol->scale	 = 4;//2; //scale the image width*2 and height *2
 //  sprintf(temp,"%d",QR_Str);
  error = ZBarcode_Encode(my_symbol, QR_Str, 0); 


    
if(error != 0){
    	lcdDisplay(0, 2, DISP_ASCII, "ZBarcode_Encode: %s\n", my_symbol->errtxt);
    }

    error = ZBarcode_Buffer(my_symbol, 180);

    if(error != 0){
    	lcdCls();
    	lcdDisplay(0, 2, DISP_ASCII, "ZBarcode_Encode: %s\n", my_symbol->errtxt);
    	kbGetKey();
    }

    //translate to bmp format and print
    test_bmp(my_symbol);


    ZBarcode_Delete(my_symbol);

    return 0;
}



