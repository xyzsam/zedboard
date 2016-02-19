.PHONY: all hls

MODE = ENA_SIDECHANNELS
BIN = main
SRCS = main.c accel.c

ifeq ($(MODE),ENA_SIDECHANNELS)
	CPPFLAGS += -D$(MODE)
endif
CFLAGS = -O3 $(CPPFLAGS)

all: $(BIN)

hls:
	vivado_hls ../tcl/hls.tcl

$(BIN) : $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(BIN)
