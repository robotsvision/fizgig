#include "main.h"

static httpd_handle_t server = NULL;

// Функция инициализации GPIO
static void gpio_init(void) {
    gpio_config_t conf;
    conf.intr_type = GPIO_INTR_DISABLE; // Запрещаем прерывания
    conf.mode = GPIO_MODE_OUTPUT; // Устанавливаем режим вывода
    conf.pin_bit_mask = (1ULL << GPIO_OUTPUT_PIN); // Устанавливаем маску для пина
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // Отключаем подтяжку к земле
    conf.pull_up_en = GPIO_PULLUP_DISABLE; // Отключаем подтяжку к питанию
    gpio_config(&conf); // Применяем конфигурацию
    gpio_set_level(GPIO_OUTPUT_PIN, 0); // Устанавливаем начальное состояние 1
}

// Функция-обработчик для URI /
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_send(req, HTML_CONTENT, strlen(HTML_CONTENT));
    return ESP_OK;
}

// Функция-обработчик для URI /stream
static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;

    char part_buf[64];

    static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=frame";
    static const char* _STREAM_BOUNDARY = "--frame";
    static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        } else {
            if (fb->format != PIXFORMAT_JPEG) {
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                if (!jpeg_converted) {
                    ESP_LOGE(TAG, "JPEG compression failed");
                    esp_camera_fb_return(fb);
                    res = ESP_FAIL;
                }
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }

        if (res == ESP_OK) {
            size_t hlen = snprintf(part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, part_buf, hlen);
        }

        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }

        if (fb) {
            esp_camera_fb_return(fb);
            _jpg_buf = NULL;
        } else if (_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }

        if (res != ESP_OK) {
            break;
        }

        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        if (res != ESP_OK) {
            break;
        }
    }

    return res;
}

// Новый обработчик для URI /activate
static esp_err_t activate_handler(httpd_req_t *req)
{
    gpio_set_level(GPIO_OUTPUT_PIN, 1); // Переключаем состояние
    ESP_LOGI(TAG, "GPIO state changed to %d", 1); // Логируем изменение состояния
    httpd_resp_send(req, "GPIO state changed", HTTPD_RESP_USE_STRLEN);
    vTaskDelay(1000);
    gpio_set_level(GPIO_OUTPUT_PIN, 0);
    return ESP_OK;
}

// Регистрация URI и запуск веб-сервера
static esp_err_t start_camera_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    static httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    };

    static httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    static httpd_uri_t activate_uri = {
        .uri       = "/activate",
        .method    = HTTP_GET,
        .handler   = activate_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &stream_uri);
        httpd_register_uri_handler(server, &activate_uri); // Регистрация нового URI
    }

    return ESP_OK;
}

// Инициализация точки доступа WiFi
void wifi_init_softap()
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = 1,
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
}

// Инициализация камеры
static esp_err_t init_camera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sccb_sda = 26;
    config.pin_sccb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 10000000;
    config.pixel_format = CAMERA_PIXEL_FORMAT;

    if (config.pixel_format == PIXFORMAT_JPEG) {
        config.frame_size = CAMERA_FRAME_SIZE;
        config.jpeg_quality = 12;
        config.fb_count = CAMERA_FB_COUNT;
    } else {
        config.frame_size = CAMERA_FRAME_SIZE;
        config.fb_count = CAMERA_FB_COUNT;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return err;
    }

    return ESP_OK;
}

// Основная функция
void app_main()
{
    // Инициализация NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Инициализация WiFi в режиме AP
    wifi_init_softap();

    // Инициализация камеры
    ESP_ERROR_CHECK(init_camera());

    // Инициализация GPIO
    gpio_init();

    // Запуск веб-сервера
    server = start_camera_server();

    ESP_LOGI(TAG, "Camera Stream Server Started");
}