.SILENT:
.PHONY: all build clean lib libclean analyze

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
    t08_wait_cond \
    t09_stack_wm

STRESS_TESTS=\
    st01_enter_exit

t01_sched_switch: TDEFS=-DT01
t02_idle: TDEFS=-DT02
t03_idle_run: TDEFS=-DT03
t04_yield_after: TDEFS=-DT04
t05_wait_notify_sngl: TDEFS=-DT05
t06_wait_notify_all: TDEFS=-DT06
t07_idle_wait: TDEFS=-DT07
t08_wait_cond: TDEFS=-DT08
t09_stack_wm: TDEFS=-DT09

st01_enter_exit: TDEFS=-DST01

all: build
	./tests_run.sh $(TESTS)

build: $(TESTS) $(STRESS_TESTS)

lib: libclean $(LIBOBJS)

clean: libclean
	$(RM) -r $(TESTS) $(STRESS_TESTS) compile_commands.json report report-html

libclean:
	$(RM) $(LIBOBJS)

analyze: clean
	CodeChecker log -b "$(MAKE) build" -o compile_commands.json
	CodeChecker analyze compile_commands.json -o report --analyzers clangsa clang-tidy
	-CodeChecker parse -e html -o report-html report
%: %.c
	CFLAGS="$(TDEFS)" $(MAKE) lib
	$(CC) $(CFLAGS) $(TDEFS) $< -o $@ $(LIBOBJS)

$(LIBDIR)/%.o: $(LIBDIR)/%.c $(LIBDIR)/coop_threads.h test_config.h
	$(CC) -c $(CFLAGS) $< -o $@
