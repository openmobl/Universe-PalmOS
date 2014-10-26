include ../../Makefile.conf

resources.ro: 		main.rcp
			$(PILRC) -ro -H resources.h main.rcp resources.ro

clean:
			$(RM) *.o *.s *.grc *.ld resources.ro resources.h
