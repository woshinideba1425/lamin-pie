#pragma once
#include "../../app/app.hpp"
#include <numeric>  // std::accumulate
#include <vector>
#include "BP_Manager.h"

namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {

        class BloodPressureApp : public FreeRTOSAppBase
        {
        private:
            Framework *_framework = Framework::getInstance();
            FreeRTOSAppBase *_bloodPressure_app;
            const char *TAG = "BloodPressureApp";
            //Runnable event;
            bool istest = false;
            HAL *update;
            BP_task bp_handle;
            JsonDocument userdata;
            bool first_check; 
            /* lvgl */
            static void _lvgl_bp_event_cb(lv_event_t* e);
            static void _lvgl_kb_event_cb(lv_event_t* e);

        public:
            BloodPressureApp() : _framework(Framework::getInstance()), _bloodPressure_app(nullptr), istest(false), update(nullptr) ,bp_handle(_framework),first_check(true){}
            ~BloodPressureApp() = default;

            /*App task, called when app running*/

            /**
             * @brief Lifecycle callbacks for derived to override
             *
             */
            /* Setup App configs, called when App "install()" */
            void onSetup();
            // void wattingCallback(lv_event_t *e);

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