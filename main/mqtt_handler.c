/*
 *  Copyright (c) 2018, The OpenThread Authors.
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

#include <stdbool.h>
#include <string.h>
#include <esp_log.h>
#include "mqtt_client.h"
#include "mqtt_handler.h"
#include "parson.h"
#include "config.h"

#define TAG "mqtt_handler"

static mqtt_handler_config_t mqtt_handler_config;
static esp_mqtt_client_handle_t mqtt_client = NULL;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    //esp_mqtt_client_handle_t client = event->client;
	// your_context_t *context = event->context;
	switch (event->event_id) {
	case MQTT_EVENT_CONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
		break;
	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
		break;
	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_ERROR:
		ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
		break;
	default:
		break;
	}
    return ESP_OK;
}

esp_err_t mqtt_handler_init(const mqtt_handler_config_t config)
{
	const esp_mqtt_client_config_t mqtt_cfg = {
		.host = config.host,
		.event_handle = mqtt_event_handler,
	};
	mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	if (mqtt_client == NULL)
	{
		return ESP_FAIL;
	}
	mqtt_handler_config = config;
	return ESP_OK;
}

esp_err_t mqtt_handler_start(void)
{
	return esp_mqtt_client_start(mqtt_client);
}

esp_err_t mqtt_handler_publish_values(const measurement_values_t* values)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	json_object_set_string(root_object, "id", DEVICE_ID);
	json_object_set_number(root_object, "temperature", values->temperature);
	json_object_set_number(root_object, "humidity", values->humidity);
	json_object_set_number(root_object, "utc", values->utc_timestamp);
	serialized_string = json_serialize_to_string_pretty(root_value);
	size_t length = strlen(serialized_string);
	esp_mqtt_client_publish(mqtt_client, mqtt_handler_config.topic, serialized_string, length, 1, false);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return ESP_OK;
}

void mqtt_handler_deinit(void)
{
	esp_mqtt_client_destroy(mqtt_client);
}

void mqtt_handler_stop(void)
{
	esp_mqtt_client_stop(mqtt_client);
}
