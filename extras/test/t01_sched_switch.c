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

static void thrd_proc(void *arg)
{
    int max_cnt = (int)(size_t)arg;

    for (int i = 0; i < max_cnt; i++) {
        printf("%s: %d\n", coop_thread_name(), i+1);
        usleep(50000);
        coop_yield();
    }
    printf("%s EXIT; Max stack usage: 0x%x\n",
        coop_thread_name(), (unsigned)coop_stack_wm());
}

int main(void)
{
    coop_sched_thread(thrd_proc, "thrd_1", 0, (void*)(size_t)1);
    coop_sched_thread(thrd_proc, "thrd_2", 0, (void*)(size_t)2);
    coop_sched_thread(thrd_proc, "thrd_3", 0, (void*)(size_t)3);
    coop_sched_thread(thrd_proc, "thrd_4", 0, (void*)(size_t)4);
    coop_sched_thread(thrd_proc, "thrd_5", 0, (void*)(size_t)5);
    coop_sched_service();

    coop_sched_thread(thrd_proc, "thrd_6", 0, (void*)(size_t)6);
    coop_sched_thread(thrd_proc, "thrd_7", 0, (void*)(size_t)5);
    coop_sched_thread(thrd_proc, "thrd_8", 0, (void*)(size_t)4);
    coop_sched_thread(thrd_proc, "thrd_9", 0, (void*)(size_t)3);
    coop_sched_thread(thrd_proc, "thrd_10", 0, (void*)(size_t)2);
    coop_sched_service();

    coop_sched_thread(thrd_proc, "thrd_11", 0, (void*)(size_t)1);
    coop_sched_service();

    return 0;
}
