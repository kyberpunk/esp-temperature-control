/*
 *  Copyright (c) 2019, Vit Holasek.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * @author Vit Holasek
 * @brief Main file with application start point.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <esp_sntp.h>

#include "measurement_task.h"
#include "mqtt_handler.h"
#include "config.h"

#define TAG "main"
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
const int SNTP_SYNCHRONIZED_BIT = BIT1;

static void event_handler(void* arg, esp_event_base_t event_base,
		int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if	(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "Retry to connect to the AP");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "Connect to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASSWORD);
}

static void nvs_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void time_sync_notification_cb(struct timeval *tv)
{
    uint64_t utcMs = ((uint64_t)tv->tv_sec) * 1000 + tv->tv_usec;
    ESP_LOGI(TAG, "UTC time synchronized: %llu", utcMs);
    xEventGroupSetBits(wifi_event_group, SNTP_SYNCHRONIZED_BIT);
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    //sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_init();
}

static void measures_init()
{
	measurement_config_t config;
	config.interval_ms = MEASUREMENT_INTERVAL;
	config.utc_offset_ms = MEASUREMENT_OFFSET;
	measurement_task_init(config);
}

/**
 * Publish measurement read by measurement task to MQTT broker.
 */
static void measurements_sampled_cb(const measurement_values_t* measurement_values, void* context)
{
	ESP_LOGI(TAG, "Measurements sampled: temperature=%f, humidity=%f, utc=%llu",
			measurement_values->temperature, measurement_values->humidity,
			measurement_values->utc_timestamp);
	esp_err_t result = mqtt_handler_publish_values(measurement_values);
	ESP_LOGI(TAG, "Measurement publish result: %d", result);
}

static void mqtt_init(void)
{
	mqtt_handler_config_t config;
	config.topic = MQTT_MEASUREMENT_TOPIC;
	config.host = GATEWAY_IP;
	mqtt_handler_init(config);
}

void app_main(void)
{
	wifi_event_group = xEventGroupCreate();
    nvs_init();
    tcpip_adapter_init();
    esp_event_loop_create_default();

    // Start Wi-Fi connection
    ESP_LOGI(TAG, "WiFi init");
    wifi_init();
    ESP_LOGI(TAG, "Connecting to WiFi...");
    // Wait Wi-Fi to become connected
	xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE,
			pdFALSE, portMAX_DELAY);
	ESP_LOGI(TAG, "Connected to WiFi");

	// Initialize SNTP time synchronization
    ESP_LOGI(TAG, "SNTP init");
    initialize_sntp();
    ESP_LOGI(TAG, "Waiting for SNTP synchronize...");
    // Wait for SNTP got response with current time
	xEventGroupWaitBits(wifi_event_group, SNTP_SYNCHRONIZED_BIT, pdFALSE,
			pdFALSE, portMAX_DELAY);
	ESP_LOGI(TAG, "SNTP synchronized");

	// Init and connect to MQTT
	ESP_LOGI(TAG, "Connecting to MQTT...");
	mqtt_init();
	mqtt_handler_start();

	ESP_LOGI(TAG, "Measurement started");
	measures_init();
	// Run measurements task
	measurement_task_start(measurements_sampled_cb, NULL);
}
