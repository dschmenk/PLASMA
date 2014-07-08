.SUFFIXES	=
AFLAGS	= -o $@
PLVM    = plvm
PLVM01  = A1PLASMA\#060280
PLVM02	= PLASMA.SYSTEM\#FF2000
PLVM03	= SOS.INTERP\#050000
CMD     = CMD\#FF2000
ROD     = ROD\#FE1000
SIEVE   = SIEVE\#FE1000
HELLO   = HELLO\#FE1000
HGR1    = HGR1\#FE1000
HGR1TEST= HGR1TEST\#FE1000
TEST    = TEST\#FE1000
TESTLIB = TESTLIB\#FE1000
PROFILE = PROFILE\#FE1000
PLASM   = plasm
INCS    = toolsrc/tokens.h toolsrc/symbols.h toolsrc/lex.h toolsrc/parse.h toolsrc/codegen.h
OBJS	= toolsrc/plasm.c toolsrc/parse.o toolsrc/lex.o toolsrc/codegen.o
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

all: $(PLASM) $(PLVM) $(PLVM01) $(PLVM02) $(PLVM03) $(CMD) $(PROFILE) $(ROD) $(SIEVE) $(HGR1)

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

#
# PLASMA VMs
#
$(PLVM): vmsrc/plvm.c
	cc vmsrc/plvm.c -o $(PLVM)

vmsrc/a1cmd.a: vmsrc/a1cmd.pla $(PLASM)
	./$(PLASM) -A < vmsrc/a1cmd.pla > vmsrc/a1cmd.a

$(PLVM01): vmsrc/plvm01.s vmsrc/a1cmd.a
	acme -o $(PLVM01) -l vmsrc/plvm01.sym vmsrc/plvm01.s

$(CMD): vmsrc/cmd.pla vmsrc/cmdstub.s $(PLVM02) $(PLASM)
	./$(PLASM) -A < vmsrc/cmd.pla > vmsrc/cmd.a
	acme --setpc 8192 -o $(CMD) vmsrc/cmdstub.s

$(PLVM02): vmsrc/plvm02.s
	acme -o $(PLVM02) -l vmsrc/plvm02.sym vmsrc/plvm02.s

vmsrc/soscmd.a: vmsrc/soscmd.pla $(PLASM)
	./$(PLASM) -A < vmsrc/soscmd.pla > vmsrc/soscmd.a

$(PLVM03): vmsrc/plvm03.s vmsrc/soscmd.a
	acme -o $(PLVM03) -l vmsrc/plvm03.sym vmsrc/plvm03.s

#
# Sample code
#
test:	samplesrc/test.pla samplesrc/testlib.pla $(PLVM) $(PLASM)
	m4 -I inc < samplesrc/test.pla | ./$(PLASM) -AM  > samplesrc/test.a
	acme --setpc 4094 -o $(TEST) samplesrc/test.a
	m4 -I inc < samplesrc/testlib.pla | ./$(PLASM) -AM  > samplesrc/testlib.a
	acme --setpc 4094 -o $(TESTLIB) samplesrc/testlib.a
	./$(PLVM) TEST

$(ROD): samplesrc/rod.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AM < samplesrc/rod.pla > samplesrc/rod.a
	acme --setpc 4094 -o $(ROD) samplesrc/rod.a

$(SIEVE): samplesrc/sieve.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AM < samplesrc/sieve.pla > samplesrc/sieve.a
	acme --setpc 4094 -o $(SIEVE) samplesrc/sieve.a

$(PROFILE): samplesrc/profile.pla $(PLVM02) $(PLASM)
	m4 -I inc < samplesrc/profile.pla | ./$(PLASM) -AM > samplesrc/profile.a
	acme --setpc 4094 -o $(PROFILE) samplesrc/profile.a

$(HGR1):	samplesrc/hgr1.pla samplesrc/hgr1test.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AM < samplesrc/hgr1test.pla > samplesrc/hgr1test.a
	acme --setpc 4094 -o $(HGR1TEST) samplesrc/hgr1test.a
	./$(PLASM) -AM < samplesrc/hgr1.pla > samplesrc/hgr1.a
	acme --setpc 4094 -o $(HGR1) samplesrc/hgr1.a

hello:	samplesrc/hello.pla $(PLVM) $(PLASM)
	m4 -I inc < samplesrc/hello.pla | ./$(PLASM) -AM > samplesrc/hello.a
	acme --setpc 4094 -o $(HELLO) samplesrc/hello.a
	./$(PLVM) HELLO
