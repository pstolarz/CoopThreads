.SILENT:
.PHONY: all build clean lib libclean

LIBDIR=../../src
CFLAGS+=-Wall -DCOOP_CONFIG_FILE="\"test_config.h\"" -I$(LIBDIR) -I.

LIBOBJS=\
    $(LIBDIR)/coop_threads.o \
    $(LIBDIR)/platform/unix.o

TESTS=\
    t01_sched_switch \
    t02_idle \
    t03_idle_run \
    t04_yield_after

t02_idle: TDEFS=-DT02
t03_idle_run: TDEFS=-DT03
t04_yield_after: TDEFS=-DT04

all: build
	for t in $(TESTS); do echo "TEST: $$t"; ./$$t; echo; done;

build: $(TESTS)

lib: libclean $(LIBOBJS)

clean: libclean
	$(RM) *.o $(TESTS)

libclean:
	$(RM) $(LIBDIR)/*.o
	$(RM) $(LIBDIR)/platform/*.o

%: %.c
	CFLAGS="$(TDEFS)" $(MAKE) lib
	$(CC) $(CFLAGS) $(TDEFS) $< -o $@ $(LIBOBJS)

$(LIBDIR)/%.o: $(LIBDIR)/%.c $(LIBDIR)/coop_threads.h test_config.h
	$(CC) -c $(CFLAGS) $< -o $@
