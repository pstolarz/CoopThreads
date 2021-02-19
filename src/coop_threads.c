/*
 * Copyright (c) 2020,2021 Piotr Stolarz
 * Lightweight cooperative threads library
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
#include <string.h> /* memset() */
#include "coop_threads.h"

#ifdef CONFIG_NOEXIT_STATIC_THREADS
# include <assert.h>
#endif

/** Stack padding byte: 0b10100101 */
#define STACK_PADD  0xA5

/**
 * Thread states.
 */
typedef enum
{
    EMPTY = 0,  /** Empty context slot on the pool. Id must be 0. */
#ifndef CONFIG_NOEXIT_STATIC_THREADS
    HOLE,       /** Thread terminated but its stack still occupies the main
                    stack, where threads stacks are allocated. */
#endif
    NEW,        /** Already created thread, not yet started. */
    RUN,        /** Running thread. */
#ifdef CONFIG_OPT_IDLE
    IDLE,       /** Thread is idle. */
#endif
#ifdef CONFIG_OPT_WAIT
    WAIT,       /** Waiting thread. */
#endif
} coop_thrd_state_t;

#ifdef CONFIG_OPT_IDLE
# define _IS_IDLE(_state) ((_state) == IDLE)
#else
# define _IS_IDLE(_state) (0)
#endif

#ifdef CONFIG_OPT_WAIT
# define _IS_WAIT(_state) ((_state) == WAIT)
#else
# define _IS_WAIT(_state) (0)
#endif

/* NEW thread is not considered as started */
#define _IS_STARTED(_state) \
    ((_state) == RUN || _IS_IDLE(_state) || _IS_WAIT(_state))

/**
 * Thread context.
 */
typedef struct
{
    /** Thread routine. */
    coop_thrd_proc_t proc;

    /** Thread name (may be NULL). */
    const char *name;

    /** Thread stack. */
    void *stack;
    size_t stack_sz;

    /** User passed argument. */
    void *arg;

    /** Thread state. */
    coop_thrd_state_t state;

#ifdef CONFIG_OPT_IDLE
    /** Clock tick the thread is idle up to. */
    coop_tick_t idle_to;
#endif
#ifdef CONFIG_OPT_YIELD_AFTER
    /** Scheduler to thread switch clock tick */
    coop_tick_t switch_tick;
#endif
#ifdef CONFIG_OPT_WAIT
    /** Semaphore id. */
    int sem_id;

    /** Waiting-predicate routine */
    coop_predic_proc_t predic;

    /** User defined conditional-variable */
    void *cv;

    /** Clock tick the thread is waiting up to. */
    coop_tick_t wait_to;

    /** Waiting related flags. */
    struct {
        unsigned char notif: 1; /** Notified flag. */
        unsigned char inf:   1; /** Infinite wait; @c wait_to not applied. */
        unsigned char res:   6; /** Reserved. */
    } wait_flgs;
#endif
#ifndef CONFIG_NOEXIT_STATIC_THREADS
    /**
     * Thread stack depth on the main stack. 1 for the first started (deepest)
     * thread. @c coop_sched_ctx_t::depth for latest (most shallow) thread.
     */
    unsigned depth;

    /** Thread entry execution context (used for stack unwinding). */
    jmp_buf entry_ctx;
#endif
    /** Thread execution context. */
    jmp_buf exe_ctx;
} coop_thrd_ctx_t;

/**
 * Scheduler context.
 */
typedef struct
{
    /** Scheduler currently processed thread. */
    unsigned cur_thrd;

    /** Number of occupied (non empty) thread slots. */
    unsigned busy_n;

#ifdef CONFIG_OPT_IDLE
    /** Number of idle and waiting threads. */
    unsigned idle_n;
#endif
#ifndef CONFIG_NOEXIT_STATIC_THREADS
    /** Number of holes (terminated threads occupying the main stack). */
    unsigned hole_n;

    /** Number of threads currently occupying the main stack. */
    unsigned depth;
#endif
    /** Scheduler execution context. */
    jmp_buf exe_ctx;

    /** Threads pool of contexts. */
    coop_thrd_ctx_t thrds[CONFIG_MAX_THREADS];
} coop_sched_ctx_t;

static coop_sched_ctx_t sched = {0};

#ifdef CONFIG_NOEXIT_STATIC_THREADS
# define _ACTIVE_THREADS() (sched.busy_n)
#else
# define _ACTIVE_THREADS() (sched.busy_n - sched.hole_n)
#endif

#ifdef COOP_DEBUG
static const char *_state_name(unsigned i)
{
    switch (sched.thrds[i].state)
    {
    case EMPTY:
        return "EMPTY";
# ifndef CONFIG_NOEXIT_STATIC_THREADS
    case HOLE:
        return "HOLE";
# endif
    case NEW:
        return "NEW";
    case RUN:
        return "RUN";
# ifdef CONFIG_OPT_IDLE
    case IDLE:
        return "IDLE";
# endif
# ifdef CONFIG_OPT_WAIT
    case WAIT:
        return "WAIT";
# endif
    }
    return "???";
}
#endif

static inline void _sched_init(bool force)
{
    static bool inited = false;

    if (!inited || force) {
        inited = true;
        memset(&sched, 0, sizeof(sched));
        sched.cur_thrd = (unsigned)-1;
    }
}

/*
 * NOTE: to reduce stack usage by coop_sched_service() helper routines, these
 * are defined as inline with all their local variables stored in registers.
 */

#ifndef CONFIG_NOEXIT_STATIC_THREADS
/**
 * Mark threads whose stacks need to be unwinded.
 *
 * Return terminated thread index at which the unwinded stack shall finish.
 * The new scheduler stack frame will be set at the thread context after the
 * unwinding process completes.
 */
static inline unsigned _mark_unwind_thrds()
{
    register unsigned i, depth, unwnd_thrd = sched.cur_thrd;

    /* mark the terminating (most shallow) thread as EMPTY */
    coop_dbg_log_cb("Thread #%d: RUN -> EMPTY\n", sched.cur_thrd);
    sched.thrds[sched.cur_thrd].state = EMPTY;
    sched.busy_n--;

    /* calculate current main stack depth */
    for (i = depth = 0; i < CONFIG_MAX_THREADS; i++) {
        if (_IS_STARTED(sched.thrds[i].state)) {
            if (depth < sched.thrds[i].depth)
                depth = sched.thrds[i].depth;
        }
    }

    if (depth + 1 < sched.depth)
    {
        /*
         * All holes between the terminating thread and the most shallow
         * started thread are marked as EMPTY to indicate stack space occupied
         * by these threads stacks as to be freed.
         */
        for (i = 0; i < CONFIG_MAX_THREADS; i++) {
            if (sched.thrds[i].state == HOLE) {
                if (depth + 1 <= sched.thrds[i].depth)
                {
                    if (depth + 1 == sched.thrds[i].depth) {
                        unwnd_thrd = i;
                    }
                    coop_dbg_log_cb("Thread #%d: HOLE -> EMPTY\n", i);
                    sched.thrds[i].state = EMPTY;
                    sched.busy_n--;
                    sched.hole_n--;
                }
            }
        }
    } else {
        /*
         * No hole between the most shallow started thread and the terminating
         * thread. Unwinded stack will be set to the terminating thread stack.
         */
    }
    sched.depth = depth;

    return unwnd_thrd;
}
#endif

#ifdef CONFIG_OPT_IDLE
/**
 * Check conditions and enter the system idle state if necessary.
 */
static inline void _system_idle(void)
{
    register unsigned i = 0;
    register coop_tick_t min_idle = 0, cur_tick = 0;

    /* system is considered idle-ready if all active threads are idle or waiting */
    while (sched.idle_n > 0 && _ACTIVE_THREADS() <= sched.idle_n)
    {
        if (i) {
            /* min_idle was set in the previous loop pass */
# ifdef COOP_DEBUG
            if (min_idle == COOP_MAX_TICK) {
                coop_dbg_log_cb("System going idle infinitely\n");
            } else {
                coop_dbg_log_cb("System going idle for %lu ticks\n",
                    (unsigned long)min_idle);
            }
# endif
            /* system is idle up to nearest wake-up time */
            coop_idle_cb(min_idle == COOP_MAX_TICK ? 0 : min_idle);
        }

        min_idle = COOP_MAX_TICK;
        cur_tick = coop_tick_cb();  /* current tick */

        for (i = 0; i < CONFIG_MAX_THREADS; i++)
        {
            if (_IS_IDLE(sched.thrds[i].state)
# ifdef CONFIG_OPT_WAIT
                || (_IS_WAIT(sched.thrds[i].state) &&
                    !sched.thrds[i].wait_flgs.inf)
# endif
                )
            {
                register coop_tick_t idle_to = (
# ifdef CONFIG_OPT_WAIT
                    !_IS_IDLE(sched.thrds[i].state) ? sched.thrds[i].wait_to :
# endif
                    sched.thrds[i].idle_to);

                if (COOP_IS_TICK_OVER(cur_tick, idle_to)) {
                    coop_dbg_log_cb("Thread #%d %s -> RUN (via idle-loop)\n",
                        i, _state_name(i));

                    /* idle time passed; the idle-loop will be finished */
                    sched.thrds[i].state = RUN;
                    sched.idle_n--;
                } else
                if ((idle_to - cur_tick) < min_idle) {
                    /* calculate nearest wake-up time */
                    min_idle = (idle_to - cur_tick);
                }
            }
        }
    }
}
#endif /* CONFIG_OPT_IDLE */

void coop_sched_service(void)
{
    while (sched.busy_n > 0)
    {
#ifdef CONFIG_OPT_IDLE
        /*
         * The routine is called if currently handled thread passed through
         * NEW or RUN states, therefore circumstances which could switch the
         * thread to idle or waiting states may occur and checking conditions
         * for suspending the platform should be performed. In other cases
         * (EMPTY, HOLE and IDLE/WAIT states with the idle/waiting state
         * still pending) the control passes through 'next_iter' label. This
         * eliminates unnecessary checks in _system_idle() and increases
         * performance of the scheduler service.
         */
        _system_idle();
#endif
        /*
         * coop_sched_service() routine is called recursively during building
         * stack frames for newly created threads. Each time the recursion
         * occurs the cur_thrd index need to be updated for the next thread to
         * process. For this reason the incrementation takes place at the loop
         * entry stage.
         */
next_iter:
        sched.cur_thrd = (sched.cur_thrd + 1) % CONFIG_MAX_THREADS;

        switch (sched.thrds[sched.cur_thrd].state)
        {
        case EMPTY:
#ifndef CONFIG_NOEXIT_STATIC_THREADS
        case HOLE:
#endif
        default:
            goto next_iter;

#ifdef CONFIG_OPT_IDLE
        case IDLE:
            if (!COOP_IS_TICK_OVER(
                    coop_tick_cb(), sched.thrds[sched.cur_thrd].idle_to))
            {
                /* the current thread is idle but other threads are running;
                   system can't switch to the idle state in this case */
                goto next_iter;
            }

            /* idle time passed; continue as in RUN state  */
            coop_dbg_log_cb("Thread #%d IDLE -> RUN (via sched-loop)\n",
                sched.cur_thrd);
            sched.thrds[sched.cur_thrd].state = RUN;
            sched.idle_n--;
            goto run;
#endif

#ifdef CONFIG_OPT_WAIT
        case WAIT:
            if (sched.thrds[sched.cur_thrd].wait_flgs.inf ||
                !COOP_IS_TICK_OVER(
                    coop_tick_cb(), sched.thrds[sched.cur_thrd].wait_to))
            {
                /* not-notified infinite or not yet timed-out waiting thread */
                goto next_iter;
            }

            coop_dbg_log_cb(
                "Thread #%d WAIT -> RUN (timed-out)\n", sched.cur_thrd);

            /* wait time passed; continue as in RUN state  */
            sched.thrds[sched.cur_thrd].state = RUN;
# ifdef CONFIG_OPT_IDLE
            sched.idle_n--;
# endif
            goto run;
#endif

        case RUN:
#if defined(CONFIG_OPT_IDLE) || defined(CONFIG_OPT_WAIT)
run:
#endif
            /* sched_pos_run: main-running scheduler execution context */
            if (!setjmp(sched.exe_ctx))
            {
                coop_dbg_log_cb("setjmp sched_pos_run; run thread #%d: "
                    "longjmp thrd_pos_[new/run]\n", sched.cur_thrd);

#ifdef CONFIG_OPT_YIELD_AFTER
                sched.thrds[sched.cur_thrd].switch_tick = coop_tick_cb();
#endif
                /* jump to running thread: thrd_pos_new, thrd_pos_run */
                longjmp(sched.thrds[sched.cur_thrd].exe_ctx, 1);
            } else {
                /* return from yielded running thread or restore
                   scheduler stack after thread terminated as a hole */
                coop_dbg_log_cb("Back to scheduler from #%d thread\n",
                    sched.cur_thrd);
            }
            break;

        case NEW:
#ifdef CONFIG_NOEXIT_STATIC_THREADS
            coop_dbg_log_cb("New thread #%d\n", sched.cur_thrd);

# ifdef CONFIG_OPT_YIELD_AFTER
            sched.thrds[sched.cur_thrd].switch_tick = coop_tick_cb();
# endif
            /* enter the thread routine */
            sched.thrds[sched.cur_thrd].proc(sched.thrds[sched.cur_thrd].arg);

            /* thread configured with CONFIG_NOEXIT_STATIC_THREADS
               is not expected to finish */
            coop_dbg_log_cb("UNEXPECTED: Thread #%d: RUN -> EMPTY\n",
                sched.cur_thrd);
            sched.thrds[sched.cur_thrd].state = EMPTY;
            sched.busy_n--;
            break;
#else
            /* sched_pos_entry_thrd: save a new thread entry stack state */
            if (!setjmp(sched.thrds[sched.cur_thrd].entry_ctx))
            {
                coop_dbg_log_cb("setjmp sched_pos_entry_thrd; new thread #%d\n",
                    sched.cur_thrd);

                sched.depth++;
                sched.thrds[sched.cur_thrd].depth = sched.depth;

# ifdef CONFIG_OPT_YIELD_AFTER
                sched.thrds[sched.cur_thrd].switch_tick = coop_tick_cb();
# endif
                /* enter the thread routine */
                sched.thrds[sched.cur_thrd].proc(sched.thrds[sched.cur_thrd].arg);

                /*
                 * At this point the current thread is being terminated.
                 * Scheduler stack is set at the thread stack frame, therefore
                 * need to be updated:
                 *
                 * - If the terminating thread stack depth is less than the
                 *   number of threads stacks occupying the main stack, the
                 *   thread constitutes a hole (a terminated thread with its
                 *   stack still occupying the main stack space). In this case
                 *   the scheduler stack frame is restored to its previous
                 *   position. No stack unwind occurs.
                 *
                 * - If the terminating thread stack depth is equal to the
                 *   number of threads stacks occupying the main stack - the
                 *   stack unwind occurs. In this case the scheduler stack
                 *   frame is set to the position of an already terminated
                 *   thread stack frame (possibly a hole) just above a started
                 *   thread with most shallow stack.
                 */
                if (sched.thrds[sched.cur_thrd].depth < sched.depth)
                {
                    coop_dbg_log_cb("Thread #%d: RUN -> HOLE; "
                        "scheduler stack-restore: longjmp sched_pos_run\n",
                        sched.cur_thrd);

                    sched.thrds[sched.cur_thrd].state = HOLE;
                    sched.hole_n++;

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
#endif /* CONFIG_NOEXIT_STATIC_THREADS */
        }
    }

#ifdef CONFIG_NOEXIT_STATIC_THREADS
    /*
     * Can't exit the routine since stack has not been unwinded up to
     * its entry point. Assertion will raise the exception in this case.
     */
    coop_dbg_log_cb("UNEXPECTED: coop_sched_service() exits!\n");
    assert(false);
#else
    _sched_init(true);
#endif
}

coop_error_t coop_sched_thread(coop_thrd_proc_t proc, const char *name,
    size_t stack_sz, void *arg)
{
    if (!proc) {
        return COOP_ERR_INV_ARG;
    } else
    if (sched.busy_n >= CONFIG_MAX_THREADS) {
        return COOP_ERR_LIMIT;
    }

    _sched_init(false);

    for (unsigned i = 0; i < CONFIG_MAX_THREADS; i++) {
        if (sched.thrds[i].state == EMPTY)
        {
            sched.thrds[i].proc = proc;
            sched.thrds[i].name = name;
            sched.thrds[i].stack = NULL;
            sched.thrds[i].stack_sz =
                (!stack_sz ? CONFIG_DEFAULT_STACK_SIZE : stack_sz);
            sched.thrds[i].arg = arg;
            sched.thrds[i].state = NEW;
#ifndef CONFIG_NOEXIT_STATIC_THREADS
            sched.thrds[i].depth = 0;
            memset(sched.thrds[i].entry_ctx, 0, sizeof(sched.thrds[i].entry_ctx));
#endif
            memset(sched.thrds[i].exe_ctx, 0, sizeof(sched.thrds[i].exe_ctx));

            sched.busy_n++;
            coop_dbg_log_cb("Thread #%d scheduled to run\n", i);
            break;
        }
    }
    return COOP_SUCCESS;
}

const char *coop_thread_name(void)
{
    return sched.thrds[sched.cur_thrd].name;
}

/**
 * @c new_state specifies a state to set before yielding (RUN, IDLE, WAIT).
 */
static inline void _yield(coop_thrd_state_t new_state)
{
    if (sched.thrds[sched.cur_thrd].state == NEW) {
        sched.thrds[sched.cur_thrd].state = new_state;

        /* thrd_pos_new: newly created thread context */
        if (!setjmp(sched.thrds[sched.cur_thrd].exe_ctx))
        {
            coop_dbg_log_cb("setjmp thrd_pos_new; thread #%d: NEW -> %s\n",
                sched.cur_thrd, _state_name(sched.cur_thrd));

            /* allocate thread stack */
            /*
             * NOTE: For performance reason the allocation takes place after
             * the call to the thread routine while the routine yields its
             * control to the scheduler for the first time (there is no point
             * to allocate thread's stack for a routine which simply enters and
             * immediately exists without yielding to the scheduler, since the
             * stack would not be used in such case).
             * For this reason the stack used by the thread is (in most cases)
             * larger than the size indicated by its requested value. This extra
             * space is the one which has already been allocated while entering
             * the thread routine for the first time (so embracing all or part
             * of its local variables created on the stack new frame).
             * This approach has this benefit, a library user needs not to take
             * care about thread local variables size calculation occupying its
             * stack, since by moving them to the routine's main scope they will
             * be allocated on the thread stack frame while entering the routine.
             * Library user's requested stack size must in this case embrace
             * stack space which is used dynamically by the thread during its
             * lifetime (including preemptive ISRs).
             */
            sched.thrds[sched.cur_thrd].stack =
                alloca(sched.thrds[sched.cur_thrd].stack_sz);
            memset(sched.thrds[sched.cur_thrd].stack, STACK_PADD,
                sched.thrds[sched.cur_thrd].stack_sz);

            /* build new thread stack via recurrent scheduler service call */
            coop_sched_service();
        } else {
            /* return from scheduler; first run */
            coop_dbg_log_cb("Back to #%d thread (via thrd_pos_new)\n",
                sched.cur_thrd);
        }
    } else {
        sched.thrds[sched.cur_thrd].state = new_state;
#ifdef COOP_DEBUG
        if (new_state != RUN) {
            coop_dbg_log_cb("Thread #%d: RUN -> %s\n",
                sched.cur_thrd, _state_name(sched.cur_thrd));
        }
#endif

        /* thrd_pos_run: main-running thread context */
        if (!setjmp(sched.thrds[sched.cur_thrd].exe_ctx))
        {
            coop_dbg_log_cb("setjmp thrd_pos_run; back from #%d thread to "
                "scheduler: longjmp sched_pos_run\n", sched.cur_thrd);

            /* back to scheduler: sched_pos_run jump */
            longjmp(sched.exe_ctx, 1);
        } else {
            /* return from scheduler; regular run */
            coop_dbg_log_cb("Back to #%d thread (via thrd_pos_run)\n",
                sched.cur_thrd);
        }
    }
}

#ifdef CONFIG_OPT_IDLE
void coop_idle(coop_tick_t period)
{
    coop_thrd_state_t new_state = RUN;

    if (period > 0) {
        coop_dbg_log_cb("Thread #%d going idle for %lu ticks\n",
            sched.cur_thrd, (unsigned long)period);

        new_state = IDLE;
        sched.idle_n++;
        sched.thrds[sched.cur_thrd].idle_to = coop_tick_cb() + period;
    }
    _yield(new_state);
}
#else
void coop_yield(void)
{
    _yield(RUN);
}
#endif

#ifdef CONFIG_OPT_YIELD_AFTER
void coop_yield_after(coop_tick_t *after, coop_tick_t period)
{
    if (COOP_IS_TICK_OVER(coop_tick_cb(), *after))
    {
        coop_dbg_log_cb("Thread #%d yields after %lu tick\n",
            sched.cur_thrd, (unsigned long)*after);

        _yield(RUN);
        *after = coop_tick_cb() + period;
    }
}
#endif

#ifdef CONFIG_OPT_WAIT
coop_error_t coop_wait_cond(
    int sem_id, coop_tick_t timeout, coop_predic_proc_t predic, void *cv)
{
    sched.thrds[sched.cur_thrd].sem_id = sem_id;
    sched.thrds[sched.cur_thrd].predic = predic;
    sched.thrds[sched.cur_thrd].cv = cv;
    sched.thrds[sched.cur_thrd].wait_flgs.notif = 0;
    if (timeout) {
        sched.thrds[sched.cur_thrd].wait_to = coop_tick_cb() + timeout;
        sched.thrds[sched.cur_thrd].wait_flgs.inf = 0;

        coop_dbg_log_cb("Thread #%d waiting with timeout %lu ticks; "
            "sem_id: %d\n", sched.cur_thrd, (unsigned long)timeout, sem_id);
    } else {
        sched.thrds[sched.cur_thrd].wait_to = 0;
        sched.thrds[sched.cur_thrd].wait_flgs.inf = 1;

        coop_dbg_log_cb("Thread #%d waiting infinitely; sem_id: %d\n",
            sched.cur_thrd, sem_id);
    }
# ifdef CONFIG_OPT_IDLE
    sched.idle_n++;
# endif

    _yield(WAIT);

    if (sched.thrds[sched.cur_thrd].wait_flgs.notif != 0) {
        coop_dbg_log_cb("Thread #%d notified on sem_id: %d\n",
            sched.cur_thrd, sem_id);
        return COOP_SUCCESS;
    } else {
        coop_dbg_log_cb("Thread #%d wait-timeout; sem_id: %d\n",
            sched.cur_thrd, sem_id);
        return COOP_ERR_TIMEOUT;
    }
}

static inline void _notify(int sem_id, bool single)
{
    for (unsigned i = 0; i < CONFIG_MAX_THREADS; i++) {
        if (_IS_WAIT(sched.thrds[i].state) &&
            sched.thrds[i].sem_id == sem_id &&
            (!sched.thrds[i].predic || sched.thrds[i].predic(sched.thrds[i].cv)))
        {
            coop_dbg_log_cb("Thread #%d WAIT -> RUN (%s-notify on sem_id: %d)\n",
                i, (single ? "single" : "all"), sem_id);

            sched.thrds[i].wait_flgs.notif = 1;
            sched.thrds[i].state = RUN;
# ifdef CONFIG_OPT_IDLE
            sched.idle_n--;
# endif
            if (single) break;
        }
    }
}

void coop_notify(int sem_id)
{
    _notify(sem_id, true);
}

void coop_notify_all(int sem_id)
{
    _notify(sem_id, false);
}
#endif /* CONFIG_OPT_WAIT */

#ifdef CONFIG_OPT_STACK_WM
size_t coop_stack_wm()
{
    size_t stack_sz = sched.thrds[sched.cur_thrd].stack_sz;
    unsigned char *stack = (unsigned char*)sched.thrds[sched.cur_thrd].stack;
    size_t f, f2; /* free space water-marks */

    if (!stack) {
        /* stack not yet allocated (the routine called before first yield) */
        return 0;
    }

    /* first check most common type of stack (growing into lower addresses) */
    for (f = stack_sz; f && stack[f - 1] == STACK_PADD; f--);
    f = stack_sz - f;

    if (f < sizeof(void*)) {
        /* whole stack was filled up or the stack grows into higher addresses */
        for (f2 = 0; f2 < stack_sz && stack[f2] == STACK_PADD; f2++);

        /* assume growing into higher addresses type of stack */
        if (f2 > f) f = f2;
    }
    return (stack_sz - f);
}
#endif /* CONFIG_OPT_STACK_WM */

#ifdef __TEST__
bool coop_test_is_shallow()
{
# ifdef CONFIG_NOEXIT_STATIC_THREADS
    return false;
# else
    return (sched.depth == sched.thrds[sched.cur_thrd].depth);
# endif
}

unsigned coop_test_get_cur_thrd() {
    return sched.cur_thrd;
}

void coop_test_set_cur_thrd(unsigned thrd) {
    sched.cur_thrd = thrd;
}

void *coop_test_get_stack(unsigned thrd) {
    return sched.thrds[thrd].stack;
}

void coop_test_set_stack(unsigned thrd, void *stack) {
    sched.thrds[thrd].stack = stack;
}
#endif
