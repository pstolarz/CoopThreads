/*
 * Copyright (c) 2020 Piotr Stolarz
 * Lightweight Cooperative Threads library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#include <stdio.h>
#include <unistd.h>
#include "coop_threads.h"

void coop_idle_cb(coop_tick_t period)
{
    printf("coop_idle_cb(%lu) called-back\n", (unsigned long)period);
    usleep((useconds_t)period * 1000U);
}

static void thrd_idle(void *arg)
{
    coop_tick_t start;
    coop_tick_t period = (coop_tick_t)(size_t)arg;

    printf("%s going idle for %lu\n",
        coop_get_thread_name(), (unsigned long)period);

    start = coop_tick_cb();
    coop_idle(period);

    printf("%s EXIT; idle time: %lu\n", coop_get_thread_name(),
        (unsigned long)(coop_tick_cb() - start));
}

#define SLEEP_STEP 2
#define SLEEP_TIME 100000U

static void thrd_sleep(void *arg)
{
    coop_tick_t start;
    coop_tick_t period = (coop_tick_t)(size_t)arg;

    for (int i = 0; i < SLEEP_STEP; i++) {
        printf("%s sleep for %d\n", coop_get_thread_name(), (SLEEP_TIME/1000));
        usleep(SLEEP_TIME);
        coop_yield();
    }

    printf("%s going idle for %lu\n",
        coop_get_thread_name(), (unsigned long)period);

    start = coop_tick_cb();
    coop_idle(period);

    printf("%s EXIT; idle time: %lu\n", coop_get_thread_name(),
        (unsigned long)(coop_tick_cb() - start));
}

int main(int argc, char *argv[])
{
    coop_sched_thread(thrd_idle, "thrd_1", 0, (void*)1000);
    coop_sched_thread(thrd_idle, "thrd_2", 0, (void*)2500);
    coop_sched_thread(thrd_sleep, "thrd_3", 0, (void*)3000);
    coop_sched_service();

    return 0;
}
