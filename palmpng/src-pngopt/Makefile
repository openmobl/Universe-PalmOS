include ../Makefile.conf

pngopt:			pngopt.c 
			$(CCLX86) pngopt.c -o pngopt -lm -lpng -lz
			$(CP) pngopt ../

clean:
			$(RM) *.o *.s pngopt
