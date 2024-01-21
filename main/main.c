
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_err.h"

#include "button.h"
#include "espnow.h"
#include "pindef.h"
#include "rssi.h"
#include "ws2812.h"

static const char __attribute__((unused)) *TAG = "app_main";

static esp_peer_t *the_one_connected_peer;
static espnow_config_t espnow_config;
static espnow_send_param_t espnow_send_param;
static esp_connection_handle_t esp_connection_handle;

float constrain(float value, float min, float max)
{
	if (value >= max)
		return max;
	if (value <= min)
		return min;
	return value;
}

float map(float value, float in_max, float in_min, float out_max, float out_min)
{
	return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

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
	for (;;)
	{
		if (ping_count)
			ping_count--;
		else
		{
			ping_count = 3;
			espnow_send_text(&espnow_send_param, "ping");
		}
		rssi_event_t rssi_event;
		while (xQueueReceive(rssi_event_queue, &rssi_event, 0))
		{
			const int rssi_min = -20;
			// print_rssi_event(&rssi_event);
			if (rssi_event.rssi > rssi_min)
			{
				esp_peer_t *peer = esp_connection_mac_add_to_entry(&esp_connection_handle, rssi_event.recv_mac);
				peer->rssi = rssi_event.rssi;
				if (peer->status == ESP_PEER_STATUS_CONNECTED)
					peer->lastseen_unicast_us = esp_timer_get_time();

				if (peer != NULL && peer->status == ESP_PEER_STATUS_IN_RANGE)
				{
					peer->lastseen_broadcast_us = esp_timer_get_time();
					esp_peer_set_status(peer, ESP_PEER_STATUS_AVAILABLE);

					/* Add unicast peer information to peer list. */
					if (!esp_now_is_peer_exist(peer->mac))
					{
						esp_now_peer_info_t peer_info = {
						    .channel = espnow_config.channel,
						    .encrypt = false,
						    .ifidx = espnow_config.esp_interface,
						};
						memcpy(peer_info.peer_addr, peer->mac, ESP_NOW_ETH_ALEN);
						ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));
					}

					// ESP_LOGI(TAG, "matching mac, %d", peer->status);
					esp_connection_show_entries(&esp_connection_handle);
					// print_mem(peer, sizeof(esp_peer_t));
				}

				countdown = 5;
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
		vTaskDelay(pdMS_TO_TICKS(100));
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

	the_one_connected_peer = NULL;
	espnow_wifi_default_config(&espnow_config);
	espnow_wifi_init(&espnow_config);
	espnow_default_send_param(&espnow_send_param);
	esp_connection_handle_init(&esp_connection_handle);
	QueueHandle_t espnow_event_queue = espnow_init(&espnow_config, &esp_connection_handle);

	if (espnow_send_text(&espnow_send_param, "device init") != ESP_OK)
	{
		ESP_LOGE(TAG, "Send error, quitting");
		espnow_deinit(&espnow_send_param);
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
			ESP_LOGI(TAG, "GPIO event: pin %d, state = %s --> %s", button_event.pin, BUTTON_STATE_STRING[button_event.prev_state], BUTTON_STATE_STRING[button_event.new_state]);

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
				ESP_LOGV(TAG, "Send data to " MACSTR ", status1: %4s", MAC2STR(send_cb->mac_addr), send_cb->status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
				break;
			case ESPNOW_RECV_CB:
				espnow_event_recv_cb_t *recv_cb = &espnow_evt.info.recv_cb;
				if (!(recv_data = espnow_data_parse(recv_data, recv_cb)))
				{
					free(recv_cb->data);
					break;
				}

				esp_peer_t *peer = esp_connection_mac_add_to_entry(&esp_connection_handle, recv_cb->mac_addr);
				if (peer->status < ESP_PEER_STATUS_IN_RANGE)
					esp_peer_set_status(peer, ESP_PEER_STATUS_IN_RANGE);
				espnow_get_send_param(&espnow_send_param, peer);

				if (recv_data->broadcast == ESPNOW_DATA_BROADCAST)
				{
					peer->lastseen_broadcast_us = esp_timer_get_time();
					if (recv_data->ack == ESPNOW_PARAM_ACK_ACK)
						asm("nop");
					// ESP_LOGI(TAG, "Packet id:[%4d] acknowleded by receiver", recv_data->seq_num);
					else
					{
						// espnow_reply(&espnow_send_param, recv_data);
						ESP_LOGV(TAG, "Receive %dth broadcast data from: " MACSTR ", len: %d",
							 recv_data->seq_num,
							 MAC2STR(recv_cb->mac_addr),
							 recv_cb->data_len);
						print_mem(recv_data->payload, recv_data->len);
					}
				}
				else if (recv_data->broadcast == ESPNOW_DATA_UNICAST)
				{
					peer->lastseen_unicast_us = esp_timer_get_time();
					if (peer->status == ESP_PEER_STATUS_CONNECTING)
					{
						the_one_connected_peer = peer;
						esp_peer_set_status(peer, ESP_PEER_STATUS_CONNECTED);
					}
					ESP_LOGV(TAG, "Receive %dth unicast data from: " MACSTR ", len: %d", recv_data->seq_num, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);
					print_mem(recv_data->payload, recv_data->len);
				}
				else
					ESP_LOGW(TAG, "Receive error data from: " MACSTR "", MAC2STR(recv_cb->mac_addr));
				free(recv_cb->data);
				break;
			default:
				ESP_LOGE(TAG, "Callback type error: %d", espnow_evt.id);
				break;
			}
		}
		esp_connection_handle_update(&esp_connection_handle);
		vTaskDelay(1);
	}
}