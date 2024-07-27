#ifndef MAIN_BIGB_H_
#define MAIN_BIGB_H_

#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_event.h"

#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG
#define CAMERA_FRAME_SIZE FRAMESIZE_QVGA
#define CAMERA_FB_COUNT 1

#define WIFI_SSID "fizgig.my (big bro)"
#define WIFI_PASS "1029384756"
#define MAX_STA_CONN 4

#define GPIO_OUTPUT_PIN 15 // GPIO для двигателя

static const char *TAG = "camera_web_server";

// HTML-контент для страницы
static const char* HTML_CONTENT = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>ESP32-CAM Stream</title>"
"<meta charset=\"utf-8\">"
"<style>body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px; } img { width: auto; max-width: 100%; height: auto; } .data { font-size: 24px; }</style>"
"</head>"
"<body>"
"<h1>ESP32-CAM Stream</h1>"
"<img src=\"/stream\">"
"</body>"
"</html>";

// Прототипы функций
static esp_err_t index_handler(httpd_req_t *req);
static esp_err_t stream_handler(httpd_req_t *req);
static esp_err_t activate_handler(httpd_req_t *req); // Новый прототип функции
static void wifi_init_softap(void);
static esp_err_t init_camera(void);
static esp_err_t start_camera_server(void);

static void gpio_init(void);

#endif // MAIN_BIGB_H_