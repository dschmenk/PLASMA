.SUFFIXES	=
AFLAGS	   = -o $@

DHGRLIB       = dhgr.tk/libs/DHGRLIB.REL
DHGRUTILS     = dhgr.tk/libs//DHGRUTILS.REL
DHGRTEST      = dhgr.tk/test/DHGRTEST.REL
FONTTEST      = dhgr.tk/test/FONTTEST.REL
DHGRSPRED     = dhgr.tk/utils/DHGRSPRED.REL
DHGREASY      = dhgr.tk/utils/DHGREASY.REL
DHGRRGB       = dhgr.tk/utils/DHGRRGB.REL
DHGRSHOW      = dhgr.tk/utils/DHGRSHOW.REL
DLGRCONV      = dhgr.tk/utils/DLGRCONV.REL
TTFCONV       = dhgr.tk/utils/ttfconv
IIGSCONV      = dhgr.tk/utils/gsfonconv
A2CONV        = dhgr.tk/utils/a2fonconv
A2DEFAULT     = dhgr.tk/utils/a2fondef
DDCONV        = dhgr.tk/utils/ddfonconv
PENGUINS      = dhgr.tk/demos/penguins/PENGUINS.REL
DARTHGR       = dhgr.tk/demos/darthgr/DARTHGR.REL
DROGUE        = dhgr.tk/drogue/DROGUE.REL
DROGUE.MAP    = dhgr.tk/drogue/DROGUE.MAP.REL
DROGUE.COMBAT = dhgr.tk/drogue/DROGUE.COMBAT.REL


libs: $(DHGRLIB) $(DHGRUTILS)
	cp $(DHGRLIB)   prodos/sys
	cp $(DHGRUTILS) prodos/sys
	-mkdir prodos/sys/fonts
	cp dhgr.tk/utils/fonts/APPLE2.FON.BIN prodos/sys/fonts
	cp dhgr.tk/utils/fonts/APPLE2THIN.FON.BIN prodos/sys/fonts
	cp dhgr.tk/utils/fonts/A2SLIM8.FON.BIN prodos/sys/fonts

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

demos: $(PENGUINS) $(DARTHGR)
	-mkdir prodos/dhgr.tk/
	-mkdir prodos/dhgr.tk/demos/
	-mkdir prodos/dhgr.tk/demos/penguins/
	-mkdir prodos/dhgr.tk/demos/darthgr/
	cp $(PENGUINS) prodos/dhgr.tk/demos/penguins/
	cp dhgr.tk/demos/penguins/*.BIN prodos/dhgr.tk/demos/penguins/
	cp $(DARTHGR) prodos/dhgr.tk/demos/darthgr/
	cp dhgr.tk/demos/darthgr/*.BIN prodos/dhgr.tk/demos/darthgr/

drogue: $(DROGUE) $(DROGUE.MAP) $(DROGUE.COMBAT)
	-mkdir prodos/dhgr.tk/demos
	-mkdir prodos/dhgr.tk/demos/drogue
	cp dhgr.tk/drogue/DROGUE.REL prodos/dhgr.tk/demos/drogue
	cp dhgr.tk/drogue/DROGUE.MAP.REL prodos/dhgr.tk/demos/drogue/DROGUEMAP.REL
	cp dhgr.tk/drogue/DROGUE.COMBAT.REL prodos/dhgr.tk/demos/drogue/DROGUECOMBAT.REL
	cp samplesrc/LEVEL0#040000 prodos/dhgr.tk/demos/drogue/LEVEL0.TXT
	cp samplesrc/LEVEL1#040000 prodos/dhgr.tk/demos/drogue/LEVEL1.TXT

bld: $(DHGRLIB) $(DHGRUTILS) $(DHGRTEST) $(DHGRRGB) $(DHGRSHOW)
	-mkdir prodos/bld/dhgr.tk/
	-mkdir prodos/bld/dhgr.tk/inc/
	cp dhgr.tk/inc/dhgrlib.plh prodos/bld/dhgr.tk/inc/DHGRLIB.PLH.TXT
	cp dhgr.tk/inc/dhgrutils.plh prodos/bld/dhgr.tk/inc/DHGRUTILS.PLH.TXT
	cp dhgr.tk/test/dhgrtest.pla prodos/bld/dhgr.tk/DHGRTEST.PLA.TXT
	cp dhgr.tk/test/fonttest.pla prodos/bld/dhgr.tk/FONTTEST.PLA.TXT
	cp dhgr.tk/demos/darthgr/darthgr.pla prodos/bld/dhgr.tk/DARTHGR.PLA.TXT
	cp dhgr.tk/utils/dhgrrgb.pla prodos/bld/dhgr.tk/DHGRRGB.PLA.TXT
	cp dhgr.tk/utils/dhgrshow.pla prodos/bld/dhgr.tk/DHGRSHOW.PLA.TXT

fontconv: $(TTFCONV) $(IIGSCONV) $(A2CONV) $(A2DEFAULT) $(DDCONV)

all: libs test utils demos bld

clean:
	-rm dhgr.tk/demos/pengiuns/*.o dhgr.tk/demos/pengiuns/*.REL dhgr.tk/demos/pengiuns/*.a
	-rm dhgr.tk/demos/darthgr/*.o dhgr.tk/demos/darthgr/*.REL dhgr.tk/demos/darthgr/*.a
	-rm dhgr.tk/test/*.o dhgr.tk/test/*.REL dhgr.tk/test/*.a
	-rm dhgr.tk/utils/*.o dhgr.tk/utils/*.REL dhgr.tk/utils/*.a


$(TTFCONV): dhgr.tk/utils/ttfconv.c
	cc `pkg-config --cflags freetype2` dhgr.tk/utils/ttfconv.c -o dhgr.tk/utils/ttfconv -lfreetype

$(IIGSCONV): dhgr.tk/utils/gsfonconv.c
	cc dhgr.tk/utils/gsfonconv.c -o dhgr.tk/utils/gsfonconv

$(A2CONV): dhgr.tk/utils/a2fonconv.c
	cc dhgr.tk/utils/a2fonconv.c -o dhgr.tk/utils/a2fonconv

$(A2DEFAULT): dhgr.tk/utils/a2fondef.c
	cc dhgr.tk/utils/a2fondef.c -o dhgr.tk/utils/a2fondef

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

$(DARTHGR): dhgr.tk/demos/darthgr/darthgr.pla dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/demos/darthgr/darthgr.pla
	acme --setpc 4094 -o $(DARTHGR) dhgr.tk/demos/darthgr/darthgr.a

$(DROGUE): dhgr.tk/drogue/drogue.pla  dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/drogue/drogue.pla
	acme --setpc 4094 -o $(DROGUE) dhgr.tk/drogue/drogue.a

$(DROGUE.MAP): dhgr.tk/drogue/drogue.map.pla dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/drogue/drogue.map.pla
	acme --setpc 4094 -o $(DROGUE.MAP) dhgr.tk/drogue/drogue.map.a

$(DROGUE.COMBAT): dhgr.tk/drogue/drogue.combat.pla dhgr.tk/inc/dhgrlib.plh dhgr.tk/inc/dhgrutils.plh
	./xplasm -AMOW dhgr.tk/drogue/drogue.combat.pla
	acme --setpc 4094 -o $(DROGUE.COMBAT) dhgr.tk/drogue/drogue.combat.a
