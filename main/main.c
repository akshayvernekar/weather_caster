#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "openweathermap.h"
#include "ws2812_control.h"

/*Enter your SSID and PASSWORD*/
#define EXAMPLE_ESP_WIFI_SSID      "YOUR_SSID"
#define EXAMPLE_ESP_WIFI_PASS      "YOUR_PASSWORD"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

/*How often do you want the weather to be fetched*/
#define REFRESH_PERIOD 	1800000

/*Enter the city ID. Find your city ID here :http://bulk.openweathermap.org/sample/ */
#define SHENZHEN_CITY_ID 1795563
#define BANGALORE_CITY_ID 1277333
#define NUM_LEDS 8

/* Color codes. The format is GRB */
#define GREEN   0xFF0000
#define RED  0x00FF00
#define BLUE 0x0000FF
#define SKY_BLUE 0xBF00FF
#define YELLOW 0xFFFF00
#define ORANGE 0xA5FF00
#define INDIGO 0x004B82
#define WHITE 0xFFFFFF

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        {
            if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
                esp_wifi_connect();
                xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                s_retry_num++;
                ESP_LOGI(TAG,"retry to connect to the AP");
            }
            ESP_LOGI(TAG,"connect to the AP fail\n");
            break;
        }
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init_sta()
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void set_led_color(int color)
{
	struct led_state new_state;
	for(int i = 0 ; i < NUM_LEDS; i++)
	{
	    new_state.leds[i] = color;
	 }

	ws2812_write_leds(new_state);
}

void update_weather()
{
    cJSON* response = fetch_weather_by_city_id(SHENZHEN_CITY_ID);
    int current_id = 0;
    if(response != NULL)
    {
	    cJSON * weather_array = cJSON_GetObjectItem(response,"weather");
	    cJSON * weather = cJSON_GetArrayItem(weather_array,0);
	    current_id = cJSON_GetObjectItem(weather,"id")->valueint;
	    cJSON_Delete(response);
    }

    ESP_LOGI(TAG,"Current climeate id = %d",current_id/100);
    switch(current_id/100)
    {
    case 2:
    	//Group 2xx: Thunderstorm = Purple
    	set_led_color(INDIGO);
    	break;
    case 3:
    case 5:
    	//Group 3xx: Drizzle & Group 5xx: Rain = blue
    	set_led_color(BLUE);
    	break;
    case 6:
    	//Group 6xx: Snow = white
    	set_led_color(WHITE);
    	break;
    case 7:
    	//Group 7xx: Atmosphere = light yellow
    	set_led_color(YELLOW);
    	break;
    case 8:
    	//Group 800: Clear = orange
    	//Group 80x: Clouds = indigo
    	if(current_id == 800)
        	set_led_color(ORANGE);
    	else
    		set_led_color(SKY_BLUE);
    	break;
    default:
    	//Lets set default value as clear
    	set_led_color(ORANGE);
    }
}

void app_main()
{
	EventBits_t uxBits;

	//Initialize openweather key and neopixel gpios
    set_openweather_api_key("bc4177261288e5a33a1011e923aeee10");
    ws2812_control_init();

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    uxBits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, true, false, portMAX_DELAY);
    if(uxBits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi Connected to ap");
    }

    // Lets update the weather every 5 mins
    while(1)
    {
        update_weather();
    	vTaskDelay(REFRESH_PERIOD / portTICK_RATE_MS);
    }
}


