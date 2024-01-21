
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_err.h"

#include "button.h"
#include "espnow.h"
#include "pindef.h"
#include "rssi.h"
#include "ws2812.h"

#include "logging.h"
#include "mathop.h"

static const char __attribute__((unused)) *TAG = "app_main";

static espnow_send_param_t espnow_send_param;
static esp_connection_handle_t esp_connection_handle;

void rssi_task()
{
	ws2812_hsv_t hsv = {.h = 350, .s = 75, .v = 0};
	ws2812_handle_t ws2812_handle;
	ws2812_default_config(&ws2812_handle);
	ws2812_init(&ws2812_handle);
	ws2812_set_hsv(&ws2812_handle, &hsv);
	ws2812_update(&ws2812_handle);

	QueueHandle_t rssi_event_queue = rssi_init();
	uint8_t countdown = 0;
	uint8_t ping_count = 0;
	const uint8_t ping_reset = 30;
	for (;;)
	{
		if (ping_count)
			ping_count--;
		else
		{
			ping_count = ping_reset;
			esp_connection_send_heartbeat(&esp_connection_handle);
			// espnow_send_text(&espnow_send_param, "ping");
			// esp_connection_show_entries(&esp_connection_handle);
		}
		rssi_event_t rssi_event;
		while (xQueueReceive(rssi_event_queue, &rssi_event, 0))
		{
			// print_rssi_event(&rssi_event);
			esp_connection_update_rssi(&esp_connection_handle, &rssi_event);

			const int rssi_min = -20;
			if (rssi_event.rssi > rssi_min)
			{
				countdown = ping_reset * 3;
				float led_volume = map(rssi_event.rssi, 0, rssi_min, 50, 0);
				led_volume = constrain(led_volume, 0, 100);
				hsv.v = led_volume;
				ws2812_set_hsv(&ws2812_handle, &hsv);
				ws2812_update(&ws2812_handle);
			}
		}
		if (countdown)
			countdown--;
		else
		{
			if (esp_connection_handle.remote_connected)
				hsv.v = 3 * esp_connection_handle.remote_connected;
			else
				hsv.v = 0;
			ws2812_set_hsv(&ws2812_handle, &hsv);
			ws2812_update(&ws2812_handle);
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void app_main(void)
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	espnow_config_t espnow_config;
	espnow_wifi_default_config(&espnow_config);
	espnow_wifi_init(&espnow_config);
	espnow_default_send_param(&espnow_send_param);
	esp_connection_handle_init(&esp_connection_handle);
	esp_connection_set_peer_limit(&esp_connection_handle, 1);
	QueueHandle_t espnow_event_queue = espnow_init(&espnow_config, &esp_connection_handle);

	ret = espnow_send_text(&espnow_send_param, "device init");
	if (ret != ESP_OK)
	{
		LOG_ERROR("ESP_NOW send error, quitting");
		espnow_deinit(&espnow_send_param);
		ESP_ERROR_CHECK(ret);
		vTaskDelete(NULL);
	}

	QueueHandle_t button_event_queue = button_init();
	button_register(GPIO_BUTTON_UP, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(GPIO_BUTTON_DOWN, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(GPIO_BUTTON_LEFT, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(GPIO_BUTTON_RIGHT, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(GPIO_BUTTON_SHOOT, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(GPIO_BUTTON_TILT_LEFT, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(GPIO_BUTTON_TILT_RIGHT, BUTTON_CONFIG_ACTIVE_LOW);

	xTaskCreate(rssi_task, "rssi_task", 4096, NULL, 4, NULL);

	while (true)
	{
		button_event_t button_event;
		while (xQueueReceive(button_event_queue, &button_event, 0))
		{
			LOG_INFO("GPIO event: pin %d, state = %s --> %s", button_event.pin, BUTTON_STATE_STRING[button_event.prev_state], BUTTON_STATE_STRING[button_event.new_state]);

			esp_err_t ret;
			ret = espnow_send_data(&espnow_send_param, ESP_PEER_PACKET_TEXT, &button_event, sizeof(button_event));
			ESP_ERROR_CHECK_WITHOUT_ABORT(ret);

			char temp[64];
			switch (button_event.pin)
			{
			case GPIO_BUTTON_UP:
				snprintf(temp, 64, "GPIO_BUTTON_UP, %s", BUTTON_STATE_STRING[button_event.new_state]);
				break;
			case GPIO_BUTTON_DOWN:
				snprintf(temp, 64, "GPIO_BUTTON_DOWN, %s", BUTTON_STATE_STRING[button_event.new_state]);
				break;
			case GPIO_BUTTON_LEFT:
				snprintf(temp, 64, "GPIO_BUTTON_LEFT, %s", BUTTON_STATE_STRING[button_event.new_state]);
				break;
			case GPIO_BUTTON_RIGHT:
				snprintf(temp, 64, "GPIO_BUTTON_RIGHT, %s", BUTTON_STATE_STRING[button_event.new_state]);
				break;
			case GPIO_BUTTON_SHOOT:
				snprintf(temp, 64, "GPIO_BUTTON_SHOOT, %s", BUTTON_STATE_STRING[button_event.new_state]);
				break;
			case GPIO_BUTTON_TILT_LEFT:
				snprintf(temp, 64, "GPIO_BUTTON_TILT_LEFT, %s", BUTTON_STATE_STRING[button_event.new_state]);
				break;
			case GPIO_BUTTON_TILT_RIGHT:
				snprintf(temp, 64, "GPIO_BUTTON_TILT_RIGHT, %s", BUTTON_STATE_STRING[button_event.new_state]);
				break;
			default:
				break;
			}
			// espnow_send_text(&espnow_send_param, temp);
		}

		espnow_event_t espnow_evt;
		while (xQueueReceive(espnow_event_queue, &espnow_evt, 0))
		{
			espnow_data_t *recv_data = NULL;
			switch (espnow_evt.id)
			{
			case ESPNOW_SEND_CB:
				espnow_event_send_cb_t *send_cb = &espnow_evt.info.send_cb;
				if (send_cb->status != ESP_NOW_SEND_SUCCESS)
				{
					LOG_WARNING("Send data to peer " MACSTR " failed", MAC2STR(send_cb->mac_addr));
				}
				else
				{
					LOG_VERBOSE("Send data to peer " MACSTR " success", MAC2STR(send_cb->mac_addr));
				}
				break;
			case ESPNOW_RECV_CB:
				espnow_event_recv_cb_t *recv_cb = &espnow_evt.info.recv_cb;
				if (!(recv_data = espnow_data_parse(recv_data, recv_cb)))
				{
					LOG_WARNING("bad data packet from peer " MACSTR, MAC2STR(recv_cb->mac_addr));
					free(recv_cb->data);
					break;
				}

				esp_peer_t *peer = esp_connection_mac_add_to_entry(&esp_connection_handle, recv_cb->mac_addr);
				espnow_get_send_param(&espnow_send_param, peer);
				esp_peer_process_received(peer, recv_data);
				free(recv_cb->data);
				break;
			default:
				LOG_ERROR("Callback type error: %d", espnow_evt.id);
				break;
			}
		}
		esp_connection_handle_update(&esp_connection_handle);
		vTaskDelay(1);
	}
}