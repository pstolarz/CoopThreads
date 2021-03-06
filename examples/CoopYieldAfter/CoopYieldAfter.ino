/*
 * Copyright (c) 2020,2021 Piotr Stolarz
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
 * The example presents mechanism controlling time a thread spends running
 * via coop_yield_after() usage. This enables simple priority implementation.
 *
 * Required configuration:
 *     CONFIG_MAX_THREADS >= 3
 *     CONFIG_OPT_YIELD_AFTER
 */
#include "coop_threads.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
# define THREAD_STACK_SIZE 0x250U
#elif ARDUINO_ARCH_AVR
# define THREAD_STACK_SIZE 0x50U
#else
/* use default */
# define THREAD_STACK_SIZE 0
#endif

#if CONFIG_MAX_THREADS < 3
# error CONFIG_MAX_THREADS >= 3 is required
#endif
#ifndef CONFIG_OPT_YIELD_AFTER
# error CONFIG_OPT_YIELD_AFTER need to be configured
#endif

/*
 * Thread routine
 */
extern "C" void thrd_proc(void *arg)
{
    char msg[16] = {};
    unsigned prio = (int)(size_t)arg;
    coop_tick_t after = coop_tick_cb() + 100*prio;

    for (int i = 0; i < 10; i++) {
        sprintf(msg, "%s: %d\n", coop_thread_name(), i+1);
        Serial.print(msg);

        delay(100);
        coop_yield_after(&after, 100*prio);
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
    coop_sched_service();
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
