#pragma once
#include "../../app/app.hpp"
#include "../../framework/framwork.h"
#include <numeric>  // std::accumulate
#include <vector>
#define byte uint8_t
#define millis() (esp_timer_get_time() / 1000)

namespace LAMINATEPIE
{   
    
    namespace BUILTIN_APP
    {   
        struct rgb_t {
            byte red;
            byte green; 
            byte blue;
        };

        struct MLX90640_cfg
        {
            bool measure = true;
            float centerTemp;
            unsigned long tempTime = millis();
            unsigned long tempTime2 = 0;
            // start with some initial colors
            float minTemp = 20.0;
            float maxTemp = 40.0;
        };

        struct TemperatureParams {
            float intPoint;
            float val;
            float a;
            float b;
            float c;
            float d;
            float ii;
        };

        struct Indices {
            int x;
            int y;
            int i;
            int j;
        };

        struct InterpolateParams {
            int row;
            float temp;
            float temp2;
            float **interpolated = NULL;
        };

        struct FpsMontor{
            int millis;
            int fps_cnt;
            float fps;
        };

        class InfraredApp : public FreeRTOSAppBase
        {
        private:
            Framework *_framework = Framework::getInstance();
            FreeRTOSAppBase *_infrared_app;
            const char *TAG = "InfraredApp";
            HAL *update;
            MLX90640::MLX90640 *m640;

            lv_obj_t * canvas;
            lv_color_t *canvasBuf1;
            lv_color_t *canvasBuf2;

            bool isexdevcie = false;
            rgb_t rgb;
            MLX90640_cfg mlx640_cfg;
            TemperatureParams tmpa;
            float minTemp = 20.0;
            float maxTemp = 40.0;
            float *tempValues;
            MLX90640::paramsMLX90640 mlx90640;
            InterpolateParams ipp;
            Indices ids;
            FpsMontor fps;

            EventGroupHandle_t syncEventGroup;
            lv_color_t *currentBuf;
            lv_color_t *nextBuf;
            
        public:
            InfraredApp() : _framework(Framework::getInstance()), _infrared_app(nullptr), update(nullptr) {
                syncEventGroup = xEventGroupCreate();
            }
            ~InfraredApp() = default;

            /*App task, called when app running*/

            /**
             * @brief Lifecycle callbacks for derived to override
             *
             */
            /* Setup App configs, called when App "install()" */
            void onSetup();
            static void mbox_event_handler(lv_event_t *e);
            bool check_ex_device();

            void readTempValues();
            void interpolate();
            void drawPicture();
            void drawLegend();
            void setAbcd();
            lv_color_t getColor(float val);
            float constrain(float x, float a, float b);
            void setTempScale();
            /* Life cycle */
            void onCreate();
            void onResume();
            void taskLoop();
            void onRunningBG();
            void onPause();
            void onDestroy();
            static void update_ui(lv_event_t *e);
            
        };



    }
}