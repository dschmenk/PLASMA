.SUFFIXES	=
AFLAGS	    = -o $@
HILOPWM     = hilopwm.bin
PLASM       = ../../plasm
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

all: $(HILOPWM)

clean:
	-rm *.o *~ *.a *.bin

$(HILOPWM): a2pwm.s util.pla hilopwm.pla pwmvm.s $(PLASM)
	$(PLASM) -A < hilopwm.pla > hilopwm.a
	acme -o $(HILOPWM) pwmvm.s
