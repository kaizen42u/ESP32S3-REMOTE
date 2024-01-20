
#include "connection.h"

static const char __attribute__((unused)) *TAG = "connection";

void esp_mac_handle_init(esp_mac_handle_t *handle)
{
        handle->size = 0;
        handle->entries = malloc(sizeof(esp_peer_t));
        ESP_MAP_CHECK_NULL_HANDLE(handle->entries);
}

void esp_mac_handle_clear(esp_mac_handle_t *handle)
{
        ESP_MAP_CHECK_NULL_HANDLE(handle->entries);
        free(handle->entries);
}

bool esp_mac_check_equals(const uint8_t *mac1, const uint8_t *mac2)
{
        for (uint8_t i = 0; i < ESP_NOW_ETH_ALEN; i++)
                if (mac1[i] != mac2[i])
                        return false;
        return true;
}

esp_peer_t *esp_mac_lookup(esp_mac_handle_t *handle, const uint8_t *mac)
{
        ESP_MAP_CHECK_NULL_HANDLE_RV(handle);
        for (size_t i = 0; i < handle->size; i++)
        {
                esp_peer_t *peer = handle->entries + i;
                if (esp_mac_check_equals(peer->mac, mac))
                        return peer;
        }
        return NULL;
}

esp_peer_t *esp_mac_add_to_entry(esp_mac_handle_t *handle, const uint8_t *mac)
{
        esp_peer_t *peer = esp_mac_lookup(handle, mac);
        if (peer != NULL)
        {
                ESP_LOGV(TAG, "peer mac " MACSTR " already logged", MAC2STR(mac));
                // print_mem(peer, sizeof(esp_peer_t));
                return peer;
        }

        size_t new_capacity = handle->size + 1;
        esp_peer_t *new_addr = realloc(handle->entries, sizeof(esp_peer_t) * new_capacity);
        ESP_MAP_CHECK_NULL_HANDLE_MSG_RV(new_addr, "realloc failed, cannot resize MAC mac pool");
        handle->entries = new_addr;
        esp_peer_t *new_peer = handle->entries + handle->size;
        memcpy(new_peer->mac, mac, ESP_NOW_ETH_ALEN);
        new_peer->status = ESP_PEER_STATUS_UNKNOWN;
        handle->size = new_capacity;
        ESP_LOGI(TAG, "added " MACSTR " to known node, total: %d", MAC2STR(mac), handle->size);
        // print_mem(new_peer, sizeof(esp_peer_t));
        return new_peer;
}

void esp_mac_show_entries(esp_mac_handle_t *handle)
{
        for (size_t i = 0; i < handle->size; i++)
        {
                esp_peer_t *peer = handle->entries + i;
                ESP_LOGW(TAG, "[%d] addr: " MACSTR " status: %d", i, MAC2STR(peer->mac), peer->status);
        }
}
