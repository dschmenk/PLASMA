.SUFFIXES	=
AFLAGS	   = -o $@

IMPORT   = IMPORT.REL
SEAN     = SEAN.REL
ARTIFACT = ARTIFACT.REL

gg: $(SEAN) $(IMPORT) $(ARTIFACT)
	-mkdir ../../prodos/dhgr.tk/gg/
	cp $(SEAN) ../../prodos/dhgr.tk/gg
	cp $(IMPORT) ../../prodos/dhgr.tk/gg
	cp $(ARTIFACT) ../../prodos/dhgr.tk/gg

all: gg

clean:
	-rm *.REL *.a

$(SEAN): sean.pla sean.plh
	../../xplasm -AMOW sean.pla
	acme --setpc 4094 -o $(SEAN) sean.a

$(IMPORT): import.pla sean.plh
	../../xplasm -AMOW import.pla
	acme --setpc 4094 -o $(IMPORT) import.a

$(ARTIFACT): artifact.pla
	../../xplasm -AMOW artifact.pla
	acme --setpc 4094 -o $(ARTIFACT) artifact.a
