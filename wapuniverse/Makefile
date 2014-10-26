#
# Universe - Web Browser
# Copyright (c) 2007 OpenMobl Systems
# Portions (c) 2000-2007 Filip Onkelinx
#
# http://www.openmobl.com/
#

# Build: RELEASE

OUTDIR = Release/
APP = Universe
RESDIR = res/
SRCDIR = src/
PNG_DIR = ../palmpng/

CC = m68k-palmos-gcc 

CFLAGS = -palmos5r3 -g -O2 -Wall
CFLAGS += -I$(PNG_DIR)
#If you have the palmOne SDK, point this to were the headers are located, comment it out if you don't have.
#This affects the 5way support
CFLAGS += -I/palmdev/palmOneSDK/Incs/ -DHAVE_PALMONE_SDK
CFLAGS += -I/palmdev/sony-sdk/Incs/ -I/palmdev/sony-sdk/Incs/Libraries/
CFLAGS += -I/palmdev/sony-sdk/Incs/System/ -DHAVE_SONY_SDK
#CFLAGS += -DENABLE_IMG_THREAD
CFLAGS += -I../Incs/

#SRCFILES =  	frmPrefs.c about.c dbConn.c dbUrl.c formConnectionList.c formEditConnection.c \
#		frmScroll.c rss.c Adjustor.c Entity.c Form.c dbOffline.c libhtmlparse.c xhtml_handler.c xhtml.c \
#		xhtml_string.c xhtml_end.c xhtml_access.c xhtml_tag_misc.c formBrowser.c  \
#		formEditURL.c variable.c  DIA.c resize.c \
#		MenuHnd.c PalmUtils.c WAP.c formUrlList.c \
#		WAPUniverse.c wml.c wsp.c history.c \
#		frmUrl.c frmHistory.c wtp.c wmls.c Bitmap.c \
#		image.c http.c String.c base64.c Tab.c \
#		color.c vfs.c process.c Layout.c Cache.c \
#		URL.c BrowserAPI.c Cookie.c Text.c DOM.c \
#		Browser.c Area.c Font.c FiveWay.c debug.c


SRCFILES =  	frmPrefs.c about.c dbConn.c dbUrl.c formConnectionList.c \
		formEditConnection.c frmScroll.c rss.c Adjustor.c Entity.c \
		Media.c Form.c FileURI.c dbOffline.c libhtmlparse.c xhtml_handler.c xhtml.c \
		xhtml_string.c xhtml_end.c xhtml_access.c xhtml_tag_misc.c \
		debug.c Hyperlinks.c Download.c formBrowser.c Net.c image.c BitmapV3.c \
		http.c formEditURL.c variable.c  DIA.c resize.c MenuHnd.c PalmUtils.c \
		WAP.c formUrlList.c WAPUniverse.c MemMgr.c wml.c wsp.c history.c \
		frmUrl.c frmHistory.c wtp.c wmls.c Bitmap.c String.c base64.c \
		Tab.c color.c vfs.c process.c Layout.c Cache.c URL.c BrowserAPI.c \
		Cookie.c Text.c DOM.c Pointer.c Browser.c Area.c Font.c FiveWay.c \
		Time.c

OBJS = $(SRCFILES:%.c=$(OUTDIR)%.o)

RCPFILE = $(RESDIR)$(APP).rcp

ifeq ($(shell test -e /PalmDev/lib/m68k-palmos-multilink && echo y),y)
MULTILINK = /PalmDev/lib/m68k-palmos-multilink
else
MULTILINK = m68k-palmos-multilink
endif

MULTILINKFLAGS = -fid "Wapu" -L/PalmDev/sdk-5r3/lib/m68k-palmos-coff -L/usr/local/share/palmdev/sdk/lib/m68k-palmos-coff/
MULTILINKFLAGS += -L/usr/m68k-palmos/lib -lc -L/usr/lib/gcc-lib/m68k-palmos/2.95.3-kgpd/mown-gp/ -lgcc -lPalmOSGlue
MULTILINKFLAGS += -gdb-script gdb.script
# This is to deal with the fact that multilink uses C:\PalmDev\lib\ as the default location.
# You can comment out under normal circumstances, unless multilink is not on C:
MULTILINKFLAGS += -libdir /PalmDev/lib/
MULTILINKFLAGS += ../$(PNG_DIR)palmpng.a


all : $(OUTDIR)Universe.prc

$(OUTDIR)$(APP).prc: $(OBJS) $(OUTDIR)bin.stamp $(APP).def $(PNG_DIR)palmpng.ro
	@echo "Building program file $(OUTDIR)Universe.prc..." && \
	(cd $(OUTDIR); $(MULTILINK) $(MULTILINKFLAGS) $(SRCFILES:.c=.o)) && \
	build-prc $(APP).def -o $(OUTDIR)$(APP).prc $(OUTDIR)*.bin $(OUTDIR)*.grc $(PNG_DIR)palmpng.ro

$(OUTDIR)%.o: $(SRCDIR)%.c 
	@echo "Compiling $<..." && \
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)bin.stamp: $(RCPFILE)
	@echo "Compiling resource $(RCPFILE)..." && \
	( cd $(RESDIR); pilrc -allowEditID -I ../ ../$(RCPFILE)) && \
	( cd $(RESDIR); mv -f *.bin ../$(OUTDIR)) && \
	touch $(OUTDIR)bin.stamp

png:
	@echo "Building PNG components..." && \
	(cd $(PNG_DIR); make) && \
	cd ../wapuniverse

clean:
	@echo "Cleaning up..." && \
	rm -f $(OUTDIR)*.o && \
	rm -f $(OUTDIR)*.bin $(OUTDIR)$(APP) $(OUTDIR)$(APP).prc $(OUTDIR)bin.stamp && \
	rm -f $(OUTDIR)*.grc $(OUTDIR)*.out $(OUTDIR)*.script && \
	cd ../../
