.SUFFIXES	=
AFLAGS	= -o $@
LFLAGS	= -C default.cfg
PLVM    = plvm
PLVM02	= PLASMA.SYSTEM\#FF2000
PLVM03	= SOS.INTERP\#050000
CMD     = CMD\#FF2000
ROD     = ROD\#FE1000
HGR1    = HGR1\#FE1000
PLASM   = plasm
INCS    = tokens.h symbols.h lex.h parse.h codegen.h
OBJS	= plasm.c parse.o lex.o codegen.o
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
#RELTYPE	= \#fe1000
#BINTYPE	= \#060000
#SYSTYPE	= \#ff2000
#TXTTYPE	= \#040000

all: $(PLASM) $(PLVM) $(PLVM02) $(PLVM03) $(CMD) $(ROD) $(HGR1)

clean:
	-rm *.o *~ *.a *.SYM $(CMD) *\#FE1000 $(ROD) $(HGR1) $(PLASM) $(PLVM)

$(PLASM): $(OBJS) $(INCS)
	cc $(OBJS) -o $(PLASM)

$(PLVM): plvm.c
	cc plvm.c -o $(PLVM)

cmdexec.a: cmdexec.pla $(PLASM)
	./$(PLASM) -A < cmdexec.pla > cmdexec.a

$(PLVM02): plvm02.s cmdexec.a
	acme -o $(PLVM02) -l PLVM02.SYM plvm02.s

soscmd.a: soscmd.pla $(PLASM)
	./$(PLASM) -A < soscmd.pla > soscmd.a

$(PLVM03): plvm03.s soscmd.a
	acme -o $(PLVM03) -l PLVM03.SYM plvm03.s

$(CMD): cmd.pla cmdstub.s $(PLVM02) $(PLASM)
	./$(PLASM) -A < cmd.pla > cmd.a
	acme --setpc 8192 -o $(CMD) cmdstub.s

test:	test.pla testlib.pla $(PLVM) $(PLASM)
	./$(PLASM) -AM < test.pla > test.a
	acme --setpc 4094 -o TEST#FE1000 test.a
	./$(PLASM) -AM < teslib.pla >testlib.a 
	acme --setpc 4094 -o TESTLIB#FE1000 testlib.a
	./$(PLVM) TEST\#FE1000

$(ROD): rod.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AM < rod.pla > rod.a
	acme --setpc 4094 -o $(ROD) rod.a

$(HGR1):	hgr1.pla hgr1test.pla $(PLVM02) $(PLASM)
	./$(PLASM) -AM < hgr1test.pla > hgr1test.a
	acme --setpc 4094 -o HGR1TEST.REL#FE1000 hgr1test.a
	./$(PLASM) -AM < hgr1.pla > hgr1.a
	acme --setpc 4094 -o $(HGR1) hgr1.a

hello:	hello.pla $(PLVM) $(PLASM)
	m4 < hello.pla | ./$(PLASM) -AM > hello.a
	acme --setpc 4094 -o HELLO#FE1000 hello.a
	./$(PLVM) HELLO#FE1000
