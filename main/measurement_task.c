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
 * @brief Measurement task implementation.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <esp_log.h>

#include "measurement_task.h"
#include "measurement.h"

#define TAG "measurement_task"

static measurement_config_t measurement_task_current_config;
static measurement_task_cb_t measurement_task_callback = NULL;
static void* measurement_task_context = NULL;
static TaskHandle_t current_task = NULL;

/**
 * Get current UTC time in ms (synchronized by SNTP)
 */
static uint64_t get_utc_now()
{
    struct timeval tv;
    uint64_t millisecondsSinceEpoch = 0;
    gettimeofday(&tv, NULL);
    millisecondsSinceEpoch = (uint64_t) (tv.tv_sec) * 1000 + (uint64_t) (tv.tv_usec) / 1000;
    return millisecondsSinceEpoch;
}

/**
 * Calculate time difference to next cycle from interval and offset
 */
static uint64_t get_next_cycle_start(uint64_t current_utc)
{
	uint64_t offset = measurement_task_current_config.utc_offset_ms;
	uint64_t interval = measurement_task_current_config.interval_ms;
	uint64_t difference = (current_utc + interval - offset) % interval;
	uint64_t next_cycle = interval - difference;
	// Avoid elapsing twice in the same cycle because of time shift
	if (next_cycle < interval / 10)
	{
		next_cycle += interval;
	}
	return next_cycle;
}

static void measurement_task_measure()
{
	float temperature = 0;
	float humidity = 0;
	uint64_t utc_now = get_utc_now();
	esp_err_t result = measurement_read(&temperature, &humidity);
	if (result == ESP_OK)
	{
		measurement_values_t values;
		values.humidity = humidity;
		values.temperature = temperature;
		values.utc_timestamp = utc_now;

		measurement_task_callback(&values, measurement_task_context);
	}
	else
	{
		const char* error_string = esp_err_to_name(result);
		ESP_LOGW(TAG, "Measurement read failed: %s", error_string);
	}
}

/**
 * Wait for next measurements
 */
static void wait_for_next_cycle(void)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	uint64_t utc_now = get_utc_now();
	uint64_t next_cycle = get_next_cycle_start(utc_now);
	ESP_LOGI(TAG, "Current time: %llu, next cycle: %llu", utc_now, utc_now + next_cycle);
	// Check time iteratively since chip clock source and real time can be shifted after long intervals
	vTaskDelayUntil(&xLastWakeTime, next_cycle / portTICK_PERIOD_MS);
}

static void measurement_task_run(void* pvParameters)
{
	for (;;)
	{
		wait_for_next_cycle();
		ESP_LOGI(TAG, "Taking measurement sample");
		measurement_task_measure();
	}
}

esp_err_t measurement_task_init(measurement_config_t config)
{
	esp_err_t result = ESP_OK;
	result = measurement_init();
	if (result != ESP_OK)
	{
		return result;
	}
	if (config.interval_ms == 0)
	{
		return ESP_ERR_INVALID_ARG;
	}
	measurement_task_current_config = config;
	return result;
}

esp_err_t measurement_task_start(measurement_task_cb_t callback, void* context)
{
	if (current_task != NULL)
	{
		return ESP_FAIL;
	}
	measurement_task_callback = callback;
	measurement_task_context = context;
	xTaskCreate(measurement_task_run, "measurement_task_run", 4096,
			NULL, tskIDLE_PRIORITY, &current_task);
	return ESP_OK;
}

void measurement_task_stop()
{
	if (current_task != NULL)
	{
		vTaskDelete(current_task);
	}
}

esp_err_t measurement_task_deinit()
{
	measurement_task_stop();
	return ESP_OK;
}
