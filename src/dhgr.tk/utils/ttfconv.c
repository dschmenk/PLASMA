/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <freetype2/ft2build.h>
//#include FT_FREETYPE2_H
#include <freetype2/freetype/freetype.h>

#define FONT_WIDTH  32
#define FONT_HEIGHT 12
#define GLYPH_FIRST 32
#define GLYPH_LAST  127
#define GLYPH_COUNT (GLYPH_LAST-GLYPH_FIRST+1)
#define FONT_BITMAP 0
#define FONT_PIXMAP 1
int fontFormat = FONT_PIXMAP;
/*
 * Bit reversals
 */
unsigned char bitReverse[256];
unsigned char clrSwap[256];
unsigned char clrRot[] = {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,
                           0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0F};
void write_glyph(FILE *fp, int left, int top, int width, int height, int advance, unsigned char *buf, int pitch)
{
    unsigned char glyphdef[5], *swapbuf;
    int i;

    glyphdef[0] = left;
    glyphdef[1] = -top;
    glyphdef[2] = fontFormat == FONT_PIXMAP ? (width + 3) / 4 : width;
    glyphdef[3] = height;
    glyphdef[4] = advance;
    fwrite(&glyphdef, 1, 5, fp);
    swapbuf = malloc(pitch * height);
    if (fontFormat == FONT_PIXMAP)
    {
        for (i = 0; i < pitch * height; i++)
            swapbuf[i] = clrSwap[buf[i]];
    }
    else
    {
        for (i = 0; i < pitch * height; i++)
            swapbuf[i] = bitReverse[buf[i]];
    }
    buf = swapbuf;
    while (height--)
    {
        fwrite(buf, 1, (width + 7) / 8, fp);
        buf = buf + pitch;
    }
    free(swapbuf);
}
void write_font_file(char *filename, char *fontfile, FT_Face face, int glyph_width, int glyph_height)
{
    FILE *fp;
    unsigned char ch;
    char *fontname, *scanname, font_header[16];
    int c;

    if ((fp = fopen(filename, "wb")))
    {
        fontname = scanname = fontfile;
        while (*scanname) // Strip leading directory names
        {
            if (*scanname == '/')
                fontname = scanname + 1;
            scanname++;
        }
        while (scanname > fontfile)
        {
            if (*scanname == '.')
            {
                *scanname = '\0';
            }
            scanname--;
        }
        memset(font_header, 0, 16);
        strncpy(&font_header[1], fontname, 15);
        ch = strlen(fontname);
        font_header[0] = ch < 16 ? ch : 15;
        font_header[0] |= fontFormat == FONT_PIXMAP ? 0x80 | 0x40 : 0x80; // FONT_PROP
        fwrite(font_header, 1, 16, fp);
        ch = GLYPH_FIRST;
        fwrite(&ch, 1, 1, fp);
        ch = GLYPH_LAST;
        fwrite(&ch, 1, 1, fp);
        ch = fontFormat == FONT_PIXMAP ? (glyph_width + 3) / 4 : glyph_width;
        fwrite(&ch, 1, 1, fp);
        ch = glyph_height;
        fwrite(&ch, 1, 1, fp);
        for (c = GLYPH_FIRST; c <= GLYPH_LAST; c++)
        {
            FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
            write_glyph(fp,
                        face->glyph->bitmap_left,
                        face->glyph->bitmap_top,
                        face->glyph->bitmap.width,
                        face->glyph->bitmap.rows,
                        face->glyph->advance.x >> 6,
                        face->glyph->bitmap.buffer,
                        face->glyph->bitmap.pitch);
        }
        fclose(fp);
    }
}
/*
 * Glyph render routines.
 */
void bitblt(int xorg, int yorg, int width, int height, unsigned char *srcbuf, int srcpitch, unsigned char *dstbuf, int dstpitch)
{
    int i, j;

    dstbuf = dstbuf + yorg * dstpitch;
    width = (width + 7) / 8; // Width in bytes
    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
            dstbuf[i] = srcbuf[i];
        srcbuf += srcpitch;
        dstbuf += dstpitch;
    }
}
void write_bitmap_file(char *filename, FT_Face face, int glyph_width, int glyph_height)
{
    int bitmap_top, bitmap_rows;
    int c, glyph_base, glyph_byte_width, glyph_pitch;
    unsigned char *glyph_buf, *bitmap_buf;
    FILE *fp;

    glyph_base       = glyph_height * 3 / 4 ; //+ 1;
    glyph_width      = glyph_width  * 1.25;
    //glyph_height    = glyph_height * 1.25 + 1;
    glyph_byte_width = (glyph_width + 7) / 8;
    glyph_pitch      = GLYPH_COUNT * glyph_byte_width;
    glyph_buf        = malloc(glyph_pitch * glyph_height);
    memset(glyph_buf, 0, glyph_pitch * glyph_height);
    for (c = GLYPH_FIRST; c <= GLYPH_LAST; c++)
    {
        FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
        bitmap_buf  = face->glyph->bitmap.buffer,
        bitmap_top  = glyph_base - face->glyph->bitmap_top;
        bitmap_rows = face->glyph->bitmap.rows;
        if (bitmap_top < 0)
        {
            bitmap_rows += bitmap_top;
            bitmap_buf   = bitmap_buf - face->glyph->bitmap.pitch * bitmap_top;
            bitmap_top   = 0;
        }
        if (bitmap_top < glyph_height)
        {
            if (bitmap_top + bitmap_rows > glyph_height)
                bitmap_rows = glyph_height - bitmap_top;
            bitblt(0,
                   bitmap_top,
                   face->glyph->bitmap.width,
                   bitmap_rows,
                   bitmap_buf,
                   face->glyph->bitmap.pitch,
                   glyph_buf + (c - GLYPH_FIRST) * glyph_byte_width,
                   glyph_pitch);
        }
        else
            printf("Glyph (%c) height: %d, top: %d, rows: %d Skipped!\n", c, glyph_height, face->glyph->bitmap_top, face->glyph->bitmap.rows);
    }
    if ((fp = fopen(filename, "wb")))
    {
        fprintf(fp, "P4\n%d\n%d\n", glyph_width * GLYPH_COUNT, glyph_height);
        fwrite(glyph_buf, 1, glyph_pitch * glyph_height, fp);
        fclose(fp);
    }
}
void die(char *str)
{
    fprintf(stderr,"error: %s\n", str);
    exit(-1);
}
int main(int argc, char **argv)
{
    char *ttf_file, *font_file;
    FT_Library ft;
    FT_Face face;
    unsigned char *font_buf;
    int glyph_width, glyph_height, font_pitch;
    int i;

    glyph_width  = FONT_WIDTH;
    glyph_height = FONT_HEIGHT;
    ttf_file  = NULL;
    font_file = NULL;
    while (*++argv)
    {
        if (argv[0][0] == '-')
        {
            switch (toupper(argv[0][1]))
            {
                case 'B':
                    fontFormat = FONT_BITMAP;
                    break;
                case 'P':
                    fontFormat = FONT_PIXMAP;
                    break;
                case 'W':
                    glyph_width = atoi(&argv[0][2]);
                    break;
                case 'H':
                    glyph_height = atoi(&argv[0][2]);
                    break;
            }
        }
        else
        {
            if (!ttf_file)
                ttf_file = *argv;
            else if (!font_file)
                font_file = *argv;
            else
                fprintf(stderr, "? %s\n", *argv);
        }
    }
    if (!ttf_file || !font_file)
        die( "Missing font file");
    /*
     * Bit/color reversal
     */
    for (i = 0; i < 256; i++)
    {
        bitReverse[i] = ((i & 0x01) << 7)
                     | ((i & 0x02) << 5)
                     | ((i & 0x04) << 3)
                     | ((i & 0x08) << 1)
                     | ((i & 0x10) >> 1)
                     | ((i & 0x20) >> 3)
                     | ((i & 0x40) >> 5)
                     | ((i & 0x80) >> 7);
        clrSwap[i] =  clrRot[bitReverse[i] & 0x0F]
                   | (clrRot[bitReverse[i] >> 4] << 4);
    }
    /*
     * Init FreeType library.
     */
    if (FT_Init_FreeType(&ft))
        die("FreeType error");
    if (FT_New_Face(ft, ttf_file, 0, &face))
        die("Missing font");
    if (face->num_fixed_sizes)
    {
        glyph_width  = face->available_sizes[0].width;
        glyph_height = face->available_sizes[0].height;
    }
    //if (glyph_width  < 14) glyph_width  = 14;
    //if (glyph_height < 8)  glyph_height = 8;
    FT_Set_Pixel_Sizes(face, glyph_width, glyph_height);
    //write_bitmap_file(font_file, face, glyph_width, glyph_height);
    write_font_file(font_file, ttf_file, face, glyph_width, glyph_height);
    return 0;
}

