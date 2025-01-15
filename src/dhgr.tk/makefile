.SUFFIXES	=
AFLAGS	   = -o $@

DHGRLIB    = dhgr.tk/libs/DHGRLIB.REL
DHGRUTILS  = dhgr.tk/libs//DHGRUTILS.REL
DHGRTEST   = dhgr.tk/test/DHGRTEST.REL
FONTTEST   = dhgr.tk/test/FONTTEST.REL
DHGRSPRED  = dhgr.tk/utils/DHGRSPRED.REL
DHGREASY   = dhgr.tk/utils/DHGREASY.REL
DHGRRGB    = dhgr.tk/utils/DHGRRGB.REL
DHGRSHOW   = dhgr.tk/utils/DHGRSHOW.REL
DLGRCONV   = dhgr.tk/utils/DLGRCONV.REL
TTFCONV    = dhgr.tk/utils/ttfconv
IIGSCONV   = dhgr.tk/utils/gsfonconv
DDCONV     = dhgr.tk/utils/ddfonconv
PENGUINS   = dhgr.tk/demos/penguins/PENGUINS.REL


libs: $(DHGRLIB) $(DHGRUTILS)
	cp $(DHGRLIB)   prodos/sys
	cp $(DHGRUTILS) prodos/sys

test: $(DHGRTEST) $(FONTTEST)
	-mkdir prodos/dhgr.tk/
	-mkdir prodos/dhgr.tk/test
	cp dhgr.tk/test/*.REL prodos/dhgr.tk/test
	cp dhgr.tk/test/*.BIN prodos/dhgr.tk/test

utils: $(DHGRSPRED) $(DHGREASY) $(DHGRRGB) $(DHGRSHOW) $(DLGRCONV)
	-mkdir prodos/dhgr.tk/
	-mkdir prodos/dhgr.tk/utils
	-mkdir prodos/dhgr.tk/utils/samples
	cp dhgr.tk/utils/*.REL prodos/dhgr.tk/utils
	cp dhgr.tk/utils/samples/* prodos/dhgr.tk/utils/samples
	cp dhgr.tk/utils/samples/racecar.pnm prodos/dhgr.tk/utils/samples/RACECAR.PNM.BIN

demos: $(PENGUINS)
	-mkdir prodos/dhgr.tk/
	-mkdir prodos/dhgr.tk/demos/
	-mkdir prodos/dhgr.tk/demos/penguins/
	cp $(PENGUINS) prodos/dhgr.tk/demos/penguins/
	cp dhgr.tk/demos/penguins/*.BIN prodos/dhgr.tk/demos/penguins/

fontconv: $(TTFCONV) $(IIGSCONV) $(DDCONV)

all: libs test utils demos

clean:
	-rm dhgr.tk/demos/pengiuns/*.o dhgr.tk/demos/pengiuns/*.REL dhgr.tk/demos/pengiuns/*.a
	-rm dhgr.tk/test/*.o dhgr.tk/test/*.REL dhgr.tk/test/*.a
	-rm dhgr.tk/utils/*.o dhgr.tk/utils/*.REL dhgr.tk/utils/*.a


$(TTFCONV): dhgr.tk/utils/ttfconv.c
	cc `pkg-config --cflags freetype2` dhgr.tk/utils/ttfconv.c -o dhgr.tk/utils/ttfconv -lfreetype

$(IIGSCONV): dhgr.tk/utils/gsfonconv.c
	cc dhgr.tk/utils/gsfonconv.c -o dhgr.tk/utils/gsfonconv

$(DDCONV): dhgr.tk/utils/ddfonconv.c
	cc dhgr.tk/utils/ddfonconv.c -o dhgr.tk/utils/ddfonconv

$(DHGRLIB): dhgr.tk/libs/dhgrlib.pla dhgr.tk/inc/dhgrlib.plh
	./xplasm -AMOW dhgr.tk/libs/dhgrlib.pla
	acme --setpc 4094 -o $(DHGRLIB) dhgr.tk/libs/dhgrlib.a

$(DHGRUTILS): dhgr.tk/libs/dhgrutils.pla $(DHGRLIB) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/libs/dhgrutils.pla
	acme --setpc 4094 -o $(DHGRUTILS) dhgr.tk/libs/dhgrutils.a

$(DHGRTEST): dhgr.tk/test/dhgrtest.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/test/dhgrtest.pla
	acme --setpc 4094 -o $(DHGRTEST) dhgr.tk/test/dhgrtest.a

$(FONTTEST): dhgr.tk/test/fonttest.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/test/fonttest.pla
	acme --setpc 4094 -o $(FONTTEST) dhgr.tk/test/fonttest.a

$(DHGRSPRED): dhgr.tk/utils/dhgrspred.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/utils/dhgrspred.pla
	acme --setpc 4094 -o $(DHGRSPRED) dhgr.tk/utils/dhgrspred.a

$(DHGREASY): dhgr.tk/utils/dhgreasy.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/utils/dhgreasy.pla
	acme --setpc 4094 -o $(DHGREASY) dhgr.tk/utils/dhgreasy.a

$(DHGRSHOW): dhgr.tk/utils/dhgrshow.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/utils/dhgrshow.pla
	acme --setpc 4094 -o $(DHGRSHOW) dhgr.tk/utils/dhgrshow.a

$(DLGRCONV): dhgr.tk/utils/dlgrconv.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/utils/dlgrconv.pla
	acme --setpc 4094 -o $(DLGRCONV) dhgr.tk/utils/dlgrconv.a

$(DHGRRGB): dhgr.tk/utils/dhgrrgb.pla $(DHGRLIB) $(DHGRUTILS) dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/utils/dhgrrgb.pla
	acme --setpc 4094 -o $(DHGRRGB) dhgr.tk/utils/dhgrrgb.a

$(PENGUINS): dhgr.tk/demos/penguins/penguins.pla dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/demos/penguins/penguins.pla
	acme --setpc 4094 -o $(PENGUINS) dhgr.tk/demos/penguins/penguins.a
