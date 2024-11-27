.SUFFIXES	=
AFLAGS	   = -o $@

DHGRLIB    = dhgr.tk/libs/DHGRLIB.REL
DHGRUTILS  = dhgr.tk/libs//DHGRUTILS.REL
DHGRTEST   = dhgr.tk/test/DHGRTEST.REL
DHGRSPRED  = dhgr.tk/utils/DHGRSPRED.REL
DHGREASY   = dhgr.tk/utils/DHGREASY.REL
DHGRRGB    = dhgr.tk/utils/DHGRRGB.REL
DHGRSHOW   = dhgr.tk/utils/DHGRSHOW.REL
DLGRSHOW   = dhgr.tk/utils/DLGRSHOW.REL
PENGUINS   = dhgr.tk/demos/penguins/PENGUINS.REL


libs: $(DHGRLIB) $(DHGRUTILS)
	cp $(DHGRLIB)   prodos/sys
	cp $(DHGRUTILS) prodos/sys

test: $(DHGRTEST)
	-mkdir prodos/dhgr.tk/
	-mkdir prodos/dhgr.tk/test
	cp $(DHGRTEST) prodos/dhgr.tk/test
	cp dhgr.tk/test/*.BIN prodos/dhgr.tk/test

utils: $(DHGRSPRED) $(DHGREASY) $(DHGRRGB) $(DHGRSHOW) $(DLGRSHOW)
	-mkdir prodos/dhgr.tk/
	-mkdir prodos/dhgr.tk/utils
	-mkdir prodos/dhgr.tk/utils/samples
	cp dhgr.tk/utils/*.REL prodos/dhgr.tk/utils
	cp dhgr.tk/utils/samples/* prodos/dhgr.tk/utils/samples

demos: $(PENGUINS)
	-mkdir prodos/dhgr.tk/
	-mkdir prodos/dhgr.tk/demos/
	-mkdir prodos/dhgr.tk/demos/penguins/
	cp $(PENGUINS) prodos/dhgr.tk/demos/penguins/
	cp dhgr.tk/demos/penguins/*.BIN prodos/dhgr.tk/demos/penguins/

all: libs test utils demos

clean:
	-rm dhgr.tk/demos/pengiuns/*.o dhgr.tk/demos/pengiuns/*.REL dhgr.tk/demos/pengiuns/*.a
	-rm dhgr.tk/test/*.o dhgr.tk/test/*.REL dhgr.tk/test/*.a
	-rm dhgr.tk/utils/*.o dhgr.tk/utils/*.REL dhgr.tk/utils/*.a

$(DHGRLIB): dhgr.tk/libs/dhgrlib.pla dhgr.tk/inc/dhgrlib.plh
	./plasm -AMOW dhgr.tk/libs/dhgrlib.pla
	acme --setpc 4094 -o $(DHGRLIB) dhgr.tk/libs/dhgrlib.a

$(DHGRUTILS): dhgr.tk/libs/dhgrutils.pla $(DHGRLIB) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./plasm -AMOW dhgr.tk/libs/dhgrutils.pla
	acme --setpc 4094 -o $(DHGRUTILS) dhgr.tk/libs/dhgrutils.a

$(DHGRTEST): dhgr.tk/test/dhgrtest.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./plasm -AMOW dhgr.tk/test/dhgrtest.pla
	acme --setpc 4094 -o $(DHGRTEST) dhgr.tk/test/dhgrtest.a

$(DHGRSPRED): dhgr.tk/utils/dhgrspred.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./plasm -AMOW dhgr.tk/utils/dhgrspred.pla
	acme --setpc 4094 -o $(DHGRSPRED) dhgr.tk/utils/dhgrspred.a

$(DHGREASY): dhgr.tk/utils/dhgreasy.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./plasm -AMOW dhgr.tk/utils/dhgreasy.pla
	acme --setpc 4094 -o $(DHGREASY) dhgr.tk/utils/dhgreasy.a

$(DHGRSHOW): dhgr.tk/utils/dhgrshow.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./plasm -AMOW dhgr.tk/utils/dhgrshow.pla
	acme --setpc 4094 -o $(DHGRSHOW) dhgr.tk/utils/dhgrshow.a

$(DLGRSHOW): dhgr.tk/utils/dlgrshow.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./plasm -AMOW dhgr.tk/utils/dlgrshow.pla
	acme --setpc 4094 -o $(DLGRSHOW) dhgr.tk/utils/dlgrshow.a

$(DHGRRGB): dhgr.tk/utils/dhgrrgb.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./plasm -AMOW dhgr.tk/utils/dhgrrgb.pla
	acme --setpc 4094 -o $(DHGRRGB) dhgr.tk/utils/dhgrrgb.a

$(PENGUINS): dhgr.tk/demos/penguins/penguins.pla dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./plasm -AMOW dhgr.tk/demos/penguins/penguins.pla
	acme --setpc 4094 -o $(PENGUINS) dhgr.tk/demos/penguins/penguins.a
