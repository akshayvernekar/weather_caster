/*
 * openweathermap.h
 *
 *  Created on: Nov 15, 2019
 *      Author: akki
 */

#ifndef MAIN_INCLUDE_OPENWEATHERMAP_H_
#define MAIN_INCLUDE_OPENWEATHERMAP_H_
#include <string.h>
#include "cJSON.h"

void set_openweather_api_key(const char* api_key);

cJSON* fetch_weather_by_city_name(char* city);

cJSON* fetch_weather_by_city_id(int city_id);

cJSON* fetch_weather_by_lat_long(float lat, float lon);

cJSON* fetch_weather_by_city_zip(int zip, char* country);

#endif /* MAIN_INCLUDE_OPENWEATHERMAP_H_ */
