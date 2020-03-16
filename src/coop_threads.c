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

#include <alloca.h>
#include <setjmp.h>
#include <stdbool.h>
#include <string.h>     /* memset() */

#include "coop_threads.h"

/**
 * Thread states.
 */
typedef enum
{
    EMPTY = 0,  /** Empty context slot on the pool. Id must be 0. */
    HOLE,       /** Thread terminated but its stack still occupies the main
                    stack, where threads stacks are allocated. */
    NEW,        /** Already created thread, not yet run. */
    RUN         /** Running thread. */
} coop_thrd_state_t;

/**
 * Thread context.
 */
typedef struct
{
    /** Thread routine. */
    coop_thrd_proc_t proc;

    /** Thread name (may be NULL). */
    const char *name;

    /** Thread stack size. */
    size_t stack_sz;

    /** User passed argument. */
    void *arg;

    /** Thread state. */
    coop_thrd_state_t state;

    /**
     * Thread stack depth on the main stack. 1 for the first started (deepest)
     * thread. @c coop_sched_ctx_t::depth for latest (most shallow) thread.
     */
    unsigned depth;

    /** Thread execution context. */
    jmp_buf exe_ctx;

    /** Thread entry execution context (used for stack unwinding). */
    jmp_buf entry_ctx;
} coop_thrd_ctx_t;

/**
 * Scheduler context.
 */
typedef struct
{
    /** Scheduler currently processed thread. */
    unsigned cur_thrd;

    /** Number of currently running threads. */
    unsigned thrds_n;

    /**
     * Number of threads currently occupying the main stack: running or holes
     * (finished but occupying the main stack space).
     */
    unsigned depth;

    /** Scheduler execution context. */
    jmp_buf exe_ctx;

    /** Threads pool of contexts. */
    coop_thrd_ctx_t thrds[CONFIG_MAX_THREADS];
} coop_sched_ctx_t;

static coop_sched_ctx_t sched = {};


static void _sched_init(bool force)
{
    static bool inited = false;

    if (!inited || force) {
        inited = true;
        memset(&sched, 0, sizeof(sched));
        sched.cur_thrd = (unsigned)-1;
    }
}

/*
 * NOTE: to reduce stack usage by the routine, it's defined as inline with
 * all its local variables stored in registers.
 */
static inline unsigned _mark_unwind_thrds()
{
    register unsigned i, depth, unwnd_thrd = sched.cur_thrd;

    /* mark the terminating (most shallow) thread as EMPTY */
    sched.thrds[sched.cur_thrd].state = EMPTY;
    sched.thrds_n--;

    /* calculate current main stack depth */
    for (i = depth = 0; i < CONFIG_MAX_THREADS; i++) {
        if (sched.thrds[i].state == RUN) {
            if (depth < sched.thrds[i].depth)
                depth = sched.thrds[i].depth;
        }
    }

    if (depth + 1 < sched.depth)
    {
        /*
         * All holes between the terminating thread and the most shallow
         * running thread are marked as EMPTY to mark stack space occupied
         * by these threads stacks as to be freed.
         */
        for (i = 0; i < CONFIG_MAX_THREADS; i++) {
            if (sched.thrds[i].state == HOLE) {
                if (depth + 1 <= sched.thrds[i].depth)
                {
                    if (depth + 1 == sched.thrds[i].depth) {
                        unwnd_thrd = i;
                    }
                    sched.thrds[i].state = EMPTY;
                    sched.thrds_n--;
                }
            }
        }
    } else {
        /*
         * No hole between the most shallow running thread and the terminating
         * thread. Unwinded stack will be set to the terminating thread stack.
         */
    }
    sched.depth = depth;

    return unwnd_thrd;
}

void coop_sched_service(void)
{
    while (sched.thrds_n > 0)
    {
        /*
         * coop_sched_service() routine is called recursively during building
         * stack frames for newly created threads. Each time the recursion
         * occurs the cur_thrd index need to be updated for the next thread to
         * process. For this reason the incrementation takes place at the loop
         * entry stage.
         */
        sched.cur_thrd = (sched.cur_thrd + 1) % CONFIG_MAX_THREADS;

        switch (sched.thrds[sched.cur_thrd].state)
        {
        default:
            continue;

        case NEW:
            /* sched_pos_entry_thrd: save a new thread entry stack state */
            if (!setjmp(sched.thrds[sched.cur_thrd].entry_ctx))
            {
                coop_dbg_log_cb("setjmp sched_pos_entry_thrd; new thread #%d\n",
                    sched.cur_thrd);

                sched.depth++;
                sched.thrds[sched.cur_thrd].depth = sched.depth;

                /* enter the thread routine */
                sched.thrds[sched.cur_thrd].proc(sched.thrds[sched.cur_thrd].arg);

                /*
                 * At this point the current thread is being terminated.
                 * Scheduler stack is set at the thread stack frame, therefore
                 * need to be updated:
                 *
                 * - If the terminating thread is below the current main stack
                 *   depth, the thread constitutes a hole (a terminated thread
                 *   with its stack still occupying the main stack space).
                 *   Scheduler stack frame is restored to its previous position.
                 *   No stack unwind occurs in this case.
                 *
                 * - If the terminating thread stack depth is at the main stack
                 *   depth level (most shallow thread), the stack unwind occurs.
                 *   The scheduler stack frame is set to the position of an
                 *   already terminated thread stack frame (may to be a hole)
                 *   just above a running thread with most shallow stack frame.
                 */
                if (sched.thrds[sched.cur_thrd].depth < sched.depth)
                {
                    sched.thrds[sched.cur_thrd].state = HOLE;

                    coop_dbg_log_cb("Hole-thread #%d; scheduler stack-restore: "
                        "longjmp sched_pos_run\n", sched.cur_thrd);

                    /* restore previous scheduler stack frame; sched_pos_run jump */
                    longjmp(sched.exe_ctx, 1);
                } else
                {
                    register unsigned unwnd_thrd = _mark_unwind_thrds();

                    coop_dbg_log_cb("Unwind scheduler stack at #%d thread entry "
                        "context: longjmp sched_pos_entry_thrd\n", unwnd_thrd);

                    /* unwind scheduler stack; sched_pos_entry_thrd jump */
                    longjmp(sched.thrds[unwnd_thrd].entry_ctx, 1);
                }
            } else {
                /* return with unwinded stack; new scheduler stack frame
                   from this point */
                coop_dbg_log_cb("Back to scheduler; stack unwinded\n");
            }
            break;

        case RUN:
            /* sched_pos_run: main-running scheduler execution context */
            if (!setjmp(sched.exe_ctx))
            {
                coop_dbg_log_cb("setjmp sched_pos_run; run thread #%d: "
                    "longjmp thrd_pos_new/thrd_pos_run\n", sched.cur_thrd);

                /* jump to runnung thread: thrd_pos_new or thrd_pos_run */
                longjmp(sched.thrds[sched.cur_thrd].exe_ctx, 1);
            } else {
                /* return from yielded running thread or restore
                   scheduler stack after thread terminated as a hole */
                coop_dbg_log_cb("Back to scheduler from #%d thread\n",
                    sched.cur_thrd);
            }
            break;
        }
    }

    _sched_init(true);
}

coop_error_t coop_sched_thread(coop_thrd_proc_t proc, const char *name,
    size_t stack_sz, void *arg)
{
    if (!proc) {
        return COOP_INV_ARG;
    } else
    if (sched.thrds_n >= CONFIG_MAX_THREADS) {
        return COOP_ERR_LIMIT;
    }

    _sched_init(false);

    for (unsigned i = 0; i < CONFIG_MAX_THREADS; i++) {
        if (sched.thrds[i].state == EMPTY)
        {
            sched.thrds[i].proc = proc;
            sched.thrds[i].name = name;
            sched.thrds[i].stack_sz =
                (!stack_sz ? CONFIG_DEFAULT_STACK_SIZE : stack_sz);
            sched.thrds[i].arg = arg;
            sched.thrds[i].state = NEW;
            sched.thrds[i].depth = 0;
            memset(sched.thrds[i].exe_ctx, 0, sizeof(sched.thrds[i].exe_ctx));
            memset(sched.thrds[i].entry_ctx, 0, sizeof(sched.thrds[i].entry_ctx));

            sched.thrds_n++;
            break;
        }
    }
    return COOP_SUCCESS;
}

void coop_sched_yield(void)
{
    if (sched.thrds[sched.cur_thrd].state == NEW) {
        sched.thrds[sched.cur_thrd].state = RUN;

        /* thrd_pos_new: newly created thread context */
        if (!setjmp(sched.thrds[sched.cur_thrd].exe_ctx))
        {
            coop_dbg_log_cb("setjmp thrd_pos_new; new thread #%d\n",
                sched.cur_thrd);

            /* allocate thread stack */
            memset(alloca(sched.thrds[sched.cur_thrd].stack_sz), 0,
                sched.thrds[sched.cur_thrd].stack_sz);

            /* build new thread stack via recurrent scheduler service call */
            coop_sched_service();
        } else {
            /* return from scheduler; first in RUN state */
            coop_dbg_log_cb("Back to #%d thread; NEW -> RUN switch\n",
                sched.cur_thrd);
        }
    } else {
        /* thrd_pos_run: main-running thread context */
        if (!setjmp(sched.thrds[sched.cur_thrd].exe_ctx))
        {
            coop_dbg_log_cb("setjmp thrd_pos_run; back from #%d thread to "
                "scheduler: longjmp sched_pos_run\n", sched.cur_thrd);

            /* back to scheduler: sched_pos_run jump */
            longjmp(sched.exe_ctx, 1);
        } else {
            /* return from scheduler; regular RUN */
            coop_dbg_log_cb("Back to #%d thread\n", sched.cur_thrd);
        }
    }
}

const char *coop_get_thread_name(void)
{
    return sched.thrds[sched.cur_thrd].name;
}
