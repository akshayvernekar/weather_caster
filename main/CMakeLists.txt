# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES 
	json
	esp_http_client
	)
set(COMPONENT_PRIV_REQUIRES nvs_flash )

set(COMPONENT_SRCS "main.c openweathermap.c" "ws2812_control.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")

register_component()
