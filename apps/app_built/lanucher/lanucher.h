#pragma once
#include "../../app/app.hpp"
#include "../../framework/framwork.h"




namespace LAMINATEPIE {
    namespace BUILTIN_APP {


        struct LauncherData_t {
            /* Display data */
            int16_t* dispHor = nullptr;
            int16_t* dispVer = nullptr;
            bool dispModePortrait = false;

            /* Widget */
            lv_obj_t* screenMain = nullptr;

            lv_obj_t* appPanel = nullptr;
            lv_coord_t appPanelHor = 0;
            lv_coord_t appPanelVer = 0;

            lv_obj_t* infoPanel = nullptr;
            lv_coord_t infoPanelHor = 0;
            lv_coord_t infoPanelVer = 0;
            lv_obj_t* infoClockHour = nullptr;
            lv_obj_t* infoClockMin = nullptr;
            lv_obj_t* infoStepCounter = nullptr;
            lv_obj_t* infoBatLevel = nullptr;
            lv_obj_t* infoBatIcon = nullptr;
            lv_obj_t* infoWifiIcon = nullptr;
            lv_obj_t* infoBleIcon = nullptr;
            lv_obj_t* infoNoteIcon = nullptr;
            lv_obj_t* lottie_animation = nullptr;

            uint32_t infoUpdateTickCount = 0;
            char infoUpdateBuffer[24];
        };


        struct BubbleConfig_t {
            lv_coord_t iconColMax = 0;
            lv_coord_t iconColNum = 0;
            lv_coord_t iconRowMax = 0;
            lv_coord_t iconSpaceX = 0;
            lv_coord_t iconSpaceY = 0;
            lv_coord_t iconXoffset = 0;
            lv_coord_t iconYoffset = 0;
        };

    
        class Launcher : public FreeRTOSAppBase {
            private:
                Framework* _framework = Framework::getInstance();
                FreeRTOSAppBase* _launch_app;
                const char* TAG = "Launcher";
                bool is_lottie_paused = false; 

                /* Lvgl */
                LauncherData_t _data;
                BubbleConfig_t _bubble_cfg;
                static void _lvgl_event_cb(lv_event_t* e);
                static void _lvgl_event_cb2(lv_event_t *e);
                void _create_app_panel();
                void _create_info_panel();
                void stopLottieAnimation();


            public:
                Launcher() : _framework(nullptr), _launch_app(nullptr) {}
                ~Launcher() = default;


                void updateAppIconZoom();
                void updateInfos();


                /**
                 * @brief Lifecycle callbacks for derived to override
                 * 
                 */
                /* Setup App configs, called when App "install()" */
                void onSetup();
                //void wattingCallback(lv_event_t* e);
                /* Life cycle */
            
                void onCreate();
                void onResume();
                void taskLoop();
                void onRunningBG();
                void onPause();
                void onDestroy();
            
        };


        struct LauncherBubbleData__t {
            int16_t* dispHor = nullptr;
            int16_t* dispVer = nullptr;
            bool dispModePortrait = false;

            lv_obj_t* screenMain = nullptr;
            lv_obj_t* screenCanvas = nullptr;
            lv_color_t* canvasBuffer = nullptr;
            lv_draw_img_dsc_t iconDsc;
        };



    }
}
