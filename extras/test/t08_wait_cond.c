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

#include <stdio.h>
#include <unistd.h>
#include "coop_threads.h"

typedef struct {
    unsigned thrshld;
    unsigned *counter;
} thrd_arg_t;

static bool wait_predic(void *cv)
{
    thrd_arg_t thrd_arg = *(thrd_arg_t*)cv;
    return thrd_arg.thrshld == *thrd_arg.counter;
}

static void thrd_proc(void *arg)
{
    coop_tick_t start;
    thrd_arg_t thrd_arg = *(thrd_arg_t*)arg;

    start =  coop_tick_cb();
    if (coop_wait_cond(1, (coop_tick_t)(10 + thrd_arg.thrshld * 100U),
        wait_predic, arg) == COOP_SUCCESS)
    {
        printf("%s: waited %lu ticks for signal\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    } else {
        printf("%s: time-out; %lu ticks passed\n",
            coop_thread_name(), (unsigned long)(coop_tick_cb() - start));
    }
    printf("%s EXIT\n", coop_thread_name());
}

static void thrd_notify(void *arg)
{
    for (int i = 0; i < 6; i++) {
        coop_idle(100);
        (*(unsigned*)arg)++;
        coop_notify_all(1);
    }
    printf("%s EXIT\n", coop_thread_name());
}

int main(int argc, char *argv[])
{
    unsigned counter = 0;

    coop_sched_thread(thrd_notify, "thrd_notify", 0, &counter);

    coop_sched_thread(thrd_proc, "thrd_1", 0, &(thrd_arg_t){2U, &counter});
    coop_sched_thread(thrd_proc, "thrd_2", 0, &(thrd_arg_t){4U, &counter});
    coop_sched_thread(thrd_proc, "thrd_3", 0, &(thrd_arg_t){6U, &counter});
    coop_sched_thread(thrd_proc, "thrd_4", 0, &(thrd_arg_t){8U, &counter});

    coop_sched_service();

    return 0;
}
