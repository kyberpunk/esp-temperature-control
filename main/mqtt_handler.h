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
 * @brief This file defines necessary functions for publishing data to MQTT protocol.
 */

#ifndef MAIN_MQTT_HANDLER_H_
#define MAIN_MQTT_HANDLER_H_

#include <esp_err.h>
#include "measurement_task.h"

/**
 * Structure with MQTT configuration data
 */
typedef struct mqtt_handler_config
{
	/**
	 * Hostname or IP of MQTT broker
	 */
	char* host;
	/**
	 * Topic ID for publishing measurements
	 */
	char* topic;
} mqtt_handler_config_t;

/**
 * Initialize MQTT handler.
 * @param config  MQTT connection configuration
 */
esp_err_t mqtt_handler_init(mqtt_handler_config_t config);

/**
 * Connect to the broker and start MQTT communication.
 */
esp_err_t mqtt_handler_start(void);

/**
 * Publish measured values serialized into JSON to configured topic.
 * @param values  Measured values
 */
esp_err_t mqtt_handler_publish_values(const measurement_values_t* values);

/**
 * Deinitialize MQTT handler to free allocated resources
 */
void mqtt_handler_deinit(void);

/**
 * Stop MQTT communication and disconnect from the broker,
 */
void mqtt_handler_stop(void);

#endif /* MAIN_MQTT_HANDLER_H_ */
