#include "psram_checker.h"
#include "esp_psram.h"

size_t xiao_get_psram_size() { return esp_psram_get_size(); }