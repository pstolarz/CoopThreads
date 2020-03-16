.SILENT:
.PHONY: all clean

LIBDIR=../../src
CFLAGS+=-D__TEST__ -Wall -DCOOP_THREADS_CONFIG_FILE="\"test_config.h\"" -I$(LIBDIR) -I.

TESTS=\
     t01_sched_switch

LIBOBJS=\
    $(LIBDIR)/coop_threads.o

all: $(TESTS)
	@echo "./t01_sched_switch" && ./t01_sched_switch

clean:
	$(RM) $(LIBDIR)/*.o
	$(RM) *.o $(TESTS)

%: %.c $(LIBOBJS)
	$(CC) $(CFLAGS) $< -o $@ $(LIBOBJS)

$(LIBDIR)/coop_threads.o: $(LIBDIR)/coop_threads.h

$(LIBDIR)/%.o: $(LIBDIR)/%.c test_config.h
	$(CC) -c $(CFLAGS) $< -o $@
