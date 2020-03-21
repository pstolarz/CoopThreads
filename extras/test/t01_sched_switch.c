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

static void thrd_proc(void *arg)
{
    int max_cnt = (int)(size_t)arg;

    for (int cnt = 0; cnt < max_cnt; cnt++) {
        printf("%s: %d\n", coop_get_thread_name(), cnt+1);
        usleep(50000);
        coop_yield();
    }
    printf("%s EXIT\n", coop_get_thread_name());
}

int main(int argc, char *argv[])
{
    coop_sched_thread(thrd_proc, "thrd_1", 0, (void*)1);
    coop_sched_thread(thrd_proc, "thrd_2", 0, (void*)2);
    coop_sched_thread(thrd_proc, "thrd_3", 0, (void*)3);
    coop_sched_thread(thrd_proc, "thrd_4", 0, (void*)4);
    coop_sched_thread(thrd_proc, "thrd_5", 0, (void*)5);
    coop_sched_service();

    coop_sched_thread(thrd_proc, "thrd_6", 0, (void*)6);
    coop_sched_thread(thrd_proc, "thrd_7", 0, (void*)5);
    coop_sched_thread(thrd_proc, "thrd_8", 0, (void*)4);
    coop_sched_thread(thrd_proc, "thrd_9", 0, (void*)3);
    coop_sched_thread(thrd_proc, "thrd_10", 0, (void*)2);
    coop_sched_service();

    coop_sched_thread(thrd_proc, "thrd_11", 0, (void*)1);
    coop_sched_service();

    return 0;
}
