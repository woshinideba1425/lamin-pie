#ifndef SYSTEM_DATA_DEF_H
#define SYSTEM_DATA_DEF_H

#include <cstdint>

/* Time */
#if 1
#define LA_TIME                 "_TIME"         // MOONCAKE::DataTime_t
namespace LAMINATEPIE {
    struct DataTime_t {
        uint8_t sec = 0;		/* Seconds.	[0-60] (1 leap second) */
        uint8_t min = 32;		/* Minutes.	[0-59] */
        uint8_t hour = 23;		/* Hours.	[0-23] */
        uint8_t mday = 4;		/* Day.		[1-31] */
        uint8_t mon = 6;		/* Month.	[0-11] */
        uint8_t wday = 0;		/* Day of week.	[0-6] */
        int year = 99;			/* Year	- 1900.  */
        long int gmtoff = 0;	/* Seconds east of UTC */
        bool wasSet = false;    /* Flag to set time*/

        // Helper function to calculate total seconds since a reference point
        long long TotalSeconds() const {
            const int daysInYear = 365;

            // 每个月的实际天数（闰年非闰年均适配）
            const int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

            // 计算闰年
            int fullYear = year + 1900;
            bool isLeapYear = ((fullYear % 4 == 0 && fullYear % 100 != 0) || (fullYear % 400 == 0));

            // 处理从 1970 年开始的闰年数
            int leapYears = 0;
            for (int y = 1970; y < fullYear; ++y) {
                if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) {
                    leapYears++;
                }
            }

            // 计算到年份开始的天数
            int totalDays = (fullYear - 1970) * daysInYear + leapYears;

            // 累加本年的天数
            for (int i = 0; i < mon; ++i) {
                totalDays += daysInMonth[i];
            }
            // 如果是闰年并且已经过了 2 月，则加一天
            if (isLeapYear && mon > 1) {
                totalDays++;
            }

            // 加上当前月的天数
            totalDays += (mday - 1);

            // 转换为秒数
            long long totalSeconds = totalDays * 24 * 60 * 60;  // Days to seconds
            totalSeconds += hour * 3600;                        // Hours to seconds
            totalSeconds += min * 60;                           // Minutes to seconds
            totalSeconds += sec;                                // Add seconds

            // 应用时区偏移
            totalSeconds += gmtoff;

            return totalSeconds;
        }

        // Overload the less-than operator
        bool operator < (const DataTime_t& right) const {
            return this->TotalSeconds() < right.TotalSeconds();
        }

        bool operator > (const DataTime_t& right) const {
            return this->TotalSeconds() > right.TotalSeconds();
        }
    };
}
#define LA_TIME_JSUT_SET        "_TIME_SET"     // bool
#endif

/* Hardware */
/* Hardware */
#if 1
/* Display */
#define LA_DISP_HOR             "_DIS_HOR"      // int16_t
#define LA_DISP_VER             "_DIS_VER"      // int16_t
#define LA_DISP_BRIGHTNESS      "_DIS_BRI"      // uint8_t

/* System tick */
#define LA_SYSTEM_TICKS         "_SYS_TICK"     // uint32_t

/* Boot mode */
#define LA_SYSTEM_DEVELOPMENT   "_SYS_DEV"     // bool

/* Power */
#define LA_BATTERY_LEVEL        "_BAT_LV"       // uint8_t
#define LA_BATTERY_IS_CHARGING  "_BAT_CHG"      // bool


/* Wireless */
#define LA_WIFI_IS_CONNECTED    "_WIFI_CN"      // bool
#define LA_BLE_IS_CONNECTED     "_BLE_CN"       // bool


/* Notification */
#define LA_NOTIFICATION_IS_ON   "_NOTE_ON"      // bool

/* Setps */
#define LA_STEPS                "_STEPS"        // uint32_t

/* Pressure */
#define LA_PRESSURE             "_PRE"         // uint32_t

/* Temperature */
#define LA_TEMPERATURE          "_TEM"         // int32_t

/* Magnetic */
#define LA_MAGNETIC             "_MAG"         // float

/* Humidity */
#define LA_HUMIDITY             "_HUM"         // uint32_t

/*CO2*/
#define LA_CO2                  "_CO2"         //int16_t

/*tVoC*/
#define LA_TVOC                 "_TVOC"         //int16_t

/* Lumines */
#define LA_LUMINES              "_LUM"         // float

/* Flag of just wake up from sleep */
#define LA_JUST_WAKEUP          "_WAKE_UP"      // bool


#endif


/* Userdata */
/* Userdata */
#if 1
/* Wireless */
#define LA_WIFI_LAST_SSID       "_WIFI_LA_ID"   // string
#define LA_WIFI_LAST_PASSWORD   "_WIFI_LA_PW"   // string

/* HeartRate */
#define LA_HEARTRATE            "_H_RATE"        // uin8_t

/* Blood OXYZEN */
#define LA_BLOOD_OXYZEN         "_BLD_OXY"       // uin8_t

/* Blood Pressure */
#define LA_BLOOD_PRESSURE       "_BLD_PRU"       // uint16_t

/* Figure */
#define LA_USER_WEIGHT          "_USR_WEI"       // uint16_t
#define LA_USER_HIGHT           "_USR_HIG"       // uint16_t

#endif

#endif //SYSTEM_DATA_DEF_H