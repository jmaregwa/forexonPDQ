/*
 * bmp.c
 *
 *  Created on: 2013-2-21
 *      Author: Administrator
 */

#include "bmp.h"
#include <stdio.h>
#include <string.h>

int bmp_get_size(int width, int height)
{
	int byte_per_line  = (width+7)/8 + (4-(width+7)/8%4);
	int image_size  = byte_per_line * height;

	return image_size+54+8;
}

static int bmp24_to_bmp1(unsigned char *std_bmp, const unsigned char *raw_bmp, int width, int height)
{
	int row, col, bit;
	unsigned char red, blue, green;
	unsigned char* pbmp = std_bmp;
	int offset=width *3;
	
	for(row=0; row <height; row++){
		for(bit=7, col = 0; col < width; col++){
			red	  = raw_bmp[(row+1)*offset-(col+1)*3];
			green = raw_bmp[(row+1)*offset-(col+1)*3+1];
			blue  = raw_bmp[(row+1)*offset-(col+1)*3+2];
			if(red != 0 || green != 0 || blue !=0){
				*pbmp |= (1<<bit);
			}else{
				*pbmp &= (~(1<<bit));
			}
			bit--;
			if(bit==-1){
				bit=7;
				pbmp++;
			}
		}
		//四个字节对齐
		if((pbmp-std_bmp+1)%4!=0){
			pbmp +=4-(pbmp-std_bmp+1)%4+1;
		}
	}

	return 0;
}



int bmp_create_bmpfile(unsigned char *bmp_image, const unsigned char *bmp_data, int pixel_width, int pixel_height)
{
	BITMAPFILEHEADER bmfhdr;
	BITMAPINFOHEADER bi;
	RGBQUAD 		 bColors[]={
			{0x00,0x00,0x00,0x00},
			{0xFF,0xFF,0xFF,0x00},
	};

	DWORD	ImagSize = ((pixel_width+7)/8 + (4-(pixel_width+7)/8%4)) * pixel_height;

	//填充文件头
	bmfhdr.bfType		= 0x4d42;
	bmfhdr.bfSize		= 14+40+8+ImagSize;
	bmfhdr.bfReserved1 = 0;
	bmfhdr.bfReserved2 = 0;
	bmfhdr.bfOffBits   = 14+40+8;

	//填充位图信息头
	bi.biSize			= 40;
	bi.biWidth			= pixel_width; //3个字节代表一个像素
	bi.biHeight			= pixel_height;
	bi.biPlanes			= 1;
	bi.biBitCount 		= 1;
	bi.biCompression 	= 0;
	bi.biSizeImage		= ImagSize;   //位图数据的大小
    bi.biXPelsPerMeter	= 0;
	bi.biYPelsPerMeter	= 0;
	bi.biClrUsed		= 0;
	bi.biClrImportant	= 0;

	{
		int len = 0;
		//1.文件头
		memcpy(bmp_image+len, &bmfhdr.bfType, 2);
		len+=2;
		memcpy(bmp_image+len, &bmfhdr.bfSize, 4);
		len+=4;
		memcpy(bmp_image+len, &bmfhdr.bfReserved1, 2);
		len+=2;
		memcpy(bmp_image+len, &bmfhdr.bfReserved2, 2);
		len+=2;
		memcpy(bmp_image+len, &bmfhdr.bfOffBits, 4);
		len+=4;

		//2.填充位图信息头
		memcpy(bmp_image+len, &bi.biSize, 4);
		len+=4;
		memcpy(bmp_image+len, &bi.biWidth, 4);
		len+=4;
		memcpy(bmp_image+len, &bi.biHeight, 4);
		len+=4;
		memcpy(bmp_image+len, &bi.biPlanes, 2);
		len+=2;
		memcpy(bmp_image+len, &bi.biBitCount, 2);
		len+=2;
		memcpy(bmp_image+len, &bi.biCompression, 4);
		len+=4;
		memcpy(bmp_image+len, &bi.biSizeImage, 4);
		len+=4;
		memcpy(bmp_image+len, &bi.biXPelsPerMeter, 4);
		len+=4;
		memcpy(bmp_image+len, &bi.biYPelsPerMeter, 4);
		len+=4;
		memcpy(bmp_image+len, &bi.biClrUsed, 4);
		len+=4;
		memcpy(bmp_image+len, &bi.biClrImportant, 4);
		len+=4;

		//3.填充调色板
		memcpy(bmp_image+len, bColors, sizeof(bColors));
		len+=sizeof(bColors);

		//4.填充数据
		bmp24_to_bmp1(bmp_image+len, bmp_data, pixel_width, pixel_height);
	}

	return 0;
}





