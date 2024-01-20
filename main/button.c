
#include "button.h"

static const char *TAG = "button";

typedef struct
{
        gpio_num_t pin;
        button_state_t state;
        bool inverted;
        uint16_t history;
        uint64_t down_time_us;
} __packed button_data_t;

int num_buttons = -1;
button_data_t *button_data = NULL;
QueueHandle_t button_queue = NULL;
TaskHandle_t button_task_handle = NULL;

static void update_button(button_data_t *button)
{
        button->history = (button->history << 1) | gpio_get_level(button->pin);
}

static bool button_rose(button_data_t *button)
{
        if ((button->history & BUTTON_ACTIVITY_MASK) == BUTTON_PRESSED_HISTORY)
        {
                button->history = ~(0x0000);
                return true;
        }
        return false;
}
static bool button_fell(button_data_t *button)
{
        if ((button->history & BUTTON_ACTIVITY_MASK) == BUTTON_RELEASED_HISTORY)
        {
                button->history = 0x0000;
                return true;
        }
        return false;
}
static bool button_down(button_data_t *button)
{
        if (button->inverted)
                return button_fell(button);
        return button_rose(button);
}
static bool button_up(button_data_t *button)
{
        if (button->inverted)
                return button_rose(button);
        return button_fell(button);
}

static void button_send_event(button_data_t *button, button_state_t prev_state)
{
        button_event_t new_state = {
            .pin = button->pin,
            .prev_state = prev_state,
            .new_state = button->state,
        };

        if (xQueueSend(button_queue, &new_state, BUTTON_QUEUE_MAX_WAIT_TIME) != pdTRUE)
                LOG_WARNING("Send queue failed");
}

static void button_task(void *pvParameter)
{
        for (;;)
        {
                for (int idx = 0; idx < num_buttons; idx++)
                {
                        update_button(&button_data[idx]);
                        button_state_t old_state = button_data[idx].state;
                        switch (button_data[idx].state)
                        {
                        case BUTTON_DOWN:
                                if (button_up(&button_data[idx]))
                                        button_data[idx].state = BUTTON_UP;
                                else if (esp_timer_get_time() - button_data[idx].down_time_us > BUTTON_LONG_PRESS_DURATION_US)
                                        button_data[idx].state = BUTTON_LONG;
                                break;
                        case BUTTON_UP:
                                if (button_down(&button_data[idx]))
                                {
                                        button_data[idx].down_time_us = esp_timer_get_time();
                                        button_data[idx].state = BUTTON_DOWN;
                                }
                                break;
                        case BUTTON_LONG:
                                if (button_up(&button_data[idx]))
                                        button_data[idx].state = BUTTON_UP;
                                break;
                        default:
                                LOG_ERROR("unhandled switch statement");
                                vTaskDelete(NULL);
                                break;
                        }

                        if (old_state != button_data[idx].state)
                        {
                                ESP_LOGV(TAG, "gpio: %d, %s --> %s", button_data[idx].pin, BUTTON_STATE_STRING[old_state], BUTTON_STATE_STRING[button_data[idx].state]);
                                button_send_event(&button_data[idx], old_state);
                        }
                }
                vTaskDelay(pdMS_TO_TICKS(10));
        }
}

QueueHandle_t button_init(uint64_t pin_select)
{
        if (num_buttons != -1)
        {
                LOG_WARNING("already initialized");
                return NULL;
        }

        // Configure the pins
        gpio_config_t io_conf = {
            .pin_bit_mask = pin_select,
            .mode = GPIO_MODE_INPUT,          // input only
            .pull_up_en = GPIO_PULLUP_ENABLE, // with internal pullup
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&io_conf));

        // Scan the pin map to determine number of pins
        num_buttons = 0;
        for (uint8_t pin = 0; pin < GPIO_NUM_MAX; pin++)
                if ((1ULL << pin) & pin_select)
                        num_buttons++;

        // Initialize global state and queue
        button_data = calloc(num_buttons, sizeof(button_data_t)); // TODO: statically allow memory
        if (button_data == NULL)
        {
                LOG_ERROR("calloc falied");
                button_deinit();
                return NULL;
        }
        button_queue = xQueueCreate(BUTTON_QUEUE_DEPTH, sizeof(button_event_t)); // TODO: statically allow memory
        if (button_queue == NULL)
        {
                LOG_ERROR("Create queue falied");
                button_deinit();
                return NULL;
        }

        // Scan the pin map to determine each pin number, populate the state
        uint8_t idx = 0;
        for (int pin = 0; pin < GPIO_NUM_MAX; pin++)
                if ((1ULL << pin) & pin_select)
                {
                        ESP_LOGI(TAG, "Registering button on gpio: %d", pin);
                        button_data[idx].pin = pin;
                        button_data[idx].down_time_us = 0;
                        button_data[idx].inverted = true;
                        button_data[idx].state = (gpio_get_level(button_data[idx].pin) ? BUTTON_DOWN : BUTTON_UP);
                        if (button_data[idx].inverted)
                        {
                                button_data[idx].history = 0xffff;
                                button_data[idx].state = (gpio_get_level(button_data[idx].pin) ? BUTTON_UP : BUTTON_DOWN);
                        }
                        idx++;
                }

        // Spawn a task to monitor the pins
        xTaskCreate(button_task, "button_task", 4096, NULL, 10, &button_task_handle);

        return button_queue;
}

void button_deinit(void)
{
        if (button_task_handle != NULL)
        {
                vTaskDelete(button_task_handle);
                button_task_handle = NULL;
        }
        if (button_queue != NULL)
        {
                vQueueDelete(button_queue);
                button_queue = NULL;
        }
        if (button_data != NULL)
        {
                for (int idx = 0; idx < num_buttons; idx++)
                        gpio_reset_pin(button_data[idx].pin);
                free(button_data);
                button_data = NULL;
        }
        num_buttons = -1;
}