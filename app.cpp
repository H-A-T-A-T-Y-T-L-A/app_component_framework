#include "app_component_base.h"
#include "app.h"
#include <vector>
#include <memory>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

// component list management
void App::add_component(AppComponentReference component, bool allow_multiple)
{
    if (!allow_multiple && this->is_unique(component))
        return;

    appComponents.push_back(component);
}
void App::remove_component(AppComponentReference component)
{
    std::vector<AppComponentReference>::iterator remove_at = appComponents.begin();

    while (remove_at != appComponents.end() && *remove_at != component)
        ++remove_at;

    if (remove_at == appComponents.end())
        return;

    appComponents.erase(remove_at);
}
bool App::is_unique(AppComponentReference component)
{
    if (component->log_tag() == nullptr)
        return false;
    
    for (AppComponentReference compRef : this->appComponents)
        if (component->log_tag() == compRef->log_tag())
            return false;

    return true;
}

// overrides
const char* App::log_tag()
{
    return "app";
}
esp_err_t App::init_self(void)
{
    this->components = &(this->appComponents);
    
    std::vector<TaskHandle_t> task_handles;
    SemaphoreHandle_t task_counting_semaphore = xSemaphoreCreateCounting(this->appComponents.size(), 0);
    if (task_counting_semaphore == nullptr)
        return ESP_FAIL;

    for (AppComponentReference component : this->appComponents)
    {
        TaskHandle_t taskHandle = component->init_task(this->components, &task_counting_semaphore);
        if (taskHandle == nullptr)
            return ESP_FAIL;

        task_handles.push_back(taskHandle);
    }

    for (AppComponentReference component : this->appComponents)
        if (xSemaphoreTake(task_counting_semaphore, portMAX_DELAY) == pdFALSE)
            return ESP_FAIL;

    return ESP_OK;
}
esp_err_t App::dispose_self(void)
{
    std::vector<TaskHandle_t> task_handles;
    SemaphoreHandle_t task_counting_semaphore = xSemaphoreCreateCounting(this->appComponents.size(), 0);
    if (task_counting_semaphore == nullptr)
        return ESP_FAIL;

    for (AppComponentReference component : this->appComponents)
    {
        TaskHandle_t taskHandle = component->dispose_task(&task_counting_semaphore);
        if (taskHandle == nullptr)
            return ESP_FAIL;

        task_handles.push_back(taskHandle);
    }

    for (AppComponentReference component : this->appComponents)
        if (xSemaphoreTake(task_counting_semaphore, portMAX_DELAY) == pdFALSE)
            return ESP_FAIL;

    return ESP_OK;
}

