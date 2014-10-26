WARNINGS = -W -Wall -Wpointer-arith

OPTIMIZATION = -O0 

all: armc.ro

armc.ro: arm.c arm2.c ../../arm/pealstub.c ../../arm/pealstub.h
	arm-palmos-gcc -fPIC -Os -march=armv4t -c ../../arm/pealstub.c -o pealstub.o

	arm-palmos-gcc -fPIC -Os -Wl,--split-by-file=1 -march=armv4t -msingle-pic-base -Wl,--emit-relocs -nostartfiles $(WARNINGS) -I../../arm arm.c arm2.c pealstub.o -o armc03e8.bin
	../../postlink/peal-postlink -v -s 1000 -o armc.ro armc03e8.bin
	../../postlink/peal-postlink -v -o armc.unsplit armc03e8.bin

clean: 
	rm -f *.o *~ armc.ro armc03e8.bin
