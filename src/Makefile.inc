.SUFFIXES	=
AFLAGS	= -o $@
# Tools
PLASM   = plasm
PLASMAPLASM = PLASM\#FE1000
CODEOPT = CODEOPT\#FE1000
PLVM    = plvm
ED      = ED\#FE1000
SB      = SB\#FF2000
# Platform
PLVM01  = A1PLASMA\#060280
PLVM02	= PLASMA.SYSTEM\#FF2000
PLVM802	= PLASMA16.SYSTEM\#FF2000
PLVM03	= SOS.INTERP\#050000
PLVMBBC	= BBPLASMA\#FF2000
CMD     = CMD\#061000
# Lib
ARGS    = ARGS\#FE1000
CONIO   = CONIO\#FE1000
DGR     = DGR\#FE1000
DHCP    = DHCP\#FE1000
ETHERIP = ETHERIP\#FE1000
FIBER   = FIBER\#FE1000
FILEIO  = FILEIO\#FE1000
FPSTR   = FPSTR\#FE1000
FPU     = FPU\#FE1000
INET    = INET\#FE1000
LONGJMP = LONGJMP\#FE1000
MEMMGR  = MEMMGR\#FE1000
PORTIO  = PORTIO\#FE1000
SANE    = SANE\#FE1000
SDFAT   = SDFAT\#FE1000
SNDSEQ  = SNDSEQ\#FE1000
SPIPORT = SPIPORT\#FE1000
TONE    = TONE\#FE1000
UTHERNET= UTHERNET\#FE1000
UTHERNET2= UTHERNET2\#FE1000
#WIZNET  = WIZNET\#FE1000
# Samples
DGRTEST = DGRTEST\#FE1000
FATCAT  = FATCAT\#FE1000
FATGET  = FATGET\#FE1000
FATPUT  = FATPUT\#FE1000
FATRDSK = FATREADDSK\#FE1000
FATWDSK = FATWRITEDSK\#FE1000
FIBERTEST = FIBERTEST\#FE1000
HELLO   = HELLO\#FE1000
HTTPD   = HTTPD\#FE1000
MEMTEST = MEMTEST\#FE1000
MON     = MON\#FE1000
PLAYSEQ = PLAYSEQ\#FE1000
ROD     = ROD\#FE1000
ROGUE   = ROGUE\#FE1000
ROGUECOMBAT= ROGUECOMBAT\#FE1000
ROGUEMAP= ROGUEMAP\#FE1000
RPNCALC = RPNCALC\#FE1000
SANITY  = SANITY\#FE1000
SIEVE   = SIEVE\#FE1000
TEST    = TEST\#FE1000
TESTLIB = TESTLIB\#FE1000
SAMPLES := $(DGRTEST) $(FATCAT) $(FATGET) $(FATPUT) $(FATRDSK) $(FATWDSK) \
$(FIBERTEST) $(HTTPD) $(MEMTEST) $(MON) $(PLAYSEQ) $(ROD) $(ROGUE) \
$(ROGUECOMBAT) $(ROGUEMAP) $(RPNCALC) $(SANITY) $(SIEVE)
# C Tools Source
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
#
# Platform configuration
#
LIBSRC 		:= libsrc/$(PLASM_PLAT)
SAMPLESRC 	:= samplesrc/$(PLASM_PLAT)
VMSRC 		:= vmsrc/$(PLASM_PLAT)

ifeq (apple, $(PLASM_PLAT))
	PLVMSYS := $(PLVM02)
	ifeq (1, $(PLASM_MODEL))
		PLVMSYS := $(PLVM01)
	else ifeq (///, $(PLASM_MODEL))
		PLVMSYS := $(PLVM03)
	else ifeq (IIGS, $(PLASM_MODEL))
		PLVMSYS := $(PLVM802)
	endif
	LIBS := $(ARGS) $(CONIO) $(DGR) $(DHCP) $(ETHERIP) $(FIBER) $(FILEIO) $(FPSTR)\
		$(FPU) $(INET) $(LONGJMP) $(MEMMGR) $(PORTIO) $(SANE) $(SDFAT) $(SNDSEQ) $(SPIPORT)\
		$(TONE) $(UTHERNET) $(UTHERNET2) $(WIZNET)
else ifeq (bbc, $(PLASM_PLAT))
	PLASM_MODEL := B 	# Only support Model B so far
	PLVMSYS := $(PLVMBBC)
	# LIBS := $(CONIO)
endif
#
# Main targets list
#
tools: $(PLASM) $(PLVM) $(PLASMAPLASM) $(CODEOPT)

lib: tools $(LIBS)

plat: lib $(PLVMSYS)
	$(ECHO) Built for $(PLASM_PLAT) $(PLASM_MODEL).

sample: plat $(SAMPLES)

all: plat sample $(ED) # $(SB) Sandbox source broken

release: all mkrel-$(PLASM_PLAT)
	-bash mkrel-$(PLASM_PLAT)

test: $(SAMPLESRC)/test.pla $(SAMPLESRC)/testlib.pla tools
	./$(PLASM) -AMOW  < $(SAMPLESRC)/test.pla > $(SAMPLESRC)/test.a
	acme --setpc 4094 -o $(TEST) $(SAMPLESRC)/test.a
	./$(PLASM) -AMOW  < $(SAMPLESRC)/testlib.pla > $(SAMPLESRC)/testlib.a
	acme --setpc 4094 -o $(TESTLIB) $(SAMPLESRC)/testlib.a
	./$(PLVM) TEST

clean:
	-rm -f *FE1000 *FF2000 $(PLASM) $(PLVM) $(PLVM01) $(PLVM02) $(PLVM03) $(PLVMBBC) $(CMD)
	-rm -f toolsrc/*.o toolsrc/*~ toolsrc/*.a
	-rm -f vmsrc/*.o vmsrc/*~ $(VMSRC)/*.a $(VMSRC)/*.sym
	-rm -f $(SAMPLESRC)/*.o $(SAMPLESRC)/*~ $(SAMPLESRC)/*.a
	-rm -f $(LIBSRC)/*.a
	
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

$(PLVM01): $(VMSRC)/plvm01.s $(VMSRC)/a1cmd.pla
	./$(PLASM) -AOW < $(VMSRC)/a1cmd.pla > $(VMSRC)/a1cmd.a
	acme -o $(PLVM01) -l $(VMSRC)/plvm01.sym $(VMSRC)/plvm01.s

$(PLVM02): $(VMSRC)/plvm02.s $(VMSRC)/cmd.pla $(VMSRC)/cmdstub.s
	./$(PLASM) -AOW < $(VMSRC)/cmd.pla > $(VMSRC)/cmd.a
	acme --setpc 8192 -o $(CMD) $(VMSRC)/cmdstub.s
	acme -o $(PLVM02) -l $(VMSRC)/plvm02.sym $(VMSRC)/plvm02.s

$(PLVM802): $(VMSRC)/plvm802.s $(VMSRC)/cmd.pla $(VMSRC)/cmdstub.s
	./$(PLASM) -AOW < $(VMSRC)/cmd.pla > $(VMSRC)/cmd.a
	acme --setpc 8192 -o $(CMD) $(VMSRC)/cmdstub.s
	acme -o $(PLVM802) -l $(VMSRC)/plvm802.sym $(VMSRC)/plvm802.s

$(PLVM03): $(VMSRC)/plvm03.s $(VMSRC)/soscmd.pla
	./$(PLASM) -AOW < $(VMSRC)/soscmd.pla > $(VMSRC)/soscmd.a
	acme -o $(PLVM03) -l $(VMSRC)/plvm03.sym $(VMSRC)/plvm03.s

$(PLVMBBC): $(VMSRC)/plvm.s $(VMSRC)/vminit.s $(VMSRC)/cmd.pla
	./$(PLASM) -AOW < $(VMSRC)/cmd.pla > $(VMSRC)/cmd.a
	acme -o $(PLVMBBC) $(VMSRC)/plvm.s
	beebasm -i skel.beebasm -do plasma.ssd

#
# Sample code
#
$(ED): toolsrc/ed.pla
	./$(PLASM) -AMOW < toolsrc/ed.pla > toolsrc/ed.a
	acme --setpc 4094 -o $(ED) toolsrc/ed.a

$(SB): toolsrc/sb.pla plat
	./$(PLASM) -AOW < toolsrc/sb.pla > toolsrc/sb.a
	acme --setpc 8192 -o $(SB) toolsrc/sb.a

$(ARGS): $(LIBSRC)/args.pla
	./$(PLASM) -AMOW < $(LIBSRC)/args.pla > $(LIBSRC)/args.a
	acme --setpc 4094 -o $(ARGS) $(LIBSRC)/args.a

$(MEMMGR): $(LIBSRC)/memmgr.pla
	./$(PLASM) -AMOW < $(LIBSRC)/memmgr.pla > $(LIBSRC)/memmgr.a
	acme --setpc 4094 -o $(MEMMGR) $(LIBSRC)/memmgr.a

$(MEMTEST): $(SAMPLESRC)/memtest.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/memtest.pla > $(SAMPLESRC)/memtest.a
	acme --setpc 4094 -o $(MEMTEST) $(SAMPLESRC)/memtest.a

$(FIBER): $(LIBSRC)/fiber.pla
	./$(PLASM) -AMOW < $(LIBSRC)/fiber.pla > $(LIBSRC)/fiber.a
	acme --setpc 4094 -o $(FIBER) $(LIBSRC)/fiber.a

$(FIBERTEST): $(SAMPLESRC)/fibertest.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/fibertest.pla > $(SAMPLESRC)/fibertest.a
	acme --setpc 4094 -o $(FIBERTEST) $(SAMPLESRC)/fibertest.a

$(SNDSEQ): $(LIBSRC)/sndseq.pla
	./$(PLASM) -AMOW < $(LIBSRC)/sndseq.pla > $(LIBSRC)/sndseq.a
	acme --setpc 4094 -o $(SNDSEQ) $(LIBSRC)/sndseq.a

$(PLAYSEQ): $(SAMPLESRC)/playseq.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/playseq.pla > $(SAMPLESRC)/playseq.a
	acme --setpc 4094 -o $(PLAYSEQ) $(SAMPLESRC)/playseq.a

$(LONGJMP): $(LIBSRC)/longjmp.pla
	./$(PLASM) -AMOW < $(LIBSRC)/longjmp.pla > $(LIBSRC)/longjmp.a
	acme --setpc 4094 -o $(LONGJMP) $(LIBSRC)/longjmp.a

$(MON): $(SAMPLESRC)/mon.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/mon.pla > $(SAMPLESRC)/mon.a
	acme --setpc 4094 -o $(MON) $(SAMPLESRC)/mon.a

$(ROD): $(SAMPLESRC)/rod.pla
	./$(PLASM) -AMO < $(SAMPLESRC)/rod.pla > $(SAMPLESRC)/rod.a
	acme --setpc 4094 -o $(ROD) $(SAMPLESRC)/rod.a

$(SIEVE): $(SAMPLESRC)/sieve.pla
	./$(PLASM) -AMW < $(SAMPLESRC)/sieve.pla > $(SAMPLESRC)/sieve.a
	acme --setpc 4094 -o $(SIEVE) $(SAMPLESRC)/sieve.a

$(UTHERNET): $(LIBSRC)/uthernet.pla
	./$(PLASM) -AMOW < $(LIBSRC)/uthernet.pla > $(LIBSRC)/uthernet.a
	acme --setpc 4094 -o $(UTHERNET) $(LIBSRC)/uthernet.a

$(UTHERNET2): $(LIBSRC)/uthernet2.pla
	./$(PLASM) -AMOW < $(LIBSRC)/uthernet2.pla > $(LIBSRC)/uthernet2.a
	acme --setpc 4094 -o $(UTHERNET2) $(LIBSRC)/uthernet2.a

$(ETHERIP): $(LIBSRC)/etherip.pla
	./$(PLASM) -AMOW < $(LIBSRC)/etherip.pla > $(LIBSRC)/etherip.a
	acme --setpc 4094 -o $(ETHERIP) $(LIBSRC)/etherip.a

$(INET): $(LIBSRC)/inet.pla
	./$(PLASM) -AMOW < $(LIBSRC)/inet.pla > $(LIBSRC)/inet.a
	acme --setpc 4094 -o $(INET) $(LIBSRC)/inet.a

$(DHCP): $(LIBSRC)/dhcp.pla
	./$(PLASM) -AMOW < $(LIBSRC)/dhcp.pla > $(LIBSRC)/dhcp.a
	acme --setpc 4094 -o $(DHCP) $(LIBSRC)/dhcp.a

$(HTTPD): $(SAMPLESRC)/httpd.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/httpd.pla > $(SAMPLESRC)/httpd.a
	acme --setpc 4094 -o $(HTTPD) $(SAMPLESRC)/httpd.a

$(FILEIO): $(LIBSRC)/fileio.pla
	./$(PLASM) -AMOW < $(LIBSRC)/fileio.pla > $(LIBSRC)/fileio.a
	acme --setpc 4094 -o $(FILEIO) $(LIBSRC)/fileio.a

$(CONIO): $(LIBSRC)/conio.pla
	./$(PLASM) -AMOW < $(LIBSRC)/conio.pla > $(LIBSRC)/conio.a
	acme --setpc 4094 -o $(CONIO) $(LIBSRC)/conio.a

$(SANE): $(LIBSRC)/sane.pla
	./$(PLASM) -AMOW < $(LIBSRC)/sane.pla > $(LIBSRC)/sane.a
	acme --setpc 4094 -o $(SANE) $(LIBSRC)/sane.a

$(FPSTR): $(LIBSRC)/fpstr.pla
	./$(PLASM) -AMOW < $(LIBSRC)/fpstr.pla > $(LIBSRC)/fpstr.a
	acme --setpc 4094 -o $(FPSTR) $(LIBSRC)/fpstr.a

$(FPU): $(LIBSRC)/fpu.pla
	./$(PLASM) -AMOW < $(LIBSRC)/fpu.pla > $(LIBSRC)/fpu.a
	acme --setpc 4094 -o $(FPU) $(LIBSRC)/fpu.a

$(SANITY): $(SAMPLESRC)/sanity.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/sanity.pla > $(SAMPLESRC)/sanity.a
	acme --setpc 4094 -o $(SANITY) $(SAMPLESRC)/sanity.a

$(RPNCALC): $(SAMPLESRC)/rpncalc.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/rpncalc.pla > $(SAMPLESRC)/rpncalc.a
	acme --setpc 4094 -o $(RPNCALC) $(SAMPLESRC)/rpncalc.a

$(TONE): $(LIBSRC)/tone.pla
	./$(PLASM) -AMOW < $(LIBSRC)/tone.pla > $(LIBSRC)/tone.a
	acme --setpc 4094 -o $(TONE) $(LIBSRC)/tone.a

$(FATCAT): $(SAMPLESRC)/fatcat.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/fatcat.pla > $(SAMPLESRC)/fatcat.a
	acme --setpc 4094 -o $(FATCAT) $(SAMPLESRC)/fatcat.a

$(FATGET): $(SAMPLESRC)/fatget.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/fatget.pla > $(SAMPLESRC)/fatget.a
	acme --setpc 4094 -o $(FATGET) $(SAMPLESRC)/fatget.a

$(FATPUT): $(SAMPLESRC)/fatput.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/fatput.pla > $(SAMPLESRC)/fatput.a
	acme --setpc 4094 -o $(FATPUT) $(SAMPLESRC)/fatput.a

$(FATWDSK): $(SAMPLESRC)/fatwritedsk.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/fatwritedsk.pla > $(SAMPLESRC)/fatwritedsk.a
	acme --setpc 4094 -o $(FATWDSK) $(SAMPLESRC)/fatwritedsk.a

$(FATRDSK): $(SAMPLESRC)/fatreaddsk.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/fatreaddsk.pla > $(SAMPLESRC)/fatreaddsk.a
	acme --setpc 4094 -o $(FATRDSK) $(SAMPLESRC)/fatreaddsk.a

$(SDFAT): $(LIBSRC)/sdfat.pla
	./$(PLASM) -AMOW < $(LIBSRC)/sdfat.pla > $(LIBSRC)/sdfat.a
	acme --setpc 4094 -o $(SDFAT) $(LIBSRC)/sdfat.a

$(SPIPORT): $(LIBSRC)/spiport.pla
	./$(PLASM) -AMOW < $(LIBSRC)/spiport.pla > $(LIBSRC)/spiport.a
	acme --setpc 4094 -o $(SPIPORT) $(LIBSRC)/spiport.a

$(PORTIO): $(LIBSRC)/portio.pla
	./$(PLASM) -AMOW < $(LIBSRC)/portio.pla > $(LIBSRC)/portio.a
	acme --setpc 4094 -o $(PORTIO) $(LIBSRC)/portio.a

$(DGR): $(LIBSRC)/dgr.pla
	./$(PLASM) -AMOW < $(LIBSRC)/dgr.pla > $(LIBSRC)/dgr.a
	acme --setpc 4094 -o $(DGR) $(LIBSRC)/dgr.a

$(DGRTEST): $(SAMPLESRC)/dgrtest.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/dgrtest.pla > $(SAMPLESRC)/dgrtest.a
	acme --setpc 4094 -o $(DGRTEST) $(SAMPLESRC)/dgrtest.a

$(ROGUE): $(SAMPLESRC)/rogue.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/rogue.pla > $(SAMPLESRC)/rogue.a
	acme --setpc 4094 -o $(ROGUE) $(SAMPLESRC)/rogue.a

$(ROGUECOMBAT): $(SAMPLESRC)/rogue.combat.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/rogue.combat.pla > $(SAMPLESRC)/rogue.combat.a
	acme --setpc 4094 -o $(ROGUECOMBAT) $(SAMPLESRC)/rogue.combat.a

$(ROGUEMAP): $(SAMPLESRC)/rogue.map.pla
	./$(PLASM) -AMOW < $(SAMPLESRC)/rogue.map.pla > $(SAMPLESRC)/rogue.map.a
	acme --setpc 4094 -o $(ROGUEMAP) $(SAMPLESRC)/rogue.map.a

hello:	$(SAMPLESRC)/hello.pla $(PLVM) $(PLASM)
	./$(PLASM) -AMOW < $(SAMPLESRC)/hello.pla > $(SAMPLESRC)/hello.a
	acme --setpc 4094 -o $(HELLO) $(SAMPLESRC)/hello.a
	./$(PLVM) HELLO
