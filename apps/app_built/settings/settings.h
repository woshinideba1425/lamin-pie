#pragma once
#include "../../app/app.hpp"
#include "../../framework/framwork.h"

namespace LAMINATEPIE {
    namespace BUILTIN_APP {
        namespace SETTINGS {

            struct Data_t {

                lv_obj_t* screen = nullptr;
                lv_obj_t* roller_hour = nullptr;
                lv_obj_t* roller_min = nullptr;
                lv_obj_t* roller_month = nullptr;
                lv_obj_t* roller_mday = nullptr;
                lv_obj_t* button_set = nullptr;

                bool* key_home_ptr = nullptr;
                
            };

        }

        class Settings : public FreeRTOSAppBase {
            private:
                Framework* _framework;
                FreeRTOSAppBase* _launch_app;
                SETTINGS::Data_t _data;
                const char* TAG = "Settings";

                static void _lvgl_event_cb(lv_event_t* e);
                static void _button_set_event_cb(lv_event_t* e);
                
                void _set_time();

            public:
                Settings() : _framework(Framework::getInstance()), _launch_app(nullptr) {}
                ~Settings() = default;

                /**
                 * @brief Lifecycle callbacks for derived to override
                 * 
                 */
                /* Setup App configs, called when App "install()" */
                void onSetup();

                /* Life cycle */
                void onCreate();
                void onResume();
                void taskLoop();
                void onRunningBG();
                void onPause();
                void onDestroy();
            
        };
    }
}