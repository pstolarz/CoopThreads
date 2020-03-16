#include "coop_threads.h"

#define THREAD_STACK_SIZE 0x50

#ifdef __DEBUG__
#ifdef ARDUINO_ARCH_AVR
extern "C" uint16_t get_sp(void) {
    return (((uint16_t)SPH << 8) | (uint16_t)SPL);
}
#endif

extern "C" void coop_dbg_log_cb(const char *format, ...)
{
    static char msg[128] = {};
    va_list args;

    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);

    Serial.print(msg);
}
#endif

/*
 * Thread routine
 */
extern "C" void thrd_proc(void *arg)
{
    int max_cnt = (int)(size_t)arg;
    char msg[16] = {};

    for (int cnt = 0; cnt < max_cnt; cnt++)
    {
        sprintf(msg, "%s: %d\n", coop_get_thread_name(), cnt+1);
        Serial.print(msg);
        coop_sched_yield();
    }
    sprintf(msg, "%s EXIT\n", coop_get_thread_name());
    Serial.print(msg);
}

void setup()
{
    Serial.begin(9600);

    coop_sched_thread(thrd_proc, "thrd_1", THREAD_STACK_SIZE, (void*)1);
    coop_sched_thread(thrd_proc, "thrd_2", THREAD_STACK_SIZE, (void*)2);
    coop_sched_thread(thrd_proc, "thrd_3", THREAD_STACK_SIZE, (void*)3);
    coop_sched_thread(thrd_proc, "thrd_4", THREAD_STACK_SIZE, (void*)4);
    coop_sched_thread(thrd_proc, "thrd_5", THREAD_STACK_SIZE, (void*)5);
    coop_sched_service();
    Serial.println("Threads 1-5 finished\n");

    coop_sched_thread(thrd_proc, "thrd_6", THREAD_STACK_SIZE, (void*)6);
    coop_sched_thread(thrd_proc, "thrd_7", THREAD_STACK_SIZE, (void*)5);
    coop_sched_thread(thrd_proc, "thrd_8", THREAD_STACK_SIZE, (void*)4);
    coop_sched_thread(thrd_proc, "thrd_9", THREAD_STACK_SIZE, (void*)3);
    coop_sched_thread(thrd_proc, "thrd_10", THREAD_STACK_SIZE, (void*)2);
    coop_sched_service();
    Serial.println("Threads 6-10 finished\n");

    coop_sched_thread(thrd_proc, "thrd_11", THREAD_STACK_SIZE, (void*)1);
    coop_sched_service();
    Serial.println("Thread 11 finished\n");
}

void loop()
{
    static bool info = false;
    if (!info) {
        Serial.println("All scheduled threads finished; loop() reached");
        info = true;
    }
}
