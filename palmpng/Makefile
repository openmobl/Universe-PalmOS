include Makefile.conf

all:			palmpng.a palmpng.ro palmpng.prc

#testrsc:		test/rsrc
#			$(CD) test/rsrc && $(MAKE)

palmpng.prc: 		palmpng.a palmpng.ro test
			$(CD) test && $(MAKE)

palmpng.a:		src-m68k
			$(CD) src-m68k && $(MAKE)

palmpng.ro:		src-arm
			$(CD) src-arm && $(MAKE)

clean:
			$(RM) *.o *.s *.grc palmpng.prc *.ld palmpng.a palmpng.ro
			$(CD) src-m68k && $(MAKE) clean
			$(CD) src-arm && $(MAKE) clean
			$(CD) test && $(MAKE) clean
			