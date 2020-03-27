/* common configuration */
#define CONFIG_DEFAULT_STACK_SIZE 0x1000
#define CONFIG_MAX_THREADS 10

#if defined(T02) || defined(T03)
# define CONFIG_OPT_IDLE
# define CONFIG_IDLE_CB_ALT
#endif

#ifdef T04
# define CONFIG_OPT_YIELD_AFTER
#endif

#ifdef T05
# define CONFIG_OPT_WAIT
#endif

#ifdef T06
# define CONFIG_OPT_WAIT
# define CONFIG_OPT_IDLE
#endif

#ifdef T07
# define CONFIG_OPT_WAIT
# define CONFIG_OPT_IDLE
# define CONFIG_OPT_IDLE_WAIT
# define CONFIG_IDLE_CB_ALT
#endif

#ifdef ST01
# define CONFIG_OPT_IDLE
#endif
