#**************************************************************************************************
#
#	Makefile for PLASMA
#
#**************************************************************************************************

# List of supported platforms.
# Add extra valid platforms to this list as they are supported.

PLASM_PLAT_LIST	:=		apple
#PLASM_PLAT_LIST	+=		bbc
#PLASM_PLAT_LIST	+=		commodore
#PLASM_PLAT_LIST	+=		atari

# List of supported models for each platform

PLASM_APPLE_MODELS	:=	1
PLASM_APPLE_MODELS	+=	][
PLASM_APPLE_MODELS	+=	///
PLASM_APPLE_MODELS	+=	IIGS

#PLASM_BBC_MODELS	:=	B
#PLASM_BBC_MODELS	+=	128

ECHO				:=	@@echo

# PLASM_PLAT and PLASM_MODEL can be defined on the command line
# But defaults to Apple ][
ifndef PLASM_PLAT
PLASM_PLAT 		:= apple
PLASM_MODEL 	:= ][
endif

# Show helpful options.
help:
	$(ECHO) PLASMA Makefile
	$(ECHO) 
	$(ECHO) "Syntax: make <option> [PLASM_PLAT=<platform>] [PLASM_MODEL=<model>]"
	$(ECHO) Default is to build for Apple ][
	$(ECHO)
	$(ECHO) Possible options:
	$(ECHO) make tools ... Build cross-compiler and Unix VM
	$(ECHO) make plat .... Build compiler, libraries, and VM
	$(ECHO) make sample .. Build sample code
	$(ECHO) make all ..... All the above
	$(ECHO)
	$(ECHO) make hello ... Hello World test
	$(ECHO) make test .... Build and run simple tests
	$(ECHO) make clean ... Clean code
	$(ECHO) make release . Build platform distribution
	$(ECHO)
	$(ECHO) make help .... Display this message
	$(ECHO)
	$(ECHO) Possible platforms: $(PLASM_PLAT_LIST)
	$(ECHO) Possible models: $(PLASM_APPLE_MODELS) $(PLASM_BBC_MODELS)
	$(ECHO)

#--------------------------------------------------------------------------------------------------
include Makefile.inc
