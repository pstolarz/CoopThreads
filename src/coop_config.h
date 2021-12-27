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

#ifndef __COOP_CONFIG_H__
#define __COOP_CONFIG_H__

#ifdef COOP_CONFIG_FILE
/* use user defined config file instead of this one */
# include COOP_CONFIG_FILE
#elif !defined(COOP_DISABLE_DEFAULT_CONFIG)

/**
 * Default thread stack size.
 */
#ifndef CONFIG_DEFAULT_STACK_SIZE
# define CONFIG_DEFAULT_STACK_SIZE 0x100U
#endif

/**
 * Maximum number of threads supported by the library.
 * Defined as threads pool size.
 */
#ifndef CONFIG_MAX_THREADS
# define CONFIG_MAX_THREADS 5
#endif

/**
 * Enable feature: idle threads support.
 */
#ifndef CONFIG_OPT_IDLE
# define CONFIG_OPT_IDLE
#endif

/**
 * Enable feature: @ref coop_yield_after() support.
 */
#ifndef CONFIG_OPT_YIELD_AFTER
# define CONFIG_OPT_YIELD_AFTER
#endif

/**
 * Enable feature: @ref coop_wait(), @ref coop_notify() support.
 */
#ifndef CONFIG_OPT_WAIT
# define CONFIG_OPT_WAIT
#endif

/**
 * Enable feature: @ref coop_stack_wm() support.
 */
#ifndef CONFIG_OPT_STACK_WM
//# define CONFIG_OPT_STACK_WM
#endif

/**
 * If the library is used to create static number of threads at its startup
 * and the threads are not intended to exit, this parameter may be configured
 * to reduce the library footprint and memory usage.
 *
 * @note The parameter reduces the library by the code required for thread's
 *     stack memory deallocation (stack unwinding), therefore must not be used
 *     for dynamically created threads.
 *
 * @note For use for resource-constrained platforms only, where the standard
 *     configuration may be problematic.
 */
#ifndef CONFIG_NOEXIT_STATIC_THREADS
//# define CONFIG_NOEXIT_STATIC_THREADS
#endif

/**
 * Uncomment to log debugging messages.
 *
 * @note Usage of this option usually requires substantial increasing of the
 *     thread stack sizes due to additional logging overhead. This may limit
 *     usage of this parameter on resource-constrained platforms.
 */
#ifndef COOP_DEBUG
//# define COOP_DEBUG
#endif

/**
 * Alternative implementation of @ref coop_dbg_log_cb() callback used for
 * logging debug messages.
 *
 * Debug messages are logged if the library is compiled with @c COOP_DEBUG
 * macro-define. Default implementation of the @c coop_dbg_log_cb routine
 * depends on the underlying platform. By defining @c CONFIG_DBG_LOG_CB_ALT
 * there is possible to provide custom implementation for the logging routine.
 * Use the following code snippet:
 *
 * @code
 * #ifdef COOP_DEBUG
 * void coop_dbg_log_cb(const char *format, ...)
 * {
 *     // custom implementation goes here
 * }
 * #endif
 * @endcode
 */
#ifndef CONFIG_DBG_LOG_CB_ALT
//# define CONFIG_DBG_LOG_CB_ALT
#endif

/**
 * Alternative implementation of @ref coop_tick_cb() callback.
 * Default implementation depends on the underlying platform.
 *
 * @note The configuration parameter is valid only if at least one of
 *     the following features is enabled: @ref CONFIG_OPT_YIELD_AFTER,
 *     @ref CONFIG_OPT_IDLE, @ref CONFIG_OPT_WAIT.
 */
#ifndef CONFIG_TICK_CB_ALT
//# define CONFIG_TICK_CB_ALT
#endif

/**
 * Alternative implementation of @ref coop_idle_cb() callback.
 * Default implementation depends on the underlying platform.
 *
 * @note The configuration parameter is valid only if at least one of
 *     the following features is enabled: @ref CONFIG_OPT_IDLE.
 */
#ifndef CONFIG_IDLE_CB_ALT
//# define CONFIG_IDLE_CB_ALT
#endif

#endif /* !COOP_DISABLE_DEFAULT_CONFIG */
#endif /* __COOP_CONFIG_H__ */
