
#pragma once

#include <string.h>
#include <ctype.h>
#include "stdbool.h"

#include "esp_heap_caps.h"

#include "esp_err.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_now.h"
#include "mem_probe.h"

#define ESP_MAP_CHECK_NULL_HANDLE(handle) if(handle == NULL) {ESP_LOGE(TAG, "runtime nullptr error on %s:%d", __FILE__, __LINE__); return;}
#define ESP_MAP_CHECK_NULL_HANDLE_RV(handle) if(handle == NULL) {ESP_LOGE(TAG, "runtime nullptr error on %s:%d", __FILE__, __LINE__); return NULL;}
#define ESP_MAP_CHECK_NULL_HANDLE_MSG(handle, msg) if(handle == NULL) {ESP_LOGE(TAG, "runtime nullptr error on %s:%d | %s", __FILE__, __LINE__, msg); return;}
#define ESP_MAP_CHECK_NULL_HANDLE_MSG_RV(handle, msg) if(handle == NULL) {ESP_LOGE(TAG, "runtime nullptr error on %s:%d | %s", __FILE__, __LINE__, msg); return NULL;}

typedef enum
{
        ESP_PEER_STATUS_UNKNOWN,
        ESP_PEER_STATUS_AVAILABLE,
        ESP_PEER_STATUS_CONNECTED,
        ESP_PEER_STATUS_PROTOCOL_ERROR,
        ESP_PEER_STATUS_NOREPLY,
        ESP_PEER_STATUS_MAX,
} esp_peer_status_t;

typedef struct
{
        uint8_t mac[ESP_NOW_ETH_ALEN];
        int64_t timestamp_us;
        size_t rx;
        size_t tx;
        esp_peer_status_t status;
} esp_peer_t;

typedef struct
{
        esp_peer_t *entries;
        size_t size;
} esp_mac_handle_t;

void esp_mac_handle_init(esp_mac_handle_t *handle);
void esp_mac_handle_clear(esp_mac_handle_t *handle);

esp_peer_t *esp_mac_lookup(esp_mac_handle_t *handle, const uint8_t *mac);
esp_peer_t *esp_mac_add_to_entry(esp_mac_handle_t *handle, const uint8_t *mac);

void esp_mac_show_entries(esp_mac_handle_t *handle);