/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freetype2/ft2build.h>
#include <freetype/freetype.h>
//#include FT_FREETYPE2_H
#define FONT_WIDTH  32 // 12
#define FONT_HEIGHT 12 // 20
/*
 * Bit reversals
 */
unsigned char pixReverse[256];
unsigned char clrSwap[256];
unsigned char clrRot[] = {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,
                           0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0F};
/*
 * Glyph render routines.
 */
unsigned char *glyph_buf;
int glyph_width, glyph_byte_width, glyph_height, glyph_base, glyph_pitch;
void bitblt(unsigned char *srcbuf, int srcpitch, unsigned char *dstbuf, int dstpitch, int width, int height)
{
    int i, j;

    width = (width + 7) / 8;
    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
            dstbuf[i] = srcbuf[i];
        srcbuf += srcpitch;
        dstbuf += dstpitch;
    }
}
void strblt(char *string, unsigned char *dstbuf, int dstpitch, int x, int y)
{
    int c, l = strlen(string);
    dstbuf += x + y * dstpitch;
    for (c = 0; c < l; c++)
    {
        bitblt(glyph_buf + (string[c] - 32) * glyph_byte_width,
               glyph_pitch,
               dstbuf,
               dstpitch,
               glyph_width,
               glyph_height);
        dstbuf += glyph_width;
    }
}
void create_font_glyphs(FT_Face face)
{
    int c, glyph_top;

    FT_Set_Pixel_Sizes(face, glyph_width * 1.5, glyph_height);
    glyph_pitch = (128-32+1) * glyph_byte_width;
    glyph_buf = malloc(glyph_pitch * glyph_height);
    memset(glyph_buf, 0, glyph_pitch * glyph_height);
    for (c = 32; c < 128; c++)
    {
        FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
        glyph_top = glyph_base - face->glyph->bitmap_top;
        if (glyph_top > 0 && glyph_top < glyph_height)
            /*
            bitblt(face->glyph->bitmap.buffer,
               face->glyph->bitmap.pitch,
               glyph_buf + ((c - 32) * glyph_width + face->glyph->bitmap_left) / 8 + glyph_top * glyph_pitch,
               glyph_pitch,
               face->glyph->bitmap.width,
               face->glyph->bitmap.rows);
            */
            bitblt(face->glyph->bitmap.buffer, // + face->glyph->bitmap_left,
               face->glyph->bitmap.pitch,
               glyph_buf + (c - 32) * glyph_byte_width + glyph_top * glyph_pitch,
               glyph_pitch,
               face->glyph->bitmap.width,
               face->glyph->bitmap.rows);
    }
}
void die(char *str)
{
    fprintf(stderr,"error: %s\n", str);
    exit(-1);
}
int writeGlyphFile(char *filename, int xorg, int yorg, int width, int height, unsigned char *buf, int pitch)
{
    FILE *fp;
    unsigned char sprdef[4], *swapbuf;
    int i;

    if ((fp = fopen(filename, "wb")))
    {
        //fprintf(fp, "P4\n%d\n%d\n", width, height);
        sprdef[0] = xorg;
        sprdef[1] = yorg;
        sprdef[2] = width/4;
        sprdef[3] = height;
        fwrite(&sprdef, 1, 4, fp);
        swapbuf = malloc(pitch * height);
        for (i = 0; i < pitch * height; i++)
            swapbuf[i] = clrSwap[buf[i]];
        fwrite(swapbuf, 1, pitch * height, fp);
        free(swapbuf);
        fclose(fp);
        return 1;
    }
    return 0;
}
int main(int argc, char **argv)
{
    char *fontfile, *glyph_file;
    FT_Library ft;
    FT_Face face;
    unsigned char pixReverse;
    int i;

    glyph_file = NULL;
    if (argc > 1)
    {
        fontfile = argv[1];
        if (argc > 2)
            glyph_file = argv[2];
    }
    else
        die( "Missing font file");
    /*
     * Bit/color reversal
     */
    for (i = 0; i < 256; i++)
    {
        pixReverse = ((i & 0x01) << 7)
                   | ((i & 0x02) << 5)
                   | ((i & 0x04) << 3)
                   | ((i & 0x08) << 1)
                   | ((i & 0x10) >> 1)
                   | ((i & 0x20) >> 3)
                   | ((i & 0x40) >> 5)
                   | ((i & 0x80) >> 7);
        clrSwap[i] =  clrRot[pixReverse & 0x0F]
                   | (clrRot[pixReverse >> 4] << 4);
    }
    /*
     * Init FreeType library.
     */
    if (FT_Init_FreeType(&ft))
        die("FreeType error");
    if (FT_New_Face(ft, fontfile, 0, &face))
        die("Missing font");
    if (face->num_fixed_sizes)
    {
        glyph_width  = face->available_sizes[0].width;
        glyph_height = face->available_sizes[0].height;
    }
    else
    {
        glyph_width  = FONT_WIDTH;
        glyph_height = FONT_HEIGHT;
    }
    glyph_byte_width = (glyph_width + 7) / 8;
    glyph_base = glyph_height * 3 / 4;
    //create_font_glyphs(face);
    FT_Set_Pixel_Sizes(face, glyph_width /* 1.5*/, glyph_height);
    FT_Load_Char(face, 'A', FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
    if (glyph_file)
    {
        if (!writeGlyphFile(glyph_file,
                           -face->glyph->bitmap_left,
                            face->glyph->bitmap_top,
                            face->glyph->bitmap.width,
                            face->glyph->bitmap.rows,
                            face->glyph->bitmap.buffer,
                            face->glyph->bitmap.pitch))
            die("Error writing glyph file");
    }
    return 0;
}

