
#include "esp_log.h"

#define LOG_ERROR(msg) ESP_LOGE(TAG, "ERROR: %s | on %s:%d", msg, __FILE__, __LINE__);
#define LOG_WARNING(msg) ESP_LOGE(TAG, "WARNING: %s | on %s:%d", msg, __FILE__, __LINE__);
