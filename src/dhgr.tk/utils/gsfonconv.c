/*
 * GS font converter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdbit.h>
#include <stdint.h>
#include <ctype.h>

#define FONT_WIDTH  28
#define FONT_HEIGHT 12
#define GLYPH_FIRST 32
#define GLYPH_LAST  127
#define GLYPH_COUNT (GLYPH_LAST-GLYPH_FIRST+1)
#define FONT_AA     0x40
int fontFormat = 0;
struct gsfont {
    uint16_t offsetMacFont;
    uint16_t family;
    uint16_t style;
    uint16_t pointSize;
    uint16_t version;
    uint16_t extent;
};
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
/*
 * White on Black and black is xpar
 */
unsigned char clrRotWonB[] = {0x05,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,
                           0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0F};
/*
 * Black on white and white is xpar
 */
unsigned char clrRotBonW[] = {0x05,0x0D,0x0B,0x09,0x07,0x0A,0x03,0x01,
                              0x0E,0x0C,0x0A,0x08,0x06,0x04,0x02,0x00};
/*
 * Default to black on white
 */
unsigned char *clrRot = clrRotBonW;

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
    glyphdef[2] = fontFormat == FONT_AA ? (width + 3) / 4 : width;
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
        if (fontFormat == FONT_AA)
        {
            glyphdef[0] = clrSwap[rowbits         & 0xFF];
            glyphdef[1] = clrSwap[(rowbits >> 8)  & 0xFF];
            glyphdef[2] = clrSwap[(rowbits >> 16) & 0xFF];
            glyphdef[3] = clrSwap[(rowbits >> 24) & 0xFF];
        }
        else
        {
            glyphdef[0] = rowbits;
            glyphdef[1] = rowbits >> 8;
            glyphdef[2] = rowbits >> 16;
            glyphdef[3] = rowbits >> 24;
        }
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
        font_header[0] = (ch < 16 ? ch : 15) | fontFormat | 0x80; // FONT_PROP
        fwrite(font_header, 1, 16, fp);
        glyph_width  = macf->rectWidth;
        glyph_height = macf->rectHeight;
        ch = GLYPH_FIRST;
        fwrite(&ch, 1, 1, fp);
        ch = GLYPH_LAST;
        fwrite(&ch, 1, 1, fp);
        ch = fontFormat == FONT_AA ? (glyph_width + 3) / 4 : glyph_width;
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
void dispGlyph(uint8_t *glyphBits, int glyphSpan, int offset, int width, int height)
{
    uint32_t rowbits;
    int byteOffset, bitShift, bitWidth,  i;

    byteOffset = offset / 8;
    bitShift   = offset & 7;
    bitWidth   = bitShift + width;
    while (height--)
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
        for (i = 0; i < width; i++)
        {
            putchar(rowbits & 1 ? '*' : ' ');
            rowbits >>= 1;
        }
        puts("");
        glyphBits += glyphSpan;
    }
}
void load_iigs_file(char *filename)
{
    FILE *fp;
    char *fontdef;
    char fontname[64];
    int fontdefsize, c, i;
    char msg[] = "Hello, world";

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
    gsf = (struct gsfont *)(fontdef + fontdef[0] + 1);
    printf("Mac Font offset: %d\n", gsf->offsetMacFont * 2);
    macf = (struct macfont *)((char *)gsf + gsf->offsetMacFont * 2);
    printf("font type: $%04X\n", macf->fontType);
    printf("first char: %d\n", macf->firstChar);
    printf("last char: %d\n", macf->lastChar);
    printf("rect width: %d\n", macf->rectWidth);
    printf("rect height: %d\n", macf->rectHeight);
    printf("ascent: %d\n", macf->ascent);
    printf("descent: %d\n", macf->descent);
    printf("leading: %d\n", macf->leading);
    printf("rowWords: %d rowBytes: %d\n", macf->rowWords, macf->rowWords * 2);
    locTable = (int16_t *)((char *)&(macf->bitImage) + macf->rectHeight * macf->rowWords * 2);
    printf("locTable[0]: $%04X\n", locTable[0]);
    owTable = (uint16_t *)((char *)locTable + (macf->lastChar - macf->firstChar + 3) * 2);
    printf("owTable[0]: $%04X\n", owTable[0]);
    //for (i = macf->firstChar; i <= macf->lastChar; i++)
    for (c = 0; msg[c]; c++)
    {
        i = msg[c];
        printf("Char[%3d] : pix offset = %3d,%1d, pix width = %2d, Origin/Width = $%04X (%3d, %3d)\n", i, locTable[i] / 8, locTable[i] & 7, locTable[i+1] - locTable[i], owTable[i], owTable[i] >> 8, owTable[i] & 0xFF);
        if (owTable[i] != 0xFFFF)
            dispGlyph((uint8_t *)&(macf->bitImage), macf->rowWords * 2, locTable[i], locTable[i+1] - locTable[i], macf->rectHeight);
    }
}
int main(int argc, char **argv)
{
    char *iigs_file, *font_file;
    int glyph_width, glyph_height;
    int i;

    iigs_file = NULL;
    font_file = NULL;
    while (*++argv)
    {
        if (argv[0][0] == '-')
        {
            switch (toupper(argv[0][1]))
            {
                case 'A':
                    fontFormat |= FONT_AA;
                    break;
                case 'B':
                    fontFormat &= ~FONT_AA;
                    break;
                case 'I':
                    clrRot = clrRotWonB;
                    break;
            }
        }
        else
        {
            if (!iigs_file)
                iigs_file = *argv;
            else if (!font_file)
                font_file = *argv;
            else
                fprintf(stderr, "? %s\n", *argv);
        }
    }
    if (!iigs_file || !font_file)
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
    load_iigs_file(iigs_file);
    write_font_file(font_file, iigs_file);
    return 0;
}

