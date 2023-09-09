#include "app_component_base.h"
#include <vector>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_check.h"
#include "esp_log.h"


// dispose
TaskHandle_t AppComponentBase::dispose_task(SemaphoreHandle_t* finished_semaphore)
{
    std::string task_name = "dispose_";
    task_name += this->log_tag();

    SemaphoreHandle_t task_started_semaphore = xSemaphoreCreateCounting(1, 0);

    std::shared_ptr<AppComponentBase::TaskParameters> task_parameters = std::make_shared<AppComponentBase::TaskParameters>();
    task_parameters->component_reference = AppComponentReference(this);
    task_parameters->parameters = nullptr;
    task_parameters->error = ESP_OK;
    task_parameters->started_semaphore = &task_started_semaphore;
    task_parameters->finished_semaphore = finished_semaphore;

    ESP_LOGI(this->log_tag(), "dispose task begin");

    TaskHandle_t handle;
    xTaskCreate(
        AppComponentBase::dispose_task_wrapper,
        task_name.c_str(),
        INIT_STACK,
        &task_parameters,
        tskIDLE_PRIORITY + 1,
        &handle
    );

    while (xSemaphoreTake(task_started_semaphore, portMAX_DELAY) == pdFALSE);
    return handle;
}
void AppComponentBase::dispose_task_wrapper(void* parameters)
{
    std::shared_ptr<AppComponentBase::TaskParameters>* task_parameters_ptr = static_cast<std::shared_ptr<AppComponentBase::TaskParameters>*>(parameters);
    std::shared_ptr<AppComponentBase::TaskParameters> task_parameters = *task_parameters_ptr;
    xSemaphoreGive(*(task_parameters->started_semaphore));

    task_parameters->error = task_parameters->component_reference->dispose();

    xSemaphoreGive(*(task_parameters->finished_semaphore));
    vTaskDelete(NULL);
}
esp_err_t AppComponentBase::dispose(void)
{
    if (!this->is_initialised)
        return ESP_OK;

    this->dispose_self();
    this->is_initialised = false;

    return ESP_OK;
}

// init
TaskHandle_t AppComponentBase::init_task(std::vector<AppComponentReference>* components, SemaphoreHandle_t* finished_semaphore)
{
    std::string task_name = "init_";
    task_name += this->log_tag();

    SemaphoreHandle_t task_started_semaphore = xSemaphoreCreateCounting(1, 0);

    std::shared_ptr<AppComponentBase::TaskParameters> task_parameters = std::make_shared<AppComponentBase::TaskParameters>();
    task_parameters->component_reference = AppComponentReference(this);
    task_parameters->parameters = components;
    task_parameters->error = ESP_OK;
    task_parameters->started_semaphore = &task_started_semaphore;
    task_parameters->finished_semaphore = finished_semaphore;

    ESP_LOGI(this->log_tag(), "init task starting");

    TaskHandle_t handle;
    xTaskCreate(
        AppComponentBase::init_task_wrapper,
        task_name.c_str(),
        INIT_STACK,
        &task_parameters,
        tskIDLE_PRIORITY + 1,
        &handle
    );

    while (xSemaphoreTake(task_started_semaphore, portMAX_DELAY) == pdFALSE);
    return handle;
}
void AppComponentBase::init_task_wrapper(void* parameters)
{
    std::shared_ptr<AppComponentBase::TaskParameters>* task_parameters_ptr = static_cast<std::shared_ptr<AppComponentBase::TaskParameters>*>(parameters);
    std::shared_ptr<AppComponentBase::TaskParameters> task_parameters = *task_parameters_ptr;
    xSemaphoreGive(*(task_parameters->started_semaphore));

    task_parameters->error = task_parameters->component_reference->init(static_cast<std::vector<AppComponentReference>*>(task_parameters->parameters));

    xSemaphoreGive(*(task_parameters->finished_semaphore));
    xSemaphoreGive(task_parameters->component_reference->init_semaphore);
    vTaskDelete(NULL);
}
esp_err_t AppComponentBase::init(std::vector<AppComponentReference>* components)
{
    if (this->is_initialised)
    {
        ESP_LOGI(this->log_tag(), "already initialised");
        return ESP_OK;
    }

    ESP_LOGI(this->log_tag(), "init begin");

    this->components = components;
    ESP_RETURN_ON_ERROR(this->init_self(), this->log_tag(), "init failed");
    this->is_initialised = true;

    ESP_LOGI(this->log_tag(), "init finished successfully");

    return ESP_OK;
}