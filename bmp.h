/*
 * bmp.h
 *
 *  Created on: 2013-2-21
 *      Author: Administrator
 */

#ifndef BMP_H_
#define BMP_H_

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

typedef unsigned short  WORD;
typedef	unsigned int	DWORD;
typedef int				LONG;
typedef unsigned char	BYTE;
typedef void*			LPVOID;


//位图文件头
typedef struct tagBITMAPFILEHEADER{
	WORD 	bfType;
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
}BITMAPFILEHEADER;


//位图信息头
typedef struct tagBITMAPINFOHEADER{
		DWORD	biSize;
		LONG	biWidth;
		LONG	biHeight;
		WORD	biPlanes;
		WORD	biBitCount;
		DWORD	biCompression;
		DWORD	biSizeImage;
		LONG	biXPelsPerMeter;
		LONG	biYPelsPerMeter;
		DWORD	biClrUsed;
		DWORD	biClrImportant;
}BITMAPINFOHEADER;

typedef struct tagRGBQUAD{
	BYTE	rgbBlue;
	BYTE	rgbGreen;
	BYTE	rgbRed;
	BYTE	rgbReserved;
}RGBQUAD;

int bmp_get_size(int width, int height);
int bmp_create_bmpfile(unsigned char *bmp_image, const unsigned char *bmp_data, int pixel_width, int pixel_height);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* BMP_H_ */
