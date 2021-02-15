# CoopThreads

`CoopThreads` is a lightweight, platform agnostic, stackful cooperative threads
library with round-robin scheduler. The library is intended to be used on
resource constrained platforms (e.g. 8-bit AVR) where using fully-fledged RTOS
would be problematic.

The library has been tested on the following platforms:

* Arduino AVR.
    * Tested on Arduino UNO (ATmega328P).
* Arduino ESP32.
    * Tested on ESP32-DevKitC (ESP32-WROOM-32)
* Arduino ESP8266.
    * Tested on WeMos D1
* Unix/POSIX
    * Mostly used for unit testing. See [`extras/test`](extras/test) directory
      content as a reference how to use the library on POSIX conforming platforms.

## Features

* Core of the library uses `setjmp(3)`/`longjmp(3)` (part of standard C library)
  and `alloca(3)` to save/restore execution context and allocate thread stacks
  respectively. Therefore it shall be possible to use for large number of
  conforming platforms.

* Idle related API allows switching the platform to a desired sleep mode and
  reduce power consumption.

* Wait/notify support for effective threads synchronization.

* Small and configurable footprint. Unused features may be turned off and reduce
  footprint of a compiled image.

* Although the library was created for Arduino environment in mind, it may be
  easily ported for other development platforms. See [Platform Callbacks](#platform-callbacks)
  section for more details.

## Usage

Refer to [`examples`](examples) directory for examples presenting usage of the
various library features in Arduino environment. Thorough API specification is
contained as inline documentation in [`src/coop_threads.h`](src/coop_threads.h)
header.

File [`src/coop_config.h`](src/coop_config.h) contains parameters
configuring the library functionality. See the file for more details.

## Thread Stack

`CoopThreads` is a stackful threads library, which means each thread running
under control of the library works on its own stack. The stack size may be set
for each thread separately during thread creation. It's important to note the
thread stacks are created on the main stack the library code is running on,
therefore it is **critical to assure proper main stack size while using the
library.**

The library controls thread stack creation and removal as explained on the
following example.

```
|  Scheduler stack  |  ^        |  Scheduler stack  |
+-------------------+  |        +-------------------+
|                   |  |        |                   |      |                   |
|  Thread 3 stack   |  |        |  Thread 3 stack   |      |Thread 3 terminated|
|                   |  |        |                   |      |                   |
+-------------------+  |        +-------------------+      | Thread 2,3 stacks |
|                   |  | Main   |                   |      |  freed (unwinded) |
|  Thread 2 stack   |  | Stack  |Thread 2 terminated|      |                   |
|                   |  |        |    (stack-hole)   |      |New scheduler stack|
+-------------------+  |        +-------------------+      +-------------------+
|                   |  |        |                   |      |                   |
|  Thread 1 stack   |  |        |  Thread 1 stack   |      |  Thread 1 stack   |
|                   |  |        |                   |      |                   |
+-------------------+  |        +-------------------+      +-------------------+
        Fig 1                           Fig 2                      Fig 3
```
1. Fig. 1. Three threads are created - thread 1 (as first), 2, 3 (as last) by
   calling `coop_sched_thread()` routine. The threads are created on the main
   stack with their stack sizes specified during the thread creation by passing
   appropriate argument to `coop_sched_thread()`. The scheduler stack is now
   located above the lastly created thread stack (that is thread 3), therefore
   doesn't interfere with working thread stacks.

2. Fig. 2. Thread 2 terminates. Since its stack is not located as the last on
   the stacks chain, the already terminated thread 2 stack starts a *stack-hole*.
   That is it still occupies the main stack space even its thread is already
   terminated.

3. Fig. 3. Thread 3 terminates. Since there is no working thread stacks between
   the thread 3 stack and the thread 1 stack (now the last stack on the chain)
   the stack is unwinded and the scheduler stack is now located on the position
   previously occupied by thread 2. From now all new thread stacks will be located
   over the thread 1 stack.

**IMPORTANT NOTE**: Setting up thread stack size shall take into account not
only dynamic changes of the thread stack resulting from activities performed by
a thread during its run-time, but also must foresee some additional stack space
required by preemptive ISRs activities. For this reason exact minimal thread
stack size for a given thread routine is fluent and may substantially differ
for various platforms and development environments. If low memory RAM usage is
critical, it's usually feasible to start with the trial and error method - try
with some minimal thread stack size value and increase its size in case of
platform instability/crashes. If the library is configured with
`CONFIG_OPT_STACK_WM`, `coop_stack_wm()` may be used to assess maximum thread
stack usage while choosing the optimal thread stack size configuration.

## Platform Callbacks

The library uses callbacks routines to access platform specific functionality.
The following callbacks are defined:

* `coop_tick_cb()` - callback used to get clock tick value at the routine call
  time. The routine is called-back if the library was configured with time
  related functionality (configuration parameters: `CONFIG_OPT_IDLE`,
  `CONFIG_OPT_YIELD_AFTER`,`CONFIG_OPT_WAIT`). Note, the library doesn't define
  the *tick* in terms of time duration. This quantity is platform specific.

* `coop_idle_cb()` - switch the platform into the idle mode. The routine is
  called-back if the library was configured with idle API support (`CONFIG_OPT_IDLE`
  configuration parameter) and all active threads are going to be idle for a
  specific amount of time. Implementing this routine enables switching the
  platform into a desired sleep-mode therefore reducing power consumption while
  in no-activity time.

* `coop_dbg_log_cb()` - callback used to log debug messages. Called only if
  compiled with debug logs turned on (`COOP_DEBUG` parameter).

[`src/platform`](src/platform) directory contains basic, default implementation
of `CoopThreads` library callbacks for various platforms. The implementation
serves as an example, which should be sufficient for most of the needs. Every
of the callbacks may be overridden by defining appropriate configuration parameter
to provide custom version of the callback.

For example if there is a need to provide low-power sleep modes support during
the system idle state, a library user may require to:

* Provide custom implementation of `coop_idle_cb()` (`CONFIG_IDLE_CB_ALT` parameter)
  to switch the platform into a desired sleep mode.

* Provide custom implementation of `coop_tick_cb()` (`CONFIG_TICK_CB_ALT` parameter)
  to adjust the clock ticks with the time spent during the sleep mode.

## License

2 clause BSD license. See [`LICENSE`](LICENSE) file for details.
