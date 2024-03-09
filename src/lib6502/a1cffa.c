/* run6502.c -- 6502 emulator shell			-*- C -*- */

/* Copyright (c) 2005 Ian Piumarta
 * 
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the 'Software'),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, provided that the above copyright notice(s) and this
 * permission notice appear in all copies of the Software and that both the
 * above copyright notice(s) and this permission notice appear in supporting
 * documentation.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.
 */

/* Last edited: 2005-11-02 01:18:58 by piumarta on margaux.local
 */

/* Apple 1 + CFFA1 support for PLASMA by resman
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>

#include "config.h"
#include "lib6502.h"

#define VERSION	PACKAGE_NAME " " PACKAGE_VERSION " " PACKAGE_COPYRIGHT

typedef uint8_t  byte;
typedef uint16_t word;

int keycode_to_a2code[128] = 
{
    -1,   // KEY_RESERVED
    0x35, // KEY_ESC		
    0x12, // KEY_1
    0x13, // KEY_2
    0x14, // KEY_3
    0x15, // KEY_4
    0x17, // KEY_5
    0x16, // KEY_6
    0x1A, // KEY_7
    0x1C, // KEY_8
    0x19, // KEY_9
    0x1D, // KEY_0
    0x1B, // KEY_MINUS
    0x18, // KEY_EQUAL
    0x3B, // KEY_BACKSPACE0
    0x30, // KEY_TAB
    0x0C, // KEY_Q
    0x0D, // KEY_W
    0x0E, // KEY_E
    0x0F, // KEY_R
    0x11, // KEY_T
    0x10, // KEY_Y
    0x20, // KEY_U
    0x22, // KEY_I
    0x1F, // KEY_O
    0x23, // KEY_P
    0x21, // KEY_LEFTBRACE
    0x1E, // KEY_RIGHTBRACE
    0x24, // KEY_ENTER
    0x36, // KEY_LEFTCTRL
    0x00, // KEY_A
    0x01, // KEY_S
    0x02, // KEY_D
    0x03, // KEY_F
    0x05, // KEY_G
    0x04, // KEY_H
    0x26, // KEY_J
    0x28, // KEY_K
    0x25, // KEY_L
    0x29, // KEY_SEMICOLON
    0x27, // KEY_APOSTROPHE
    0x32, // KEY_GRAVE
    0x38, // KEY_LEFTSHIFT
    0x2A, // KEY_BACKSLASH
    0x06, // KEY_Z
    0x07, // KEY_X
    0x08, // KEY_C
    0x09, // KEY_V
    0x0B, // KEY_B
    0x2D, // KEY_N
    0x2E, // KEY_M
    0x2B, // KEY_COMMA
    0x2F, // KEY_DOT
    0x2C, // KEY_SLASH
    0x38, // KEY_RIGHTSHIFT
    0x43, // KEY_KPASTERISK
    0x37, // KEY_LEFTALT
    0x31, // KEY_SPACE
    0x39, // KEY_CAPSLOCK
    0x7A, // KEY_F1
    0x78, // KEY_F2
    0x63, // KEY_F3
    0x76, // KEY_F4
    0x60, // KEY_F5
    0x61, // KEY_F6
    0x62, // KEY_F7
    0x64, // KEY_F8
    0x65, // KEY_F9
    0x6D, // KEY_F10
    0x47, // KEY_NUMLOCK
    0x37, // KEY_SCROLLLOCK
    0x59, // KEY_KP7
    0x5B, // KEY_KP8
    0x5C, // KEY_KP9
    0x4E, // KEY_KPMINUS
    0x56, // KEY_KP4
    0x57, // KEY_KP5
    0x58, // KEY_KP6
    0x45, // KEY_KPPLUS
    0x53, // KEY_KP1
    0x54, // KEY_KP2
    0x55, // KEY_KP3
    0x52, // KEY_KP0
    0x41, // KEY_KPDOT
    -1,    
    -1,   // KEY_ZENKAKUHANKAKU
    -1,   // KEY_102ND
    0x67, // KEY_F11
    0x6F, // KEY_F12
    -1,   // KEY_RO
    -1,   // KEY_KATAKANA
    -1,   // KEY_HIRAGANA
    -1,   // KEY_HENKAN
    -1,   // KEY_KATAKANAHIRAGANA
    -1,   // KEY_MUHENKAN
    -1,   // KEY_KPJPCOMMA
    0x4C, // KEY_KPENTER
    0x36, // KEY_RIGHTCTRL
    0x4B, // KEY_KPSLASH
    0x7F, // KEY_SYSRQ
    0x37, // KEY_RIGHTALT
    0x6E, // KEY_LINEFEED
    0x73, // KEY_HOME
    0x3E, // KEY_UP
    0x74, // KEY_PAGEUP
    0x3B, // KEY_LEFT
    0x3C, // KEY_RIGHT
    0x77, // KEY_END
    0x3D, // KEY_DOWN
    0x79, // KEY_PAGEDOWN
    0x72, // KEY_INSERT
    0x33, // KEY_DELETE
    -1,   // KEY_MACRO
    -1,   // KEY_MUTE
    -1,   // KEY_VOLUMEDOWN
    -1,   // KEY_VOLUMEUP
    0x7F, // KEY_POWER	/* SC System Power Down */
    0x51, // KEY_KPEQUAL
    0x4E, // KEY_KPPLUSMINUS
    -1,   // KEY_PAUSE
    -1,   // KEY_SCALE	/* AL Compiz Scale (Expose) */    
    0x2B, // KEY_KPCOMMA
    -1,   // KEY_HANGEUL
    -1,   // KEY_HANJA
    -1,   // KEY_YEN
    0x3A, // KEY_LEFTMETA
    0x3A, // KEY_RIGHTMETA
    -1    // KEY_COMPOSE
};
struct termios org_tio;

void pfail(const char *msg)
{
  fflush(stdout);
  perror(msg);
  exit(1);
}

#define rts							\
  {								\
    word pc;							\
    pc  = mpu->memory[++mpu->registers->s + 0x100];		\
    pc |= mpu->memory[++mpu->registers->s + 0x100] << 8;	\
    return pc + 1;						\
  }

int save(M6502 *mpu, word address, unsigned length, const char *path)
{
  FILE *file;
  int   count;
  if (!(file = fopen(path, "wb")))
    return 0;
  while ((count = fwrite(mpu->memory + address, 1, length, file)))
    {
      address += count;
      length -= count;
    }
  fclose(file);
  return 1;
}

int load(M6502 *mpu, word address, const char *path)
{
  FILE  *file;
  int    count;
  size_t max   = 0x10000 - address;
  if (!(file = fopen(path, "rb")))
    return 0;
  while ((count = fread(mpu->memory + address, 1, max, file)) > 0)
  {
    address += count;
    max -= count;
  }
  fclose(file);
  return 1;
}

//
// CFFA1 zero page addresses.
//
#define CFFADest     0x00
#define CFFAFileName 0x02
#define CFFAOldName  0x04
#define CFFAFileType 0x06
#define CFFAAuxType  0x07
#define CFFAFileSize 0x09
#define CFFAEntryPtr 0x0B

int cffa1(M6502 *mpu, word address, byte data)
{
  char *fileptr, filename[64];
  int addr;
  struct stat sbuf;

  switch (mpu->registers->x)
  {
    case 0x02:	/* quit */
      exit(0);
      break;
    case 0x14:	/* find dir entry */
      addr = mpu->memory[CFFAFileName] | (mpu->memory[CFFAFileName + 1] << 8);
      memset(filename, 0, 64);
      strncpy(filename, (char *)(mpu->memory + addr + 1), mpu->memory[addr]);
      strcat(filename, "#FE1000");
      if (!(stat(filename, &sbuf)))
      {
        /* DirEntry @ $9100 */
        mpu->memory[CFFAEntryPtr]     = 0x00;
        mpu->memory[CFFAEntryPtr + 1] = 0x91;
        mpu->memory[0x9115] = sbuf.st_size;
        mpu->memory[0x9116] = sbuf.st_size >> 8;
        mpu->registers->a = 0;
      }
      else
        mpu->registers->a = -1;
      break;
    case 0x22:	/* load file */
      addr = mpu->memory[CFFAFileName] | (mpu->memory[CFFAFileName + 1] << 8);
      memset(filename, 0, 64);
      strncpy(filename, (char *)(mpu->memory + addr + 1), mpu->memory[addr]);
      strcat(filename, "#FE1000");
      addr = mpu->memory[CFFADest] | (mpu->memory[CFFADest + 1] << 8);
      mpu->registers->a = load(mpu, addr, filename) - 1;
      break;
    default:
      {
	      char state[64];
        fprintf(stderr, "Unimplemented CFFA function: %02X\n", mpu->registers->x);
	      M6502_dump(mpu, state);
	      fflush(stdout);
	      fprintf(stderr, "\nCFFA1 %s\n", state);
	      pfail("ABORT");
      }
      break;
  }
  rts;
}

int bye(M6502 *mpu, word addr, byte data)	{ exit(0); return 0; }
int cout(M6502 *mpu, word addr, byte data)	{ if (mpu->registers->a == 0x8D) putchar('\n'); putchar(mpu->registers->a & 0x7F); fflush(stdout); rts; }

unsigned keypending = 0;
unsigned char keypressed(void)
{
  unsigned char cin;
  if (read(STDIN_FILENO, &cin, 1) > 0)
    keypending = cin | 0x80;
  return keypending & 0x80;
}
unsigned char keyin(void)
{
  unsigned char cin;

  if (!keypending)
    keypressed();
  cin = keypending;
  keypending = 0;
  return cin;
}
int rd6820kbdctl(M6502 *mpu, word addr, byte data) { return keypressed(); }
int rd6820vidctl(M6502 *mpu, word addr, byte data) { return 0x00; }
int rd6820kbd(M6502 *mpu, word addr, byte data)	   { return keyin(); }
int rd6820vid(M6502 *mpu, word addr, byte data)	   { return 0x80; }
int wr6820vid(M6502 *mpu, word addr, byte data)	   { if (data == 0x8D) putchar('\n'); putchar(data & 0x7F); fflush(stdout); return 0; }

int setTraps(M6502 *mpu)
{
  /* Apple 1 memory-mapped IO */
  M6502_setCallback(mpu, read,  0xD010, rd6820kbd);
  M6502_setCallback(mpu, read,  0xD011, rd6820kbdctl);
  M6502_setCallback(mpu, read,  0xD012, rd6820vid);
  M6502_setCallback(mpu, write, 0xD012, wr6820vid);
  M6502_setCallback(mpu, read,  0xD013, rd6820vidctl);
  /* CFFA1 and ROM calls */
  M6502_setCallback(mpu, call, 0x9000, bye);
  M6502_setCallback(mpu, call, 0x900C, cffa1);
  M6502_setCallback(mpu, call, 0xFFEF, cout);
  return 0;
}

void resetInput(void)
{
  tcsetattr(STDIN_FILENO, TCSANOW, &org_tio);
}
void setRawInput(void)
{
  struct termios termio;

  // Save input settings.
  tcgetattr(STDIN_FILENO, &termio); /* save current port settings */
  memcpy(&org_tio, &termio, sizeof(struct termios));
  termio.c_cflag     = /*BAUDRATE | CRTSCTS |*/ CS8 | CLOCAL | CREAD;
  termio.c_iflag     = IGNPAR;
  termio.c_oflag     = 0;
  termio.c_lflag     = 0; /* set input mode (non-canonical, no echo,...) */
  termio.c_cc[VTIME] = 0; /* inter-character timer unused */
  termio.c_cc[VMIN]  = 0; /* non-blocking read */
  tcsetattr(STDIN_FILENO, TCSANOW, &termio);
  atexit(resetInput);
}
int main(int argc, char **argv)
{
  char *interpfile = "A1PLASMA#060280";
  char *program= argv[0];
  M6502 *mpu = M6502_new(0, 0, 0);

  while (++argv, --argc > 0)
  {
    if (!strcmp(*argv, "-t")) mpu->flags |= M6502_SingleStep;
    else
      if (argv[0][0] != '-') interpfile = *argv;
      else fprintf(stderr, "Usage: %s [-t] [interpreter file]", program);
  }
  if (!load(mpu, 0x280, interpfile))
    pfail(interpfile);
  setRawInput();
  setTraps(mpu);
  M6502_reset(mpu);
  mpu->registers->pc = 0x0280;
  while (M6502_run(mpu))
  {
    char state[64];
    char insn[64];
    M6502_dump(mpu, state);
    M6502_disassemble(mpu, mpu->registers->pc, insn);
    printf("%s : %s\n", state, insn);
  }
  M6502_delete(mpu);
  return 0;
}
