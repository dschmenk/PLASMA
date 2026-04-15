.SUFFIXES	=
AFLAGS	   	= -o $@

SHRLIB		= shr.tk/libs/SHRLIB.REL

libs: $(SHRLIB)
	cp $(SHRLIB)   prodos/sys

bld: $(SHRLIB)
	-mkdir prodos/bld/shr.tk/
	-mkdir prodos/bld/shr.tk/inc/
	cp shr.tk/inc/shrlib.plh prodos/bld/shr.tk/inc/SHRLIB.PLH.TXT
	cp shr.tk/shrtest.pla prodos/bld/shr.tk/SHRTEST.PLA.TXT

all: libs bld

clean:

$(SHRLIB): shr.tk/libs/shrlib.pla shr.tk/inc/shrlib.plh
	./xplasm -AMOW shr.tk/libs/shrlib.pla
	acme --setpc 4094 -o $(SHRLIB) shr.tk/libs/shrlib.a

