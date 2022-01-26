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

/* max 10 ticks to run the thread before yielding */
#define MAX_RUN_TIME 10

static void thrd_1(void *arg)
{
    coop_tick_t after = coop_tick_cb() + MAX_RUN_TIME;

    for (int i = 0; i < 10; i++)
    {
        printf("%s: %d\n", coop_thread_name(), i+1);
        usleep(5000);

        coop_yield_after(&after, MAX_RUN_TIME);
    }
    printf("%s EXIT\n", coop_thread_name());
}

static void thrd_2(void *arg)
{
    for (int i = 0; i < 5; i++) {
        printf("%s: %d\n", coop_thread_name(), i+1);
        coop_yield();
    }
    printf("%s EXIT\n", coop_thread_name());
}

int main(int argc, char *argv[])
{
    coop_sched_thread(thrd_1, "thrd_1", 0, NULL);
    coop_sched_thread(thrd_2, "thrd_2", 0, NULL);
    coop_sched_service();

    return 0;
}
