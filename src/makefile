.SUFFIXES	=
AFLAGS	   = -o $@
PLVM       = plvm
PLVMZP_APL = vmsrc/apple/plvmzp.inc
PLVM01     = rel/apple/A1PLASMA\#060280
PLVM02	   = rel/apple/PLASMA.SYSTEM\#FF2000
PLVM802	   = rel/apple/PLASMA16.SYSTEM\#FF2000
PLVM03	   = rel/apple/SOS.INTERP\#050000
CMD        = rel/apple/CMD\#061000
PLVMZP_C64 = vmsrc/c64/plvmzp.inc
PLVMC64    = rel/c64/PLASMA
ED         = rel/ED\#FE1000
SOS        = rel/apple/SOS\#FE1000
ROD        = rel/ROD\#FE1000
SIEVE      = rel/SIEVE\#FE1000
ARGS       = rel/ARGS\#FE1000
SPIPORT    = rel/apple/SPIPORT\#FE1000
SDFAT      = rel/apple/SDFAT\#FE1000
FATCAT     = rel/apple/FATCAT\#FE1000
FATGET     = rel/apple/FATGET\#FE1000
FATPUT     = rel/apple/FATPUT\#FE1000
FATWDSK    = rel/apple/FATWRITEDSK\#FE1000
FATRDSK    = rel/apple/FATREADDSK\#FE1000
FILEIO_APL = rel/apple/FILEIO\#FE1000
CONIO_APL  = rel/apple/CONIO\#FE1000
SANE       = rel/SANE\#FE1000
FPSTR      = rel/FPSTR\#FE1000
FPU        = rel/FPU\#FE1000
SNDSEQ     = rel/apple/SNDSEQ\#FE1000
PLAYSEQ    = rel/apple/PLAYSEQ\#FE1000
SANITY     = rel/SANITY\#FE1000
RPNCALC    = rel/RPNCALC\#FE1000
UTHERNET2  = rel/apple/UTHERNET2\#FE1000
UTHERNET   = rel/apple/UTHERNET\#FE1000
ETHERIP    = rel/ETHERIP\#FE1000
INET       = rel/INET\#FE1000
DHCP       = rel/DHCP\#FE1000
HTTPD      = rel/HTTPD\#FE1000
DGR        = rel/apple/DGR\#FE1000
GRAFIX     = rel/apple/GRAFIX\#FE1000
GFXDEMO    = rel/apple/GFXDEMO\#FE1000
PORTIO     = rel/apple/PORTIO\#FE1000
ROGUE      = rel/ROGUE\#FE1000
ROGUEMAP   = rel/ROGUEMAP\#FE1000
ROGUECOMBAT= rel/ROGUECOMBAT\#FE1000
MON        = rel/apple/MON\#FE1000
DGRTEST    = rel/apple/DGRTEST\#FE1000
MEMMGR     = rel/MEMMGR\#FE1000
MEMTEST    = rel/MEMTEST\#FE1000
FIBERTEST  = rel/FIBERTEST\#FE1000
FIBER      = rel/FIBER\#FE1000
LONGJMP    = rel/LONGJMP\#FE1000
HELLO      = HELLO\#FE1000
TEST       = TEST\#FE1000
TESTLIB    = TESTLIB\#FE1000
PLASM      = plasm
PLASMAPLASM = rel/PLASM\#FE1000
CODEOPT    = rel/CODEOPT\#FE1000
INCS       = toolsrc/plasm.h toolsrc/tokens.h toolsrc/symbols.h toolsrc/lex.h toolsrc/parse.h toolsrc/codegen.h
OBJS	   = toolsrc/plasm.c toolsrc/parse.c toolsrc/lex.c toolsrc/codegen.c
#
# Image filetypes for Virtual ][
#
PLATYPE	= .\$$ED
BINTYPE	= .BIN
SYSTYPE	= .SYS
TXTTYPE	= .TXT
#
# Image filetypes for CiderPress
#
#RELTYPE	= \#FE1000
#INTERPTYPE	= \#050000
#BINTYPE	= \#060000
#SYSTYPE	= \#FF2000
#TXTTYPE	= \#040000

apple: $(PLVMZP_APL) $(PLASM) $(PLVM) $(PLVM01) $(PLVM02) $(PLVM802) $(PLVM03) $(CMD) $(PLASMAPLASM) $(CODEOPT) $(ARGS) $(MEMMGR) $(MEMTEST) $(FIBER) $(FIBERTEST) $(LONGJMP) $(ED) $(MON) $(SOS) $(ROD) $(SIEVE) $(UTHERNET2) $(UTHERNET) $(ETHERIP) $(INET) $(DHCP) $(HTTPD) $(ROGUE) $(ROGUEMAP) $(ROGUECOMBAT) $(GRAFIX) $(GFXDEMO) $(DGR) $(DGRTEST) $(FILEIO_APL) $(CONIO_APL) $(PORTIO) $(SPIPORT) $(SDFAT) $(FATCAT) $(FATGET) $(FATPUT) $(FATWDSK) $(FATRDSK) $(SANE) $(FPSTR) $(FPU) $(SANITY) $(RPNCALC) $(SNDSEQ) $(PLAYSEQ)
	-rm vmsrc/plvmzp.inc

c64: $(PLVMZP_C64) $(PLASM) $(PLVM) $(PLVMC64) 
	-rm vmsrc/plvmzp.inc

all: apple c64

clean:
	-rm *FE1000 *FF2000 $(PLASM) $(PLVM) $(PLVM01) $(PLVM02) $(PLVM03)
	-rm rel/*
	-rm rel/apple/*
	-rm rel/c64/*
	-rm samplesrc/*.o samplesrc/*~ samplesrc/*.a
	-rm toolsrc/*.o toolsrc/*~ toolsrc/*.a
	-rm toolsrc/apple/*.o toolsrc/apple/*~ toolsrc/apple/*.a
	-rm toolsrc/c64/*.o toolsrc/c64/*~ toolsrc/c64/*.a
	-rm vmsrc/*.o vmsrc/*~ vmsrc/*.a vmsrc/*.sym
	-rm vmsrc/apple/*.o vmsrc/apple/*~ vmsrc/apple/*.a vmsrc/apple/*.sym
	-rm vmsrc/c64/*.o vmsrc/c64/*~ vmsrc/c64/*.a vmsrc/c64/*.sym
	-rm libsrc/*.o libsrc/*~ libsrc/*.a
	-rm libsrc/apple/*.o libsrc/apple/*~ libsrc/apple/*.a
	-rm libsrc/c64/*.o libsrc/c64/*~ libsrc/c64/*.a
	-rm libsrc/*.o libsrc/*~ libsrc/*.a

#
# PLASMA compiler: plasm
#
$(PLASM): $(OBJS) $(INCS)
	cc $(OBJS) -o $(PLASM)

$(PLASMAPLASM): toolsrc/plasm.pla toolsrc/lex.pla toolsrc/parse.pla toolsrc/codegen.pla toolsrc/codeseq.plh
	./$(PLASM) -AMOW < toolsrc/plasm.pla > toolsrc/plasm.a
	acme --setpc 4094 -o $(PLASMAPLASM) toolsrc/plasm.a

$(CODEOPT): toolsrc/codeopt.pla toolsrc/codeseq.plh
	./$(PLASM) -AMOW < toolsrc/codeopt.pla > toolsrc/codeopt.a
	acme --setpc 4094 -o $(CODEOPT) toolsrc/codeopt.a

#
# PLASMA VMs
#
$(PLVM): vmsrc/plvm.c
	cc vmsrc/plvm.c -o $(PLVM)

$(PLVMZP_APL): FORCE
	-rm vmsrc/plvmzp.inc
	-ln -s apple/plvmzp.inc vmsrc/plvmzp.inc

$(PLVMZP_C64): FORCE
	-rm vmsrc/plvmzp.inc
	-ln -s c64/plvmzp.inc vmsrc/plvmzp.inc

FORCE:

vmsrc/c64/cmd.a: vmsrc/c64/cmd.pla $(PLASM)
	./$(PLASM) -AOW < vmsrc/c64/cmd.pla > vmsrc/c64/cmd.a

$(PLVMC64): vmsrc/c64/plvmc64.s vmsrc/c64/cmd.a
	acme -o $(PLVMC64) -l vmsrc/c64/plvmc64.sym vmsrc/c64/plvmc64.s

vmsrc/apple/a1cmd.a: vmsrc/apple/a1cmd.pla $(PLASM)
	./$(PLASM) -AOW < vmsrc/apple/a1cmd.pla > vmsrc/apple/a1cmd.a

$(PLVM01): vmsrc/apple/plvm01.s vmsrc/apple/a1cmd.a
	acme -o $(PLVM01) -l vmsrc/apple/plvm01.sym vmsrc/apple/plvm01.s

$(CMD): vmsrc/apple/cmd.pla vmsrc/apple/cmdstub.s $(PLVM02) $(PLASM)
	./$(PLASM) -AOW < vmsrc/apple/cmd.pla > vmsrc/apple/cmd.a
	acme --setpc 8192 -o $(CMD) vmsrc/apple/cmdstub.s

$(PLVM02): vmsrc/apple/plvm02.s
	acme -o $(PLVM02) -l vmsrc/apple/plvm02.sym vmsrc/apple/plvm02.s

$(PLVM802): vmsrc/apple/plvm802.s
	acme -o $(PLVM802) -l vmsrc/apple/plvm802.sym vmsrc/apple/plvm802.s

vmsrc/apple/soscmd.a: vmsrc/apple/soscmd.pla $(PLASM)
	./$(PLASM) -AOW < vmsrc/apple/soscmd.pla > vmsrc/apple/soscmd.a

$(PLVM03): vmsrc/apple/plvm03.s vmsrc/apple/soscmd.a
	acme -o $(PLVM03) -l vmsrc/apple/plvm03.sym vmsrc/apple/plvm03.s

#
# Sample code
#
test:	samplesrc/test.pla samplesrc/testlib.pla $(PLVM) $(PLASM)
	./$(PLASM) -AMOW  < samplesrc/test.pla > samplesrc/test.a
	acme --setpc 4094 -o $(TEST) samplesrc/test.a
	./$(PLASM) -AMOW  < samplesrc/testlib.pla > samplesrc/testlib.a
	acme --setpc 4094 -o $(TESTLIB) samplesrc/testlib.a
	./$(PLVM) TEST

$(ED): toolsrc/ed.pla $(PLVM02) $(PLASM) toolsrc/ed.pla
	./$(PLASM) -AMOW < toolsrc/ed.pla > toolsrc/ed.a
	acme --setpc 4094 -o $(ED) toolsrc/ed.a

$(ARGS): libsrc/args.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/args.pla > libsrc/args.a
	acme --setpc 4094 -o $(ARGS) libsrc/args.a

$(MEMMGR): libsrc/memmgr.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/memmgr.pla > libsrc/memmgr.a
	acme --setpc 4094 -o $(MEMMGR) libsrc/memmgr.a

$(MEMTEST): samplesrc/memtest.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/memtest.pla > samplesrc/memtest.a
	acme --setpc 4094 -o $(MEMTEST) samplesrc/memtest.a

$(FIBER): libsrc/fiber.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/fiber.pla > libsrc/fiber.a
	acme --setpc 4094 -o $(FIBER) libsrc/fiber.a

$(FIBERTEST): samplesrc/fibertest.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/fibertest.pla > samplesrc/fibertest.a
	acme --setpc 4094 -o $(FIBERTEST) samplesrc/fibertest.a

$(LONGJMP): libsrc/longjmp.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/longjmp.pla > libsrc/longjmp.a
	acme --setpc 4094 -o $(LONGJMP) libsrc/longjmp.a

$(ROGUE): samplesrc/rogue.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/rogue.pla > samplesrc/rogue.a
	acme --setpc 4094 -o $(ROGUE) samplesrc/rogue.a

$(ROGUECOMBAT): samplesrc/rogue.combat.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/rogue.combat.pla > samplesrc/rogue.combat.a
	acme --setpc 4094 -o $(ROGUECOMBAT) samplesrc/rogue.combat.a

$(ROGUEMAP): samplesrc/rogue.map.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/rogue.map.pla > samplesrc/rogue.map.a
	acme --setpc 4094 -o $(ROGUEMAP) samplesrc/rogue.map.a

hello:	samplesrc/hello.pla $(PLVM) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/hello.pla > samplesrc/hello.a
	acme --setpc 4094 -o $(HELLO) samplesrc/hello.a
	./$(PLVM) HELLO
$(ROD): samplesrc/rod.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMO < samplesrc/rod.pla > samplesrc/rod.a
	acme --setpc 4094 -o $(ROD) samplesrc/rod.a

$(SIEVE): samplesrc/sieve.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMW < samplesrc/sieve.pla > samplesrc/sieve.a
	acme --setpc 4094 -o $(SIEVE) samplesrc/sieve.a

$(SANE): libsrc/sane.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/sane.pla > libsrc/sane.a
	acme --setpc 4094 -o $(SANE) libsrc/sane.a

$(FPSTR): libsrc/fpstr.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/fpstr.pla > libsrc/fpstr.a
	acme --setpc 4094 -o $(FPSTR) libsrc/fpstr.a

$(FPU): libsrc/fpu.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/fpu.pla > libsrc/fpu.a
	acme --setpc 4094 -o $(FPU) libsrc/fpu.a

$(SANITY): samplesrc/sanity.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/sanity.pla > samplesrc/sanity.a
	acme --setpc 4094 -o $(SANITY) samplesrc/sanity.a

$(RPNCALC): samplesrc/rpncalc.pla libsrc/fpu.pla inc/fpu.plh libsrc/fpstr.pla inc/fpstr.plh inc/conio.plh $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/rpncalc.pla > samplesrc/rpncalc.a
	acme --setpc 4094 -o $(RPNCALC) samplesrc/rpncalc.a

$(ETHERIP): libsrc/etherip.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/etherip.pla > libsrc/etherip.a
	acme --setpc 4094 -o $(ETHERIP) libsrc/etherip.a

$(INET): libsrc/inet.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/inet.pla > libsrc/inet.a
	acme --setpc 4094 -o $(INET) libsrc/inet.a

$(DHCP): libsrc/dhcp.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/dhcp.pla > libsrc/dhcp.a
	acme --setpc 4094 -o $(DHCP) libsrc/dhcp.a

$(HTTPD): samplesrc/httpd.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/httpd.pla > samplesrc/httpd.a
	acme --setpc 4094 -o $(HTTPD) samplesrc/httpd.a

$(UTHERNET): libsrc/apple/uthernet.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/uthernet.pla > libsrc/apple/uthernet.a
	acme --setpc 4094 -o $(UTHERNET) libsrc/apple/uthernet.a

$(UTHERNET2): libsrc/apple/uthernet2.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/uthernet2.pla > libsrc/apple/uthernet2.a
	acme --setpc 4094 -o $(UTHERNET2) libsrc/apple/uthernet2.a

$(FILEIO_APL): libsrc/apple/fileio.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/fileio.pla > libsrc/apple/fileio.a
	acme --setpc 4094 -o $(FILEIO_APL) libsrc/apple/fileio.a

$(CONIO_APL): libsrc/apple/conio.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/conio.pla > libsrc/apple/conio.a
	acme --setpc 4094 -o $(CONIO_APL) libsrc/apple/conio.a

$(FILEIO_C64): libsrc/c64/fileio.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/c64/fileio.pla > libsrc/c64/fileio.a
	acme --setpc 4094 -o $(FILEIO_C64) libsrc/c64/fileio.a

$(CONIO_C64): libsrc/c64/conio.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/c64/conio.pla > libsrc/c64/conio.a
	acme --setpc 4094 -o $(CONIO_C64) libsrc/c64/conio.a

$(SNDSEQ): libsrc/apple/sndseq.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/sndseq.pla > libsrc/apple/sndseq.a
	acme --setpc 4094 -o $(SNDSEQ) libsrc/apple/sndseq.a

$(PLAYSEQ): samplesrc/playseq.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/playseq.pla > samplesrc/playseq.a
	acme --setpc 4094 -o $(PLAYSEQ) samplesrc/playseq.a

$(GRAFIX): libsrc/apple/grafix.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/grafix.pla > libsrc/apple/grafix.a
	acme --setpc 4094 -o $(GRAFIX) libsrc/apple/grafix.a

$(GFXDEMO): samplesrc/gfxdemo.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/gfxdemo.pla > samplesrc/gfxdemo.a
	acme --setpc 4094 -o $(GFXDEMO) samplesrc/gfxdemo.a

$(FATCAT): samplesrc/fatcat.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/fatcat.pla > samplesrc/fatcat.a
	acme --setpc 4094 -o $(FATCAT) samplesrc/fatcat.a

$(FATGET): samplesrc/fatget.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/fatget.pla > samplesrc/fatget.a
	acme --setpc 4094 -o $(FATGET) samplesrc/fatget.a

$(FATPUT): samplesrc/fatput.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/fatput.pla > samplesrc/fatput.a
	acme --setpc 4094 -o $(FATPUT) samplesrc/fatput.a

$(FATWDSK): samplesrc/fatwritedsk.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/fatwritedsk.pla > samplesrc/fatwritedsk.a
	acme --setpc 4094 -o $(FATWDSK) samplesrc/fatwritedsk.a

$(FATRDSK): samplesrc/fatreaddsk.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/fatreaddsk.pla > samplesrc/fatreaddsk.a
	acme --setpc 4094 -o $(FATRDSK) samplesrc/fatreaddsk.a

$(SDFAT): libsrc/apple/sdfat.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/sdfat.pla > libsrc/apple/sdfat.a
	acme --setpc 4094 -o $(SDFAT) libsrc/apple/sdfat.a

$(SPIPORT): libsrc/apple/spiport.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/spiport.pla > libsrc/apple/spiport.a
	acme --setpc 4094 -o $(SPIPORT) libsrc/apple/spiport.a

$(PORTIO): libsrc/apple/portio.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/portio.pla > libsrc/apple/portio.a
	acme --setpc 4094 -o $(PORTIO) libsrc/apple/portio.a

$(DGR): libsrc/apple/dgr.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/apple/dgr.pla > libsrc/apple/dgr.a
	acme --setpc 4094 -o $(DGR) libsrc/apple/dgr.a

$(DGRTEST): samplesrc/dgrtest.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/dgrtest.pla > samplesrc/dgrtest.a
	acme --setpc 4094 -o $(DGRTEST) samplesrc/dgrtest.a

$(MON): samplesrc/mon.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/mon.pla > samplesrc/mon.a
	acme --setpc 4094 -o $(MON) samplesrc/mon.a

$(SOS): libsrc/apple/sos.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMO < libsrc/apple/sos.pla > libsrc/apple/sos.a
	acme --setpc 4094 -o $(SOS) libsrc/apple/sos.a

