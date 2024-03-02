
#include "joystick.h"

static const char *TAG = "joystick";

typedef struct
{
        gpio_num_t high_pin, low_pin;
        adc1_channel_t channel;
        button_state_t high_state, low_state;

        bool inverted;
        int raw;
        int voltage;
} __packed joystick_data_t;

uint64_t joystick_pinmask = 0;
static esp_adc_cal_characteristics_t adc1_chars = {0};
joystick_data_t joystick_data[BUTTON_MAX_ARRAY_SIZE];
QueueHandle_t joystick_queue = NULL;
TaskHandle_t joystick_task_handle = NULL;

static bool adc1_calibration_init(void)
{
        esp_err_t ret;

        ret = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP_FIT);
        if (ret == ESP_ERR_NOT_SUPPORTED)
        {
                LOG_WARNING("Calibration scheme not supported, skip software calibration");
                return false;
        }

        if (ret == ESP_ERR_INVALID_VERSION)
        {
                LOG_WARNING("eFuse not burnt, skip software calibration");
                return false;
        }

        if (ret != ESP_OK)
        {
                ESP_LOGE(TAG, "Invalid arg");
                return false;
        }

        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc1_chars);
        return true;
}

static void joystick_send_event(int pin, button_state_t state, const button_state_t prev_state)
{
        button_event_t new_state = {
            .pin = pin,
            .prev_state = prev_state,
            .new_state = state,
        };

        if (xQueueSend(joystick_queue, &new_state, 0) != pdTRUE)
                LOG_WARNING("Send queue failed");
}

static void update_joystick(joystick_data_t *joystick)
{
        button_state_t old_high_state = joystick->high_state;
        button_state_t old_low_state = joystick->low_state;
        joystick->raw = adc1_get_raw(joystick->channel);
        // LOG_INFO("adc channel [%d], raw  data: %d", joystick->channel, joystick->raw);
        joystick->voltage = esp_adc_cal_raw_to_voltage(joystick->raw, &adc1_chars);
        // LOG_INFO("adc channel [%d], cali data: %d mV", joystick->channel, joystick->voltage);

        if (joystick->voltage < 700)
                joystick->low_state = BUTTON_DOWN;
        if (joystick->voltage > 900 && joystick->voltage < 2400)
                joystick->low_state = BUTTON_UP;

        if (joystick->voltage > 2400)
                joystick->high_state = BUTTON_DOWN;

        if (joystick->voltage < 2300 && joystick->voltage > 1400)
                joystick->high_state = BUTTON_UP;

        if (joystick->low_state != old_low_state)
                joystick_send_event(joystick->low_pin, joystick->low_state, old_low_state);

        if (joystick->high_state != old_high_state)
                joystick_send_event(joystick->high_pin, joystick->high_state, old_high_state);
}

uint8_t count_num_joysticks(const uint64_t bitfield)
{
        uint64_t field = bitfield;
        uint8_t count = 0;
        while (field)
        {
                field &= (field - 1);
                count++;
        }
        return count;
}

static void joystick_task(void *pvParameter)
{
        adc1_calibration_init();
        adc1_config_width(ADC_WIDTH_BIT_12);
        for (;;)
        {
                uint8_t num_joysticks = count_num_joysticks(joystick_pinmask);
                for (int idx = 0; idx < num_joysticks; idx++)
                        update_joystick(&joystick_data[idx]);
                vTaskDelay(pdMS_TO_TICKS(10));
        }
}

QueueHandle_t joystick_init(void)
{
        if ((joystick_queue != NULL) || (joystick_task_handle != NULL))
        {
                LOG_WARNING("Already initialized, queue=0x%X, task=0x%X", (uintptr_t)joystick_queue, (uintptr_t)joystick_task_handle);
                return NULL;
        }

        // Initialize queue
        joystick_queue = xQueueCreate(BUTTON_QUEUE_DEPTH, sizeof(button_event_t)); // TODO: statically allow memory
        if (joystick_queue == NULL)
        {
                LOG_ERROR("Create queue falied");
                joystick_deinit();
                return NULL;
        }

        // Spawn a task to monitor the pins
        xTaskCreate(joystick_task, "joystick_task", 4096, NULL, 10, &joystick_task_handle);

        return joystick_queue;
}

void joystick_register(const gpio_num_t high_pin, const gpio_num_t low_pin, const gpio_num_t adc_pin, const bool inverted)
{
        adc_channel_t channel = adc_pin - 1;
        if (joystick_pinmask & (1ULL << channel))
        {
                LOG_WARNING("The adc1 channel [%d] has been already initialized as an input", channel);
                return;
        }

        uint8_t num_joysticks = count_num_joysticks(joystick_pinmask);
        LOG_INFO("Registering joystick on channel: %d, id: %d", channel, num_joysticks);

        joystick_pinmask = joystick_pinmask | (1ULL << channel);
        joystick_data[num_joysticks].high_pin = high_pin;
        joystick_data[num_joysticks].low_pin = low_pin;
        joystick_data[num_joysticks].channel = channel;
        joystick_data[num_joysticks].inverted = inverted;
        adc1_config_channel_atten(joystick_data[num_joysticks].channel, ADC_ATTEN_DB_11);
}

void joystick_deinit(void)
{
        if (joystick_task_handle != NULL)
        {
                vTaskDelete(joystick_task_handle);
                joystick_task_handle = NULL;
        }
        if (joystick_queue != NULL)
        {
                vQueueDelete(joystick_queue);
                joystick_queue = NULL;
        }

        // TODO
        joystick_pinmask = 0;
}