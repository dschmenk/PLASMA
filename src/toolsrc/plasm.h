/*
 * Global flags.
 */
#define ACME            (1<<0)
#define MODULE          (1<<1)
#define OPTIMIZE        (1<<2)
#define BYTECODE_SEG    (1<<3)
#define INIT            (1<<4)
#define SYSFLAGS        (1<<5)
#define WARNINGS        (1<<6)
extern int outflags;
#include "tokens.h"
#include "lex.h"
#include "symbols.h"
#include "parse.h"
#include "codegen.h"
