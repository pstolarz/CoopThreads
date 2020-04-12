.SILENT:
.PHONY: all build clean lib libclean

LIBDIR=../../src
CFLAGS+=-D__TEST__ -Wall -DCOOP_CONFIG_FILE="\"test_config.h\"" -I$(LIBDIR) -I.

LIBOBJS=\
    $(LIBDIR)/coop_threads.o \
    $(LIBDIR)/platform/unix.o

TESTS=\
    t01_sched_switch \
    t02_idle \
    t03_idle_run \
    t04_yield_after \
    t05_wait_notify_sngl \
    t06_wait_notify_all \
    t07_idle_wait \
    t08_wait_cond

STRESS_TESTS=\
    st01_enter_exit

t01_sched_switch: TDEFS=-DT01
t02_idle: TDEFS=-DT02
t03_idle_run: TDEFS=-DT03
t04_yield_after: TDEFS=-DT04
t05_wait_notify_sngl: TDEFS+=-DT05
t06_wait_notify_all: TDEFS+=-DT06
t07_idle_wait: TDEFS+=-DT07
t08_wait_cond: TDEFS+=-DT08

st01_enter_exit: TDEFS+=-DST01

all: build
	for t in $(TESTS); do echo "TEST: $$t"; ./$$t; echo; done;

build: $(TESTS) $(STRESS_TESTS)

lib: libclean $(LIBOBJS)

clean: libclean
	$(RM) *.o $(TESTS) $(STRESS_TESTS)

libclean:
	$(RM) $(LIBDIR)/*.o
	$(RM) $(LIBDIR)/platform/*.o

%: %.c
	CFLAGS="$(TDEFS)" $(MAKE) lib
	$(CC) $(CFLAGS) $(TDEFS) $< -o $@ $(LIBOBJS)

$(LIBDIR)/%.o: $(LIBDIR)/%.c $(LIBDIR)/coop_threads.h test_config.h
	$(CC) -c $(CFLAGS) $< -o $@
