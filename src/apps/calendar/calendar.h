#pragma once
#include "../../app/app.hpp"
#include "../../framework/framwork.h"
#include <numeric>  // std::accumulate
#include <vector>


namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {

        class CalendarApp : public FreeRTOSAppBase
        {
        private:
            Framework *_framework = Framework::getInstance();
            FreeRTOSAppBase *_calendar_app;
            const char *TAG = "CalendarApp";
            HAL *update;

        public:
            CalendarApp() : _framework(Framework::getInstance()), _calendar_app(nullptr), update(nullptr) {}
            ~CalendarApp() = default;

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