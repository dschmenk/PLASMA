.SUFFIXES	=
AFLAGS	    = -o $@
MBTEST      = mbtest.bin
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

all: $(MBTEST)

clean:
	-rm *.o *~ *.a *.bin

$(MBTEST): test.seq mbtest.pla mbvm.s $(PLASM)
	$(PLASM) -AO < mbtest.pla > mbtest.a
	acme -o $(MBTEST) mbvm.s
