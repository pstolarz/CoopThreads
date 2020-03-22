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

#ifndef __COOP_THREADS_H__
#define __COOP_THREADS_H__

#include <stdbool.h>
#include <stddef.h> /* size_t */
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    COOP_SUCCESS = 0,   /** No error. */
    COOP_INV_ARG,       /** Invalid argument error */
    COOP_ERR_LIMIT      /** Max. limit reached error */
} coop_error_t;

typedef void (*coop_thrd_proc_t)(void*);

/**
 * Clock tick type (must be some sort of unsigned integer).
 */
typedef unsigned long coop_tick_t;

#define COOP_MAX_TICK ((coop_tick_t)-1)

/*
 * Max ticks distance for which COOP_OVER_TICKS() return @c true.
 * 0xf for uint8_t, 0xff for uint16_t, 0xffff for uint32_t, etc.
 */
#define COOP_OVER_TICKS ((coop_tick_t)(-1) >> (4 * sizeof(coop_tick_t)))

/*
 * To avoid issues related with @c coop_tick_t type overflow, the library uses
 * distance approach while comparing if @c _t1 tick is greater or equal than
 * @c _t2. If the difference between them is less than @c COOP_OVER_TICKS then
 * @c _t1 is considered greater or equal, otherwise it's assumed lesser.
 */
#define COOP_IS_TICK_OVER(_t1, _t2) (((_t1) - (_t2)) < COOP_OVER_TICKS)

/**
 * Max allowed ticks period value.
 */
#define COOP_MAX_PERIOD (COOP_MAX_TICK - COOP_OVER_TICKS + 1)

/**
 * Run scheduler service to run scheduled threads.
 * The routine returns when last scheduled thread ends.
 */
void coop_sched_service(void);

/**
 * Schedule a thread to run.
 *
 * @param proc Thread routine. The argument is required.
 * @param name Thread name. May be @c NULL.
 * @param stack_sz Thread stack size. If 0 default value configured by
 *     @c CONFIG_DEFAULT_STACK_SIZE is used.
 * @param arg User argument passed untouched to the thread routine.
 *
 * @return COOP_INV_ARG Invalid argument.
 * @return COOP_ERR_LIMIT Maximum number of threads reached.
 */
coop_error_t coop_sched_thread(coop_thrd_proc_t proc, const char *name,
    size_t stack_sz, void *arg);

/**
 * Get currently running thread name (as passed to @ref coop_sched_thread()
 * during thread creation).
 *
 * @note To be called from thread routine only.
 */
const char *coop_get_thread_name(void);

#ifdef CONFIG_OPT_IDLE
/**
 * Declare the currently running thread shall be idle for specific @c period
 * of ticks. The @period argument must be not greater than @ref COOP_MAX_PERIOD.
 *
 * @note To be called from thread routine only.
 *
 * @note Due to character of the cooperative scheduling, @c coop_idle() routine
 *     doesn't guarantee the calling thread will be re-run (waken-up) after the
 *     exact period of time. In fact the wake-up time will be equal or greater
 *     than specified in the @c period argument. To reduce this side effect
 *     special precautions shall be taken while implementing the threads:
 *
 *     - Consequently use coop_idle() routine in threads implementation instead
 *       of platform specific sleeps (freezing the execution for a specific
 *       amount of time).
 *     - Reduce time spend in the thread up to minimum. @ref coop_yield_after()
 *       may be helpful in this case.
 */
void coop_idle(coop_tick_t period);
#endif

#ifdef CONFIG_OPT_YIELD_AFTER
/**
 * Similar to @ref coop_yield() but the yield happens only if current clock
 * tick occurs after the tick passed by @c after argument.
 *
 * @return @c true - the thread was yielded to the scheduler and returned back,
 *     @c false otherwise.
 *
 * @note To be called from thread routine only.
 *
 * @note The routine is intended to be used in time-consuming loop to call
 *     periodically to check if the routine doesn't spend too much time running
 *     and return to the scheduler if so. To be used as in the following code
 *     snippet:
 *
 * @code
 * void thrd_proc(void *arg)
 * {
 *     coop_tick_t after = coop_tick_cb() + MAX_RUN_TIME;
 *
 *     // time-consuming loop
 *     while (...) {
 *         ...
 *         // Yield the thread if the time passed. The loop will be continued
 *         // after the scheduler will schedule the thread to run in the next
 *         // scheduling round.
 *         if (coop_yield_after(after))
 *             after = coop_tick_cb() + MAX_RUN_TIME;
 *     }
 * }
 * @endcode
 */
bool coop_yield_after(coop_tick_t after);
#endif

#ifdef CONFIG_OPT_IDLE
/**
 * coop_yield() is an alias to coop_idle(0).
 */
# define coop_yield() coop_idle(0)
#else
/**
 * Yield currently running thread back to scheduler.
 *
 * @note To be called from thread routine only.
 */
void coop_yield(void);
#endif

/*
 * Platform specific callbacks specification section.
 */

#if defined(CONFIG_OPT_IDLE) || defined(CONFIG_OPT_YIELD_AFTER)
/**
 * Get clock tick for the routine call time callback.
 */
coop_tick_t coop_tick_cb();
#endif

#ifdef CONFIG_OPT_IDLE
/**
 * System idle callback.
 *
 * The routine is called when no threads are scheduled to run for @c period of
 * ticks. Implementation may leverage this to save battery energy consumption
 * by switch the system to a desired sleep mode.
 *
 * @param period Number of clock ticks the idle state shall last.
 */
void coop_idle_cb(coop_tick_t period);
#endif

#ifdef COOP_DEBUG
/**
 * Debug message log callback.
 */
void coop_dbg_log_cb(const char *format, ...);
#else
/* debug turned off */
#define coop_dbg_log_cb(...)
#endif /* COOP_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* __COOP_THREADS_H__ */
