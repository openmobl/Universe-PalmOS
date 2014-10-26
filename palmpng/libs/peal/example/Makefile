WARNINGS = -W -Wall -Wpointer-arith

OPTIMIZATION = -O0

all: pealexample.prc

pealexample.prc: pealexample.def pealexample.o arm/armc.ro pealexample.ro
	build-prc -o pealexample.prc pealexample.def pealexample.o pealexample.ro arm/armc.ro 

arm/armc.ro: arm
	make -C arm

pealexample.ro: pealexample.rcp
	pilrc -ro pealexample.rcp pealexample.ro

pealexample.o: m68k.o peal.o
	m68k-palmos-gcc m68k.o peal.o -o pealexample.o -lPalmOSGlue

m68k.o: m68k.c ../m68k/peal.h
	m68k-palmos-gcc $(OPTIMIZATION) $(WARNINGS) -I../m68k -c m68k.c -o m68k.o

peal.o: ../m68k/peal.c ../m68k/peal.h ../m68k/elf.h ../m68k/elf_common.h ../m68k/elf32.h
	m68k-palmos-gcc $(OPTIMIZATION) $(WARNINGS) -I../m68k -c ../m68k/peal.c -o peal.o


clean:
	rm -f *~ *.o *.ro pealexample.o pealexample.prc
	make -C arm clean