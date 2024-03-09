# Makefile for lib6502, run6502

# Copyright (c) 2005 Ian Piumarta
# 
# All rights reserved.
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the 'Software'), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, provided
# that the above copyright notice(s) and this permission notice appear
# in all copies of the Software and that both the above copyright
# notice(s) and this permission notice appear in supporting
# documentation.
#
# THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.

# last edited: 2013-06-08 01:08:02 by piumarta on vps2.piumarta.com

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

# ----------------------------------------------------------------

INSTALLDIRS  = $(BINDIR) $(LIBDIR) $(INCDIR) $(MANDIR) $(MAN1DIR) $(MAN3DIR) $(DOCDIR) $(EGSDIR)

BINFILES = $(BINDIR)/run6502

LIBFILES = $(LIBDIR)/lib6502.a

INCFILES = $(INCDIR)/lib6502.h

MANFILES = $(MAN1DIR)/run6502.1 \
	   $(MAN3DIR)/lib6502.3 \
	   $(MAN3DIR)/M6502_delete.3 \
	   $(MAN3DIR)/M6502_disassemble.3 \
	   $(MAN3DIR)/M6502_dump.3 \
	   $(MAN3DIR)/M6502_getCallback.3 \
	   $(MAN3DIR)/M6502_getVector.3 \
	   $(MAN3DIR)/M6502_irq.3 \
	   $(MAN3DIR)/M6502_new.3 \
	   $(MAN3DIR)/M6502_nmi.3 \
	   $(MAN3DIR)/M6502_reset.3 \
	   $(MAN3DIR)/M6502_run.3 \
	   $(MAN3DIR)/M6502_setCallback.3 \
	   $(MAN3DIR)/M6502_setVector.3

DOCFILES = $(DOCDIR)/ChangeLog \
	   $(DOCDIR)/COPYING \
	   $(DOCDIR)/README \
	   $(EGSDIR)/README \
	   $(EGSDIR)/lib1.c \
	   $(EGSDIR)/hex2bin

MKDIR = install -d
RMDIR = rmdir
INSTALL = install -c
RM = rm -f

$(BINDIR)/% $(LIBDIR)/% $(INCDIR)/% $(DOCDIR)/% : %
	$(INSTALL) $< $@

$(MAN1DIR)/% $(MAN3DIR)/% : man/%
	$(INSTALL) $< $@

$(EGSDIR)/% : examples/%
	$(INSTALL) $< $@

$(INSTALLDIRS) :
	$(MKDIR) $@

install :  $(INSTALLDIRS) $(BINFILES) $(LIBFILES) $(INCFILES) $(MANFILES) $(DOCFILES)

uninstall :  .FORCE
	-$(RM) $(BINFILES) $(LIBFILES) $(INCFILES) $(MANFILES) $(DOCFILES)
	-$(RMDIR) $(EGSDIR) $(DOCDIR)

# ----------------------------------------------------------------

PACKAGE_VERSION = 1.3
PACKAGE_TARNAME = lib6502

TARNAME= $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)

DISTFILES = \
	$(TARNAME)/ChangeLog \
	$(TARNAME)/COPYING \
	$(TARNAME)/README \
	$(TARNAME)/Makefile \
	$(TARNAME)/BSDmakefile \
	$(TARNAME)/config.h \
	$(TARNAME)/lib6502.h \
	$(TARNAME)/lib6502.c \
	$(TARNAME)/run6502.c \
	$(TARNAME)/test.out \
	$(TARNAME)/man/run6502.1 \
	$(TARNAME)/man/lib6502.3 \
	$(TARNAME)/man/M6502_delete.3 \
	$(TARNAME)/man/M6502_disassemble.3 \
	$(TARNAME)/man/M6502_dump.3 \
	$(TARNAME)/man/M6502_getCallback.3 \
	$(TARNAME)/man/M6502_getVector.3 \
	$(TARNAME)/man/M6502_irq.3 \
	$(TARNAME)/man/M6502_new.3 \
	$(TARNAME)/man/M6502_nmi.3 \
	$(TARNAME)/man/M6502_reset.3 \
	$(TARNAME)/man/M6502_run.3 \
	$(TARNAME)/man/M6502_setCallback.3  \
	$(TARNAME)/man/M6502_setVector.3 \
	$(TARNAME)/examples/hex2bin \
	$(TARNAME)/examples/lib1.c \
	$(TARNAME)/examples/README

dist : .FORCE
	rm -f $(TARNAME)
	ln -s . $(TARNAME)
	tar -cf $(TARNAME).tar $(DISTFILES)
	gzip -v9 $(TARNAME).tar
	rm -f $(TARNAME)

dist-test : .FORCE
	rm -rf $(TARNAME)
	tar -xz -f $(TARNAME).tar.gz
	ln -s ../images $(TARNAME)/images
	$(MAKE) -C $(TARNAME) test
	rm -rf $(TARNAME)

# ----------------------------------------------------------------

image :
	./run6502		\
	  -l C000 images/os1.2	\
	  -l 8000 images/basic2	\
	  -s 0000 +10000 image	\
	  -x

newimage : .FORCE
	rm -f image
	$(MAKE) image

test1 : run6502 .FORCE
	echo a2418a20eeffe8e05bd0f7a90a20eeff0000 | perl -e '$$_=pack"H*",<STDIN>;print' > temp.img
	./run6502		\
	  -l 1000 temp.img	\
	  -d 1000 +11		\
	  -R 1000		\
	  -P FFEE		\
	  -X 0

lib1 : lib6502.a
	$(CC) -I. -o lib1 examples/lib1.c lib6502.a

test2 : lib1 .FORCE
	./lib1

test3 : run6502 image .FORCE
	echo 'PRINT:FORA%=1TO10:PRINTA%:NEXT:PRINT"HELLO WORLD"' | ./run6502 image

test4 : run6502 image .FORCE
	echo 'P%=&2800:O%=P%:[opt3:ldx#65:.l txa:jsr&FFEE:inx:cpx#91:bnel:lda#13:jsr&FFEE:lda#10:jmp&FFEE:]:CALL&2800' | ./run6502 image

test : run6502 lib1 image .FORCE
	@$(MAKE) test1 test2 test3 test4 | grep -v '^make.* directory' | tee test.log
	cmp test.log test.out
	@echo
	@echo SUCCESS
	@echo

# ----------------------------------------------------------------

# I don't know what it is (probably me, who knows?) but every single
# time I try to write a Makefile that is compatible with both GNU and
# BSD make I spend three hours getting absolutely nowhere.  It's
# telling when a program consisting of precisely TWO SOURCE FILES and
# a few man pages and examples is already TOO COMPLEX to be installed
# with BSD make, in the absence of an explicit rule for every single
# target, be it installed or intermediate, from a Makefile that won't
# also break GNU make.  Good Grief Charlie Brown.

# Yes I know I can compose the sed substitutions into a single script,
# but it looks even uglier that way.

BSDmakefile : .FORCE
	$(MAKE)
	rm -rf /tmp/bsd
	echo '# THIS FILE WAS GENERATED AUTOMATICALLY'	>  BSDmakefile
	echo '# EDIT AT YOUR OWN RISK'			>> BSDmakefile
	echo '# '					>> BSDmakefile
	sed  '/# -/,$$d' < Makefile			>> BSDmakefile
	echo 'install : .FORCE'				>  BSDtemp
	$(MAKE) install PREFIX=/tmp/bsd			>> BSDtemp
	echo						>> BSDtemp
	echo 'uninstall : .FORCE'			>> BSDtemp
	$(MAKE) uninstall PREFIX=/tmp/bsd		>> BSDtemp
	cat BSDtemp					    | \
	sed  's,/tmp/bsd/doc/lib6502/examples,$$(EGSDIR),g' | \
	sed  's,/tmp/bsd/doc/lib6502,$$(DOCDIR),g'	    | \
	sed  's,/tmp/bsd/man/man1,$$(MAN1DIR),g'	    | \
	sed  's,/tmp/bsd/man/man3,$$(MAN3DIR),g'	    | \
	sed  's,/tmp/bsd/man,$$(MANDIR),g'		    | \
	sed  's,/tmp/bsd/include,$$(INCDIR),g'		    | \
	sed  's,/tmp/bsd/lib,$$(LIBDIR),g'		    | \
	sed  's,/tmp/bsd/lib,$$(LIBDIR),g'		    | \
	sed  's,/tmp/bsd/bin,$$(BINDIR),g'		    | \
	sed  's,^,	,g'				>> BSDmakefile
	rm -f BSDtemp
	rm -rf /tmp/bsd
