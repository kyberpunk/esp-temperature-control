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

#ifndef MAIN_CONFIG_H_
#define MAIN_CONFIG_H_

/**
 * Wi-Fi SSID
 */
#ifndef WIFI_SSID
#define WIFI_SSID "wifi"
#endif

/**
 * Wi-Fi password
 */
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "password"
#endif

/**
 * MQTT broker hostname or IP
 */
#ifndef GATEWAY_IP
#define GATEWAY_IP "example.com"
#endif

/**
 * Topic name for where measurements will be published
 */
#ifndef MQTT_MEASUREMENT_TOPIC
#define MQTT_MEASUREMENT_TOPIC "sensor/temp"
#endif

/**
 * Unique ID of this device in the system
 */
#ifndef DEVICE_ID
#define DEVICE_ID "SENSOR1"
#endif

/**
 * Measurement period in ms
 */
#ifndef MEASUREMENT_INTERVAL
#define MEASUREMENT_INTERVAL 60000
#endif

/**
 * Offset in ms to measurement period calculated as: sample_utc_ms % MEASUREMENT_INTERVAL
 */
#ifndef MEASUREMENT_OFFSET
#define MEASUREMENT_OFFSET 0
#endif

/**
 * Number of samples from which median value is chosen as relevant sample,
 * it can filter out measurement errors. If not defined only one samly will be read.
 */
//#ifndef MEDIAN_SAMPLES
//#define MEDIAN_SAMPLES 5
//#endif

/**
 * Delay in ms between samples used for median.
 */
//#ifndef MEDIAN_SAMPLES_DELAY
//#define MEDIAN_SAMPLES_DELAY 500
//#endif

#endif /* MAIN_CONFIG_H_ */
