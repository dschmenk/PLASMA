.SUFFIXES	=
AFLAGS	    = -o $@
MBTEST      = mbtest.bin
SPKRTEST    = spkrtest.bin
SEQPLAY     = seqplay.rel
PLASM       = ../plasm
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

all: $(SEQPLAY)

clean:
	-rm *.o *~ *.a *.bin

$(MBTEST): test.seq mbtest.pla mbvm.s $(PLASM)
	$(PLASM) -AO < mbtest.pla > mbtest.a
	acme -o $(MBTEST) mbvm.s

$(SPKRTEST): test.seq spkrtest.pla spkrvm.s $(PLASM)
	$(PLASM) -AO < spkrtest.pla > spkrtest.a
	acme -o $(SPKRTEST) spkrvm.s

$(SEQPLAY): seqplay.pla test.seq $(PLASM)
	$(PLASM) -AOM < seqplay.pla > seqplay.a
	acme --setpc 4094 -o $(SEQPLAY) seqplay.a
	acme --setpc 1024 -o testseq.bin test.seq
