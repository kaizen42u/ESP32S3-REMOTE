
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

#include "logging.h"

typedef enum
{
        ESP_PEER_STATUS_UNKNOWN,
        ESP_PEER_STATUS_AVAILABLE,
        ESP_PEER_STATUS_CONNECTED,
        ESP_PEER_STATUS_PROTOCOL_ERROR,
        ESP_PEER_STATUS_NOREPLY,
        ESP_PEER_STATUS_MAX,
} esp_peer_status_t;

static const char __attribute__((unused)) * ESP_PEER_STATUS_STRING[] = {
    "ESP_PEER_STATUS_UNKNOWN",
    "ESP_PEER_STATUS_AVAILABLE",
    "ESP_PEER_STATUS_CONNECTED",
    "ESP_PEER_STATUS_PROTOCOL_ERROR",
    "ESP_PEER_STATUS_NOREPLY",
    "ESP_PEER_STATUS_MAX"};

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