#
# makefile for GNU make on UNIX
#

AS	= nasm
AOPT	= -f bin

ALL = GETBIOS.COM PWOFF.COM HOSTDRV.COM
SRCS = $(ALL:.COM=.ASM)
LSTS = $(ALL:.COM=.LST)

all: $(ALL)

.SUFFIXES: .ASM .COM
.ASM.COM:
	$(AS) $(AOPT) $< -o $@ -l $*.LST

GETBIOS.COM:
PWOFF.COM: PWOFF.ASM
HOSTDRV.COM: HOSTDRV.ASM HOSTDRV.INC

clean:
	@rm -f $(ALL) $(LSTS)
