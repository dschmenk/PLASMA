# THIS FILE WAS GENERATED AUTOMATICALLY
# EDIT AT YOUR OWN RISK
# 
# Makefile for lib6502, run6502

# Copyright (c) 2005 Ian Piumarta
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sub-license, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
#   The above copyright notice and this permission notice shall be included in
#   all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS".  USE ENTIRELY AT YOUR OWN RISK.

# last edited: 2005-11-01 22:48:49 by piumarta on margaux.local

CFLAGS = -g -O3

PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
LIBDIR  = $(PREFIX)/lib
INCDIR  = $(PREFIX)/include
DOCDIR  = $(PREFIX)/doc/lib6502
EGSDIR  = $(DOCDIR)/examples
MANDIR  = $(PREFIX)/man
MAN1DIR = $(MANDIR)/man1
MAN3DIR = $(MANDIR)/man3

all : run6502

run6502 : run6502.o lib6502.a

lib6502.a : lib6502.o
	$(AR) -rc $@.new lib6502.o
	mv $@.new $@
	-ranlib $@

clean : .FORCE
	rm -f run6502 lib1 *~ *.o *.a .gdb* *.img *.log

.FORCE :

	install : .FORCE
	install -d $(BINDIR)
	install -d $(LIBDIR)
	install -d $(INCDIR)
	install -d $(MANDIR)
	install -d $(MAN1DIR)
	install -d $(MAN3DIR)
	install -d $(DOCDIR)
	install -d $(EGSDIR)
	install -c run6502 $(BINDIR)/run6502
	install -c lib6502.a $(LIBDIR)/lib6502.a
	install -c lib6502.h $(INCDIR)/lib6502.h
	install -c man/run6502.1 $(MAN1DIR)/run6502.1
	install -c man/lib6502.3 $(MAN3DIR)/lib6502.3
	install -c man/M6502_delete.3 $(MAN3DIR)/M6502_delete.3
	install -c man/M6502_disassemble.3 $(MAN3DIR)/M6502_disassemble.3
	install -c man/M6502_dump.3 $(MAN3DIR)/M6502_dump.3
	install -c man/M6502_getCallback.3 $(MAN3DIR)/M6502_getCallback.3
	install -c man/M6502_getVector.3 $(MAN3DIR)/M6502_getVector.3
	install -c man/M6502_irq.3 $(MAN3DIR)/M6502_irq.3
	install -c man/M6502_new.3 $(MAN3DIR)/M6502_new.3
	install -c man/M6502_nmi.3 $(MAN3DIR)/M6502_nmi.3
	install -c man/M6502_reset.3 $(MAN3DIR)/M6502_reset.3
	install -c man/M6502_run.3 $(MAN3DIR)/M6502_run.3
	install -c man/M6502_setCallback.3 $(MAN3DIR)/M6502_setCallback.3
	install -c man/M6502_setVector.3 $(MAN3DIR)/M6502_setVector.3
	install -c ChangeLog $(DOCDIR)/ChangeLog
	install -c COPYING $(DOCDIR)/COPYING
	install -c README $(DOCDIR)/README
	install -c examples/README $(EGSDIR)/README
	install -c examples/lib1.c $(EGSDIR)/lib1.c
	install -c examples/hex2bin $(EGSDIR)/hex2bin
	
	uninstall : .FORCE
	rm -f $(BINDIR)/run6502 $(LIBDIR)/lib6502.a $(INCDIR)/lib6502.h $(MAN1DIR)/run6502.1 $(MAN3DIR)/lib6502.3 $(MAN3DIR)/M6502_delete.3 $(MAN3DIR)/M6502_disassemble.3 $(MAN3DIR)/M6502_dump.3 $(MAN3DIR)/M6502_getCallback.3 $(MAN3DIR)/M6502_getVector.3 $(MAN3DIR)/M6502_irq.3 $(MAN3DIR)/M6502_new.3 $(MAN3DIR)/M6502_nmi.3 $(MAN3DIR)/M6502_reset.3 $(MAN3DIR)/M6502_run.3 $(MAN3DIR)/M6502_setCallback.3 $(MAN3DIR)/M6502_setVector.3 $(DOCDIR)/ChangeLog $(DOCDIR)/COPYING $(DOCDIR)/README $(EGSDIR)/README $(EGSDIR)/lib1.c $(EGSDIR)/hex2bin
	rmdir $(EGSDIR) $(DOCDIR)
