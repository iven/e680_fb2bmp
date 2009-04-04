#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD  unsigned int
#define LONG  int

typedef struct
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} bmp_info_header;

typedef struct
{
    WORD bfType;              //BBMM
    DWORD bfSize;              //文件大小
    WORD bfReserved1;              //保留字1
    WORD bfReserved2;              //保保留字2留字2
    DWORD bfOffBits;              //文件头到实际位图数据偏移字节数
} bmp_file_header;
