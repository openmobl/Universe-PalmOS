include ../Makefile.conf

test.prc:		rsrc/resources.ro rsrc/resources.h palmpng.def palmpng.prg ../palmpng.ro
			$(BUILD) palmpng.def palmpng.prg rsrc/resources.ro ../palmpng.ro -o palmpng.prc
			$(CP) palmpng.prc ../

palmpng.prg: 		../palmpng.a test.c ../palmpng.h rsrc/resources.h
			$(CCL) test.c ../palmpng.a $(GLUE) -o palmpng.prg

rsrc/resources.h:	
			$(CD) rsrc && $(MAKE)

rsrc/resources.ro:	
			$(CD) rsrc && $(MAKE)
clean:
			$(RM) *.o *.s *.grc *.ld palmpng.prg palmpng.prc
			$(CD) rsrc && $(MAKE) clean
