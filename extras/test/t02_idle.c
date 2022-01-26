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

void coop_idle_cb(coop_tick_t period)
{
    /* short or infinite periods are ignored */
    if (period > 1) {
        printf("coop_idle_cb called-back for %lu\n", (unsigned long)period);
        usleep((useconds_t)period * 1000);
    }
}

void thrd_proc(void *arg)
{
    coop_tick_t idle_time = (coop_tick_t)(size_t)arg;

    for (int i = 0; i < 5; i++)
    {
        coop_tick_t start = coop_tick_cb();
        coop_idle(idle_time);
        printf("%s: %d; was idle for %lu\n", coop_thread_name(), i+1,
            (unsigned long)(coop_tick_cb() - start));
    }
    printf("%s EXIT\n", coop_thread_name());
}

int main(int argc, char *argv[])
{
    coop_sched_thread(thrd_proc, "thrd_1", 0, (void*)(size_t)100U);
    coop_sched_thread(thrd_proc, "thrd_2", 0, (void*)(size_t)200U);
    coop_sched_thread(thrd_proc, "thrd_3", 0, (void*)(size_t)300U);
    coop_sched_service();

    return 0;
}
