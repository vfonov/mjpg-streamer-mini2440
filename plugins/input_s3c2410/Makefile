###############################################################
#
# Purpose: Makefile for "M-JPEG Streamer"
# Author.: Tom Stoeveken (TST)
# Version: 0.3
# License: GPL
#
###############################################################

CC = arm-linux-gcc

OTHER_HEADERS = ../../mjpg_streamer.h ../../utils.h ../output.h ../input.h

CFLAGS += -O3 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC 
#CFLAGS += -DDEBUG -O2 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC
LFLAGS += -lpthread -ldl 

all: input_s3c2410.so

clean:
	rm -f *.a *.o core *~ *.so *.lo

input_s3c2410.so: $(OTHER_HEADERS) input_s3c2410.c s3c2410.lo utils.lo encoder.lo huffman.lo marker.lo quant.lo
	$(CC) $(CFLAGS) -o $@ input_s3c2410.c s3c2410.lo utils.lo encoder.lo huffman.lo marker.lo quant.lo

s3c2410.lo:	s3c2410.c s3c2410.h
	$(CC) -c $(CFLAGS) -o $@ $<
		
utils.lo:	utils.c utils.h
	$(CC) -c $(CFLAGS) -o $@ $<

encoder.lo:	encoder.c encoder.h
	$(CC) -c $(CFLAGS) -o $@ $<

huffman.lo:	huffman.c huffman.h
	$(CC) -c $(CFLAGS) -o $@ $<

marker.lo:	marker.c marker.h
	$(CC) -c $(CFLAGS) -o $@ $<

quant.lo:	quant.c quant.h
	$(CC) -c $(CFLAGS) -o $@ $<
