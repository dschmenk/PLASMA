/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freetype2/ft2build.h>
#include <freetype/freetype.h>
//#include FT_FREETYPE2_H
#define FONT_WIDTH 12
#define FONT_HEIGHT 20
/*
 * Glyph render routines.
 */
unsigned char *glyph_buf;
int glyph_width, glyph_height, glyph_base, glyph_pitch;
void pixblt(unsigned char *srcbuf, int srcpitch, unsigned char *dstbuf, int dstpitch, int width, int height)
{
    int i, j;

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
    pixblt(glyph_buf + (string[c] - 32) * glyph_width,
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
    glyph_pitch = glyph_width * (128-32);
    glyph_buf = malloc(glyph_pitch * glyph_height);
    memset(glyph_buf, 0, glyph_pitch * glyph_height);
    for (c = 32; c < 128; c++)
    {
    FT_Load_Char(face, c, FT_LOAD_RENDER);
    glyph_top = glyph_base - face->glyph->bitmap_top;
    if (glyph_top > 0 && glyph_top < glyph_height)
        pixblt(face->glyph->bitmap.buffer,
           face->glyph->bitmap.pitch,
           glyph_buf + (c - 32) * glyph_width + face->glyph->bitmap_left + glyph_top * glyph_pitch,
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
int main(int argc, char **argv)
{
    char *fontfile;
    FT_Library ft;
    FT_Face face;

    if (argc > 1)
        fontfile = argv[1];
    else
        die( "Missing font file");
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
    glyph_base = glyph_height * 3 / 4;
    create_font_glyphs(face);

    return 0;
}

