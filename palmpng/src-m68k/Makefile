include ../Makefile.conf

palmpng.a:		peal.o palmpng.o
			$(AR) palmpng.a peal.o palmpng.o
			$(CP) palmpng.a ../

palmpng.o:		palmpng.c peal.h macros.h ../palmpng.h
			$(CCC) palmpng.c

peal.o:			peal.c peal.h elf.h
			$(CCC) peal.c

elf.h:			elf32.h			
			

elf32.h:		elf_common.h			
			

clean:
			$(RM) *.o *.s *.grc *.ld m68k.prg
