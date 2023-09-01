#ifndef COMPONENT_BASE_H
#define COMPONENT_BASE_H

#include <vector>
#include <memory>
#include <typeinfo>
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

#include "bsp/esp-bsp.h"

#define INIT_STACK 4096
#define DISPOSE_STACK 4096

class AppComponentBase;
typedef std::shared_ptr<AppComponentBase> AppComponentReference;

class AppComponentBase {
    public:
        class TaskParameters {
            public:
                AppComponentReference component_reference;
                esp_err_t error;
                void* parameters;
                SemaphoreHandle_t* started_semaphore;
                SemaphoreHandle_t* finished_semaphore;
        };

        // dispose
        TaskHandle_t dispose_task(SemaphoreHandle_t* finished_semaphore);
        esp_err_t dispose(void);
        // init
        TaskHandle_t init_task(std::vector<AppComponentReference>* parameters, SemaphoreHandle_t* finished_semaphore);
        esp_err_t init(std::vector<AppComponentReference>* parameters);

        // search for component of type
        template<typename T> std::shared_ptr<T> component()
        {
            for (AppComponentReference compRef : *components)
            {
                std::shared_ptr<T> found = std::dynamic_pointer_cast<T>(compRef);
                if (found != nullptr)
                    return found;
            }
            return nullptr;
        }

        // tag for logs and for checking uniquenes
        virtual const char* log_tag() = 0;

    protected:
        // pointer to components vector from the main app object
        std::vector<AppComponentReference>* components;

        // virtual methods for subclasses
        virtual esp_err_t init_self(void) = 0;
        virtual esp_err_t dispose_self(void) = 0;

    private:
        // rtos wrapper
        static void init_task_wrapper(void* parameters);
        static void dispose_task_wrapper(void* parameters);
        // to not do init twice
        bool is_initialised = false;
};

#endif // COMPONENT_BASE_H