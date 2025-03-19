#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <ctype.h>

/* CGA high resolution */
#define X_RES              560
#define Y_RES              192
/* According to what I could find out about the NTSC color wheel:
 *   Red maxes at 103.5 degrees
 *   Green maxes at 240.7 degrees
 *   Blue maxes at 347.1 degrees
 */
#define RED_PHASE_NTSC     104
#define GREEN_PHASE_NTSC   241
#define BLUE_PHASE_NTSC    347
/* Ideal phase angles for 4 phase */
#define RED_PHASE_IDEAL    90
#define GREEN_PHASE_IDEAL  270
#define BLUE_PHASE_IDEAL   360
/* Equal 120 deg phase angles */
#define RED_PHASE_EQUAL    120
#define GREEN_PHASE_EQUAL  240
#define BLUE_PHASE_EQUAL   360
/* Simplified phase angles: 90 deg between R and B, 135 between RG and BG */
#define RED_PHASE_SIMPLE   90
#define GREEN_PHASE_SIMPLE 225
#define BLUE_PHASE_SIMPLE  360
/* Flags */
#define GREY_HACK          1 /* Remap GREY1 to GREY2 */
#define DUMP_STATE         2 /* Dump internal state */
char flags = GREY_HACK;
/* Handy macros */
#ifndef min
#define min(a,b)           ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b)           ((a)>(b)?(a):(b))
#endif
#define RED                0
#define GRN                1
#define BLU                2
#define DEG_TO_RAD         0.0174533
#define TRUE               1
#define FALSE              0
unsigned char ntscChroma[4][3];
int prevRed, prevBlu, prevGrn;
unsigned char gammaRed[256]; /* RED gamma correction */
unsigned char gammaGrn[256]; /* GREEN gamma correction */
unsigned char gammaBlu[256]; /* BLUE gamma correction */
int phase[3] = {RED_PHASE_NTSC, GREEN_PHASE_NTSC, BLUE_PHASE_NTSC};
int gammacorrect = 0; /* Gamma correction */
int brightness   = 0;
int saturation   = 255; /* 1.0 */
int tint         = 22; /* imperically determined tint */
int orgmode;
int errDiv       = 4;
unsigned char rgbScanline[X_RES * 3]; /* RGB scanline */
int rgbErr[(X_RES + 1) * 3]; /* Running color error array */
#define DHGR_SIZE   16384
unsigned char dhgrScreen[DHGR_SIZE];
int scanOffset[] = {0x0000,0x0400,0x0800,0x0C00,0x1000,0x1400,0x1800,0x1C00,
                    0x0080,0x0480,0x0880,0x0C80,0x1080,0x1480,0x1880,0x1C80,
                    0x0100,0x0500,0x0900,0x0D00,0x1100,0x1500,0x1900,0x1D00,
                    0x0180,0x0580,0x0980,0x0D80,0x1180,0x1580,0x1980,0x1D80,
                    0x0200,0x0600,0x0A00,0x0E00,0x1200,0x1600,0x1A00,0x1E00,
                    0x0280,0x0680,0x0A80,0x0E80,0x1280,0x1680,0x1A80,0x1E80,
                    0x0300,0x0700,0x0B00,0x0F00,0x1300,0x1700,0x1B00,0x1F00,
                    0x0380,0x0780,0x0B80,0x0F80,0x1380,0x1780,0x1B80,0x1F80,
                    0x0028,0x0428,0x0828,0x0C28,0x1028,0x1428,0x1828,0x1C28,
                    0x00A8,0x04A8,0x08A8,0x0CA8,0x10A8,0x14A8,0x18A8,0x1CA8,
                    0x0128,0x0528,0x0928,0x0D28,0x1128,0x1528,0x1928,0x1D28,
                    0x01A8,0x05A8,0x09A8,0x0DA8,0x11A8,0x15A8,0x19A8,0x1DA8,
                    0x0228,0x0628,0x0A28,0x0E28,0x1228,0x1628,0x1A28,0x1E28,
                    0x02A8,0x06A8,0x0AA8,0x0EA8,0x12A8,0x16A8,0x1AA8,0x1EA8,
                    0x0328,0x0728,0x0B28,0x0F28,0x1328,0x1728,0x1B28,0x1F28,
                    0x03A8,0x07A8,0x0BA8,0x0FA8,0x13A8,0x17A8,0x1BA8,0x1FA8,
                    0x0050,0x0450,0x0850,0x0C50,0x1050,0x1450,0x1850,0x1C50,
                    0x00D0,0x04D0,0x08D0,0x0CD0,0x10D0,0x14D0,0x18D0,0x1CD0,
                    0x0150,0x0550,0x0950,0x0D50,0x1150,0x1550,0x1950,0x1D50,
                    0x01D0,0x05D0,0x09D0,0x0DD0,0x11D0,0x15D0,0x19D0,0x1DD0,
                    0x0250,0x0650,0x0A50,0x0E50,0x1250,0x1650,0x1A50,0x1E50,
                    0x02D0,0x06D0,0x0AD0,0x0ED0,0x12D0,0x16D0,0x1AD0,0x1ED0,
                    0x0350,0x0750,0x0B50,0x0F50,0x1350,0x1750,0x1B50,0x1F50,
                    0x03D0,0x07D0,0x0BD0,0x0FD0,0x13D0,0x17D0,0x1BD0,0x1FD0};

void dhgrSet(int x, int y)
{
  int pixbit, pixofst;

  pixbit  = 1 << (x % 7);
  pixofst = x / 7;
  pixofst = (DHGR_SIZE/2) * (pixofst & 1) + (pixofst >> 1);
  dhgrScreen[scanOffset[y] + pixofst] |= pixbit;
}
void dhgrPlot(int x, int y, int clr)
{
  int bit;
  for (bit = 0; bit < 4; bit++)
  {
    if ((1 << bit) & clr)
      dhgrSet(x + bit, y);
  }
}
long int dist(int dr, int dg, int db)
{
  long int rr, gg, bb;

  rr = (long int)dr * (long int)dr;
  gg = (long int)dg * (long int)dg;
  bb = (long int)db * (long int)db;
  return rr + gg + bb;
}

void calcChroma(int angle)
{
  int r, g, b, i;

  for (i = 0; i < 4; i++)
  {
    /* Calculate RGB for this NTSC subcycle pixel */
    r = saturation + (int)(cos((angle - phase[RED]) * DEG_TO_RAD) * 255);
    g = saturation + (int)(cos((angle - phase[GRN]) * DEG_TO_RAD) * 255);
    b = saturation + (int)(cos((angle - phase[BLU]) * DEG_TO_RAD) * 255);
    /* Make chroma add up to white */
    ntscChroma[i][RED] = min(255, max(0, (r + 2) / 4));
    ntscChroma[i][GRN] = min(255, max(0, (g + 2) / 4));
    ntscChroma[i][BLU] = min(255, max(0, (b + 2) / 4));
    /* Next NTSC chroma subcycle pixel */
    angle = angle - 90;
  }
}

int rgbMatchChroma(int r, int g, int b, int *errptr, int cx)
{
  int currRed, currGrn, currBlu;
  int errRed, errGrn, errBlu;
  int pix;

  /* Apply error propogation */
  if (errDiv)
  {
    r += errptr[RED] / errDiv;
    g += errptr[GRN] / errDiv;
    b += errptr[BLU] / errDiv;
  }
  /* Previous RGB chroma subcycles */
  prevRed = (prevRed * 3) / 4;
  prevGrn = (prevGrn * 3) / 4;
  prevBlu = (prevBlu * 3) / 4;
  /* Current potential RGB subcycle */
  currRed = prevRed + ntscChroma[cx][RED];
  currGrn = prevGrn + ntscChroma[cx][GRN];
  currBlu = prevBlu + ntscChroma[cx][BLU];
  /* Match chroma subcycle */
  pix = 0;
  if (dist(r - currRed, g - currGrn, b - currBlu) < dist(r - prevRed, g - prevGrn, b - prevBlu))
  {
    /* RGB better matched with current chroma subcycle color */
    prevRed = currRed;
    prevGrn = currGrn;
    prevBlu = currBlu;
    pix = 1;
  }
  /* Propogate error down (overwrite error at this coordinate for next row) */
  errRed = r - prevRed;
  errGrn = g - prevGrn;
  errBlu = b - prevBlu;
  errptr[RED] = errRed;
  errptr[GRN] = errGrn;
  errptr[BLU] = errBlu;
  /* And forward (add to previous row error for next match) */
  errptr[RED + 3] += errRed;
  errptr[GRN + 3] += errGrn;
  errptr[BLU + 3] += errBlu;
  return pix;
}

int rgbInit(void)
{
  int i;
  long int g32;

  switch (gammacorrect)
  {
    case -1:
      for (i = 0; i < 256; i++)
      {
        g32 = (255 - i + 255 - ((long int)i * (long int)i + 127)/255) / 2;
        gammaRed[255 - i] = g32;
        gammaGrn[255 - i] = g32;
        gammaBlu[255 - i] = g32;
      }
      break;
    case -2:
      for (i = 0; i < 256; i++)
      {
        g32 = 255 - ((long int)i * (long int)i + 127)/255;
        gammaRed[255 - i] = g32;
        gammaGrn[255 - i] = g32;
        gammaBlu[255 - i] = g32;
      }
      break;
    case 2:
      for (i = 0; i < 256; i++)
      {
        g32 = ((long int)i * (long int)i + 127) / 255;
        gammaRed[i] = g32;
        gammaGrn[i] = g32;
        gammaBlu[i] = g32;
      }
      break;
    case 1:
      for (i = 0; i < 256; i++)
      {
        g32 = (i + ((long int)i * (long int)i + 127) / 255) / 2;
        gammaRed[i] = g32;
        gammaGrn[i] = g32;
        gammaBlu[i] = g32;
      }
      break;
    default:
      for (i = 0; i < 256; i++)
      {
        gammaRed[i] = i;
        gammaGrn[i] = i;
        gammaBlu[i] = i;
      }
  }
  if (brightness)
    for (i = 0; i < 256; i++)
    {
      gammaRed[i] = max(0, min(255, gammaRed[i] + brightness));
      gammaGrn[i] = max(0, min(255, gammaGrn[i] + brightness));
      gammaBlu[i] = max(0, min(255, gammaBlu[i] + brightness));
    }
  calcChroma(tint);
  if (flags & DUMP_STATE)
  {
    printf("Err Div = %d\n", errDiv);
    printf("Gamma = %d\n", gammacorrect);
    printf("Brightness = %d\n", brightness);
    printf("Tint = %d\n", tint);
    printf("Saturation = %d\n", saturation);
    puts("Chroma cycle RGB =");
    for (i = 0; i < 4; i++)
      printf("    [%3d, %3d %3d]\n", ntscChroma[i][RED],
                                     ntscChroma[i][GRN],
                                     ntscChroma[i][BLU]);
  }
  return TRUE;
}

char *pnmReadElement(FILE *fp, char *bufptr)
{
  char *endptr;

  endptr = bufptr + 1;
  while (fread(bufptr, 1, 1, fp) == 1 && *bufptr == '#') /* Comment */
    while (fread(endptr, 1, 1, fp) == 1 && *endptr >= ' ');
  while (fread(endptr, 1, 1, fp) == 1 && *endptr > ' ' && (endptr - bufptr < 80))
    endptr++;
  *endptr = '\0';
  if (flags & DUMP_STATE)
    puts(bufptr);
  return bufptr;
}

int pnmVerifyHeader(FILE *fp)
{
  char buf[128];

  if (flags & DUMP_STATE)
    printf("PNM = ");
  pnmReadElement(fp, buf);
  if (strcmp(buf, "P6"))
  {
    printf("Invalid PNM magic #: %c%c\n", buf[0], buf[1]);
    return FALSE;
  }
  if (atoi(pnmReadElement(fp, buf)) != X_RES)
  {
    printf("Width not 640: %s\n", buf);
    return FALSE;
  }
  if (atoi(pnmReadElement(fp, buf)) != Y_RES)
  {
    printf("Height not 192: %s\n", buf);
    return FALSE;
  }
  if (atoi(pnmReadElement(fp, buf)) != 255)
  {
    printf("Depth not 255: %s\n", buf);
    return FALSE;
  }
  return TRUE;
}

int rgbImportExport(char *pnmfile, char *dhgrfile)
{
  FILE *fp;
  unsigned char *scanptr;
  unsigned char chromaBits, *rgbptr;
  int scan, pix, chroma, r, g, b, *errptr;

  if (flags & DUMP_STATE)
    printf("PNM file = %s\n", pnmfile);
  fp = fopen(pnmfile, "rb");
  if (fp)
  {
    if (pnmVerifyHeader(fp) && rgbInit())
    {
      /* Clear screen memory */
      memset(dhgrScreen, 0, DHGR_SIZE);
      /* Init error propogation array */
      memset(rgbScanline, 0, X_RES * 3);
      memset(rgbErr, 0, X_RES * 3 * sizeof(int));
      for (scan = 0; scan < Y_RES; scan++)
      {
        fread(rgbScanline, X_RES, 3, fp);
        /* Reset prev RGB to neutral color */
        prevRed = 96;
        prevGrn = 96;
        prevBlu = 96;
        /* Reset pointers */
        rgbptr  = rgbScanline;
        errptr  = rgbErr;
        for (pix = 0; pix < X_RES; pix += 4)
        {
          chromaBits = 0;
          for (chroma = 0; chroma < 4; chroma++)
          {
            /* Calc best match */
            r = gammaRed[rgbptr[RED]];
            g = gammaGrn[rgbptr[GRN]];
            b = gammaBlu[rgbptr[BLU]];
            chromaBits |= rgbMatchChroma(r, g, b, errptr, chroma) << chroma;
            rgbptr = rgbptr + 3;
            errptr = errptr + 3;
          }
          if (chromaBits == 0x0A && flags & GREY_HACK)
            chromaBits = 0x05;
          dhgrPlot(pix, scan, chromaBits);
        }
      }
    }
    fclose(fp);
    fp = fopen(dhgrfile, "wb");
    if (fp)
    {
        fwrite(dhgrScreen, DHGR_SIZE, 1, fp);
        fclose(fp);
        return 0;
    }
      printf("Unable to open %s\n", dhgrfile);
  }
  printf("Unable to open %s\n", pnmfile);
  return -1;
}

int main(int argc, char **argv)
{
  puts("DHGR RGB converter 1.0");
  while (*++argv && (*argv)[0] == '-')
  {
    switch (toupper((*argv)[1]))
    {
      case 'B': /* Set brightness */
        brightness = atoi(*argv + 2);
        break;
      case 'C': /* No grey remapping*/
        flags &= ~GREY_HACK;
      case 'D': /* Dump internal state */
        flags |= DUMP_STATE;
        break;
      case 'E': /* Set error strength */
        errDiv = atoi(*argv + 2);
        break;
      case 'G': /* Set gamma amount */
        gammacorrect = atoi(*argv + 2);
        break;
      case 'P': /* RGB phase angle */
        switch (toupper((*argv)[2]))
        {
          case 'I': /* Use ideal 4 sub-phase angles */
            phase[RED] = RED_PHASE_IDEAL;
            phase[GRN] = GREEN_PHASE_IDEAL;
            phase[BLU] = BLUE_PHASE_IDEAL;
            break;
          case 'E': /* Use equal 120 deg phase angles */
            phase[RED] = RED_PHASE_EQUAL;
            phase[GRN] = GREEN_PHASE_EQUAL;
            phase[BLU] = BLUE_PHASE_EQUAL;
            break;
          case 'S': /* Use simplified 90 degree and opposite phase angles */
            phase[RED] = RED_PHASE_SIMPLE;
            phase[GRN] = GREEN_PHASE_SIMPLE;
            phase[BLU] = BLUE_PHASE_SIMPLE;
            break;
          /* case 'N': Use theoretical NTSC phase angles */
          default:
            phase[RED] = RED_PHASE_NTSC;
            phase[GRN] = GREEN_PHASE_NTSC;
            phase[BLU] = BLUE_PHASE_NTSC;
            break;
        }
        break;
      case 'S': /* Adjust saturation */
        saturation -= atoi(*argv + 2);
        break;
      case 'T': /* Adjust tint */
        tint += atoi(*argv + 2);
        break;
      default:
        printf("? option: %c\n", (*argv)[1]);
    }
  }
  if (argv[0] && argv[1])
    return rgbImportExport(argv[0], argv[1]);
  puts("Usage:");
  puts(" dhgrrgb");
  puts("  [-b#]  = Brightness: -255..255");
  puts("  [-c]   = Composite output (no grey remapping)");
  puts("  [-d]   = Dump state");
  puts("  [-e#]  = Error strength: 1..255");
  puts("                           0 = no err");
  puts("  [-g#]  = Gamma: 2, 1, 0, -1, -2");
  puts("  [-p<I,E,S,N>] = Phase: Ideal, Equal, Simple, NTSC");
  puts("  [-s#]  = Saturation: -255..255");
  puts("  [-t#]  = Tint: -360..360 (in degrees)");
  puts(" <PNM file> <DHGR file>");
  return 0;
}
