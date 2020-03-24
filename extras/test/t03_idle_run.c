/*
 * Copyright (c) 2020 Piotr Stolarz
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
    printf("coop_idle_cb(%lu) called-back\n", (unsigned long)period);
    usleep((useconds_t)period * 1000U);
}

void thrd_1(void *arg)
{
    for (int i = 0; i < 5; i++)
    {
        coop_tick_t start = coop_tick_cb();
        coop_idle(200);
        printf("%s: %d; was idle for %lu\n", coop_thread_name(), i+1,
            (unsigned long)(coop_tick_cb() - start));
    }
    printf("%s EXIT\n", coop_thread_name());
}


void thrd_2(void *arg)
{
    for (int i = 0; i < 10; i++)
    {
        printf("%s: %d\n", coop_thread_name(), i+1);
        usleep(100000);
        coop_yield();
    }

    coop_tick_t start = coop_tick_cb();
    coop_idle(100);
    printf("%s EXIT; last idle: %lu\n", coop_thread_name(),
        (unsigned long)(coop_tick_cb() - start));
}

int main(int argc, char *argv[])
{
    coop_sched_thread(thrd_1, "thrd_1", 0, NULL);
    coop_sched_thread(thrd_2, "thrd_2", 0, NULL);
    coop_sched_service();

    return 0;
}
