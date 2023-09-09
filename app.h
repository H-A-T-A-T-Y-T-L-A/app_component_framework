#ifndef APP_H
#define APP_H

#include "app_component_base.h"
#include <memory>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class App : public AppComponentBase {
    public:
        App();
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