include ../Makefile.conf

palmpng.ro:		palmpng.o
			$(PEAL) -s 1000 -o palmpng.ro palmpng.o
			$(CP) palmpng.ro ../

palmpng.o:		*.c *.h
			$(CCLARM) *.c -o palmpng.o

clean:
			$(RM) *.o *.s *.grc *.ld *.prg *.arm *.ro
