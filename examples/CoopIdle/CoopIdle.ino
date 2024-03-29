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
 * Idle threads example.
 *
 * Required configuration:
 *     CONFIG_MAX_THREADS >= 3
 *     CONFIG_OPT_IDLE
 *     CONFIG_IDLE_CB_ALT
 */
#include "coop_threads.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
# define THREAD_STACK_SIZE 0x400U
#elif ARDUINO_ARCH_AVR
# define THREAD_STACK_SIZE 0x80U
#else
/* use default */
# define THREAD_STACK_SIZE 0
#endif

#if CONFIG_MAX_THREADS < 3
# error CONFIG_MAX_THREADS >= 3 is required
#endif
#if !CONFIG_OPT_IDLE
# error CONFIG_OPT_IDLE need to be configured
#endif
#if !CONFIG_IDLE_CB_ALT
# error CONFIG_IDLE_CB_ALT need to be configured
#endif

/*
 * The callback is called by the library to handle the idle state on the
 * system (global) level. A user may leverage this callback to save battery
 * energy consumption by switch the system to a desired sleep mode.
 */
extern "C" void coop_idle_cb(coop_tick_t period)
{
    Serial.print("Platform going idle for: ");
    Serial.println(period);
    delay(period);
}

/*
 * Thread routine
 */
extern "C" void thrd_proc(void *arg)
{
    coop_tick_t idle_time = (int)(size_t)arg;

    for (int i = 0; i < 5; i++)
    {
        coop_tick_t start = coop_tick_cb();
        coop_idle(idle_time);

        Serial.print(coop_thread_name());
        Serial.print(": ");
        Serial.print(i + 1);
        Serial.print("; was idle for ");
        Serial.println((unsigned long)(coop_tick_cb() - start));
    }
    Serial.print(coop_thread_name());
    Serial.println(" EXIT");
}

void setup()
{
    Serial.begin(115200);

    coop_sched_thread(thrd_proc, "thrd_1", THREAD_STACK_SIZE, (void*)100);
    coop_sched_thread(thrd_proc, "thrd_2", THREAD_STACK_SIZE, (void*)200);
    coop_sched_thread(thrd_proc, "thrd_3", THREAD_STACK_SIZE, (void*)300);
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
