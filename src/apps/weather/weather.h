#pragma once
#include "../../app/app.hpp"
#include "../../framework/framwork.h"
#include <numeric>  // std::accumulate
#include <vector>


namespace LAMINATEPIE
{   
    
    namespace BUILTIN_APP
    {   
        struct SensorData_t {
            uint32_t* humidity = 0;
            int32_t* temperature = 0;
            uint32_t* pressure = 0;
            uint16_t* eCO2 = 0;
            uint16_t* tVoC = 0;
        };

        class WeatherApp : public FreeRTOSAppBase
        {
        private:
            Framework *_framework = Framework::getInstance();
            FreeRTOSAppBase *_weather_app;
            SensorData_t _data;
            const char *TAG = "WeatherApp";
            HAL *update;

        public:
            WeatherApp() : _framework(Framework::getInstance()), _weather_app(nullptr), update(nullptr) {}
            ~WeatherApp() = default;

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
            void update_ui();
        };
    }
}