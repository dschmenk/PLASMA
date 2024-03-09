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

#include "config.h"
#include "lib6502.h"

#define VERSION	PACKAGE_NAME " " PACKAGE_VERSION " " PACKAGE_COPYRIGHT

typedef uint8_t  byte;
typedef uint16_t word;

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

int cffa1(M6502 *mpu, word address, byte data)
{
  switch (mpu->registers->x)
  {
    case 0x7A:	/* perform keyboard scan */
      mpu->registers->x= 0x00;
      break;

    default:
      {
	      char state[64];
	      M6502_dump(mpu, state);
	      fflush(stdout);
	      fprintf(stderr, "\nCFFA1 %s\n", state);
	      fail("ABORT");
      }
      break;
  }
  rts;
}

int bye(M6502 *mpu, word addr, byte data)	{ exit(0); return 0; }
int cout(M6502 *mpu, word addr, byte data)	{ putchar(mpu->registers->a); fflush(stdout); rts; }

int rd6820kbdctl(M6502 *mpu, word addr, byte data) { return 0x80; }
int rd6820vidctl(M6502 *mpu, word addr, byte data) { return 0x00; }
int rd6820kbd(M6502 *mpu, word addr, byte data)	   { return getchar(); }
int wr6820vid(M6502 *mpu, word addr, byte data)	   { putchar(data); fflush(stdout); return 0; }

int setTraps(M6502 *mpu)
{
  /* Apple 1 memory-mapped IO */
  M6502_setCallback(mpu, read,  0xD010, rd6820kbd);
  M6502_setCallback(mpu, read,  0xD011, rd6820kbdctl);
  M6502_setCallback(mpu, write, 0xD012, wr6820vid);
  M6502_setCallback(mpu, read,  0xD013, rd6820vidctl);
  /* CFFA1 and ROM calls */
  M6502_setCallback(mpu, call, 0x9000, bye);
  M6502_setCallback(mpu, call, 0x900C, cffa1);
  M6502_setCallback(mpu, call, 0xFFEF, cout);
  return 0;
}

int main(int argc, char **argv)
{
  char *interpfile = "A1PLASMA";
  M6502 *mpu = M6502_new(0, 0, 0);

  if (argc == 2)
    interpfile = argv[1];
  if (!load(mpu, 0x280, interpfile))
    pfail(interpfile);
  setTraps(mpu);
  M6502_reset(mpu);
  M6502_run(mpu);
  M6502_delete(mpu);
  return 0;
}
