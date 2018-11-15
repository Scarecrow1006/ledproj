OUTFILE=./test
CFLAGS=-I/usr/include/freetype2 -L/usr/local/lib -lfreetype -lm -lpthread
SOURCES=ledp10.c test.c
font:
	gcc -g -o $(OUTFILE) $(SOURCES) $(CFLAGS)
