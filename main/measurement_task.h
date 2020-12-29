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
 * @brief This file contains functions for periodical measurements of temperature and humidity data.
 */

#ifndef MAIN_MEASUREMENT_TASK_H_
#define MAIN_MEASUREMENT_TASK_H_

#include <inttypes.h>
#include <driver/gpio.h>
#include <esp_err.h>

/**
 * Structure for configuring periodicity of measurements
 */
typedef struct measurement_config
{
	/**
	 * Measurements interval in ms
	 */
	uint32_t interval_ms;
	/**
	 * Offset in ms to utc time of measured samples. Counted as: utc_timestamp % utc_offset_ms
	 */
	uint32_t utc_offset_ms;
} measurement_config_t;

/**
 * Data structure for passing measured values
 */
typedef struct measurement_values
{
	/**
	 * Temperature in C.
	 */
	float temperature;
	/**
	 * Humidity in %
	 */
	float humidity;
	/**
	 * UTC timestamp in ms when the sample was taken
	 */
	uint64_t utc_timestamp;
} measurement_values_t;

/**
 * Call back for receiving measured values.
 */
typedef void (*measurement_task_cb_t)(const measurement_values_t* measurement_values, void* context);

/**
 * Initialize measurement task functionality.
 * @param config  Measurements periodicity configuration
 */
esp_err_t measurement_task_init(measurement_config_t config);

/**
 * Start periodical measurements.
 * @param callback  Callback for receiving measured values
 * @param context   Context pointer which will be passed to callback
 */
esp_err_t measurement_task_start(measurement_task_cb_t callback, void* context);

void measurement_task_stop(void);

/**
 * Stop periodical measurements.
 */
esp_err_t measurement_task_deinit(void);

#endif /* MAIN_MEASUREMENT_TASK_H_ */
