#include "weather.h"

namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {
        void WeatherApp::onSetup()
        {
            setTaskName("SensorApp");
            setScreen(ui_sensor);
            setAllowBgRunning(false);
            setAppIcon((void *)&ICON_ENVIRONMENT);
        }

        void WeatherApp::onCreate()
        {
            _framework->enableLowPowerMode();
            update = &_framework->getHAL();
            printf("[%s] onCreate\n", getAppName().c_str());
        }

        void WeatherApp::onResume()
        {
            _framework->enableLowPowerMode();
            printf("[%s] onResume\n", getAppName().c_str());
        }

        void WeatherApp::taskLoop()
        {
            update_ui();
            vTaskDelay(100);
        }

        void WeatherApp::onRunningBG()
        {


        }
        void WeatherApp::onPause()
        {
            printf("[%s] onPause\n", getAppName().c_str());
            update->unselect(ENV_ID);
        }

        void WeatherApp::onDestroy()
        {
            printf("[%s] onDestroy\n", getAppName().c_str());
            update->unselect(ENV_ID);
        }

        void WeatherApp::update_ui()
        {
            update->select(ENV_ID);
            SIMPLEKV::SimpleKV_ESP* db = _framework->getDatabase();

            _data.humidity = (uint32_t*)db->Get(LA_HUMIDITY)->addr;
            _data.pressure = (uint32_t*)db->Get(LA_PRESSURE)->addr;
            _data.temperature = (int32_t*)db->Get(LA_TEMPERATURE)->addr;
            _data.eCO2 = (uint16_t*)db->Get(LA_CO2)->addr;
            _data.tVoC = (uint16_t*)db->Get(LA_TVOC)->addr;

            if (_data.humidity == nullptr) {
                lv_label_set_text(ui_La_dis_hum, "Humidity Error");
            } else {
                lv_label_set_text_fmt(ui_La_dis_hum, "%d %%", *_data.humidity);
            }

            if (_data.pressure == nullptr) {
                lv_label_set_text(ui_La_dis_bpre, "Pressure Error");
            } else {
                lv_label_set_text_fmt(ui_La_dis_bpre, "%d hPa", *_data.pressure);  
            }

            if (_data.temperature == nullptr) {
                lv_label_set_text(ui_La_dis_temp, "Temp Error");
            } else {
                lv_label_set_text_fmt(ui_La_dis_temp, "%d °C", *_data.temperature);
            }

            if (_data.eCO2 == nullptr) {
                lv_label_set_text(ui_La_dis_co2, "CO2 Error");
            } else {
                lv_label_set_text_fmt(ui_La_dis_co2, "%d ppm", *_data.eCO2);
            }

            if (_data.tVoC == nullptr) {
                lv_label_set_text(ui_La_dis_jiaquan, "tVoC Error");
            } else {
                lv_label_set_text_fmt(ui_La_dis_jiaquan, "%d ppm", *_data.tVoC);
            }

        }
    }
}