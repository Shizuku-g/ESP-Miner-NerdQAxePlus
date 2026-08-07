#pragma once
#include "esp_http_server.h"

esp_err_t POST_V2_factory_serial(httpd_req_t *req);
esp_err_t GET_V2_factory_status(httpd_req_t *req);
