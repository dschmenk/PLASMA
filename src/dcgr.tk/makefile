.SUFFIXES	=
AFLAGS	   = -o $@

DCGRLIB    = dcgr.tk/libs/DCGRLIB.REL
DCGRUTILS  = dcgr.tk/libs//DCGRUTILS.REL
DCGRTEST   = dcgr.tk/test/DCGRTEST.REL
DCGRSPRED  = dcgr.tk/utils/DCGRSPRED.REL
DCGREASY   = dcgr.tk/utils/DCGREASY.REL
DCGRRGB    = dcgr.tk/utils/DCGRRGB.REL
DCGRSHOW   = dcgr.tk/utils/DCGRSHOW.REL
PENGUINS   = dcgr.tk/demos/penguins/PENGUINS.REL


libs: $(DCGRLIB) $(DCGRUTILS)
	cp $(DCGRLIB)   prodos/sys
	cp $(DCGRUTILS) prodos/sys

test: $(DCGRTEST)
	-mkdir prodos/dcgr.tk/
	-mkdir prodos/dcgr.tk/test
	cp $(DCGRTEST) prodos/dcgr.tk/test
	cp dcgr.tk/test/*.BIN prodos/dcgr.tk/test

utils: $(DCGRSPRED) $(DCGREASY) $(DCGRRGB) $(DCGRSHOW)
	-mkdir prodos/dcgr.tk/
	-mkdir prodos/dcgr.tk/utils
	-mkdir prodos/dcgr.tk/utils/samples
	cp $(DCGREASY) prodos/dcgr.tk/utils
	cp $(DCGRSPRED) prodos/dcgr.tk/utils
	cp $(DCGRRGB) prodos/dcgr.tk/utils
	cp $(DCGRSHOW) prodos/dcgr.tk/utils
	cp dcgr.tk/utils/samples/* prodos/dcgr.tk/utils/samples

demos: $(PENGUINS)
	-mkdir prodos/dcgr.tk/
	-mkdir prodos/dcgr.tk/demos/
	-mkdir prodos/dcgr.tk/demos/penguins/
	cp $(PENGUINS) prodos/dcgr.tk/demos/penguins/
	cp dcgr.tk/demos/penguins/*.BIN prodos/dcgr.tk/demos/penguins/

all: libs test utils demos

clean:
	-rm dcgr.tk/demos/pengiuns/*.o dcgr.tk/demos/pengiuns/*.REL dcgr.tk/demos/pengiuns/*.a
	-rm dcgr.tk/test/*.o dcgr.tk/test/*.REL dcgr.tk/test/*.a
	-rm dcgr.tk/utils/*.o dcgr.tk/utils/*.REL dcgr.tk/utils/*.a

$(DCGRLIB): dcgr.tk/libs/dcgrlib.pla
	./plasm -AMOW dcgr.tk/libs/dcgrlib.pla
	acme --setpc 4094 -o $(DCGRLIB) dcgr.tk/libs/dcgrlib.a

$(DCGRUTILS): dcgr.tk/libs/dcgrutils.pla $(DCGRLIB)
	./plasm -AMOW dcgr.tk/libs/dcgrutils.pla
	acme --setpc 4094 -o $(DCGRUTILS) dcgr.tk/libs/dcgrutils.a

$(DCGRTEST): dcgr.tk/test/dcgrtest.pla $(DCGRLIB) $(DCGRUTILS)
	./plasm -AMOW dcgr.tk/test/dcgrtest.pla
	acme --setpc 4094 -o $(DCGRTEST) dcgr.tk/test/dcgrtest.a

$(DCGRSPRED): dcgr.tk/utils/dcgrspred.pla $(DCGRLIB) $(DCGRUTILS)
	./plasm -AMOW dcgr.tk/utils/dcgrspred.pla
	acme --setpc 4094 -o $(DCGRSPRED) dcgr.tk/utils/dcgrspred.a

$(DCGREASY): dcgr.tk/utils/dcgreasy.pla $(DCGRLIB) $(DCGRUTILS)
	./plasm -AMOW dcgr.tk/utils/dcgreasy.pla
	acme --setpc 4094 -o $(DCGREASY) dcgr.tk/utils/dcgreasy.a

$(DCGRSHOW): dcgr.tk/utils/dcgrshow.pla $(DCGRLIB) $(DCGRUTILS)
	./plasm -AMOW dcgr.tk/utils/dcgrshow.pla
	acme --setpc 4094 -o $(DCGRSHOW) dcgr.tk/utils/dcgrshow.a

$(DCGRRGB): dcgr.tk/utils/dcgrrgb.pla $(DCGRLIB) $(DCGRUTILS)
	./plasm -AMOW dcgr.tk/utils/dcgrrgb.pla
	acme --setpc 4094 -o $(DCGRRGB) dcgr.tk/utils/dcgrrgb.a

$(PENGUINS): dcgr.tk/demos/penguins/penguins.pla
	./plasm -AMOW dcgr.tk/demos/penguins/penguins.pla
	acme --setpc 4094 -o $(PENGUINS) dcgr.tk/demos/penguins/penguins.a
