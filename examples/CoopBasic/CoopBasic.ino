/*
 * Copyright (c) 2020-2022 Piotr Stolarz
 * Lightweight cooperative threads library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

/**
 * Basic example presenting threads switching by yielding them to the scheduler.
 *
 * Required configuration:
 *     CONFIG_MAX_THREADS >= 5
 */
#include "coop_threads.h"

/*
 * NOTE: The difference between stack sizes arise from sprintf(3) usage in
 * the threads routines. printf's family of functions exploits the stack in
 * extensive range which varies substantially between various platform
 * implementations.
 */
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
# define THREAD_STACK_SIZE 0x250U
#elif ARDUINO_ARCH_AVR
# define THREAD_STACK_SIZE 0x50U
#else
/* use default */
# define THREAD_STACK_SIZE 0
#endif

#if CONFIG_MAX_THREADS < 5
# error CONFIG_MAX_THREADS >= 5 is required
#endif

/*
 * Thread routine
 */
extern "C" void thrd_proc(void *arg)
{
    int max_cnt = (int)(size_t)arg;
    char msg[16] = {};

    for (int i = 0; i < max_cnt; i++)
    {
        sprintf(msg, "%s: %d\n", coop_thread_name(), i+1);
        Serial.print(msg);
        coop_yield();
    }
    sprintf(msg, "%s EXIT\n", coop_thread_name());
    Serial.print(msg);
}

void setup()
{
    Serial.begin(115200);

    coop_sched_thread(thrd_proc, "thrd_1", THREAD_STACK_SIZE, (void*)1);
    coop_sched_thread(thrd_proc, "thrd_2", THREAD_STACK_SIZE, (void*)2);
    coop_sched_thread(thrd_proc, "thrd_3", THREAD_STACK_SIZE, (void*)3);
    coop_sched_thread(thrd_proc, "thrd_4", THREAD_STACK_SIZE, (void*)4);
    coop_sched_thread(thrd_proc, "thrd_5", THREAD_STACK_SIZE, (void*)5);
    coop_sched_service();
    Serial.println("Threads 1-5 finished\n");

    coop_sched_thread(thrd_proc, "thrd_6", THREAD_STACK_SIZE, (void*)6);
    coop_sched_service();
    Serial.println("Thread 6 finished\n");
}

void loop()
{
    static bool info = false;
    if (!info) {
        Serial.println("All scheduled threads finished; loop() reached");
        info = true;
    }
    delay(1000);
}
