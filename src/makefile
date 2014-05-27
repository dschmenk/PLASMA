.SUFFIXES	=
AFLAGS	= -o $@
LFLAGS	= -C default.cfg
PLVM    = plvm
PLVM02	= PLVM02.SYS
CMD     = CMD.SYS
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
#PLATYPE	= \#ed0000
#BINTYPE	= \#060000
#SYSTYPE	= \#ff0000
#TXTTYPE	= \#040000

all: $(PLASM) $(PLVM) $(PLVM02) $(CMD) TESTLIB ROD.REL

clean:
	-rm *.o *~ *.a *.SYM *.SYS *.REL TESTLIB $(PLASM) $(PLVM)

$(PLASM): $(OBJS) $(INCS)
	cc $(OBJS) -o $(PLASM)

$(PLVM): plvm.c
	cc plvm.c -o $(PLVM)

cmdexec.a: cmdexec.pla $(PLASM)
	./$(PLASM) -A < cmdexec.pla > cmdexec.a

$(PLVM02): plvm02.s cmdexec.a
	acme -o $(PLVM02) -l PLVM02.SYM plvm02.s

$(CMD): cmd.pla cmdstub.s $(PLVM) $(PLASM)
	./$(PLASM) -A < cmd.pla > cmd.a
	acme --setpc 8192 -o $(CMD) cmdstub.s

TESTLIB: testlib.pla $(PLVM) $(PLASM)
	./$(PLASM) -AM < testlib.pla > testlib.a
	acme --setpc 4094 -o TESTLIB testlib.a

test: test.pla TESTLIB $(PLVM) $(PLASM)
	./$(PLASM) -AM < test.pla > test.a
	acme --setpc 4094 -o TEST.REL test.a
	./$(PLVM) TEST.REL

TESTCLS: testcls.pla $(PLVM) $(PLASM)
	./$(PLASM) -AM < testcls.pla > testcls.a
	acme --setpc 4094 -o TESTCLS testcls.a

class: class.pla TESTCLS $(PLVM) $(PLASM)
	./$(PLASM) -AM < class.pla > class.a
	acme --setpc 4094 -o CLASS.REL class.a
	./$(PLVM) CLASS.REL

debug: test.pla TESTLIB $(PLVM) $(PLASM)
	./$(PLASM) -AM < test.pla > test.a
	acme --setpc 4094 -o TEST.REL test.a
	./$(PLVM) -s TEST.REL MAIN

hello: hello.pla $(PLVM) $(PLASM)
	./$(PLASM) -AM < hello.pla > hello.a
	acme --setpc 4094 -o HELLO.REL hello.a
	./$(PLVM) HELLO.REL

ROD.REL: rod.pla $(PLVM) $(PLASM)
	./$(PLASM) -AM < rod.pla > rod.a
	acme --setpc 4094 -o ROD.REL rod.a

HGR1:	hgr1.pla hgr1test.pla $(PLVM) $(PLASM)
	./$(PLASM) -AM < hgr1test.pla > hgr1test.a
	acme --setpc 4094 -o HGR1TEST.REL hgr1test.a
	./$(PLASM) -AM < hgr1.pla > hgr1.a
	acme --setpc 4094 -o HGR1 hgr1.a
