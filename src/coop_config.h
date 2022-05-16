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

#ifndef __COOP_CONFIG_H__
#define __COOP_CONFIG_H__

#ifdef COOP_CONFIG_FILE
/*
 * User defined config file
 */
# include COOP_CONFIG_FILE
#else

/**
 * Default thread stack size.
 */
# ifndef CONFIG_DEFAULT_STACK_SIZE
#  define CONFIG_DEFAULT_STACK_SIZE 0x100U
# endif

/**
 * Maximum number of threads supported by the library.
 * Defined as threads pool size.
 */
# ifndef CONFIG_MAX_THREADS
#  define CONFIG_MAX_THREADS 5
# endif

/**
 * Boolean parameter to enable @ref coop_idle().
 */
# ifndef CONFIG_OPT_IDLE
#  define CONFIG_OPT_IDLE 1
# endif

/**
 * Boolean parameter to enable @ref coop_yield_after().
 */
# ifndef CONFIG_OPT_YIELD_AFTER
#  define CONFIG_OPT_YIELD_AFTER 1
# endif

/**
 * Boolean parameter to enable @ref coop_wait(), @ref coop_notify().
 */
# ifndef CONFIG_OPT_WAIT
#  define CONFIG_OPT_WAIT 1
# endif

/**
 * Boolean parameter to enable @ref coop_stack_wm().
 */
# ifndef CONFIG_OPT_STACK_WM
#  define CONFIG_OPT_STACK_WM 0
# endif

/**
 * If the library is used to create static number of threads at its startup
 * and the threads are not intended to exit, this boolean parameter may be
 * configured to reduce the library footprint and memory usage.
 *
 * @note The parameter reduces the library by the code required for thread's
 *     stack memory deallocation (stack unwinding), therefore must not be used
 *     for dynamically created threads.
 *
 * @note For use for resource-constrained platforms only, where the standard
 *     configuration may be problematic.
 */
# ifndef CONFIG_NOEXIT_STATIC_THREADS
#  define CONFIG_NOEXIT_STATIC_THREADS 0
# endif

/**
 * Boolean parameter to control logging debug messages.
 *
 * @note Usage of this option usually requires substantial increasing of the
 *     thread stack sizes due to additional logging overhead. This may limit
 *     usage of this parameter on resource-constrained platforms.
 */
# ifndef COOP_DEBUG
#  define COOP_DEBUG 0
# endif

/**
 * Alternative implementation of @ref coop_dbg_log_cb() callback used for
 * logging debug messages.
 *
 * Debug messages are logged if the library is compiled with @c COOP_DEBUG
 * macro-define. Default implementation of the @c coop_dbg_log_cb routine
 * depends on the underlying platform. By configuring the boolean parameter
 * it is possible to provide custom implementation for the logging routine.
 *
 * Use the following code snippet:
 *
 * @code
 * #if COOP_DEBUG
 * void coop_dbg_log_cb(const char *format, ...)
 * {
 *     // custom implementation goes here
 * }
 * #endif
 * @endcode
 */
# ifndef CONFIG_DBG_LOG_CB_ALT
#  define CONFIG_DBG_LOG_CB_ALT 0
# endif

/**
 * Alternative implementation of @ref coop_tick_cb() callback.
 * Default implementation depends on the underlying platform.
 *
 * @note The boolean parameter is valid only if at least one of the following
 *     features is enabled: @ref CONFIG_OPT_YIELD_AFTER, @ref CONFIG_OPT_IDLE,
 *     @ref CONFIG_OPT_WAIT.
 */
# ifndef CONFIG_TICK_CB_ALT
#  define CONFIG_TICK_CB_ALT 0
# endif

/**
 * Alternative implementation of @ref coop_idle_cb() callback.
 * Default implementation depends on the underlying platform.
 *
 * @note The boolean parameter is valid only if @ref CONFIG_OPT_IDLE feature
 *     is enabled.
 */
# ifndef CONFIG_IDLE_CB_ALT
#  define CONFIG_IDLE_CB_ALT 0
# endif

#endif

/*
 * If a boolean parameter is defined w/o value assigned, it is assumed as
 * configured.
 */
#define __XEXT1(__prm) (1##__prm)
#define __EXT1(__prm) __XEXT1(__prm)

#ifdef CONFIG_OPT_IDLE
# if (__EXT1(CONFIG_OPT_IDLE) == 1)
#  undef CONFIG_OPT_IDLE
#  define CONFIG_OPT_IDLE 1
# endif
#endif

#ifdef CONFIG_OPT_YIELD_AFTER
# if (__EXT1(CONFIG_OPT_YIELD_AFTER) == 1)
#  undef CONFIG_OPT_YIELD_AFTER
#  define CONFIG_OPT_YIELD_AFTER 1
# endif
#endif

#ifdef CONFIG_OPT_WAIT
# if (__EXT1(CONFIG_OPT_WAIT) == 1)
#  undef CONFIG_OPT_WAIT
#  define CONFIG_OPT_WAIT 1
# endif
#endif

#ifdef CONFIG_OPT_STACK_WM
# if (__EXT1(CONFIG_OPT_STACK_WM) == 1)
#  undef CONFIG_OPT_STACK_WM
#  define CONFIG_OPT_STACK_WM 1
# endif
#endif

#ifdef CONFIG_NOEXIT_STATIC_THREADS
# if (__EXT1(CONFIG_NOEXIT_STATIC_THREADS) == 1)
#  undef CONFIG_NOEXIT_STATIC_THREADS
#  define CONFIG_NOEXIT_STATIC_THREADS 1
# endif
#endif

#ifdef COOP_DEBUG
# if (__EXT1(COOP_DEBUG) == 1)
#  undef COOP_DEBUG
#  define COOP_DEBUG 1
# endif
#endif

#ifdef CONFIG_DBG_LOG_CB_ALT
# if (__EXT1(CONFIG_DBG_LOG_CB_ALT) == 1)
#  undef CONFIG_DBG_LOG_CB_ALT
#  define CONFIG_DBG_LOG_CB_ALT 1
# endif
#endif

#ifdef CONFIG_TICK_CB_ALT
# if (__EXT1(CONFIG_TICK_CB_ALT) == 1)
#  undef CONFIG_TICK_CB_ALT
#  define CONFIG_TICK_CB_ALT 1
# endif
#endif

#ifdef CONFIG_IDLE_CB_ALT
# if (__EXT1(CONFIG_IDLE_CB_ALT) == 1)
#  undef CONFIG_IDLE_CB_ALT
#  define CONFIG_IDLE_CB_ALT 1
# endif
#endif

#undef __EXT1
#undef __XEXT1

#endif /* __COOP_CONFIG_H__ */
