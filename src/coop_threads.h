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

#ifndef __COOP_THREADS_H__
#define __COOP_THREADS_H__

#include <stdbool.h>
#include <stddef.h> /* size_t */
#include "coop_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    COOP_SUCCESS = 0,   /** No error. */
    COOP_ERR_INV_ARG,   /** Invalid argument error */
    COOP_ERR_LIMIT,     /** Max. limit reached error */
    COOP_ERR_TIMEOUT    /** Timeout occured */
} coop_error_t;

/**
 * Thread routine type.
 *
 * @param arg User argument passed untouched to the thread routine.
 */
typedef void (*coop_thrd_proc_t)(void *arg);

#if CONFIG_OPT_WAIT
/**
 * Waiting-predicate routine type.
 *
 * @param cv User argument (aka @a conditional-variable) passed untouched to
 *     the routine.
 *
 * @return true Waiting-for-criteria are met and waiting thread(s) shall
 *     be notified (switched back from waiting to running state).
 * @return false Otherwise.
 */
typedef bool (*coop_predic_proc_t)(void *cv);
#endif

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
 * Start scheduler service to run scheduled threads.
 * The routine returns when the last scheduled thread ends.
 *
 * @note If the library is configured with @ref CONFIG_NOEXIT_STATIC_THREADS
 *     the routine is not intended to exit. @c coop_sched_service() fires
 *     an assertion in case all scheduled threads would finish.
 */
void coop_sched_service(void);

/**
 * Schedule a thread to run.
 *
 * @param proc Thread routine. The argument is required.
 * @param name Thread name. May be @c NULL. This string is stored in the
 *     library by a pointer therefore shall be maintained by a caller for
 *     the whole thread's lifespan.
 * @param stack_sz Thread stack size. If 0 default value configured by
 *     @c CONFIG_DEFAULT_STACK_SIZE is used.
 * @param arg User argument passed untouched to the thread routine.
 *
 * @return COOP_SUCCESS Function finished with success.
 * @return COOP_ERR_INV_ARG Invalid argument.
 * @return COOP_ERR_LIMIT Maximum number of threads reached.
 */
coop_error_t coop_sched_thread(coop_thrd_proc_t proc, const char *name,
    size_t stack_sz, void *arg);

/**
 * Get currently running thread name (as passed to @ref coop_sched_thread()
 * during thread creation).
 *
 * @note To be called from the thread routine only.
 */
const char *coop_thread_name(void);

#if CONFIG_OPT_IDLE
/**
 * Declare the currently running thread shall be idle for specific @c period
 * of ticks. The @period argument must not be greater than @ref COOP_MAX_PERIOD.
 *
 * @note To be called from the thread routine only.
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

#if CONFIG_OPT_YIELD_AFTER
/**
 * Similar to @ref coop_yield() but the yield happens only if current clock
 * tick occurs after the tick passed by @c *after argument.
 *
 * @param after In/out argument. Points to a clock tick after which yielding
 *     to the scheduler shall occur. After returning back to the yielded thread
 *     @c after is incremented by @c period value. The argument is not updated
 *     if the execution switch doesn't occur.
 * @param period Period of time in clock ticks to increment @c after value.
 *     The argument must not be greater than @ref COOP_MAX_PERIOD.
 *
 * @note To be called from the thread routine only.
 *
 * @note The routine is intended to be used in time-consuming loop to be called
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
 *         coop_yield_after(&after, MAX_RUN_TIME);
 *     }
 * }
 * @endcode
 */
void coop_yield_after(coop_tick_t *after, coop_tick_t period);
#endif

#if CONFIG_OPT_IDLE
/**
 * coop_yield() is an alias to coop_idle(0).
 */
# define coop_yield() coop_idle(0)
#else
/**
 * Yield currently running thread back to scheduler.
 *
 * @note To be called from the thread routine only.
 */
void coop_yield(void);
#endif

/*
 * Platform specific callbacks specification section.
 */

#if CONFIG_OPT_IDLE || CONFIG_OPT_YIELD_AFTER || CONFIG_OPT_WAIT
/**
 * Get clock tick at the moment of the callback-routine call.
 */
coop_tick_t coop_tick_cb();
#endif

#if CONFIG_OPT_IDLE
/**
 * System idle callback.
 *
 * The routine is called when no threads are scheduled to run for @c period of
 * ticks. Implementation may leverage this to save battery energy consumption
 * by switch the system to a desired sleep mode.
 *
 * @param period Number of clock ticks the idle state shall last.
 *     The argument may be 0 to indicate infinitive idle time, which may happen
 *     for infinitive waits. @see coop_wait().
 */
void coop_idle_cb(coop_tick_t period);
#endif

#if CONFIG_OPT_WAIT
/**
 * Switch current thread into wait-for-a-notification-signal state.
 *
 * @param sem_id Semaphore id. This is an arbitrary chosen integer value used
 *     to match waiting thread(s) with a notification signal.
 * @param timeout A timeout value the thread will wait for a notification
 *     before the timeout will be reported. Pass 0 for infinite wait.
 *
 * @return COOP_SUCCESS Notification signal received
 * @return COOP_ERR_TIMEOUT Timeout reached.
 *
 * @note To be called from the thread routine only.
 *
 * @note @c coop_wait() routine may switch the system to the idle state (via
 *     @ref coop_idle_cb() callback if @c CONFIG_OPT_IDLE is configured). It
 *     enables waiting-for-a-hardware events support. For example an incoming
 *     network packet may wake-up the hardware from a sleep-mode and its ISR
 *     will notify thread(s) about the new packet arrival. Since the configured
 *     timeout may be infinite (@c timeout set to 0) the system may be switched
 *     to indefinitely long idle time. This should be taken into account while
 *     using the routine.
 *
 * @see coop_notify()
 * @see coop_notify_all()
 */
# define coop_wait(sem_id, timeout) coop_wait_cond(sem_id, timeout, NULL, NULL)

/**
 * Conditional wait.
 *
 * @param sem_id Semaphore id.
 * @param timeout Waiting timeout.
 * @param predic Waiting-predicate routine. If @c NULL no predicate is provided
 *     and @c coop_wait_cond() is equivalent to @c coop_wait() - works as
 *     a binary semaphore. @see coop_predic_proc_t for more information about
 *     the predicate routine.
 * @param cv User argument passed untouched to the predicate routine.
 *     The argument is ignored if @c predic is @c NULL.
 *
 * @return COOP_SUCCESS Notification signal received
 * @return COOP_ERR_TIMEOUT Timeout reached.
 *
 * @note To be called from the thread routine only.
 * @see coop_wait() for additional notes.
 */
coop_error_t coop_wait_cond(
    int sem_id, coop_tick_t timeout, coop_predic_proc_t predic, void *cv);

/**
 * Send notification signal for a single thread waiting on @c sem_id.
 *
 * @note To be called from an arbitrary routine including ISR.
 *
 * @note While calling from ISR debug logs must be disabled or handled in
 *     a special way (see @ref CONFIG_DBG_LOG_CB_ALT) to avoid interrupt
 *     service related issues.
 *
 * @see coop_wait()
 */
void coop_notify(int sem_id);

/**
 * Send notification signal for all threads waiting on @c sem_id.
 *
 * @see coop_notify() for additional notes.
 */
void coop_notify_all(int sem_id);
#endif /* CONFIG_OPT_WAIT */

#if CONFIG_OPT_STACK_WM
/**
 * Get maximum stack usage water-mark for the current thread.
 *
 * @return Max stack usage water-mark in bytes.
 *
 * @note The routine tries to detect type of the platform stack (growing into
 *     lower or higher addresses). Note the algorithm may not to be always
 *     perfectly accurate therefore the returned value shall be treated merely
 *     as an indicator while experimenting with various stack sizes.
 *
 * @note To be called from the thread routine only.
 */
size_t coop_stack_wm();
#endif

#if COOP_DEBUG
/**
 * Debug message log callback.
 */
void coop_dbg_log_cb(const char *format, ...);
#else
/* debug turned off */
# define coop_dbg_log_cb(...)
#endif

#ifdef __cplusplus
}
#endif

#ifdef COOP_TEST
bool coop_test_is_shallow(void);
unsigned coop_test_get_cur_thrd();
void coop_test_set_cur_thrd(unsigned cur_thrd);
void *coop_test_get_stack(unsigned thrd);
void coop_test_set_stack(unsigned thrd, void *stack);
#endif
#endif /* __COOP_THREADS_H__ */
