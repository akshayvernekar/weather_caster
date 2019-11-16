/*
 * openweathermap.c
 *
 *  Created on: Nov 15, 2019
 *      Author: akki
 */
#include "openweathermap.h"
#include <string.h>
#include <esp_system.h>
#include <esp_http_client.h>
#include <string.h>
#include "esp_log.h"
#include "cJSON.h"

static char* OWM_API_KEY = NULL;

// For more information about the API refer
// https://openweathermap.org/current

static const char* OWM_BASE_URL="https://api.openweathermap.org/data/2.5/weather?";
static const char* TAG = "weather_api";
static char* _payload=NULL;
static int RETRY_COUNT = 3;


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
        	ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
        	ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
        	ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        	printf("%.*s", evt->data_len, (char*)evt->data);
        	if(_payload != NULL)
        	{
        		free(_payload);
        		_payload = NULL;
        	}
        	_payload = malloc(sizeof(char)*(evt->data_len));
        	memcpy(_payload,evt->data,evt->data_len);

            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                 printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
        	ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
        	ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

static esp_err_t http_rest(char* url)
{
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
		.buffer_size = 1024
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
        ESP_LOGI(TAG,"Got this:%s",_payload);
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

void set_openweather_api_key(const char* api_key)
{
	if(api_key != NULL)
	{
		if(OWM_API_KEY != NULL)
		{
			free(OWM_API_KEY);
			OWM_API_KEY = NULL;
		}

		OWM_API_KEY = malloc(sizeof(char)*strlen(api_key)+1);
		if(OWM_API_KEY != NULL)
		{
			strcpy(OWM_API_KEY,api_key);
		}
	}
}

cJSON* fetch_weather_by_city_name(char* city)
{
	char url[512]={0};
	sprintf(url,"%sq=%s&appid=%s",OWM_BASE_URL,city,OWM_API_KEY);
	//ESP_LOGI(TAG,"url=%s",url);

	cJSON* response_json=NULL;
	esp_err_t err;
	int retryCount = 0;
	do
	{
		err = http_rest(url);
		retryCount++;
	}
	while((err != ESP_OK) && (retryCount <= RETRY_COUNT));

	if(err == ESP_OK)
	{
		response_json = cJSON_Parse((char *)_payload);
	}
	return response_json;
}

cJSON* fetch_weather_by_city_id(int city_id)
{
	char url[512]={0};
	sprintf(url,"%sid=%d&appid=%s",OWM_BASE_URL,city_id,OWM_API_KEY);
	cJSON* response_json=NULL;
	esp_err_t err;
	int retryCount = 0;
	do
	{
		err = http_rest(url);
		retryCount++;
	}
	while((err != ESP_OK) && (retryCount <= RETRY_COUNT));

	if(err == ESP_OK)
	{
		response_json = cJSON_Parse((char *)_payload);
	}
	return response_json;
}

cJSON* fetch_weather_by_lat_long(float lat, float lon)
{
	char url[512]={0};
	sprintf(url,"%slat=%f&lon=%f&appid=%s",OWM_BASE_URL,lat,lon,OWM_API_KEY);
	cJSON* response_json=NULL;
	esp_err_t err;
	int retryCount = 0;
	do
	{
		err = http_rest(url);
		retryCount++;
	}
	while((err != ESP_OK) && (retryCount <= RETRY_COUNT));

	if(err == ESP_OK)
	{
		response_json = cJSON_Parse((char *)_payload);
	}
	return response_json;
}

cJSON* fetch_weather_by_city_zip(int zip, char* country)
{
	char url[512]={0};
	sprintf(url,"%szip=%d,%s&appid=%s",OWM_BASE_URL,zip,country,OWM_API_KEY);
	cJSON* response_json=NULL;
	esp_err_t err;
	int retryCount = 0;
	do
	{
		err = http_rest(url);
		retryCount++;
	}
	while((err != ESP_OK) && (retryCount <= RETRY_COUNT));

	if(err == ESP_OK)
	{
		response_json = cJSON_Parse((char *)_payload);
	}
	return response_json;
}

