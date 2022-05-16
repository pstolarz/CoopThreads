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
 * Notify/wait usage example.
 *
 * Required configuration:
 *     CONFIG_MAX_THREADS >= 2
 *     CONFIG_OPT_IDLE
 *     CONFIG_OPT_WAIT
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

#if CONFIG_MAX_THREADS < 2
# error CONFIG_MAX_THREADS >= 2 is required
#endif
#if !CONFIG_OPT_IDLE
# error CONFIG_OPT_IDLE need to be configured
#endif
#if !CONFIG_OPT_WAIT
# error CONFIG_OPT_WAIT need to be configured
#endif

static int cnt = 0;

/*
 * Thread routine - odd.
 */
extern "C" void thrd_odd(void *arg)
{
    (void)arg;

    do {
        if (!(cnt & 1)) {
            /* wait for thrd_even to increment the counter */
            coop_wait(1, 0);
        } else {
            Serial.print(coop_thread_name());
            Serial.print(": ");
            Serial.println(cnt);

            /* increment to even value and notify about the change */
            cnt++;
            coop_idle(500);
            coop_notify(1);
        }
    } while (cnt < 10);

    Serial.print(coop_thread_name());
    Serial.println(" EXIT");
}

/*
 * Thread routine - even.
 */
extern "C" void thrd_even(void *arg)
{
    (void)arg;

    do {
        if ((cnt & 1) != 0) {
            /* wait for thrd_odd to increment the counter */
            coop_wait(1, 0);
        } else {
            Serial.print(coop_thread_name());
            Serial.print(": ");
            Serial.println(cnt);

            /* increment to odd value and notify about the change */
            cnt++;
            coop_idle(500);
            coop_notify(1);
        }
    } while (cnt < 10);

    Serial.print(coop_thread_name());
    Serial.println(" EXIT");
}

void setup()
{
    Serial.begin(115200);

    coop_sched_thread(thrd_odd, "thrd_odd", THREAD_STACK_SIZE, NULL);
    coop_sched_thread(thrd_even, "thrd_even", THREAD_STACK_SIZE, NULL);
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
