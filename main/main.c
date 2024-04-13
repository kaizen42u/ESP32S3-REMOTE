
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
#include "packets.h"
#include "joystick.h"
#include "eeprom.h"
#include "device_settings.h"

static const char __attribute__((unused)) *TAG = "app_main";

static espnow_send_param_t espnow_send_param;
static esp_connection_handle_t esp_connection_handle;
static device_settings_t device_settings;

void motor_controller_print_stat(motor_group_stat_pkt_t *motor_stat)
{
	LOG_INFO("Lcnt:%6d, Rcnt:%6d | Lspd:%6.3f, Rspd:%6.3f | Lacc:%6.3f, Racc:%6.3f | Lpwm:%6.3f, Rpwm:%6.3f | Δd: %6.3f | Δs: %6.3f",
		 motor_stat->left_motor.counter, motor_stat->right_motor.counter,
		 motor_stat->left_motor.velocity, motor_stat->right_motor.velocity,
		 motor_stat->left_motor.acceleration, motor_stat->right_motor.acceleration,
		 motor_stat->left_motor.duty_cycle, motor_stat->right_motor.duty_cycle,
		 motor_stat->delta_distance,
		 motor_stat->delta_velocity);
}

void rssi_task()
{
	ws2812_hsv_t hsv = {.h = RGB_LED_HUE, .s = RGB_LED_SATURATION, .v = 0};
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

			const int rssi_min = MIN_RSSI_TO_INITIATE_CONNECTION;
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
				hsv.v = RGB_LED_VALUE * esp_connection_handle.remote_connected;
			else
				hsv.v = 0;
			ws2812_set_hsv(&ws2812_handle, &hsv);
			ws2812_update(&ws2812_handle);
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void ping_task()
{
        for (;;)
        {
                esp_connection_send_heartbeat(&esp_connection_handle);
                vTaskDelay(pdMS_TO_TICKS(300));
        }
}

void power_switch_task()
{
	// int32_t elapsed_time = 0;
	for (;;)
	{
		// (esp_connection_handle.remote_connected) ? elapsed_time = 0 : elapsed_time++;
		// (elapsed_time >= TIME_BEFORE_RESET) ? kill_power() : keep_power();
		if (SHOW_CONNECTION_STATUS)
			esp_connection_show_entries(&esp_connection_handle);
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

void app_main(void)
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		LOG_WARNING("Resetting EEPROM data!");
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	device_settings_init(&device_settings);

	espnow_config_t espnow_config;
	espnow_wifi_default_config(&espnow_config);
	espnow_wifi_init(&espnow_config);
	espnow_default_send_param(&espnow_send_param);
	esp_connection_handle_init(&esp_connection_handle);
	esp_connection_handle_connect_to_device_settings(&esp_connection_handle, &device_settings);
	esp_connection_set_peer_limit(&esp_connection_handle, 1);
	QueueHandle_t espnow_event_queue = espnow_init(&espnow_config, &esp_connection_handle);
	esp_connection_enable_broadcast(&esp_connection_handle);

	esp_connection_mac_add_to_entry(&esp_connection_handle, device_settings.remote_conn_mac);
	espnow_default_send_param(&espnow_send_param);

	ret = espnow_send_text(&espnow_send_param, "device init");
	if (ret != ESP_OK)
	{
		LOG_ERROR("ESP_NOW send error, quitting");
		espnow_deinit(&espnow_send_param);
		ESP_ERROR_CHECK(ret);
		vTaskDelete(NULL);
	}

	QueueHandle_t button_event_queue = button_init();
	button_register(JOYSTICK_SHIELD_BUTTON_A, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(JOYSTICK_SHIELD_BUTTON_B, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(JOYSTICK_SHIELD_BUTTON_C, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(JOYSTICK_SHIELD_BUTTON_D, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(JOYSTICK_SHIELD_BUTTON_E, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(JOYSTICK_SHIELD_BUTTON_F, BUTTON_CONFIG_ACTIVE_LOW);
	button_register(JOYSTICK_SHIELD_BUTTON_K, BUTTON_CONFIG_ACTIVE_LOW);

	QueueHandle_t joystick_event_queue = joystick_init();
	joystick_register(GPIO_BUTTON_UP, GPIO_BUTTON_DOWN, JOYSTICK_SHIELD_JOYSTICK_Y, false);
	joystick_register(GPIO_BUTTON_RIGHT, GPIO_BUTTON_LEFT, JOYSTICK_SHIELD_JOYSTICK_X, true);

	xTaskCreate(rssi_task, "rssi_task", 4096, NULL, 4, NULL);
        xTaskCreate(ping_task, "ping_task", 4096, NULL, 4, NULL);
	xTaskCreate(power_switch_task, "power_switch_task", 4096, NULL, 4, NULL);

	esp_connection_set_unique_peer_mac(&esp_connection_handle, device_settings.remote_conn_mac);

	while (true)
	{
		button_event_t button_event;

		while (xQueueReceive(joystick_event_queue, &button_event, 0))
		{
			LOG_INFO("GPIO event: pin %d, state = %s --> %s", button_event.pin, BUTTON_STATE_STRING[button_event.prev_state], BUTTON_STATE_STRING[button_event.new_state]);

			esp_err_t ret;
			// LOG_WARNING("sending to peer " MACSTR, MAC2STR(espnow_send_param.dest_mac));
			ret = espnow_send_data(&espnow_send_param, ESP_PEER_PACKET_TEXT, &button_event, sizeof(button_event));
			ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
		}

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

				// LOG_WARNING("peer " MACSTR " is %s", MAC2STR(recv_cb->mac_addr), recv_data->broadcast ? "BROADCAST": "UNICAST");

				if (recv_data->type == ESPNOW_PARAM_TYPE_MOTOR_STAT)
				{
					if (recv_data->len == sizeof(motor_group_stat_pkt_t))
					{
						static motor_group_stat_pkt_t motor_stat;
						memccpy(&motor_stat, recv_data->payload, sizeof(motor_group_stat_pkt_t), recv_data->len);
						motor_controller_print_stat(&motor_stat);
					}

					// print_mem(recv_data->payload, recv_data->len);
				}

				// if (recv_data->type != ESPNOW_PARAM_TYPE_ACK)
				// espnow_reply(&espnow_send_param);

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