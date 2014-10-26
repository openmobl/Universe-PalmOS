# 
# Copyright (c) 2004, TapTarget. All rights reserved.
# Copyright (c) 2002-2004, Yuri Plaksyuk (york@noir.crocodile.org).
#
# http://www.taptarget.com/
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
#      - Redistributions of source code must retain the above
#      copyright notice, this list of conditions and the following
#      disclaimer.
# 
#      - Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials
#      provided with the distribution.
# 
#      - All advertising materials mentioning features or use of this
#      software must display the following acknowledgement: This
#      product includes software developed by TapTarget.
# 
#      - The name of TapTarget may not be used to endorse or
#      promote products derived from this software without specific
#      prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY TAPTARGET "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL TAPTARGET BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# $Id: Makefile,v 1.6 2004/05/17 08:46:13 york Exp $ 
#

include ../version.inc

TARGET  = iscript
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR)

CFLAGS         = -Wall -nostartfiles -DPILOT -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR)
DEBUG_CFLAGS   = $(CFLAGS) -g -DDEBUG
RELEASE_CFLAGS = $(CFLAGS)


OBJS = \
    iscript.o \
    wmls.o \
    wmlslib.o \
    MathLib.o

DEBUG_OBJS     = $(addprefix debug/, $(OBJS))
RELEASE_OBJS   = $(addprefix release/, $(OBJS))

BINS = *.o *.s *.bin *.res $(TARGET)

##############################################################################

CC       = m68k-palmos-gcc
AS       = m68k-palmos-as
OBJRES   = m68k-palmos-obj-res
MULTIGEN = m68k-palmos-multigen
STUBGEN  = m68k-palmos-stubgen
OBJDUMP  = m68k-palmos-objdump
GDB      = m68k-palmos-gdb
BUILDPRC = build-prc
PILRC    = pilrc
SED      = sed

##############################################################################

debug: debug/$(TARGET).prc

release: release/$(TARGET).prc

all: debug release

clean:
	@rm -f $(TARGET).rcp
	@rm -f $(addprefix debug/, $(BINS))
	@rm -f $(addprefix release/, $(BINS))

veryclean: clean
	@rm -f debug/$(TARGET).prc
	@rm -f release/$(TARGET).prc

gdb:
	$(GDB) debug/$(TARGET) --command=$(TARGET).gdbrc

##############################################################################

debug/%.o : %.c
	@echo "Compiling $<..." && \
	$(CC) $(DEBUG_CFLAGS) -o $@ -c $<

release/%.o : %.c
	@echo "Compiling $<..." && \
	$(CC) $(RELEASE_CFLAGS) -o $@ -c $<

##############################################################################
# Resources

#$(TARGET).rcp : $(TARGET).rcp.template
#	@$(SED) -e "s/__version/$(VERSION)/g" $< > $@


debug/bin.res : $(TARGET).rcp
	$(PILRC) -q -D DEBUG $(TARGET).rcp debug/
	@touch debug/bin.res

release/bin.res : $(TARGET).rcp
	$(PILRC) -q $(TARGET).rcp release/
	@touch release/bin.res

##############################################################################
# Dispatch table generation

debug/$(TARGET)-dispatch.o: $(TARGET).def
	@echo "Compiling dispatch stub..."
	@cd debug; $(STUBGEN) ../$(TARGET).def
	@$(CC) $(DEBUG_CFLAGS) -o $@ -c debug/$(TARGET)-dispatch.s

release/$(TARGET)-dispatch.o: $(TARGET).def
	@echo "Compiling dispatch stub..."
	@cd release; $(STUBGEN) ../$(TARGET).def
	@$(CC) $(RELEASE_CFLAGS) -o $@ -c release/$(TARGET)-dispatch.s

##############################################################################
# Binary assembly

debug/$(TARGET) : $(DEBUG_OBJS) debug/$(TARGET)-dispatch.o
	@echo "Linking..." && \
	$(CC) $(DEBUG_CFLAGS) -o $@ $^ -g

release/$(TARGET) : $(RELEASE_OBJS) release/$(TARGET)-dispatch.o
	@echo "Linking..." && \
	$(CC) $(RELEASE_CFLAGS) -o $@ $^

##############################################################################
# Building PRC

debug/$(TARGET).prc : debug/$(TARGET) debug/bin.res
	@echo "Building program file $@..." && \
	cd debug/ && \
	$(BUILDPRC) -o $(TARGET).prc ../$(TARGET).def *.bin $(TARGET)

release/$(TARGET).prc : release/$(TARGET) release/bin.res
	@echo "Building program file $@..." && \
	cd release/ && \
	$(BUILDPRC) -o $(TARGET).prc ../$(TARGET).def *.bin $(TARGET)
