/*
 * Copyright (c) 2020 Piotr Stolarz
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
#define CONFIG_DEFAULT_STACK_SIZE 0x100

/**
 * Maximum number of threads supported by the library.
 * Defined as threads pool size.
 */
#define CONFIG_MAX_THREADS 5

/**
 * Enable feature: idle threads support.
 */
#define CONFIG_OPT_IDLE

/**
 * Enable feature: @ref coop_yield_after() support.
 */
#define CONFIG_OPT_YIELD_AFTER

/**
 * Enable feature: @ref coop_wait(), @ref coop_notify() support.
 */
#define CONFIG_OPT_WAIT

/**
 * Uncomment to log debugging messages.
 *
 * @note Usage of this option usually requires increasing of the thread stack
 *     sizes due to additional logging overhead.
 */
//#define COOP_DEBUG

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
//#define CONFIG_DBG_LOG_CB_ALT

/**
 * Alternative implementation of @ref coop_tick_cb() callback.
 * Default implementation depends on the underlying platform.
 *
 * @note The configuration parameter is valid only if at least one of
 *     the following features is enabled: @ref CONFIG_OPT_YIELD_AFTER,
 *     @ref CONFIG_OPT_IDLE, @ref CONFIG_OPT_WAIT.
 */
//#define CONFIG_TICK_CB_ALT

/**
 * Alternative implementation of @ref coop_idle_cb() callback.
 * Default implementation depends on the underlying platform.
 *
 * @note The configuration parameter is valid only if at least one of
 *     the following features is enabled: @ref CONFIG_OPT_IDLE.
 */
//#define CONFIG_IDLE_CB_ALT

#endif /* !COOP_DISABLE_DEFAULT_CONFIG */
#endif /* __COOP_CONFIG_H__ */
