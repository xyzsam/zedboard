.PHONY: hls generate

hls:
	vivado_hls ../tcl/hls.tcl

generate:
	$(CC) -m32 -o generate generate.c lfsr.c
