/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FONT_WIDTH  32
#define FONT_HEIGHT 12
#define GLYPH_FIRST 32
#define GLYPH_LAST  127
#define GLYPH_COUNT (GLYPH_LAST-GLYPH_FIRST+1)
/*
 * Bit reversals
 */
unsigned char clrSwap[256];
unsigned char clrRot[] = {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,
                           0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0F};
void write_glyph(FILE *fp, int left, int top, int width, int height, int advance, unsigned char *buf, int pitch)
{
    unsigned char glyphdef[5], *swapbuf;
    int i;

    glyphdef[0] = left;
    glyphdef[1] = -top;
    glyphdef[2] = (width + 3) / 4;
    glyphdef[3] = height;
    glyphdef[4] = advance;
    fwrite(&glyphdef, 1, 5, fp);
    swapbuf = malloc(pitch * height);
    for (i = 0; i < pitch * height; i++)
        swapbuf[i] = clrSwap[buf[i]];
    buf = swapbuf;
    while (height--)
    {
        fwrite(buf, 1, (width + 7) / 8, fp);
        buf = buf + pitch;
    }
    free(swapbuf);
}
void write_font_file(char *filename, int glyph_width, int glyph_height)
{
    FILE *fp;
    unsigned char ch;
    int c;

    if ((fp = fopen(filename, "wb")))
    {
        ch = GLYPH_FIRST;
        fwrite(&ch, 1, 1, fp);
        ch = GLYPH_COUNT;
        fwrite(&ch, 1, 1, fp);
        ch = (glyph_width + 3) / 4;
        fwrite(&ch, 1, 1, fp);
        ch = glyph_height;
        fwrite(&ch, 1, 1, fp);
        for (c = GLYPH_FIRST; c <= GLYPH_LAST; c++)
        {
#if 0
            FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
            write_glyph(fp,
                        face->glyph->bitmap_left,
                        face->glyph->bitmap_top,
                        face->glyph->bitmap.width,
                        face->glyph->bitmap.rows,
                        (face->glyph->advance.x + 0x40) >> 6,
                        face->glyph->bitmap.buffer,
                        face->glyph->bitmap.pitch);
#endif
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
void write_bitmap_file(char *filename, int glyph_width, int glyph_height)
{
    int bitmap_top, bitmap_rows;
    int c, glyph_base, glyph_byte_width, glyph_pitch;
    unsigned char *glyph_buf, *bitmap_buf;
    FILE *fp;
#if 0
    glyph_base       = glyph_height * 3 / 4 ; //+ 1;
    glyph_width      = glyph_width  * 1.25;
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
#endif
}
void die(char *str)
{
    fprintf(stderr,"error: %s\n", str);
    exit(-1);
}
void load_iigs_file(char *filename)
{
    FILE *fp;
    char *fontdef;
    char fontname[64];
    int fontdefsize;

    if ((fp = fopen(filename, "rb")))
    {
        fontdef = malloc(65536);
        fontdefsize = fread(fontdef, 1, 65536, fp);
        fclose(fp);
    }
    else
        die("Unable to open IIGS font file");
    memcpy(&fontname, fontdef + 1, fontdef[0]);
    fontname[fontdef[0]] = '\0';
    printf("Font name: %s\n", fontname);
}
int main(int argc, char **argv)
{
    char *iigs_file, *font_file;
    unsigned char *font_buf;
    int glyph_width, glyph_height, font_pitch;
    unsigned char bitReverse;
    int i;

    font_file = NULL;
    if (argc > 1)
    {
        iigs_file = argv[1];
        if (argc > 2)
            font_file = argv[2];
        if (argc > 3)
            glyph_width = atoi(argv[3]);
        else
            glyph_width = FONT_WIDTH;
        if (argc > 4)
            glyph_height = atoi(argv[4]);
        else
            glyph_height = FONT_HEIGHT;
    }
    else
        die( "Missing font file");
    /*
     * Bit/color reversal
     */
    for (i = 0; i < 256; i++)
    {
        bitReverse = ((i & 0x01) << 7)
                   | ((i & 0x02) << 5)
                   | ((i & 0x04) << 3)
                   | ((i & 0x08) << 1)
                   | ((i & 0x10) >> 1)
                   | ((i & 0x20) >> 3)
                   | ((i & 0x40) >> 5)
                   | ((i & 0x80) >> 7);
        clrSwap[i] =  clrRot[bitReverse & 0x0F]
                   | (clrRot[bitReverse >> 4] << 4);
    }
    /*
     * Load IIGS font file header.
     */
    load_iigs_file(iigs_file);
    write_bitmap_file(font_file, glyph_width, glyph_height);
    //write_font_file(font_file, face, glyph_width, glyph_height);
    return 0;
}

