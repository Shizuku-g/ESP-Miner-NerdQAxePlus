#include "handler_v2_factory.h"

#include "esp_http_server.h"
#include "esp_log.h"

#include <cstring>

#include "ArduinoJson.h"
#include "psram_allocator.h"
#include "global_state.h"
#include "nvs_config.h"
#include "http_cors.h"
#include "http_utils.h"

static const char *TAG = "http_v2_factory";

static bool is_valid_serial(const char *serial)
{
    if (!serial) {
        return false;
    }
    const size_t len = strlen(serial);
    if (len < 12 || len > 32) {
        return false;
    }
    if (serial[0] != 'V' || serial[1] != 'M') {
        return false;
    }
    for (size_t i = 2; i < 6; i++) {
        if (serial[i] < '0' || serial[i] > '9') {
            return false;
        }
    }
    for (size_t i = 6; i < len; i++) {
        const char c = serial[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
            return false;
        }
    }
    return true;
}

static void uppercase_serial(char *serial, size_t cap)
{
    if (!serial) {
        return;
    }
    for (size_t i = 0; i < cap && serial[i]; i++) {
        if (serial[i] >= 'a' && serial[i] <= 'z') {
            serial[i] = static_cast<char>(serial[i] - 'a' + 'A');
        }
    }
}

esp_err_t GET_V2_factory_status(httpd_req_t *req)
{
    ConGuard g(http_server, req);

    if (is_network_allowed(req) != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Unauthorized");
    }

    httpd_resp_set_type(req, "application/json");
    if (set_cors_headers(req) != ESP_OK) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    char *serial = Config::getSerial();

    PSRAMAllocator allocator;
    JsonDocument doc(&allocator);
    doc["hasSerial"] = serial && strlen(serial) > 0;
    doc["serial"]    = serial ? serial : "";

    if (serial) {
        free(serial);
    }

    return sendJsonResponse(req, doc);
}

esp_err_t POST_V2_factory_serial(httpd_req_t *req)
{
    ConGuard g(http_server, req);

    if (is_network_allowed(req) != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Unauthorized");
    }

    if (set_cors_headers(req) != ESP_OK) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    PSRAMAllocator allocator;
    JsonDocument doc(&allocator);

    esp_err_t err = getJsonData(req, doc);
    if (err != ESP_OK) {
        return err;
    }

    if (!doc["serial"].is<const char*>()) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "serial required");
        return ESP_FAIL;
    }

    char serialBuf[33] = {0};
    strncpy(serialBuf, doc["serial"].as<const char*>(), sizeof(serialBuf) - 1);
    uppercase_serial(serialBuf, sizeof(serialBuf));

    if (!is_valid_serial(serialBuf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid serial format");
        return ESP_FAIL;
    }

    if (Config::hasSerial()) {
        httpd_resp_set_status(req, "409 Conflict");
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_send(req, "serial already set", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    Config::setSerial(serialBuf);
    Config::flush();

    ESP_LOGI(TAG, "factory serial written: %s", serialBuf);

    JsonDocument resp(&allocator);
    resp["serial"] = serialBuf;
    resp["success"] = true;

    return sendJsonResponse(req, resp);
}
