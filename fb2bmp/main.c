/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  16bit to 24bit bmp.
 *
 *        Version:  1.0
 *        Created:  2009年04月04日 20时37分38秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Xu Lijian (ivenvd), ivenvd@gmail.com
 *        Company:  CUGB, China
 *
 * =====================================================================================
 */


#include	<errno.h>
#include	<stdio.h> 
#include	<stdlib.h>
#include	<string.h>
#include	"bmp_struct.h"

#define FB_WIDTH 240
#define FB_HEIGHT 320
#define FB_SIZE FB_WIDTH * FB_HEIGHT

typedef enum {
    BMP_BIT_16,
    BMP_BIT_24
} bmp_bit_count;				/* ----------  end of union bmp_bit_count  ---------- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_header
 *  Description:  init the header of the output file.
 * =====================================================================================
 */
    int
init_header ( bmp_file_header *file, bmp_info_header *info, bmp_bit_count bit )
{
    /*-----------------------------------------------------------------------------
     *  set headers to zero.
     *-----------------------------------------------------------------------------*/
    memset ((void *)info, 0, sizeof (bmp_info_header));
    memset ((void *)file, 0, sizeof (bmp_file_header));
    /*-----------------------------------------------------------------------------
     *  init them.
     *-----------------------------------------------------------------------------*/
    file -> bfType = 0x4D42;                    /* file type is "BM" */
    file -> bfSize = 14 + 40 + FB_SIZE * sizeof (BYTE) * 3; /* sizeof (file + info + pixels) */
    file -> bfOffBits = 14 + 40;                /* sizeof (file + info) */
    info -> biSize = 40;                        /* sizeof (info) */
    info -> biWidth = FB_WIDTH;                 /* bmp width */
    info -> biHeight = - FB_HEIGHT;             /* bmp height */
    info -> biPlanes = 1;                       /* always 1 */
    info -> biSizeImage = FB_SIZE * sizeof (BYTE) * 3; /* sizeof (pixels) */
    switch ( bit ) {
        case BMP_BIT_16:	
            file -> bfSize += sizeof (DWORD) * 3; /* 16 bit bmps need space to store masks */
            info -> biBitCount = 16;
            info -> biCompression = 3;          /* BI_BITFIELDS */
            break;

        case BMP_BIT_24:	
            info -> biBitCount = 24;
            info -> biCompression = 0;          /* BI_RGB */
            break;

        default:	
            fprintf ( stderr, "%d isn't a valid bit count.\n", bit );
            return -1;
    }				/* -----  end switch  ----- */

    return 0;
}		/* -----  end of function init_header  ----- */
    void
write_header ( FILE *fp, bmp_file_header *file, bmp_info_header *info )
{
    fwrite (&file -> bfType, sizeof (WORD), 1, fp);
    fwrite (&file -> bfSize, sizeof (DWORD), 1, fp);
    fwrite (&file -> bfReserved1, sizeof (WORD), 1, fp);
    fwrite (&file -> bfReserved2, sizeof (WORD), 1, fp);
    fwrite (&file -> bfOffBits, sizeof (DWORD), 1, fp);
    fwrite (&info -> biSize, sizeof (DWORD), 1, fp);
    fwrite (&info -> biWidth, sizeof (LONG), 1, fp);
    fwrite (&info -> biHeight, sizeof (LONG), 1, fp);
    fwrite (&info -> biPlanes, sizeof (WORD), 1, fp);
    fwrite (&info -> biBitCount, sizeof (WORD), 1, fp);
    fwrite (&info -> biCompression, sizeof (DWORD), 1, fp);
    fwrite (&info -> biSizeImage, sizeof (DWORD), 1, fp);
    fwrite (&info -> biXPelsPerMeter, sizeof (LONG), 1, fp);
    fwrite (&info -> biYPelsPerMeter, sizeof (LONG), 1, fp);
    fwrite (&info -> biClrUsed, sizeof (DWORD), 1, fp);
    fwrite (&info -> biClrImportant, sizeof (DWORD), 1, fp);

    return;
}		/* -----  end of function write_header  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  write_pixels
 *  Description:  write pixels to bmp.
 * =====================================================================================
 */
    int
write_pixels ( FILE *fp, WORD (*buf) [FB_WIDTH], bmp_bit_count bit )
{
    int i, j;
    BYTE bmp_pixel [FB_HEIGHT][FB_WIDTH][3];
    WORD pixel;
    DWORD mask_565 [3] = {0xF800, 0x07E0, 0x001F};

    switch ( bit ) {
        case BMP_BIT_16:	
            fwrite (mask_565, sizeof (DWORD), 3, fp);
            fwrite (buf, sizeof (WORD), FB_SIZE, fp);
            break;

        case BMP_BIT_24:	
            for (i = 0; i < FB_HEIGHT; i++) {
                for (j = 0; j < FB_WIDTH; j++) {
                    pixel = buf [i][j];
                    bmp_pixel [i][j][0] = (pixel & mask_565 [2]) << 3;
                    bmp_pixel [i][j][1] = (pixel & mask_565 [1]) >> 3;
                    bmp_pixel [i][j][2] = (pixel & mask_565 [0]) >> 8;
                }
            }
            fwrite (bmp_pixel, sizeof (BYTE) * 3, FB_SIZE, fp);
            break;

        default:	
            fprintf ( stderr, "%d isn't a valid bit count.\n", bit );
            return -1;
    }				/* -----  end switch  ----- */

    return 0;
}		/* -----  end of function write_pixels  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  main function
 * =====================================================================================
 */
	int
main ( int argc, char *argv[] )
{
    WORD buf [FB_HEIGHT][FB_WIDTH];
    bmp_file_header bmp_file;
    bmp_info_header bmp_info;
    bmp_bit_count bit;
    FILE	*fp;
    char	*fp_file_name;

    if (argc < 2) {
        printf ("Usage: fb2bmp INPUT OUTPUT BIT_COUNT\nBIT_COUNT may be 16 or 24.\n");
        exit (EXIT_FAILURE);
    }
    switch ( atoi (argv [3]) ) {
        case 16:	
            bit = BMP_BIT_16;
            break;

        case 24:	
            bit = BMP_BIT_24;
            break;

        default:	
            fprintf ( stderr, "%d isn't a valid bit count.\n", atoi (argv [3]) );
            exit (EXIT_FAILURE);
    }				/* -----  end switch  ----- */

    /*-----------------------------------------------------------------------------
     *  read frame buffer file to buf.
     *-----------------------------------------------------------------------------*/
    fp_file_name = argv [1];		/* input-file name    */

    fp	= fopen( fp_file_name, "r" );
    if ( fp == NULL ) {
        fprintf ( stderr, "couldn't open file '%s'; %s\n",
                fp_file_name, strerror(errno) );
        exit (EXIT_FAILURE);
    }

    fread (buf, sizeof (WORD), FB_SIZE, fp);

    if( fclose(fp) == EOF ) {			/* close input file   */
        fprintf ( stderr, "couldn't close file '%s'; %s\n",
                fp_file_name, strerror(errno) );
        exit (EXIT_FAILURE);
    }

    /*-----------------------------------------------------------------------------
     *  init headers.
     *-----------------------------------------------------------------------------*/
    if (init_header (&bmp_file, &bmp_info, bit) < 0) {
        exit (EXIT_FAILURE);
    }

    /*-----------------------------------------------------------------------------
     *  write to bmp file.
     *-----------------------------------------------------------------------------*/
    fp_file_name = argv [2];		/* input-file name    */

    fp	= fopen( fp_file_name, "ab" );
    if ( fp == NULL ) {
        fprintf ( stderr, "couldn't open file '%s'; %s\n",
                fp_file_name, strerror(errno) );
        exit (EXIT_FAILURE);
    }

    write_header (fp, &bmp_file, &bmp_info);
    if (write_pixels (fp, buf, bit)) {
        exit (EXIT_FAILURE);
    }

    if( fclose(fp) == EOF ) {			/* close output file   */
        fprintf ( stderr, "couldn't close file '%s'; %s\n",
                fp_file_name, strerror(errno) );
        exit (EXIT_FAILURE);
    }
	return EXIT_SUCCESS;
}		/* ----------  end of function main  ---------- */

