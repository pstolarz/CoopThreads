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
 * Conditional wait usage example.
 *
 * Required configuration:
 *     CONFIG_MAX_THREADS >= 3
 *     CONFIG_OPT_IDLE
 *     CONFIG_OPT_WAIT
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
#ifndef CONFIG_OPT_IDLE
# error CONFIG_OPT_IDLE need to be configured
#endif
#ifndef CONFIG_OPT_WAIT
# error CONFIG_OPT_WAIT need to be configured
#endif

#define FLAG_1   (1 << 0)
#define FLAG_2   (1 << 1)
#define FLAG_3   (1 << 2)
#define FLAG_4   (1 << 3)
#define FLAG_5   (1 << 4)

typedef struct {
    coop_predic_proc_t predic;
    void *cv;
} thrd_arg_t;

/*
 * Waiting thread routine.
 */
extern "C" void thrd_proc(void *arg)
{
    char msg[16] = {};
    thrd_arg_t thrd_arg = *(thrd_arg_t*)arg;

    /* wait until a waiting condition is fulfilled */
    coop_wait_cond(1, 0, thrd_arg.predic, thrd_arg.cv);

    sprintf(msg, "%s EXIT\n", coop_thread_name());
    Serial.print(msg);
}

/*
 * Notifying thread routine.
 */
extern "C" void thrd_notify(void *arg)
{
    char msg[16] = {};

    for (int i=0; i < 5; i++) {
        coop_idle(1000);
        *(unsigned*)arg |= (1 << i);

        sprintf(msg, "Flag %d set\n", i+1);
        Serial.print(msg);

        coop_notify_all(1);
    }

    sprintf(msg, "%s EXIT\n", coop_thread_name());
    Serial.print(msg);
}

extern "C" bool flags_2_or_5(void *cv) {
    unsigned flags = *(unsigned*)cv;
    return ((flags & (FLAG_2 | FLAG_5)) != 0);
}

extern "C" bool flags_2_and_5(void *cv) {
    unsigned flags = *(unsigned*)cv;
    return ((flags & (FLAG_2 | FLAG_5)) == (FLAG_2 | FLAG_5));
}

void setup()
{
    unsigned flags = 0U;
    thrd_arg_t flags_2_or_5_arg = { flags_2_or_5, &flags };
    thrd_arg_t flags_2_and_5_arg = { flags_2_and_5, &flags };

    Serial.begin(115200);

    coop_sched_thread(
        thrd_notify, "thrd_notify", THREAD_STACK_SIZE, &flags);
    coop_sched_thread(
        thrd_proc, "thrd_or", THREAD_STACK_SIZE, &flags_2_or_5_arg);
    coop_sched_thread(
        thrd_proc, "thrd_and", THREAD_STACK_SIZE, &flags_2_and_5_arg);

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
