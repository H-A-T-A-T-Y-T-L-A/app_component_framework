#ifndef COMPONENT_BASE_H
#define COMPONENT_BASE_H

#include <vector>
#include <memory>
#include <typeinfo>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

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
        template<typename T> std::shared_ptr<T> component();
        template<typename T> esp_err_t await_init_of_component();

        // tag for logs and for checking uniquenes
        virtual const char* log_tag() = 0;

    protected:
        // pointer to components vector from the main app object
        std::vector<AppComponentReference>* components;
        
        // semaphore for other components to await the initialisation of this component
        SemaphoreHandle_t init_semaphore = xSemaphoreCreateCounting(1, 0);

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

template<typename T> esp_err_t AppComponentBase::await_init_of_component()
{
    AppComponentReference found_component = component<T>();
    if (found_component == nullptr)
        return ESP_ERR_NOT_FOUND;

    while (xSemaphoreTake(found_component->init_semaphore, portMAX_DELAY) == pdFALSE)
        vTaskDelay(10);

    xSemaphoreGive(found_component->init_semaphore);
    return ESP_OK;
}

template<typename T> std::shared_ptr<T> AppComponentBase::component()
{
    for (AppComponentReference compRef : *components)
    {
        std::shared_ptr<T> found = std::dynamic_pointer_cast<T>(compRef);
        if (found != nullptr)
            return found;
    }
    return nullptr;
}

#endif // COMPONENT_BASE_H