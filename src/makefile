.SUFFIXES	=
AFLAGS	= -o $@
PLVM    = plvm
PLVM01  = A1PLASMA\#060280
PLVM02	= PLASMA.SYSTEM\#FF2000
PLVM802	= PLASMA16.SYSTEM\#FF2000
PLVM03	= SOS.INTERP\#050000
CMD     = CMD\#FF2000
ED      = ED\#FE1000
SB      = SB\#FF2000
ROD     = ROD\#FE1000
SIEVE   = SIEVE\#FE1000
ARGS    = ARGS\#FE1000
SPIPORT = SPIPORT\#FE1000
SDFAT   = SDFAT\#FE1000
FATCAT  = FATCAT\#FE1000
FATGET  = FATGET\#FE1000
FATPUT  = FATPUT\#FE1000
FATWDSK = FATWRITEDSK\#FE1000
FATRDSK = FATREADDSK\#FE1000
FILEIO  = FILEIO\#FE1000
CONIO   = CONIO\#FE1000
SANE    = SANE\#FE1000
FPSTR   = FPSTR\#FE1000
FPU     = FPU\#FE1000
SANITY  = SANITY\#FE1000
RPNCALC = RPNCALC\#FE1000
WIZNET  = WIZNET\#FE1000
UTHERNET2= UTHERNET2\#FE1000
UTHERNET= UTHERNET\#FE1000
ETHERIP = ETHERIP\#FE1000
INET    = INET\#FE1000
DHCP    = DHCP\#FE1000
HTTPD   = HTTPD\#FE1000
DGR     = DGR\#FE1000
TONE    = TONE\#FE1000
PORTIO  = PORTIO\#FE1000
ROGUE   = ROGUE\#FE1000
ROGUEIO = ROGUEIO\#FE1000
ROGUEMAP= ROGUEMAP\#FE1000
ROGUECOMBAT= ROGUECOMBAT\#FE1000
HELLO   = HELLO\#FE1000
MON     = MON\#FE1000
HGR1    = HGR1\#FE1000
HGR1TEST= HGR1TEST\#FE1000
DGRTEST = DGRTEST\#FE1000
TEST    = TEST\#FE1000
TESTLIB = TESTLIB\#FE1000
PROFILE = PROFILE\#FE1000
MEMMGR  = MEMMGR\#FE1000
MEMTEST = MEMTEST\#FE1000
FIBER   = FIBER\#FE1000
LONGJMP = LONGJMP\#FE1000
PLASM   = plasm
PLASMAPLASM = PLASM\#FE1000
CODEOPT = CODEOPT\#FE1000
INCS    = toolsrc/plasm.h toolsrc/tokens.h toolsrc/symbols.h toolsrc/lex.h toolsrc/parse.h toolsrc/codegen.h
OBJS	= toolsrc/plasm.c toolsrc/parse.c toolsrc/lex.c toolsrc/codegen.c
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

all: $(PLASM) $(PLVM) $(PLVM01) $(PLVM02) $(PLVM802) $(PLVM03) $(CMD) $(PLASMAPLASM) $(CODEOPT) $(ARGS) $(MEMMGR) $(MEMTEST) $(FIBER) $(LONGJMP) $(ED) $(MON) $(ROD) $(SIEVE) $(UTHERNET2) $(UTHERNET) $(ETHERIP) $(INET) $(DHCP) $(HTTPD) $(ROGUE) $(ROGUEMAP) $(ROGUECOMBAT) $(ROGUEIO) $(HGR1) $(TONE) $(DGR) $(DGRTEST) $(FILEIO) $(CONIO) $(PORTIO) $(SPIPORT) $(SDFAT) $(FATCAT) $(FATGET) $(FATPUT) $(FATWDSK) $(FATRDSK) $(SANE) $(FPSTR) $(FPU) $(SANITY) $(RPNCALC)

clean:
	-rm *FE1000 *FF2000 $(PLASM) $(PLVM) $(PLVM01) $(PLVM02) $(PLVM03)
	-rm toolsrc/*.o toolsrc/*~ toolsrc/*.a
	-rm vmsrc/*.o vmsrc/*~ vmsrc/*.a vmsrc/*.sym
	-rm samplesrc/*.o samplesrc/*~ samplesrc/*.a

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

vmsrc/a1cmd.a: vmsrc/a1cmd.pla $(PLASM)
	./$(PLASM) -AOW < vmsrc/a1cmd.pla > vmsrc/a1cmd.a

$(PLVM01): vmsrc/plvm01.s vmsrc/a1cmd.a
	acme -o $(PLVM01) -l vmsrc/plvm01.sym vmsrc/plvm01.s

$(CMD): vmsrc/cmd.pla vmsrc/cmdstub.s $(PLVM02) $(PLASM)
	./$(PLASM) -AOW < vmsrc/cmd.pla > vmsrc/cmd.a
	acme --setpc 8192 -o $(CMD) vmsrc/cmdstub.s

$(PLVM02): vmsrc/plvm02.s
	acme -o $(PLVM02) -l vmsrc/plvm02.sym vmsrc/plvm02.s

$(PLVM802): vmsrc/plvm802.s
	acme -o $(PLVM802) -l vmsrc/plvm802.sym vmsrc/plvm802.s

vmsrc/soscmd.a: vmsrc/soscmd.pla $(PLASM)
	./$(PLASM) -AOW < vmsrc/soscmd.pla > vmsrc/soscmd.a

$(PLVM03): vmsrc/plvm03.s vmsrc/soscmd.a
	acme -o $(PLVM03) -l vmsrc/plvm03.sym vmsrc/plvm03.s

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

$(SB): toolsrc/sb.pla $(PLVM02) $(PLASM) toolsrc/sb.pla
	./$(PLASM) -AOW < toolsrc/sb.pla > toolsrc/sb.a
	acme --setpc 8192 -o $(SB) toolsrc/sb.a

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

$(LONGJMP): libsrc/longjmp.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/longjmp.pla > libsrc/longjmp.a
	acme --setpc 4094 -o $(LONGJMP) libsrc/longjmp.a

$(MON): samplesrc/mon.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/mon.pla > samplesrc/mon.a
	acme --setpc 4094 -o $(MON) samplesrc/mon.a

$(ROD): samplesrc/rod.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMO < samplesrc/rod.pla > samplesrc/rod.a
	acme --setpc 4094 -o $(ROD) samplesrc/rod.a

$(SIEVE): samplesrc/sieve.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMW < samplesrc/sieve.pla > samplesrc/sieve.a
	acme --setpc 4094 -o $(SIEVE) samplesrc/sieve.a

$(UTHERNET): libsrc/uthernet.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/uthernet.pla > libsrc/uthernet.a
	acme --setpc 4094 -o $(UTHERNET) libsrc/uthernet.a

$(UTHERNET2): libsrc/uthernet2.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/uthernet2.pla > libsrc/uthernet2.a
	acme --setpc 4094 -o $(UTHERNET2) libsrc/uthernet2.a

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

$(FILEIO): libsrc/fileio.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/fileio.pla > libsrc/fileio.a
	acme --setpc 4094 -o $(FILEIO) libsrc/fileio.a

$(CONIO): libsrc/conio.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/conio.pla > libsrc/conio.a
	acme --setpc 4094 -o $(CONIO) libsrc/conio.a

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

$(RPNCALC): samplesrc/rpncalc.pla libsrc/fpu.pla inc/fpu.plh libsrc/fpstr.pla inc/fpstr.plh libsrc/conio.pla inc/conio.plh $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/rpncalc.pla > samplesrc/rpncalc.a
	acme --setpc 4094 -o $(RPNCALC) samplesrc/rpncalc.a

$(TONE): libsrc/tone.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/tone.pla > libsrc/tone.a
	acme --setpc 4094 -o $(TONE) libsrc/tone.a

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

$(SDFAT): libsrc/sdfat.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/sdfat.pla > libsrc/sdfat.a
	acme --setpc 4094 -o $(SDFAT) libsrc/sdfat.a

$(SPIPORT): libsrc/spiport.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/spiport.pla > libsrc/spiport.a
	acme --setpc 4094 -o $(SPIPORT) libsrc/spiport.a

$(PORTIO): libsrc/portio.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/portio.pla > libsrc/portio.a
	acme --setpc 4094 -o $(PORTIO) libsrc/portio.a

$(DGR): libsrc/dgr.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < libsrc/dgr.pla > libsrc/dgr.a
	acme --setpc 4094 -o $(DGR) libsrc/dgr.a

$(DGRTEST): samplesrc/dgrtest.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/dgrtest.pla > samplesrc/dgrtest.a
	acme --setpc 4094 -o $(DGRTEST) samplesrc/dgrtest.a

$(ROGUE): samplesrc/rogue.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/rogue.pla > samplesrc/rogue.a
	acme --setpc 4094 -o $(ROGUE) samplesrc/rogue.a

$(ROGUEIO): samplesrc/rogue.io.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/rogue.io.pla > samplesrc/rogue.io.a
	acme --setpc 4094 -o $(ROGUEIO) samplesrc/rogue.io.a

$(ROGUECOMBAT): samplesrc/rogue.combat.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/rogue.combat.pla > samplesrc/rogue.combat.a
	acme --setpc 4094 -o $(ROGUECOMBAT) samplesrc/rogue.combat.a

$(ROGUEMAP): samplesrc/rogue.map.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/rogue.map.pla > samplesrc/rogue.map.a
	acme --setpc 4094 -o $(ROGUEMAP) samplesrc/rogue.map.a

$(HGR1):	samplesrc/hgr1.pla samplesrc/hgr1test.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/hgr1test.pla > samplesrc/hgr1test.a
	acme --setpc 4094 -o $(HGR1TEST) samplesrc/hgr1test.a
	./$(PLASM) -AMOW < samplesrc/hgr1.pla > samplesrc/hgr1.a
	acme --setpc 4094 -o $(HGR1) samplesrc/hgr1.a

hello:	samplesrc/hello.pla $(PLVM) $(PLASM)
	./$(PLASM) -AMOW < samplesrc/hello.pla > samplesrc/hello.a
	acme --setpc 4094 -o $(HELLO) samplesrc/hello.a
	./$(PLVM) HELLO
