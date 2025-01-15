/*
 * GS font converter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdbit.h>
#include <stdint.h>

#define FONT_WIDTH  32
#define FONT_HEIGHT 12
#define GLYPH_FIRST 32
#define GLYPH_LAST  127
#define GLYPH_COUNT (GLYPH_LAST-GLYPH_FIRST+1)
struct macfont {
    uint16_t fontType;
    uint16_t firstChar;
    uint16_t lastChar;
    uint16_t widMax;
    uint16_t kernMax;
    uint16_t nDescent;
    uint16_t rectWidth;
    uint16_t rectHeight;
    uint16_t owTLoc;
    uint16_t ascent;
    uint16_t descent;
    uint16_t leading;
    uint16_t rowWords;
    uint16_t bitImage; // [rowWords * rectHeight] (fonstStrike)
    //uint16_t rlocTable; // lastChar - firstChar + 3 : pixel offset of glyph in bitImage
    //uint16_t owTable; lastChar - firstChar + 3 : offset/width table
};
struct gsfont *gsf;
struct macfont *macf;
int16_t *locTable;
uint16_t *owTable;
/*
 * Bit reversals
 */
unsigned char bitReverse[256];
unsigned char clrSwap[256];
unsigned char clrRot[] = {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,
                           0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0F};
void write_glyph(FILE *fp, int c)
{
    uint8_t glyphdef[5], *swapbuf, *glyphBits;
    uint32_t rowbits;
    int pixOffset, pixWidth, byteWidth, byteOffset, bitShift, bitWidth, glyphSpan,  i;
    int left, top, width, height;

    if (c < macf->firstChar || c > macf->lastChar)
    {
        memset(glyphdef, 0, 5);
        fwrite(&glyphdef, 1, 5, fp);
        return;
    }
    c -= macf->firstChar;
    glyphBits   = (uint8_t *)&(macf->bitImage),
    glyphSpan   = macf->rowWords * 2;
    height      = macf->rectHeight;
    width       = owTable[c] & 0xFF;
    left        = owTable[c] >> 8;
    top         = macf->ascent;

    glyphdef[0] = left;
    glyphdef[1] = -top;
#if PIXFONT
    glyphdef[2] = (width + 3) / 4;
#else
    glyphdef[2] = width;
#endif
    glyphdef[3] = height;
    glyphdef[4] = width + left;
    fwrite(&glyphdef, 1, 5, fp);

    pixOffset  = locTable[c];
    pixWidth   = locTable[c+1] - locTable[c];
    byteOffset = pixOffset / 8;
    bitShift   = pixOffset & 7;
    bitWidth   = bitShift + pixWidth;
    byteWidth  = (width + 7) / 8;
    rowbits    = 0;
    while (height--)
    {
        if (pixWidth > 0)
        {
            rowbits = bitReverse[glyphBits[byteOffset]];
                if (bitWidth > 8)
            {
                rowbits |= bitReverse[glyphBits[byteOffset + 1]] << 8;
                if (bitWidth > 16)
                {
                    rowbits |= bitReverse[glyphBits[byteOffset + 2]] << 16;
                    if (bitWidth > 24)
                        rowbits |= bitReverse[glyphBits[byteOffset + 3]] << 24;
                }
            }
            rowbits >>= bitShift;
            rowbits &= 0xFFFFFFFF >> (32 - pixWidth);
        }
#if PIXFONT
        glyphdef[0] = clrSwap[rowbits         & 0xFF];
        glyphdef[1] = clrSwap[(rowbits >> 8)  & 0xFF];
        glyphdef[2] = clrSwap[(rowbits >> 16) & 0xFF];
        glyphdef[3] = clrSwap[(rowbits >> 24) & 0xFF];
#else
        glyphdef[0] = rowbits;
        glyphdef[1] = rowbits >> 8;
        glyphdef[2] = rowbits >> 16;
        glyphdef[3] = rowbits >> 24;
#endif
        fwrite(&glyphdef, 1, byteWidth, fp);
        glyphBits += glyphSpan;
    }
}
void write_font_file(char *filename, char *fontfile)
{
    FILE *fp;
    unsigned char ch;
    char *fontname, *scanname, font_header[16];
    int glyph_width, glyph_height, c;

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
#if PIXFONT
        font_header[0] |= 0x80 | 0x40; // FONT_PROP | FONT_PIXMAP
#else
        font_header[0] |= 0x80; // FONT_PROP
#endif
        fwrite(font_header, 1, 16, fp);
        glyph_width  = macf->rectWidth;
        glyph_height = macf->rectHeight;
        ch = GLYPH_FIRST;
        fwrite(&ch, 1, 1, fp);
        ch = GLYPH_LAST;
        fwrite(&ch, 1, 1, fp);
#if PIXFONT
        ch = (glyph_width + 3) / 4;
#else
        ch = glyph_width;
#endif
        fwrite(&ch, 1, 1, fp);
        ch = glyph_height;
        fwrite(&ch, 1, 1, fp);
        for (c = GLYPH_FIRST; c <= GLYPH_LAST; c++)
            write_glyph(fp, c);
        fclose(fp);
    }
}
void die(char *str)
{
    fprintf(stderr,"error: %s\n", str);
    exit(-1);
}
void dispGlyph(uint8_t *glyphBits, int glyphSpan, int width, int height)
{
    uint32_t rowbits;
    int byteOffset, bitShift, bitWidth,  i;

    while (height--)
    {
        rowbits = glyphBits[0];
        if (width > 8)
        {
            rowbits |= glyphBits[1] << 8;
            if (width > 16)
            {
                rowbits |= glyphBits[2] << 16;
                if (width > 24)
                    rowbits |= glyphBits[3] << 24;
            }
        }
        for (i = 0; i < width; i++)
        {
            putchar(rowbits & 1 ? '*' : ' ');
            rowbits >>= 1;
        }
        puts("");
        glyphBits += glyphSpan;
    }
}
void load_dazzledraw_file(char *filename)
{
    FILE *fp;
    uint8_t *fontdef;
    int fontdefsize, c, i, s;
    char msg[] = "Hello, world";

    if ((fp = fopen(filename, "rb")))
    {
        fontdef = malloc(65536);
        fontdefsize = fread(fontdef, 1, 65536, fp);
        fclose(fp);
    }
    else
        die("Unable to open DazzleDraw font file");
    locTable = (int16_t *)fontdef;
    for (c = 32; c < 127; c++)
    {
        i = locTable[c] - 0x1000 + 0x100;
        printf("%c[%3d]: $%04X - $%04X (%d)\n", c, c, i, locTable[c + 1] - 1, locTable[c + 1] - locTable[c]);
        printf("\t$%02X, %d, %d [%d]\n", fontdef[i], fontdef[i + 1], fontdef[i + 2], (fontdef[i] & 0x0F) * fontdef[i + 1] + 3);
        dispGlyph(fontdef + i + 4, fontdef[i + 1], fontdef[i + 2], fontdef[i] & 0x0F);
    }
}
int main(int argc, char **argv)
{
    char *dd_file, *font_file;
    int glyph_width, glyph_height;
    int i;

    font_file = NULL;
    if (argc > 1)
    {
        dd_file = argv[1];
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
        bitReverse[i] = ((i & 0x01) << 7)
                      | ((i & 0x02) << 5)
                      | ((i & 0x04) << 3)
                      | ((i & 0x08) << 1)
                      | ((i & 0x10) >> 1)
                      | ((i & 0x20) >> 3)
                      | ((i & 0x40) >> 5)
                      | ((i & 0x80) >> 7);
        clrSwap[i] =  clrRot[i & 0x0F]
                   | (clrRot[i >> 4] << 4);
    }
    /*
     * Load IIGS font file header.
     */
    load_dazzledraw_file(dd_file);
    //write_font_file(font_file, "DazzleDraw");
    return 0;
}

