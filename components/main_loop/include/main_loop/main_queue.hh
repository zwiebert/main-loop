/**
 * \file  main_loop/main_queue.hh
 * \brief Execute functions from main task
 */

#pragma once


typedef void (*voidFunT)();

/**
 * \brief Setup component
 * \param queue_length  maximal number of items in queue
 * \param event_group   optional FreeRTOS EventGroup to signal messages pushed to queue
 * \param event_bit optional FreeRTOS EventGroup-EventBit used to signal messages pushed to queue
 * 
 */
bool mainLoop_setup(unsigned queue_length, void *event_group = 0, unsigned event_bit = 0);

/**
 * \brief            Process some  or all messages in queue
 * \note             Should be periodically from main task
 * \param max_count  number of messages to process (0 for all)
 * \time_out_ms      Duration in milliseconds to block. Blocking will only happen on an empty queue.
 * \return           number of messages processed
 */
unsigned mainLoop_processMessages(unsigned max_count = 0, unsigned time_out_ms = 0);

/**
 * \brief            Ask to execute a function in main task
 * \note             Should be called from some task.
 * \return           success
 */
bool mainLoop_callFun(voidFunT fun);

/**
 * \brief            Ask to execute a function in main task
 * \note             Should be called from interrupt
 * \return           success
 */
bool mainLoop_callFun_fromISR(voidFunT fun);

/**
 * \brief           Ask to execute a function in main task delayed or periodically
 * \note            Should be called from some task.
 * \param delay_ms  Delay and/or period in milliseconds
 * \param periodic  if true, call the function periodic. the first call is delayed
 * \return          timer handle if succeeded, NULL if failed
 */
void *mainLoop_callFunByTimer(voidFunT fun, unsigned delay_ms, bool periodic = false);
/**
 * \brief           Stop a periodic function call initiated by @ref mainLoop_callFunByTimer
 * \param tmr       timer handle returned by @ref mainLoop_callFunByTimer (NULL is safe  to pass)
 * \return          success
 */
bool mainLoop_stopFun(void *tmr, bool delete_timer = true);

/**
 * \brief           Ask to restart MCU from main task.
 * \param delay_ms  delay in milliseconds before restart occurs
 * 
 */
void mainLoop_mcuRestart(unsigned delay_ms);
