#ifndef APP_H
#define APP_H

#include <memory>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "mqtt_client.h"

#include "app_component_base.h"

#include "bsp/esp-bsp.h"

class App : public AppComponentBase {
    public:
        void add_component(AppComponentReference component, bool must_be_unique = true);
        void remove_component(AppComponentReference component);
        const char* log_tag() override;

    protected:
        esp_err_t init_self(void) override;
        esp_err_t dispose_self(void) override;

    private:
        std::vector<AppComponentReference> appComponents;
        bool is_unique(AppComponentReference component);
};

#endif // APP_H