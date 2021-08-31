/**
 * \file main_loop/esp32/main_queue.cc
 *
 */
#include "app_config/proj_app_cfg.h"

#include "main_loop/main_queue.hh"
#include <debug/dbg.h>
#include "utils_misc/int_macros.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>

#include <esp_system.h>

struct __attribute__ ((packed)) mainLoop_msgT_voidFun {
  voidFunT voidFun;
};

static bool mainLoop_pushMessage(const mainLoop_msgT_voidFun *msg);
static bool mainLoop_pushMessage_fromISR(const mainLoop_msgT_voidFun *msg);

static volatile QueueHandle_t Handle;
static volatile EventGroupHandle_t mainLoop_eventGroup;
static volatile EventBits_t mainLoop_eventBits;

bool mainLoop_setup(unsigned queue_length, void *event_group, unsigned event_bit) {
  precond(!Handle);

  const unsigned size = sizeof(struct mainLoop_msgT_voidFun);

  mainLoop_eventGroup = event_group;
  mainLoop_eventBits = BIT(event_bit);

  if ((Handle = xQueueCreate(queue_length, size))) {
    return true;
  }

  return false;
}

static bool mainLoop_pushMessage(const mainLoop_msgT_voidFun *msg) {
  precond(Handle);
  precond(msg->voidFun);

  if (xQueueSend(Handle, msg, ( TickType_t ) 10) != pdTRUE)
    return false;

  if (mainLoop_eventGroup) {
    if (!xEventGroupSetBits(mainLoop_eventGroup, mainLoop_eventBits))
      return false;
  }

  return true;
}

static bool IRAM_ATTR mainLoop_pushMessage_fromISR(const mainLoop_msgT_voidFun *msg) {
  precond(Handle);
  precond(msg->voidFun);

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  BaseType_t xHigherPriorityTaskWoken2 = pdFALSE;
  if (xQueueSendToFrontFromISR(Handle, msg, &xHigherPriorityTaskWoken ) != pdTRUE)
    return false;

  if (mainLoop_eventGroup) {
    if (!xEventGroupSetBitsFromISR(mainLoop_eventGroup, mainLoop_eventBits, &xHigherPriorityTaskWoken2))
      return false;
  }

  if (xHigherPriorityTaskWoken || xHigherPriorityTaskWoken2) {
    portYIELD_FROM_ISR();
  }

  return true;
}

bool mainLoop_callFun(voidFunT fun) {
  mainLoop_msgT_voidFun msg { fun };
  return mainLoop_pushMessage(&msg);
}

bool IRAM_ATTR mainLoop_callFun_fromISR(voidFunT fun) {
  mainLoop_msgT_voidFun msg { fun };
  return mainLoop_pushMessage_fromISR(&msg);
}

unsigned mainLoop_processMessages(unsigned max_count, unsigned time_out_ms) {
  precond(Handle);

  unsigned result = 0;

  mainLoop_msgT_voidFun msg;

  for (int i = 0; max_count == 0 || i < max_count; ++i) {
    if (!xQueueReceive(Handle, &msg, (TickType_t) pdMS_TO_TICKS(time_out_ms))) {
      break;
    }
    ++result;

    assert(msg.voidFun);
    (*msg.voidFun)();

  }

  return result;
}

void mainLoop_mcuRestart(unsigned delay_ms) {
  printf("mcu_restart()\n");
  vTaskDelay(pdMS_TO_TICKS(delay_ms));
  esp_restart();
  for (;;) {
  }
}

