KERN=fft
ALG=transpose

CFLAGS?=-std=c99 -O3 -Wall -Wno-unused-label

SRCS=$(KERN).c local_support.c ../../../common/src/support.c
FILES=$(SRCS) $(KERN).h ../../../common/src/support.h

$(KERN): $(FILES) ../../../common/src/machsuite_new_harness.c
	$(CC) $(CFLAGS) -I../../../common/src -o $(KERN) $(SRCS) ../../../common/src/machsuite_new_harness.c -lm

run: $(KERN) input.data check.data
	./$(KERN) input.data check.data

generate: $(FILES) generate.c
	$(CC) $(CFLAGS) -I../../../common/src -o generate $(SRCS) generate.c -lm

hls: $(KERN).c $(KERN).h
	vivado_hls ../tcl/hls.tcl

clean:
	rm -f $(KERN) generate output.data
