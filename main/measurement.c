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
 * @brief Implementation of functions for measuring temperature and humidity.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <inttypes.h>
#include <esp_log.h>
#include <esp_pm.h>

#include "algorithm.h"
#include "measurement.h"
#include "platform_measurement.h"
#include "config.h"

#define TAG "measurement"

#ifdef MEDIAN_SAMPLES
static int16_t measurements_temp[MEDIAN_SAMPLES];
static int16_t measurements_hum[MEDIAN_SAMPLES];
#endif

esp_err_t measurement_init()
{
	return platform_measurement_init();
}

static esp_err_t measurement_read_raw(int16_t* temeprature, int16_t* humidity)
{
	esp_err_t result = platform_measurement_read(temeprature, humidity);
	ESP_LOGI(TAG, ".measurement_read_raw(): temp: %d, hum: %d",
			*temeprature, *humidity);
	return result;
}

#ifdef MEDIAN_SAMPLES
static esp_err_t measurement_read_median(float* temperature, float* humidity)
{
	esp_err_t result = ESP_OK;
	for (int32_t i = 0; i < MEDIAN_SAMPLES; i++)
	{
		result = measurement_read_raw(&measurements_temp[i], &measurements_hum[i]);
		if (result != ESP_OK)
		{
			return result;
		}
		vTaskDelay(MEDIAN_SAMPLES_DELAY / portTICK_PERIOD_MS);
	}
	int16_t temp_raw = median(measurements_temp, MEDIAN_SAMPLES);
	int16_t hum_raw = median(measurements_hum, MEDIAN_SAMPLES);
	*temperature = ((float)temp_raw) / 10;
	*humidity = ((float)hum_raw) / 10;
	return result;
}
#endif

#ifndef MEDIAN_SAMPLES
static esp_err_t measurement_read_single(float* temperature, float* humidity)
{
	int16_t temp_raw;
	int16_t hum_raw;
	esp_err_t result = measurement_read_raw(&temp_raw, &hum_raw);
	if (result == ESP_OK)
	{
		*temperature = ((float)temp_raw) / 10;
		*humidity = ((float)hum_raw) / 10;
	}
	return result;
}
#endif

esp_err_t measurement_read(float* temperature, float* humidity)
{
	esp_pm_lock_handle_t lock_handle;
	// Disable power management while reading measurements
	esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, "measurement", &lock_handle);
	esp_pm_lock_acquire(lock_handle);
#ifdef MEDIAN_SAMPLES
	return measurement_read_median(temperature, humidity);
#else
	return measurement_read_single(temperature, humidity);
#endif
	esp_pm_lock_release(lock_handle);
}
