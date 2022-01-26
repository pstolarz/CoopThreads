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

static void thrd_grp1(void *arg)
{
    coop_tick_t timeout = (coop_tick_t)(size_t)arg;
    coop_tick_t start;

    coop_yield();

    start =  coop_tick_cb();
    if (coop_wait(1, timeout) == COOP_SUCCESS) {
        printf("%s: waited %lu ticks for signal\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    } else {
        printf("%s: time-out; %lu ticks passed\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    }
    printf("%s EXIT\n", coop_thread_name());
}

static void thrd_grp2(void *arg)
{
    coop_tick_t timeout = (coop_tick_t)(size_t)arg;
    coop_tick_t start =  coop_tick_cb();

    if (coop_wait(2, timeout) == COOP_SUCCESS) {
        printf("%s: waited %lu ticks for signal\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    } else {
        printf("%s: time-out; %lu ticks passed\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    }
    coop_yield();

    printf("%s EXIT\n", coop_thread_name());
}

static void thrd_notify(void *arg)
{
    coop_idle(150);
    coop_notify_all(1);

    coop_idle(100);
    coop_notify_all(2);

    printf("%s EXIT\n", coop_thread_name());
}

int main(int argc, char *argv[])
{
    coop_sched_thread(thrd_notify, "thrd_notify", 0, NULL);

    coop_sched_thread(thrd_grp1, "thrd_grp1_1", 0, (void*)(size_t)100U);
    coop_sched_thread(thrd_grp1, "thrd_grp1_2", 0, (void*)(size_t)200U);
    coop_sched_thread(thrd_grp1, "thrd_grp1_3", 0, (void*)(size_t)300U);
    coop_sched_thread(thrd_grp1, "thrd_grp1_4", 0, (void*)(size_t)0);

    coop_sched_thread(thrd_grp2, "thrd_grp2_1", 0, (void*)(size_t)100U);
    coop_sched_thread(thrd_grp2, "thrd_grp2_2", 0, (void*)(size_t)200U);
    coop_sched_thread(thrd_grp2, "thrd_grp2_3", 0, (void*)(size_t)300U);
    coop_sched_thread(thrd_grp2, "thrd_grp2_4", 0, (void*)(size_t)0);

    coop_sched_service();

    return 0;
}
