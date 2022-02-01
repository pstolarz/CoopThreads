/*
 * Copyright (c) 2020,2022 Piotr Stolarz
 * Lightweight cooperative threads library
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

static void thrd_1(void *arg)
{
    coop_tick_t timeout = (coop_tick_t)(size_t)arg;
    coop_tick_t start = coop_tick_cb();

    if (coop_wait(1, timeout) == COOP_SUCCESS) {
        printf("%s: waited %lu ticks for signal\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    } else {
        printf("%s: time-out; %lu ticks passed\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    }
    printf("%s EXIT\n", coop_thread_name());
}

static void thrd_2(void *arg)
{
    coop_tick_t sleep_time = (coop_tick_t)(size_t)arg;

    usleep(sleep_time * 1000);
    coop_yield();

    coop_notify(1);

    printf("%s EXIT\n", coop_thread_name());
}

int main(void)
{
    for (unsigned i = 1; i <= 5; i++) {
        coop_sched_thread(thrd_1, "thrd_1", 0, (void*)(size_t)350U);
        coop_sched_thread(thrd_2, "thrd_2", 0, (void*)(size_t)(i * 100U));
        coop_sched_service();
    }
    return 0;
}
