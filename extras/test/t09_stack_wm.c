/*
 * Copyright (c) 2021 Piotr Stolarz
 * Lightweight cooperative threads library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop_threads.h"

#define STACK_PADD  0xA5

#define CLEAR_STACK() \
    memset(stack, STACK_PADD, sizeof(stack));

static void thrd_proc(void *arg) {}

int main(int argc, char *argv[])
{
    unsigned i;
    unsigned char stack[0x10];

    coop_sched_thread(thrd_proc, "thrd", sizeof(stack), NULL);
    coop_test_set_cur_thrd(0);
    coop_test_set_stack(0, stack);
    assert((void*)stack == coop_test_get_stack(0));

    CLEAR_STACK();
    assert(!coop_stack_wm());

    for (i = 0; i < sizeof(stack); i++) {
        stack[i] = 0;
        assert(coop_stack_wm() == i + 1);
    }

    CLEAR_STACK();
    for (i = sizeof(stack); i; i--) {
        stack[i - 1] = 0;
        assert(coop_stack_wm() == sizeof(stack) - i + 1);
    }

    stack[0] = STACK_PADD;
    stack[sizeof(stack) - 1] = STACK_PADD;
    stack[sizeof(stack) - 2] = STACK_PADD;
    assert(coop_stack_wm() == sizeof(stack) - 2);

    stack[1] = STACK_PADD;
    stack[2] = STACK_PADD;
    assert(coop_stack_wm() == sizeof(stack) - 3);

    return 0;
}
