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
    static unsigned call_n = 0;

    call_n++;
    printf("coop_idle_cb(%lu) called-back\n", (unsigned long)period);

    if (period) {
        usleep((useconds_t)period * 1000);
        if (call_n == 2) {
            coop_notify(1);
            coop_notify_all(2);
        }
    } else {
        sleep(1);
        if (call_n == 4)
            coop_notify(1);
    }
}

static void thrd_proc(void *arg)
{
    coop_tick_t timeout = (coop_tick_t)(size_t)arg;
    coop_tick_t start;

    coop_yield();

    start =  coop_tick_cb();
    if (coop_wait(1, timeout) == COOP_SUCCESS) {
        printf("%s: waited %lu ticks for singal\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    } else {
        printf("%s: time-out; %lu ticks passed\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    }
    printf("%s EXIT\n", coop_thread_name());
}

static void thrd_grp(void *arg)
{
    coop_tick_t timeout = (coop_tick_t)(size_t)arg;
    coop_tick_t start =  coop_tick_cb();

    if (coop_wait(2, timeout) == COOP_SUCCESS) {
        printf("%s: waited %lu ticks for singal\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    } else {
        printf("%s: time-out; %lu ticks passed\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    }
    coop_yield();

    printf("%s EXIT\n", coop_thread_name());
}

int main(int argc, char *argv[])
{
    coop_sched_thread(thrd_proc, "thrd_1", 0, (void*)(size_t)100U);
    coop_sched_thread(thrd_proc, "thrd_2", 0, (void*)(size_t)200U);
    coop_sched_thread(thrd_proc, "thrd_3", 0, (void*)(size_t)0);

    coop_sched_thread(thrd_grp, "thrd_grp_1", 0, (void*)(size_t)100U);
    coop_sched_thread(thrd_grp, "thrd_grp_2", 0, (void*)(size_t)200U);
    coop_sched_thread(thrd_grp, "thrd_grp_3", 0, (void*)(size_t)0);

    coop_sched_service();

    return 0;
}
