.SUFFIXES	=
AFLAGS	    = -o $@
CHAT        = CHAT\#FE1000
UPDATE      = UPDATE\#FE1000
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

all: $(UPDATE) $(CHAT)

clean:
	-rm *FE1000 *FF2000
	-rm *.o *~ *.a

$(UPDATE): update.pla $(PLASM)
	./$(PLASM) -AM < update.pla > update.a
	acme --setpc 4094 -o $(UPDATE) update.a
	cp $(UPDATE) chatserver/clientfiles/UPDATE

$(CHAT): chat.pla $(PLASM)
	./$(PLASM) -AM < chat.pla > chat.a
	acme --setpc 4094 -o $(CHAT) chat.a
	cp $(CHAT) chatserver/clientfiles/CHAT

