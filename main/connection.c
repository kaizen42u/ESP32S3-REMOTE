
#include "connection.h"

static const char __attribute__((unused)) *TAG = "connection";

void esp_mac_handle_init(esp_mac_handle_t *handle)
{
        handle->size = 0;
        handle->entries = malloc(sizeof(esp_peer_t));
        if (handle->entries == NULL)
                ESP_LOG_ERROR("malloc failed");
}

void esp_mac_handle_clear(esp_mac_handle_t *handle)
{
        if ((handle == NULL) || (handle->entries == NULL))
        {
                ESP_LOG_ERROR("NULL pointer");
                return;
        }

        free(handle->entries);
}

bool esp_mac_check_equals(const uint8_t *mac1, const uint8_t *mac2)
{
        for (uint8_t i = 0; i < ESP_NOW_ETH_ALEN; i++)
                if (mac1[i] != mac2[i])
                        return false;
        return true;
}

void esp_peer_set_mac(esp_peer_t *peer, const uint8_t *mac)
{
        if (peer == NULL)
        {
                ESP_LOG_ERROR("NULL pointer");
                return;
        }
        memcpy(peer->mac, mac, ESP_NOW_ETH_ALEN);
}

esp_peer_t *esp_mac_lookup(esp_mac_handle_t *handle, const uint8_t *mac)
{
        if ((handle == NULL) || (handle->entries == NULL))
        {
                ESP_LOG_ERROR("NULL pointer");
                return NULL;
        }

        esp_peer_t *peer = NULL;
        for (size_t i = 0; i < handle->size; i++)
        {
                peer = handle->entries + i;
                if (esp_mac_check_equals(peer->mac, mac))
                        break;
        }

        return peer;
}

esp_peer_t *esp_mac_add_to_entry(esp_mac_handle_t *handle, const uint8_t *mac)
{
        if ((handle == NULL) || (handle->entries == NULL))
        {
                ESP_LOG_ERROR("NULL pointer");
                return NULL;
        }

        esp_peer_t *peer = esp_mac_lookup(handle, mac);
        if (peer != NULL)
        {
                ESP_LOGV(TAG, "peer mac " MACSTR " already logged", MAC2STR(mac));
                return peer;
        }

        size_t new_capacity = handle->size + 1;
        esp_peer_t *new_addr = realloc(handle->entries, sizeof(esp_peer_t) * new_capacity);

        if (new_addr == NULL)
        {
                ESP_LOG_ERROR("realloc falied");
                return NULL;
        }

        handle->entries = new_addr;

        esp_peer_t *new_peer = handle->entries + handle->size;
        esp_peer_set_mac(peer, mac);
        new_peer->status = ESP_PEER_STATUS_UNKNOWN;

        handle->size = new_capacity;
        ESP_LOGI(TAG, "added " MACSTR " to known node, total: %d", MAC2STR(mac), handle->size);
        // print_mem(new_peer, sizeof(esp_peer_t));
        return new_peer;
}

void esp_mac_show_entries(esp_mac_handle_t *handle)
{
        if ((handle == NULL) || (handle->entries == NULL))
        {
                ESP_LOG_ERROR("NULL pointer");
                return;
        }

        ESP_LOGI(TAG, "Listing available ESP-NOW nodes, %d total", handle->size);
        for (size_t i = 0; i < handle->size; i++)
        {
                esp_peer_t *peer = handle->entries + i;
                ESP_LOGI(TAG, "    id: %d, addr: " MACSTR ", status: %s", i, MAC2STR(peer->mac), ESP_PEER_STATUS_STRING[peer->status]);
        }
}
