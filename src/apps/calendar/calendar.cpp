#include "calendar.h"

namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {
        void CalendarApp::onSetup()
        {
            setTaskName("CalendarApp");
            setScreen(ui_calendar);
            setAllowBgRunning(false);
            setAppIcon((void *)&ICON_CALENDAR);
        }

        void CalendarApp::onCreate()
        {
            printf("[%s] onCreate\n", getAppName().c_str());
            
            _framework->enableLowPowerMode();
            SIMPLEKV::SimpleKV_ESP* db = _framework->getDatabase();
            /* Update date */
            if (db->Get(LA_TIME)->addr != nullptr) {
                int year = db->Get(LA_TIME)->value<DataTime_t>().year;
                uint8_t mon = db->Get(LA_TIME)->value<DataTime_t>().mon;
                uint8_t mday = db->Get(LA_TIME)->value<DataTime_t>().mday;;
                lv_calendar_set_today_date(ui_Calendar1, static_cast<uint32_t>(year), 
                                            static_cast<uint32_t>(mon),static_cast<uint32_t>(mday));
                lv_calendar_set_showed_date(ui_Calendar1, static_cast<uint32_t>(year), 
                                            static_cast<uint32_t>(mon));
            }
        }

        void CalendarApp::onResume()
        {
            _framework->enableLowPowerMode();
            printf("[%s] onResume\n", getAppName().c_str());
        }

        void CalendarApp::taskLoop()
        {
            lv_disp_trig_activity(NULL); // keep screen activity
        }

        void CalendarApp::onRunningBG()
        {


        }
        void CalendarApp::onPause()
        {
            printf("[%s] onPause\n", getAppName().c_str());
        }

        void CalendarApp::onDestroy()
        {
            printf("[%s] onDestroy\n", getAppName().c_str());
        }
    }
}