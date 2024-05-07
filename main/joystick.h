
#pragma once

#include <string.h>

#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_adc_cal.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "logging.h"
#include "button.h"
#include "mathop.h"

QueueHandle_t joystick_init(void);
void joystick_register(const gpio_num_t high_pin, const gpio_num_t low_pin, const gpio_num_t adc_pin, const bool inverted);
void joystick_deinit(void);
void joystick_calibrate(void);
void print_joystick_stat(void);