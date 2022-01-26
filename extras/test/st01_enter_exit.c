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
#include <stdlib.h>
#include <unistd.h>
#include "coop_threads.h"

static unsigned thrd_cnt = 0;

static void thrd_proc(void *arg)
{
    int next_step;
    unsigned cnt = 0, thrd_id = (unsigned)(size_t)arg;

    printf("thrd_%u: ENTER\n", thrd_id);

    for (;;)
    {
        printf("thrd_%u: %u\n", thrd_id, ++cnt);
        coop_idle(100);

        next_step = rand() % 10;

        if ((next_step >= 0 && next_step <= 5)) {
            /*
             * Probability 0.6: terminate current thread if it's most shallow
             */
            if (coop_test_is_shallow()) break;
        } else {
            /*
             * Probability 0.4: schedule new thread to run. May fail with limit
             * error in case no space is available on the threads pool (e.g.
             * terminating threads end as holes still occupying the main stack
             * and the pool).
             */
            coop_sched_thread(thrd_proc, NULL, 0, (void*)(size_t)(++thrd_cnt));
        }
    }

    printf("thrd_%u: EXIT\n", thrd_id);
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < CONFIG_MAX_THREADS; i++) {
        coop_sched_thread(thrd_proc, NULL, 0, (void*)(size_t)(++thrd_cnt));
    }
    coop_sched_service();

    return 0;
}
